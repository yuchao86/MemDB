#ifndef __UNISTOR_SUBSCRIBE_H__
#define __UNISTOR_SUBSCRIBE_H__

#include "UnistorMacro.h"
#include "CwxStl.h"
#include "CwxCommon.h"

///����modģʽ�Ķ��Ĺ���Ĺ�����Ϣ����
class UnistorSubscribeMod{
public:
    UnistorSubscribeMod(){
        m_uiMod = 0;
    }
    UnistorSubscribeMod(UnistorSubscribeMod const& item){
        m_uiMod = item.m_uiMod;
        m_indexs = item.m_indexs;
    }
    UnistorSubscribeMod& operator=(UnistorSubscribeMod const& item){
        if (this != &item){
            m_uiMod = item.m_uiMod;
            m_indexs = item.m_indexs;
        }
        return *this;
    }
public:
    ///�Ƿ���ָ����id
    inline bool isSubscribe(CWX_UINT32 uiGroup) const{
        if (m_uiMod) uiGroup %= m_uiMod;
        return m_indexs.find(uiGroup) != m_indexs.end();
    }
public:
    CWX_UINT32   m_uiMod;
    set<CWX_UINT32> m_indexs;
};

///����mod��rangeģʽ�Ķ��Ĺ���Ĺ�����Ϣ����
class UnistorSubscribeRange{
public:
    UnistorSubscribeRange(){
        m_uiMod = 0;
    }
    UnistorSubscribeRange(UnistorSubscribeRange const& item){
        m_uiMod = item.m_uiMod;
        m_range = item.m_range;
    }
    UnistorSubscribeRange& operator=(UnistorSubscribeRange const& item){
        if (this != &item){
            m_uiMod = item.m_uiMod;
            m_range = item.m_range;
        }
        return *this;
    }
public:
    inline bool isSubscribe(CWX_UINT32 uiGroup) const{
        map<CWX_UINT32, CWX_UINT32>::const_iterator iter = m_range.begin();
        while(iter != m_range.end()){
            if ((uiGroup>=iter->first) && (uiGroup<=iter->second)) return true;
            iter++;
        }
        return false;
    }
public:
    CWX_UINT32   m_uiMod;
    map<CWX_UINT32, CWX_UINT32> m_range; ///<���ĵ�group��Χ�б�
};

///����key��Χģʽ�Ķ��Ĺ���Ĺ�����Ϣ����
class UnistorSubscribeKey{
public:
    UnistorSubscribeKey(){
    }
    UnistorSubscribeKey(UnistorSubscribeKey const& item){
        m_keys = item.m_keys;
    }
    UnistorSubscribeKey& operator=(UnistorSubscribeKey const& item){
        if (this != &item){
            m_keys = item.m_keys;
        }
        return *this;
    }
public:
    bool isSubscribe(char const* key) const;
public:
    map<string,string>           m_keys;
};

///���Ĺ�����ʽ����
class UnistorSubscribe{
public:
    enum{
        SUBSCRIBE_MODE_MOD = 1,
        SUBSCRIBE_MODE_RANGE=2,
        SUBSCRIBE_MODE_KEY=3
    };
public:
    
    UnistorSubscribe(){
        m_uiMode = SUBSCRIBE_MODE_MOD;
        m_bAll = true;
    }

    UnistorSubscribe(UnistorSubscribe const& item){
        m_bAll = item.m_bAll;
        m_mod = item.m_mod;
        m_range = item.m_range;
        m_key = item.m_key;
        m_uiMode = item.m_uiMode;
    }

    UnistorSubscribe& operator=(UnistorSubscribe const& item){
        if (this != &item){
            m_bAll = item.m_bAll;
            m_mod = item.m_mod;
            m_range = item.m_range;
            m_key = item.m_key;
            m_uiMode = item.m_uiMode;
        }
        return *this;
    }

public:
    ///�Ƿ���ָ����group��type��
    inline bool isSubscribe(CWX_UINT32 uiGroup, char const* szKey) const{
        if (!m_bAll){
            if (SUBSCRIBE_MODE_MOD == m_uiMode){
                return m_mod.isSubscribe(uiGroup);
            }else if (SUBSCRIBE_MODE_RANGE == m_uiMode){
                return m_range.isSubscribe(uiGroup);
            }else if (SUBSCRIBE_MODE_KEY == m_uiMode){
                return m_key.isSubscribe(szKey);
            }
            return false;
        }
        return true;
    }
    ///�������ĵ��﷨
    /*
    ���ʽΪ
    type:group_express����Ϊ*����Ϊall
    ���У�
    type:mod��range��key�������͡�
    all������ȫ����Ϣ
    mod������group���࣬group_express����Ϊ[mod��index1,index2....]����ʾ��group��mod���࣬����Ϊindex1��index2�ȡ�modΪ0��ʾ������
    range������group�ķ�Χ��group_express����Ϊ[mod:begin-end,begin-end,...]��ʾ��group%mod��ֵ���group��Χ�������Χ�����ԡ�,���ָ��begin==end����ֻдbegin�Ϳ����ˡ�modΪ0��ʾ�����ࡣ
    key������key��Χ�Ļ�ȡ������Ϊ[key1-key2,key3-key4,...]����ʾkey[key1,key2)��[key3,key4)�뿪������䡣��key�г��֡�,����-��������,,��--��ʾ����ǰһ��Ϊ�գ����ʾ��С������һ��Ϊ�գ����ʾ���
    ��Ϊ�գ���ʾall
    */
    bool parseSubsribe(string const& strSubscribe);
private:
    ///����һ�����ı��ʽ
    int parseSubscribeKey(char const* szKey, list<string>& keys, char split);

public:
    CWX_UINT32      m_uiMode;  ///����ģʽ
    UnistorSubscribeMod     m_mod; ///<���ඩ��ģʽ
    UnistorSubscribeRange   m_range; ///<range�Ķ���ģʽ
    UnistorSubscribeKey     m_key; ///<key�Ķ���ģʽ
    bool    m_bAll;         ///<�Ƿ���ȫ����Ϣ
    string          m_strErrMsg; ///<�����Ĵ�����Ϣ
};




#endif
