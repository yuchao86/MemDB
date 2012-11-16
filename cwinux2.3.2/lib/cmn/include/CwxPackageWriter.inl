/**
*@file CwxPackageWriter.inl
*@brief CwxPackageWriter���Inlineʵ��
*@author cwinux@gmail.com
*@version 1.0
*@date  2009-06-05
*@warning  nothing
*@bug    
*/
CWINUX_BEGIN_NAMESPACE

///����һ����package
inline void CwxPackageWriter::beginPack()
{
    reset();
}
inline bool CwxPackageWriter::addKeyValue(char const* szKey, char const* szData, CWX_UINT32 uiDataLen, bool bKeyValue){
    return addKeyValue(szKey, strlen(szKey), szData, uiDataLen, bKeyValue);
}

inline bool CwxPackageWriter::addKeyValue(char const* szKey, string const& strData){
    return addKeyValue(szKey,strlen(szKey), strData);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT8 ucData){
    return addKeyValue(szKey, strlen(szKey), ucData);
}
///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_INT8 cData){
    return addKeyValue(szKey, strlen(szKey), cData);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unData){
    return addKeyValue(szKey, strlen(szKey), unData);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_INT16 nData){
    return addKeyValue(szKey, strlen(szKey), nData);
}


///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT32 uiData){
    return addKeyValue(szKey, strlen(szKey), uiData);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_INT32 iData){
    return addKeyValue(szKey, strlen(szKey), iData);
}


///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT64 ullData){
    return addKeyValue(szKey, strlen(szKey), ullData);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_INT64 llData){
    return addKeyValue(szKey, strlen(szKey), llData);
}


inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, string const& strData){
    return addKeyValue(szKey, unKeyLen, strData.c_str(), strData.length(), false);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT8 ucData){
    char buf[32];
    sprintf(buf, "%u", ucData);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}
///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT8 cData){
    char buf[32];
    sprintf(buf, "%d", cData);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT16 unData){
    char buf[32];
    sprintf(buf, "%u", unData);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT16 nData){
    char buf[32];
    sprintf(buf, "%d", nData);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}


///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT32 uiData){
    char buf[32];
    sprintf(buf, "%u", uiData);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT32 iData){
    char buf[32];
    sprintf(buf, "%d", iData);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}


///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT64 ullData){
    char buf[32];
    CwxCommon::toString(ullData, buf);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}

///Add a new key.true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT64 llData){
    char buf[32];
    CwxCommon::toString(llData, buf);
    return addKeyValue(szKey, unKeyLen, buf, strlen(buf), false);
}


///Package msg . true: success; false:failure, get err-msg by GetError().
inline bool CwxPackageWriter::pack(){
    return true;
}
///��ȡ��ǰpackage��Key��������
inline CWX_UINT32 CwxPackageWriter::getKeyNum() const{
    return m_uiKeyNum;
}
///
inline CWX_UINT32 CwxPackageWriter::getBufSize() const{
    return m_uiPackBufLen;
}
///return msg's size
inline CWX_UINT32 CwxPackageWriter::getMsgSize() const {
    return m_uiCurPackPos;
}
///return msg
inline char const* CwxPackageWriter::getMsg() const {
    return m_szPackMsg;
}
///return the errmsg
inline char const* CwxPackageWriter::getErrMsg() const{
    return m_szErr;
}


CWINUX_END_NAMESPACE
