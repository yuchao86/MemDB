#include "UnistorSubscribe.h"
#include "UnistorStoreBase.h"


///�Ƿ���
bool UnistorSubscribeKey::isSubscribe(char const* key) const{
    map<string,string>::const_iterator iter = m_keys.begin();
    while(iter != m_keys.end()){
        if ((UnistorStoreBase::getKeyAsciiLess()(key, strlen(key), iter->first.c_str(), iter->first.length())>=0) &&
            ((UnistorStoreBase::getKeyAsciiLess()(key, strlen(key), iter->second.c_str(), iter->second.length())<0)|| !iter->second.length()))
        {
            return true;
        }
        iter++;
    }
    return false;
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
bool UnistorSubscribe::parseSubsribe(string const& strSubscribe){
    char const* pSub=NULL;
    list<string> items;
    list<string>::iterator iter;
    string strSub(strSubscribe);
    string strValue;
    string strKeyBegin;
    string strKeyEnd;
    CWX_UINT32 uiValue;
    CWX_UINT32 uiValue1;
    CwxCommon::ltrim(strSub);
    m_bAll = false;
    if (strncmp("mod:", strSub.c_str(), 4)==0){
        m_uiMode = UnistorSubscribe::SUBSCRIBE_MODE_MOD;
        ///��ȡmod
        pSub = strchr(strSub.c_str() + strlen("mod:"), ':');
        if (!pSub){
            m_strErrMsg = "Subscribe rule is error: less [mod:] for mod-subscribe";
            return false;
        }
        m_mod.m_uiMod = strtoul(strSub.c_str() + strlen("mod:"), NULL, 10);
        pSub++;
        CwxCommon::split(string(pSub), items, ',');
        iter = items.begin();
        while(iter != items.end()){
            strValue = *iter;
            CwxCommon::trim(strValue);
            if (strValue.length()){
                uiValue = strtoul(strValue.c_str(), NULL, 10);
                if (m_mod.m_uiMod) uiValue %=  m_mod.m_uiMod;
                m_mod.m_indexs.insert(uiValue);
            }
            iter++;
        }
        if (!m_mod.m_indexs.size()){
            m_strErrMsg = "No subscribed index for mod mode, subscribe:";
            m_strErrMsg += strSubscribe;
            return false;
        }
        return true;

    }else if (strncmp("range:", strSub.c_str(), 6)==0){
        m_uiMode = UnistorSubscribe::SUBSCRIBE_MODE_RANGE;
        ///��ȡrange
        pSub = strchr(strSub.c_str() + strlen("range:"), ':');
        if (!pSub){
            m_strErrMsg = "Subscribe rule is error: less [range:] for range-subscribe";
            return false;
        }
        m_range.m_uiMod = strtoul(strSub.c_str() + strlen("range:"), NULL, 10);
        pSub++;
        CwxCommon::split(string(pSub), items, ',');
        iter = items.begin();
        while(iter != items.end()){
            strValue = *iter;
            CwxCommon::trim(strValue);
            if (strValue.length()){
                pSub = strchr(strValue.c_str(), '-');
                if (!pSub){
                    uiValue1 = uiValue = strtoul(strValue.c_str(), NULL, 10);
                }else{
                    uiValue = strtoul(strValue.c_str(), NULL, 10);
                    uiValue1 = strtoul(pSub + 1, NULL, 10);
                }
                if (m_range.m_uiMod){
                    uiValue %= m_range.m_uiMod;
                    uiValue1 %= m_range.m_uiMod;
                }
                m_range.m_range[uiValue] = uiValue1;
            }
            iter++;
        }
        if (!m_range.m_range.size()){
            m_strErrMsg = "No subscribed index for range mode, subscribe:";
            m_strErrMsg += strSubscribe;
            return false;
        }
        return true;
    }else if (strncmp("key:", strSub.c_str(), 4) == 0){
        list<string> keys;
        m_uiMode = UnistorSubscribe::SUBSCRIBE_MODE_KEY;
        ///��ȡrange
        pSub =strSub.c_str() + strlen("range:");
        int num=parseSubscribeKey(pSub, keys, ',');
        if (0 == num){
            m_strErrMsg = "No subscribe key, subscribe:";
            m_strErrMsg += strSubscribe;
            return false;
        }
        iter = keys.begin();
        while(iter != keys.end()){
            num = parseSubscribeKey(iter->c_str(), items, '-');
            if (2 != num){
                m_strErrMsg = "Key's rule err, must be [begin-end]:";
                m_strErrMsg += iter->c_str();
                return false;
            }
            m_key.m_keys[*(items.begin())] = *(items.begin()++);
            iter++;
        }
        return true;
    }
    CwxCommon::trim(strSub);
    if (!strSub.length() || (strSub=="*") || (strSub=="all")){
        m_bAll = true;
        return true;
    }
    m_strErrMsg = "Invalid subscribe rule:";
    m_strErrMsg += strSubscribe;
    return false;
}

///����key������key�Ľڵ�������Ҳ����key������ڵ��������ؽڵ������
int UnistorSubscribe::parseSubscribeKey(char const* szKey, list<string>& keys, char split)
{
    string strKey="";
    CWX_UINT32 uiDotNum = 0;
    CWX_UINT32 i=0;
    keys.clear();
    if (!szKey || !strlen(szKey)){
        return 0;
    }
    while(szKey[i]){
        if (split == szKey[i]){
            uiDotNum++;
            if (2 == uiDotNum){
                uiDotNum = 0;
                strKey += split;
            }
            i++;
            continue;
        }
        if(!uiDotNum){
            strKey += szKey[i];
            i++;
            continue;
        }
        //��һ��dot���µ�key
        keys.push_back(strKey);
        strKey="";
        uiDotNum = 0;
        i++;
    }
    ///������һ��
    keys.push_back(strKey);
    return keys.size();
}
