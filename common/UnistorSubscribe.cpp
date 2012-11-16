#include "UnistorSubscribe.h"
#include "UnistorStoreBase.h"


///是否订阅
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
        ///获取mod
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
        ///获取range
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
        ///获取range
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

///解析key。返回key的节点数量，也就是key的链表节点数，返回节点的数量
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
        //有一个dot，新的key
        keys.push_back(strKey);
        strKey="";
        uiDotNum = 0;
        i++;
    }
    ///添加最后一个
    keys.push_back(strKey);
    return keys.size();
}
