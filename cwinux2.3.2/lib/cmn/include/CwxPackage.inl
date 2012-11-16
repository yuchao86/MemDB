/**
*@file CwxPackage.inl
*@brief CwxPackage���Inlineʵ��
*@author cwinux@gmail.com
*@version 1.0
*@date  2009-06-05
*@warning  nothing
*@bug    
*/
CWINUX_BEGIN_NAMESPACE

//-1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
inline int CwxPackage::getKeyByIndex(char const *szMsg, CWX_UINT32 uiMsgLen, CWX_UINT32 uiIndex, CwxKeyValueItem& item)
{
    CWX_UINT32 uiPos = 0;
    int len=0;
    if (!uiMsgLen) return 0;
    for (CWX_UINT32 i=0; i<=uiIndex; i++)
    {
        len = getNextKey(szMsg + uiPos, uiMsgLen - uiPos, item);
        if (-1 == len) return -1;
        if (0 == len) return 0;
        uiPos += len;
    }
    return len;
}

//-1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
inline int CwxPackage::getKeyByName(char const *szMsg, CWX_UINT32 uiMsgLen, char const* szKey, CwxKeyValueItem& item, bool bCaseSensive)
{
    CWX_UINT32 uiPos = 0;
    int len=0;
    for ( ; uiPos<uiMsgLen; )
    {
        len = getNextKey(szMsg + uiPos, uiMsgLen - uiPos, item);
        if (-1 == len) return -1;
        if (0 == len) return 0;
        if (bCaseSensive)
        {
            if (strcmp(item.m_szKey, szKey)==0) return len;
        }
        else
        {
            if (strcasecmp(item.m_szKey, szKey)==0) return len;
        }
        uiPos += len;
    }
    return 0;
}

//-1����Ч��package��0��û�з��֣�1��ɾ����һ��KEY
inline int CwxPackage::removeKey(char *szMsg, CWX_UINT32& uiMsgLen, CWX_UINT16 unIndex)
{
    CWX_UINT32 i;
    CWX_UINT32 uiPos = 0;
    int len=0;
    CwxKeyValueItem item;
    for ( i=0; i<=unIndex; i++)
    {
        len = getNextKey(szMsg + uiPos, uiMsgLen - uiPos, item);
        if (-1 == len) return -1;
        if (0 == len) return 0;
        uiPos += len;
    }
    CwxCommon::memMove(szMsg + uiPos - len, szMsg+uiPos, uiMsgLen-uiPos);
    uiMsgLen -= len;
    return 1;
}

inline int CwxPackage::dump(char const* szMsg, CWX_UINT32 uiMsgLen, char* szOutBuf, CWX_UINT32& uiOutBufLen, char const* szTab, char const* szKeyBegin, char const* szKeyEnd, CwxEscape const* pEscape)
{
    int len=dumpEx(szMsg, uiMsgLen, szOutBuf, uiOutBufLen, szTab, 1, szKeyBegin, szKeyEnd, pEscape);
    if (0>len) return len;
    szOutBuf[len] = 0x00;
    return len;
}

//true:��Ч�İ���false����Ч�İ�.
inline bool CwxPackage::isValidPackage(char const *szMsg, CWX_UINT32 uiMsgLen)
{
    CWX_UINT32 uiPos = 0;
    int len=0;
    CwxKeyValueItem item;
    for ( ; uiPos<uiMsgLen ; )
    {
        len = getNextKey(szMsg + uiPos, uiMsgLen - uiPos, item);
        if (-1 == len) return false;
        if (0 == len) return true;
        uiPos += len;
    }
    return true;
}

///��ȡpackage��key������
inline int CwxPackage::getKeyValueNum(char const* szMsg, CWX_UINT32 uiMsgLen)
{
    int iKeyNum = 0;
    CWX_UINT32 uiPos = 0;
    int len=0;
    CwxKeyValueItem item;
    for ( ; uiPos<uiMsgLen ; )
    {
        len = getNextKey(szMsg + uiPos, uiMsgLen - uiPos, item);
        if (-1 == len) return -1;
        if (0 == len) return iKeyNum;
        uiPos += len;
        iKeyNum++;
    }
    return iKeyNum;
}



///ͨ��Key�ĳ��ȼ�data�ĳ��ȣ���ȡ������Key/value���ȡ�
inline CWX_UINT32 CwxPackage::getKvLen(CWX_UINT16 unKeyLen, CWX_UINT32 uiDataLen)
{
    return 8 + unKeyLen + uiDataLen ;
}

///ͨ��key/value�ĳ��ȼ�key�ĳ��ȣ���ȡdata�ĳ���
inline CWX_UINT32 CwxPackage::getDataLen(CWX_UINT32 uiKeyValueLen, CWX_UINT16 unKeyLen)
{
    return uiKeyValueLen - unKeyLen - 8;
}

///����key��key/value�е�ƫ��
inline CWX_UINT16 CwxPackage::getKeyOffset()
{
    return 6;
}


CWINUX_END_NAMESPACE
