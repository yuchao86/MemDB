#ifndef __UNISTOR_SUBSCRIBE_H__
#define __UNISTOR_SUBSCRIBE_H__

#include "UnistorMacro.h"
#include "CwxStl.h"
#include "CwxCommon.h"

///基于mod模式的订阅规则的规则信息对象
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
    ///是否订阅指定的id
    inline bool isSubscribe(CWX_UINT32 uiGroup) const{
        if (m_uiMod) uiGroup %= m_uiMod;
        return m_indexs.find(uiGroup) != m_indexs.end();
    }
public:
    CWX_UINT32   m_uiMod;
    set<CWX_UINT32> m_indexs;
};

///基于mod的range模式的订阅规则的规则信息对象
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
    map<CWX_UINT32, CWX_UINT32> m_range; ///<订阅的group范围列表
};

///基于key范围模式的订阅规则的规则信息对象
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

///订阅规则表达式对象
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
    ///是否订阅指定的group、type对
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
    ///解析订阅的语法
    /*
    表达式为
    type:group_express或者为*或者为all
    其中：
    type:mod、range、key四种类型。
    all：订阅全部消息
    mod：基于group求余，group_express内容为[mod：index1,index2....]，表示对group以mod求余，余数为index1、index2等。mod为0表示不求余
    range：基于group的范围，group_express内容为[mod:begin-end,begin-end,...]表示对group%mod的值后的group范围，多个范围可以以【,】分割，若begin==end，则只写begin就可以了。mod为0表示不求余。
    key：基于key范围的获取，内容为[key1-key2,key3-key4,...]，表示key[key1,key2)、[key3,key4)半开半闭区间。若key中出现【,】或【-】，则用,,或--表示。若前一个为空，则表示最小，若后一个为空，则表示最大
    若为空，表示all
    */
    bool parseSubsribe(string const& strSubscribe);
private:
    ///解析一个订阅表达式
    int parseSubscribeKey(char const* szKey, list<string>& keys, char split);

public:
    CWX_UINT32      m_uiMode;  ///定于模式
    UnistorSubscribeMod     m_mod; ///<求余订阅模式
    UnistorSubscribeRange   m_range; ///<range的订阅模式
    UnistorSubscribeKey     m_key; ///<key的订阅模式
    bool    m_bAll;         ///<是否订阅全部消息
    string          m_strErrMsg; ///<解析的错误消息
};




#endif
