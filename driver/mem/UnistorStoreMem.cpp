#include "UnistorStoreMem.h"

#define  MEM_MIN_READ_CACHE_MSIZE   128
#define  MEM_MAX_READ_CACHE_MSIZE   (64 * 1024)

#define  MEM_MIN_READ_CACHE_KEY_NUM   100000
#define  MEM_MAX_READ_CACHE_KEY_NUM   800000000

extern "C"{
    UnistorStoreBase* unistor_create_engine()
    {
        return new UnistorStoreMem();
    }
}

//0:�ɹ���-1��ʧ��
int UnistorStoreMem::parseConf(){
    string value;
    //get mem:expire_check_step
    m_memConf.m_uiExpireCheckNum = UnistorConfigMem::UNISTOR_CONFIG_DEF_CHECK_NUM;
    if (m_config->getConfFileCnf().getAttr("mem", "expire_check_step", value)){
        m_memConf.m_uiExpireCheckNum = strtoul(value.c_str(), NULL, 10);
    }
    if (m_memConf.m_uiExpireCheckNum < UnistorConfigMem::UNISTOR_CONFIG_MIN_CHECK_NUM){
        m_memConf.m_uiExpireCheckNum = UnistorConfigMem::UNISTOR_CONFIG_MIN_CHECK_NUM;
    }else if (m_memConf.m_uiExpireCheckNum > UnistorConfigMem::UNISTOR_CONFIG_MAX_CHECK_NUM){
        m_memConf.m_uiExpireCheckNum = UnistorConfigMem::UNISTOR_CONFIG_MAX_CHECK_NUM;
    }
    CWX_INFO(("*****************begin bdb conf****************"));
    CWX_INFO(("expire_check_step=%u", m_memConf.m_uiExpireCheckNum));
    CWX_INFO(("memcache_read_cache_msize=%u", m_uiReadCacheMSize));
    CWX_INFO(("memcache_read_cache_item=%u", m_uiReadCacheItemNum));
    CWX_INFO(("*****************end bdb conf****************"));
    return 0;
}


//���������ļ�.-1:failure, 0:success
int UnistorStoreMem::init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc,
                          UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<��ȡϵͳ��Ϣ��function
                          void* pApp, ///<UnistorApp����
                          UnistorConfig const* config)
{
    m_bValid = false;
    strcpy(m_szErrMsg, "Not init");
    if (0 != UnistorStoreBase::init(msgPipeFunc, getSysInfoFunc, pApp, config)) return -1;
    m_uiWriteCacheMSize = m_config->getCommon().m_uiWriteCacheMByte;

    m_uiReadCacheMSize = m_config->getCommon().m_uiReadCacheMByte;
    if (m_uiReadCacheMSize < MEM_MIN_READ_CACHE_MSIZE) m_uiReadCacheMSize = MEM_MIN_READ_CACHE_MSIZE;
    if (m_uiReadCacheMSize > MEM_MAX_READ_CACHE_MSIZE) m_uiReadCacheMSize = MEM_MAX_READ_CACHE_MSIZE;

    m_uiReadCacheItemNum = m_config->getCommon().m_uiReadCacheMaxKeyNum;
    if (m_uiReadCacheItemNum < MEM_MIN_READ_CACHE_KEY_NUM) m_uiReadCacheItemNum = MEM_MIN_READ_CACHE_KEY_NUM;
    if (m_uiReadCacheItemNum > MEM_MAX_READ_CACHE_KEY_NUM) m_uiReadCacheItemNum = MEM_MAX_READ_CACHE_KEY_NUM;
    //parse conf
    if (0 != parseConf()) return -1;
    //����cache
    unsigned long int size = m_uiReadCacheMSize;
    size *= 1024 * 1024;
    m_memCache = new UnistorStoreMemCache(size,
        m_uiReadCacheItemNum,
        m_memConf.m_uiExpireCheckNum,
        UnistorStoreMem::keyStoreCmpEqual,
        UnistorStoreMem::keyStoreCmpLess,
        UnistorStoreMem::keyStoreHash,
        1.2);
    if (0 != m_memCache->init(m_szErrMsg)){
        CWX_ERROR(("Failure to init mem cache, err=%s", m_szErrMsg));
        return -1;
    }

    m_strEngine = m_config->getCommon().m_strStoreType;
    m_uiUncommitBinlogNum = 0;
    m_uiLastCommitSecond = 0;
    m_ullStoreSid = 0;

    m_bValid = true;
    //�ָ�binlog
    if (0 != restore(m_ullStoreSid)){
        m_bValid = false;
        return -1;
    }
    m_szErrMsg[0] = 0x00;
    return 0;
}


///����Ƿ����key��1�����ڣ�0�������ڣ�-1��ʧ��
int UnistorStoreMem::isExist(UnistorTss* tss,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* ,
                             CWX_UINT32& uiVersion,
                             CWX_UINT32& uiFieldNum,
                             bool& bReadCached)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    CWX_UINT32 uiExpire = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = true;
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, uiExpire, tss->m_szBuf2K);
    uiFieldNum = 0;
    if (1 == ret){
        ret = unpackFields(*tss->m_pEngineReader, szBuf, uiBufLen, uiVersion);
        if (-1 == ret){
            strcpy(tss->m_szBuf2K, tss->m_pEngineReader->getErrMsg());
            return -1;
        }
        ///����Key/Value�ṹ
        if (0 == ret){
            uiFieldNum = 0;
            if (field) return 0;
            return 1;
        }
        uiFieldNum = tss->m_pEngineReader->getKeyNum();
        if (field){
            return tss->m_pEngineReader->getKey(field->m_szData)?1:0;
        }
        return 1;
    }
    return !ret?0:-1;
}

///���key��1���ɹ���0�����ڣ�-1��ʧ�ܣ�
int UnistorStoreMem::addKey(UnistorTss* tss,
                            CwxKeyValueItemEx const& key,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* extra,
                            CwxKeyValueItemEx const& data,
                            CWX_UINT32 uiSign,
                            CWX_UINT32& uiVersion,
                            CWX_UINT32& uiFieldNum,
                            bool& bReadCached,
                            bool& bWriteCached,
                            bool bCache,
                            CWX_UINT32 uiExpire)
{
    int iDataFieldNum=0;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        if (-1 == (iDataFieldNum = CwxPackageEx::getKeyValueNum(data.m_szData, data.m_uiDataLen))){
            strcpy(tss->m_szBuf2K, "The data is not valid key/value structure.");
            return -1;
        }
    }
    bool bNewKeyValue = false;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiOldVersion = 0;
    uiFieldNum = 0;
    ///�޸�sign
    if (uiSign > 2) uiSign = 0;
    bReadCached = true;
    bWriteCached = true;
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttNewExpire, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (1 == ret){
        getKvVersion(szBuf, uiBufLen, uiOldVersion);
        if (uiExpire){
            ttNewExpire = getNewExpire(uiExpire);
        }
    }else{
        ttNewExpire = getNewExpire(uiExpire);
    }
    if (0 == ret){//not exist
        if (0 != uiSign){///���field������key�������
            strcpy(tss->m_szBuf2K, "Key doesn't exist, you can't add any field.");
            return -1; ///���ܵ������field
        }
        if (field){
            tss->m_pEngineWriter->beginPack();
            if (!tss->m_pEngineWriter->addKeyValue(field->m_szData, field->m_uiDataLen, data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
                strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
                return -1;
            }
            tss->m_pEngineWriter->pack();
            if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
                return -1;
            }
            uiBufLen = tss->m_pEngineWriter->getMsgSize();
            memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
            bNewKeyValue = true;
            uiFieldNum = 1;
        }else{
            if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
                return -1;
            }
            uiBufLen = data.m_uiDataLen;
            memcpy(szBuf, data.m_szData, uiBufLen);
            bNewKeyValue = data.m_bKeyValue;
            uiFieldNum = iDataFieldNum;
        }
        if (!uiVersion) uiVersion = UNISTOR_KEY_START_VERION; ///��ʼ�汾
    }else if (1 == ret){//key����
        if (!uiVersion){
            uiVersion = uiOldVersion + 1;
        }
        if (0 == uiSign){//���key
            strcpy(tss->m_szBuf2K, "Key exists.");
            return 0; //key���ڡ�
        }
        bool bOldKv = isKvData(szBuf, uiBufLen);
        if (!bOldKv){///ԭ�����ݲ���key/value�ṹ����������β���add key��add field
            strcpy(tss->m_szBuf2K, "Key is not key/value, can't add field.");
            return 0; //key���ڡ�
        }
        if (!field && !data.m_bKeyValue){
            strcpy(tss->m_szBuf2K, "The added content is not field.");
            return 0; //key���ڡ�
        }
        //��ʱ��key���¡���valueһ��Ϊkey/value�ṹ
        ret = mergeAddKeyField(tss->m_pEngineWriter,
            tss->m_pEngineReader,
            tss->m_pEngineItemReader,
            key.m_szData,
            field,
            szBuf,
            uiBufLen-getKvDataSignLen(),
            bOldKv,
            data.m_szData,
            data.m_uiDataLen,
            data.m_bKeyValue,
            uiFieldNum,
            tss->m_szBuf2K);
        if (1 != ret) return ret;
        if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
            return -1;
        }
        uiBufLen = tss->m_pEngineWriter->getMsgSize();
        memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
        bNewKeyValue = true;
    }
    CwxKeyValueItemEx localData;
    localData.m_szData = szBuf;
    localData.m_uiDataLen = uiBufLen;
    localData.m_bKeyValue = bNewKeyValue;
    if (0 != appendAddBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        localData,
        uiExpire,
        uiSign,
        uiVersion,
        bCache,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();
    setKvDataSign(szBuf, uiBufLen, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K)) return -1;
    return 1;
}

///���key��1���ɹ���0�����ڣ�-1��ʧ�ܣ�
int UnistorStoreMem::syncAddKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* ,
                                CwxKeyValueItemEx const* extra,
                                CwxKeyValueItemEx const& data,
                                CWX_UINT32 ,///<not use
                                CWX_UINT32 uiVersion,
                                bool bCache,
                                CWX_UINT32 uiExpire,
                                CWX_UINT64 ullSid,
                                bool& bReadCached, ///<�����Ƿ���read cache��
                                bool& bWriteCached, ///<�����Ƿ���write cache��
                                bool  bRestore)
{
    return syncImportKey(tss, key, extra, data, uiVersion, bCache, uiExpire, ullSid, bReadCached, bWriteCached, bRestore);
}

///set key��1���ɹ���-1������0�������ڣ���������һ��key��fieldʱ��
int UnistorStoreMem::setKey(UnistorTss* tss,
                            CwxKeyValueItemEx const& key,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* extra,
                            CwxKeyValueItemEx const& data,
                            CWX_UINT32 uiSign,
                            CWX_UINT32& uiVersion,
                            CWX_UINT32& uiFieldNum,
                            bool& bReadCached,
                            bool& bWriteCached,
                            bool bCache,
                            CWX_UINT32 uiExpire)
{
    int iDataFieldNum=0;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        if (-1 == (iDataFieldNum = CwxPackageEx::getKeyValueNum(data.m_szData, data.m_uiDataLen))){
            strcpy(tss->m_szBuf2K, "The data is not valid key/value structure.");
            return -1;
        }
    }
    bool bNewKeyValue = false;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiOldVersion = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = 0;
    bReadCached = true;
    bWriteCached = true;

    ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttNewExpire, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (1 == ret){
        getKvVersion(szBuf, uiBufLen, uiOldVersion);
        if (uiExpire){
            ttNewExpire = getNewExpire(uiExpire);
        }
    }else{
        ttNewExpire = getNewExpire(uiExpire);
    }
    uiFieldNum = 0;
    if (uiSign > 1) uiSign = 0;
    if ((0 == ret/*��ֵ������*/) ||
        (0 == uiSign)/*�滻����key*/)
    {
        if (!uiVersion){
            if (0 == ret){
                uiVersion = UNISTOR_KEY_START_VERION; ///��ʼ�汾
            }else{
                uiVersion = uiOldVersion + 1;
            }
        }
        if (field){
            tss->m_pEngineWriter->beginPack();
            if (!tss->m_pEngineWriter->addKeyValue(field->m_szData, field->m_uiDataLen, data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
                strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
                return -1;
            }
            tss->m_pEngineWriter->pack();
            if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
                return -1;
            }
            uiBufLen = tss->m_pEngineWriter->getMsgSize();
            memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
            bNewKeyValue = true;
            uiFieldNum = 1;
        }else{
            if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
                return -1;
            }
            uiBufLen = data.m_uiDataLen;
            memcpy(szBuf, data.m_szData, uiBufLen);
            bNewKeyValue = data.m_bKeyValue;
            uiFieldNum = iDataFieldNum;
        }
    }else{//key���ڶ��Ҳ���ȫ���滻
        if (!isKvData(szBuf, uiBufLen)){
            strcpy(tss->m_szBuf2K, "Key is key/value.");
            return -1;
        }
        if (!field && !data.m_bKeyValue){
            strcpy(tss->m_szBuf2K, "The set content is key/value.");
            return -1;
        }
        if (!uiVersion) uiVersion = uiOldVersion + 1;
        //��ʱ��key�ľɡ���valueһ��Ϊkey/value�ṹ
        ret = mergeSetKeyField(tss->m_pEngineWriter,
            tss->m_pEngineReader,
            tss->m_pEngineItemReader,
            key.m_szData,
            field,
            szBuf,
            uiBufLen-getKvDataSignLen(),
            true,
            data.m_szData,
            data.m_uiDataLen,
            data.m_bKeyValue,
            uiFieldNum,
            tss->m_szBuf2K);
        if (1 != ret) return ret;
        if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
            return -1;
        }
        uiBufLen = tss->m_pEngineWriter->getMsgSize();
        memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
        bNewKeyValue = true;
    }
    CwxKeyValueItemEx localData;
    localData.m_szData = szBuf;
    localData.m_uiDataLen = uiBufLen;
    localData.m_bKeyValue = bNewKeyValue;
    if (0 != appendSetBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        localData,
        uiExpire,
        uiSign,
        uiVersion,
        bCache,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();
    setKvDataSign(szBuf, uiBufLen, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K)) return -1;
    return 1;
}

///import key��1���ɹ���-1��ʧ�ܣ�
int UnistorStoreMem::importKey(UnistorTss* tss, ///<tss����
                               CwxKeyValueItemEx const& key, ///<��ӵ�key
                               CwxKeyValueItemEx const* extra, ///<�洢�����extra����
                               CwxKeyValueItemEx const& data, ///<���key��field������
                               CWX_UINT32& uiVersion, ///<������0���������޸ĺ��keyΪ�˰汾
                               bool& bReadCached, ///<�����Ƿ���read cache��
                               bool& bWriteCached, ///<�����Ƿ���write cache��
                               bool bCache, ///<�Ƿ�key�ŵ���cache
                               CWX_UINT32 uiExpire ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
                               )
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    CWX_UINT32 ttNewExpire = 0;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = true;
    bWriteCached = true;
    ttNewExpire = getNewExpire(uiExpire);
    if (!uiVersion) uiVersion = UNISTOR_KEY_START_VERION; ///��ʼ�汾
    if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
        return -1;
    }
    uiBufLen = data.m_uiDataLen;
    memcpy(szBuf, data.m_szData, uiBufLen);

    if (0 != appendImportBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        extra,
        data,
        uiExpire,
        uiVersion,
        bCache,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();
    setKvDataSign(szBuf, uiBufLen,uiVersion, data.m_bKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K)) return -1;
    return 1;

}

///sync import key��1���ɹ���-1������
int UnistorStoreMem::syncImportKey(UnistorTss* tss, ///<�̵߳�tss����
                                   CwxKeyValueItemEx const& key, ///<set��key
                                   CwxKeyValueItemEx const* , ///<�洢�����extra����
                                   CwxKeyValueItemEx const& data, ///<set������
                                   CWX_UINT32 uiVersion, ///<set��key �汾��
                                   bool ,    ///<�Ƿ�key�ŵ���cache
                                   CWX_UINT32 uiExpire, ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
                                   CWX_UINT64 ullSid, ///<������Ӧ��binlog��sid
                                   bool& bReadCached, ///<�����Ƿ���read cache��
                                   bool& bWriteCached, ///<�����Ƿ���write cache��
                                   bool   ///<�Ƿ��binlog�ָ�������
                                   )
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    CWX_UINT32 ttNewExpire = getNewExpire(uiExpire);
    bReadCached = true;
    bWriteCached = true;

    char* szBuf = tss->getBuf(uiBufLen);

    if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
        return -1;
    }
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;

    uiBufLen = data.m_uiDataLen;
    memcpy(szBuf, data.m_szData, uiBufLen);
    setKvDataSign(szBuf, uiBufLen, uiVersion, data.m_bKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K)) return -1;
    return 1;
}

///set key��1���ɹ���-1������0�������ڣ���������һ��key��fieldʱ��
int UnistorStoreMem::syncSetKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* ,
                                CwxKeyValueItemEx const* extra,
                                CwxKeyValueItemEx const& data,
                                CWX_UINT32 ,
                                CWX_UINT32 uiVersion,
                                bool bCache,
                                CWX_UINT32 uiExpire,
                                CWX_UINT64 ullSid,
                                bool& bReadCached,
                                bool& bWriteCached,
                                bool  bRestore)
{
    return syncImportKey(tss, key, extra, data, uiVersion, bCache, uiExpire, ullSid, bReadCached, bWriteCached, bRestore);
}

///update key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2���汾����
int UnistorStoreMem::updateKey(UnistorTss* tss,
                               CwxKeyValueItemEx const& key,
                               CwxKeyValueItemEx const* field,
                               CwxKeyValueItemEx const* extra,
                               CwxKeyValueItemEx const& data,
                               CWX_UINT32 uiSign,
                               CWX_UINT32& uiVersion,
                               CWX_UINT32& uiFieldNum,
                               bool& bReadCached,
                               bool& bWriteCached,
                               CWX_UINT32 uiExpire)
{
    int iDataFieldNum=0;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        if (-1 == (iDataFieldNum = CwxPackageEx::getKeyValueNum(data.m_szData, data.m_uiDataLen))){
            strcpy(tss->m_szBuf2K, "The data is not valid key/value structure.");
            return -1;
        }
    }
    bool bNewKeyValue=false;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion=0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = true;
    bWriteCached = true;

    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, uiExpire, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (0 == ret){//not exist
        strcpy(tss->m_szBuf2K,"Key doesn't exist.");
        return 0; ///���ܵ������field
    }

    bool bOldKv = false;
    getKvVersion(szBuf, uiBufLen, uiKeyVersion);
    if (uiExpire){
        ttNewExpire = getNewExpire(uiExpire);
    }

    //key����
    if (uiVersion){
        if (uiKeyVersion != uiVersion){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%u] is not same with input version[%u].",
                uiKeyVersion, uiVersion);
            return -2;
        }
    }
    uiVersion = uiKeyVersion + 1;
    if (uiSign>2) uiSign = 1;
    bOldKv = isKvData(szBuf, uiBufLen);
    if (0 == uiSign){///��������key
        if (field){
            tss->m_pEngineWriter->beginPack();
            if (!tss->m_pEngineWriter->addKeyValue(field->m_szData,  field->m_uiDataLen, data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
                strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
                return -1;
            }
            tss->m_pEngineWriter->pack();
            if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
                return -1;
            }
            uiBufLen = tss->m_pEngineWriter->getMsgSize();
            memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
            bNewKeyValue = true;
            uiFieldNum = 1;
        }else{
            if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
                return -1;
            }
            uiBufLen = data.m_uiDataLen;
            memcpy(szBuf, data.m_szData, data.m_uiDataLen);
            bNewKeyValue = data.m_bKeyValue;
            uiFieldNum = iDataFieldNum;
        }
    }else{//��ʱ���¡���ֵȫ��Ϊkv
        if (!bOldKv){
            strcpy(tss->m_szBuf2K, "The old data is not key/value");
            return -1;
        }
        if (!field && !data.m_bKeyValue){
            strcpy(tss->m_szBuf2K, "The new data is not key/value");
        }
        ret = mergeUpdateKeyField(tss->m_pEngineWriter,
            tss->m_pEngineReader,
            tss->m_pEngineItemReader,
            key.m_szData,
            field,
            szBuf,
            uiBufLen-getKvDataSignLen(),
            bOldKv,
            data.m_szData,
            data.m_uiDataLen,
            data.m_bKeyValue,
            uiFieldNum,
            (2==uiSign)?true:false,
            tss->m_szBuf2K);
        if (1 != ret) return ret;
        if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
            return -1;
        }
        uiBufLen = tss->m_pEngineWriter->getMsgSize();
        memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
        bNewKeyValue = true;
    }
    CwxKeyValueItemEx localData;
    localData.m_szData = szBuf;
    localData.m_uiDataLen = uiBufLen;
    localData.m_bKeyValue = bNewKeyValue;

    if (0 != appendUpdateBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        localData,
        uiExpire,
        uiSign,
        uiVersion,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();
    setKvDataSign(szBuf, uiBufLen, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K)) return -1;
    return 1;
}

///update key��1���ɹ���0�������ڣ�-1��ʧ��
int UnistorStoreMem::syncUpdateKey(UnistorTss* tss,
                                   CwxKeyValueItemEx const& key,
                                   CwxKeyValueItemEx const* ,
                                   CwxKeyValueItemEx const* extra,
                                   CwxKeyValueItemEx const& data,
                                   CWX_UINT32 ,
                                   CWX_UINT32 uiVersion,
                                   CWX_UINT32 uiExpire,
                                   CWX_UINT64 ullSid,
                                   bool& bReadCached, 
                                   bool& bWriteCached,
                                   bool  bRestore)
{
    return syncImportKey(tss, key, extra, data, uiVersion, true, uiExpire, ullSid, bReadCached, bWriteCached, bRestore);
}


///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2:�汾����-3�������߽�
int UnistorStoreMem::incKey(UnistorTss* tss,
                            CwxKeyValueItemEx const& key,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* extra,
                            CWX_INT64 num,
                            CWX_INT64  llMax,
                            CWX_INT64  llMin,
                            CWX_UINT32  uiSign,
                            CWX_INT64& llValue,
                            CWX_UINT32& uiVersion,
                            bool& bReadCached,
                            bool& bWriteCached,
                            CWX_UINT32 uiExpire)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bool bNewKeyValue=false;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = true;
    bWriteCached = true;

    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttNewExpire, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    CWX_UINT32 uiOutBufLen = UNISTOR_MAX_KV_SIZE;
    bool bKeyValue = false;
    if (1 == ret){
        getKvVersion(szBuf, uiBufLen, uiKeyVersion);
        if (uiExpire){
            ttNewExpire = getNewExpire(uiExpire);
        }
    }else{
        ttNewExpire = getNewExpire(uiExpire);
    }

    if (1 == ret){
        if (uiVersion && (uiKeyVersion != uiVersion)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%s] is not same with input version[%s].",
                uiKeyVersion,
                uiVersion);
            return -2; ///�汾����
        }
        uiVersion ++;
    }else{
        uiVersion = UNISTOR_KEY_START_VERION; 
    }
    if (uiSign > 2) uiSign = 0;

    if (0 == ret){//not exist
        if (2 != uiSign){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s] doesn't exist.", key.m_szData);
            return 0; ///���ܵ������field
        }
        if (field){
            tss->m_pEngineWriter->beginPack();
            if (!tss->m_pEngineWriter->addKeyValue(field->m_szData, field->m_uiDataLen, num)){
                strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
                return -1;
            }
            tss->m_pEngineWriter->pack();
            if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
                return -1;
            }
            uiBufLen = tss->m_pEngineWriter->getMsgSize();
            memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
            bNewKeyValue = true;
        }else{
            CwxCommon::toString((CWX_INT64)num, szBuf, 0);
            uiBufLen = strlen(szBuf);
            bNewKeyValue=false;
        }
    }else{
        bOldKv = isKvData(szBuf, uiBufLen);
        ret = mergeIncKeyField(tss->m_pEngineWriter,
            tss->m_pEngineReader,
            key.m_szData,
            field,
            szBuf,
            uiBufLen - getKvDataSignLen(),
            bOldKv,
            num,
            NULL,
            llMax,
            llMin,
            llValue,
            szBuf,
            uiOutBufLen,
            bKeyValue,
            uiSign,
            tss->m_szBuf2K);
        if (1 != ret){
            if (-2 == ret) ret = -3;
            return ret;
        }
        if (uiOutBufLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), uiOutBufLen);
            return -1;
        }
        uiBufLen = uiOutBufLen;
        bNewKeyValue=bKeyValue;
    }
    CwxKeyValueItemEx localData;
    localData.m_szData = szBuf;
    localData.m_uiDataLen = uiBufLen;
    localData.m_bKeyValue = bNewKeyValue;

    if (0 != appendSetBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        localData,
        ttNewExpire,
        uiSign,
        uiVersion,
        true,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();

    setKvDataSign(szBuf, uiBufLen, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K))  return -1;
    return 1;
}



///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�
int UnistorStoreMem::syncIncKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& ,
                                CwxKeyValueItemEx const* ,
                                CwxKeyValueItemEx const* ,
                                CWX_INT64 ,
                                CWX_INT64 ,
                                CWX_INT64  ,
                                CWX_INT64  ,
                                CWX_UINT32 ,
                                CWX_INT64& ,
                                CWX_UINT32 ,
                                CWX_UINT32 ,
                                CWX_UINT64 ,
                                bool& ,
                                bool& ,
                                bool  )
{
    CWX_ASSERT(0);
    strcpy(tss->m_szBuf2K, "Invalid sync message");
    return -1;
}

///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2:�汾����
int UnistorStoreMem::delKey(UnistorTss* tss,
                            CwxKeyValueItemEx const& key,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* extra,
                            CWX_UINT32& uiVersion,
                            CWX_UINT32& uiFieldNum,
                            bool& bReadCached,
                            bool& bWriteCached)
{
    uiFieldNum = 0;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bool bNewKeyValue=false;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttNewExpire, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    if (0 == ret){//not exist
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s] doesn't exist.", key.m_szData);
        return 0;
    }else{
        getKvVersion(szBuf, uiBufLen, uiKeyVersion);
        if (uiVersion){
            if (uiVersion != uiKeyVersion){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%s] is not same with input version[%s].",
                    uiKeyVersion,
                    uiVersion);
                return -2;
            }
        }
        uiVersion = uiKeyVersion + 1;
        bOldKv = isKvData(szBuf, uiBufLen);
        if (field){//ɾ��һ��field
            if (!bOldKv){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s] isn't key/value, can't delete field.", key.m_szData, field->m_szData);
                return 0;
            }
            ret = mergeRemoveKeyField(tss->m_pEngineWriter,
                tss->m_pEngineReader,
                key.m_szData,
                field,
                szBuf,
                uiBufLen-getKvDataSignLen(),
                bOldKv,
                uiFieldNum,
                tss->m_szBuf2K);
            if (1 != ret) return ret;
            if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), tss->m_pEngineWriter->getMsgSize());
                return -1;
            }
            uiBufLen = tss->m_pEngineWriter->getMsgSize();
            memcpy(szBuf, tss->m_pEngineWriter->getMsg(), uiBufLen);
            bNewKeyValue = true;
        }
    }
    if (!field){
        if (0 != appendDelBinlog(*tss->m_pEngineWriter,
            *tss->m_pEngineItemWriter,
            getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
            key,
            NULL,
            extra,
            uiVersion,
            tss->m_szBuf2K))
        {
            m_bValid = false;
            strcpy(m_szErrMsg, tss->m_szBuf2K);
            return -1;
        }
    }else{
        CwxKeyValueItemEx localData;
        localData.m_szData = szBuf;
        localData.m_uiDataLen = uiBufLen;
        localData.m_bKeyValue = bNewKeyValue;
        if (0 != appendSetBinlog(*tss->m_pEngineWriter,
            *tss->m_pEngineItemWriter,
            getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
            key,
            field,
            extra,
            localData,
            ttNewExpire,
            0,
            uiVersion,
            true,
            tss->m_szBuf2K))
        {
            m_bValid = false;
            strcpy(m_szErrMsg, tss->m_szBuf2K);
            return -1;
        }
    }
    m_ullStoreSid = getCurSid();
    if (!field){
        uiFieldNum = 0;
        if (0 != _delKey(key.m_szData, key.m_uiDataLen, tss->m_szBuf2K)) return -1;
    }else{
        setKvDataSign(szBuf, uiBufLen, uiVersion, bNewKeyValue);
        if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, &ttNewExpire, tss->m_szBuf2K)) return -1;
    }
    return 1;
}
///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�
int UnistorStoreMem::syncDelKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* ,
                                CwxKeyValueItemEx const* ,
                                CWX_UINT32 ,
                                CWX_UINT64 ullSid,
                                bool& bReadCached, ///<�����Ƿ���read cache��
                                bool& bWriteCached, ///<�����Ƿ���write cache��
                                bool  )
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bReadCached = true;
    bWriteCached = true;
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    if (0 != _delKey(key.m_szData, key.m_uiDataLen, tss->m_szBuf2K)) return -1;
    return 1;
}

///��ȡkey, 1���ɹ���0�������ڣ�-1��ʧ��;
int UnistorStoreMem::get(UnistorTss* tss,
                         CwxKeyValueItemEx const& key,
                         CwxKeyValueItemEx const* field,
                         CwxKeyValueItemEx const* ,
                         char const*& szData,
                         CWX_UINT32& uiLen,
                         bool& bKeyValue,
                         CWX_UINT32& uiVersion,
                         CWX_UINT32& uiFieldNum,
                         bool& bReadCached,
                         CWX_UINT8 ucKeyInfo)
{
    CWX_UINT32 uiExpire = 0;
    uiLen = UNISTOR_MAX_KV_SIZE;
    int ret = 0;
    bReadCached = false;
    uiFieldNum = 0;
    szData = tss->getBuf(uiLen);

    if (ucKeyInfo > 2) ucKeyInfo = 0;
    if (2 == ucKeyInfo){
        bKeyValue = false;
        uiFieldNum = 0;
        uiVersion = 0;
        return _getSysKey(tss, key.m_szData, key.m_uiDataLen, (char*)szData, uiLen);
    }

    bReadCached = true;
    ret = _getKey(key.m_szData, key.m_uiDataLen, (char*)szData, uiLen, uiExpire, tss->m_szBuf2K);
    if (1 == ret){//key����
        bKeyValue = isKvData(szData, uiLen);
        getKvVersion(szData, uiLen, uiVersion);
        if (uiLen) uiLen -= getKvDataSignLen();
        if (bKeyValue){
            uiFieldNum = CwxPackageEx::getKeyValueNum(szData, uiLen);
        }
        UnistorKeyField* fieldKey = NULL;
        if (ucKeyInfo){
            uiLen = sprintf((char*)szData,"%u,%u,%u,%u", uiExpire, uiVersion, uiLen, bKeyValue?1:0);
            bKeyValue = false;
        }else if (field){
            if (!bKeyValue){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s]'s data isn't key/value.", key.m_szData);
                return -1;
            }
            UnistorStoreBase::parseMultiField(field->m_szData, fieldKey);
            if (UNISTOR_ERR_SUCCESS != UnistorStoreBase::pickField(*tss->m_pEngineReader, *tss->m_pEngineWriter, fieldKey, szData, uiLen, tss->m_szBuf2K)){
                UnistorStoreBase::freeField(fieldKey);
                return -1;
            }
            uiLen = tss->m_pEngineWriter->getMsgSize();
            memcpy((char*)szData, tss->m_pEngineWriter->getMsg(), uiLen);
        }
        if (fieldKey) UnistorStoreBase::freeField(fieldKey);
        return 1;
    }else if (0 == ret){
        return 0;
    }
    return -1;
}

///��ȡkey, 1���ɹ���0�������ڣ�-1��ʧ��;
int UnistorStoreMem::gets(UnistorTss* tss,
                          list<pair<char const*, CWX_UINT16> > const& keys,
                          CwxKeyValueItemEx const* field,
                          CwxKeyValueItemEx const* ,
                          char const*& szData,
                          CWX_UINT32& uiLen,
                          CWX_UINT32& uiReadCacheNum, ///<��read cache�е�����
                          CWX_UINT32& uiExistNum, ///<���ڵ�key������
                          CWX_UINT8 ucKeyInfo)
{
    CWX_UINT32 uiExpire = 0;
    int ret = 0;
    CWX_UINT32 uiVersion = 0;
    bool bKeyValue = false;
    UnistorKeyField* fieldKey = NULL;
    list<pair<char const*, CWX_UINT16> >::const_iterator iter = keys.begin();
    if (ucKeyInfo > 2) ucKeyInfo = 0;
    szData = tss->getBuf(UNISTOR_MAX_KV_SIZE);
    if (field) UnistorStoreBase::parseMultiField(field->m_szData, fieldKey);
    uiReadCacheNum = 0;
    uiExistNum = 0;
    tss->m_pEngineWriter->beginPack();
    while(iter != keys.end()){
        if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_KVS_SIZE){
            if (fieldKey) UnistorStoreBase::freeField(fieldKey);
            CwxCommon::snprintf(tss->m_szBuf2K, 2048, "Output's data size is too big[%u], max is %u", tss->m_pEngineWriter->getMsgSize(), UNISTOR_MAX_KVS_SIZE);
            return -1;
        }
        do{
            uiLen = UNISTOR_MAX_KV_SIZE;
            if (2 == ucKeyInfo){
                ret = _getSysKey(tss, iter->first, iter->second, (char*)szData, uiLen);
                if (1 == ret){//key����
                    tss->m_pEngineWriter->addKeyValue(iter->first,iter->second, szData, uiLen, false);
                }else{
                    tss->m_pEngineWriter->addKeyValue(iter->first, iter->second, "", 0, false);
                }

            }else{
                ret = _getKey(iter->first, iter->second, (char*)szData, uiLen, uiExpire, tss->m_szBuf2K);
                uiReadCacheNum++;
                if (1 == ret){//key����
                    uiExistNum++;
                    bKeyValue = isKvData(szData, uiLen);
                    getKvVersion(szData, uiLen, uiVersion);
                    if (uiLen) uiLen -= getKvDataSignLen();
                    if (ucKeyInfo){
                        uiLen = sprintf((char*)szData,"%u,%u,%u,%u", uiExpire, uiVersion, uiLen, bKeyValue?1:0);
                        tss->m_pEngineWriter->addKeyValue(iter->first,iter->second, szData, uiLen, false);
                    }else if (fieldKey){
                        if (!bKeyValue ||
                            (UNISTOR_ERR_SUCCESS != UnistorStoreBase::pickField(*tss->m_pEngineReader, *tss->m_pEngineItemWriter, fieldKey, szData, uiLen, tss->m_szBuf2K)))
                        {
                            tss->m_pEngineWriter->addKeyValue(iter->first, iter->second, "", 0, true);
                        }else{
                            tss->m_pEngineWriter->addKeyValue(iter->first, iter->second, tss->m_pEngineItemWriter->getMsg(), tss->m_pEngineItemWriter->getMsgSize(), true);
                        }
                    }else{
                        tss->m_pEngineWriter->addKeyValue(iter->first, iter->second, szData, uiLen, bKeyValue);
                    }
                }
            }
        }while(0);
        iter++;
    }
    tss->m_pEngineWriter->pack();
    if (fieldKey) UnistorStoreBase::freeField(fieldKey);
    uiLen = tss->m_pEngineWriter->getMsgSize();
    szData = tss->getBuf(uiLen);
    memcpy((char*)szData, tss->m_pEngineWriter->getMsg(), uiLen);
    return 1;
}


///�����αꡣ-1���ڲ�����ʧ�ܣ�0����֧�֣�1���ɹ�
int UnistorStoreMem::createCursor(UnistorStoreCursor& cursor,
                                  char const* szBeginKey,
                                  char const* szEndKey,
                                  CwxKeyValueItemEx const* field,
                                  CwxKeyValueItemEx const*,
                                  char* )
{
    closeCursor(cursor);
    ///���ÿ�ʼ��key
    if (szBeginKey){
        cursor.m_unBeginKeyLen = strlen(szBeginKey);
        if (cursor.m_unBeginKeyLen > UNISTOR_MAX_KEY_SIZE) cursor.m_unBeginKeyLen = UNISTOR_MAX_KEY_SIZE;
        memcpy(cursor.m_beginKey, szBeginKey, cursor.m_unBeginKeyLen);
    }else{
        cursor.m_unBeginKeyLen = 0;
    }
    ///���ý�����key
    if (szEndKey){
        cursor.m_unEndKeyLen = strlen(szEndKey);
        if (cursor.m_unEndKeyLen > UNISTOR_MAX_KEY_SIZE) cursor.m_unEndKeyLen = UNISTOR_MAX_KEY_SIZE;
        memcpy(cursor.m_endKey, szEndKey, cursor.m_unEndKeyLen);
    }else{
        cursor.m_unEndKeyLen = 0;
    }
    ///����field
    if (field){
        UnistorStoreBase::parseMultiField(field->m_szData, cursor.m_field);
    }
    ///�����洢cursor
    UnistorStoreMemCursor* pCursor =  new UnistorStoreMemCursor();
    cursor.m_cursorHandle = pCursor;
    return 1;
}

///��ȡ���ݡ�-1��ʧ�ܣ�0��������1����ȡһ��
int UnistorStoreMem::next(UnistorTss* tss,
                          UnistorStoreCursor& ,
                          char const*& ,
                          CWX_UINT16& ,
                          char const*& ,
                          CWX_UINT32& ,
                          bool& ,
                          CWX_UINT32& ,
                          bool )
{

    strcpy(tss->m_szBuf2K, "Not support");
    return -1;
}


///��Ϣ��ص�event����0���ɹ���-1��ʧ��
int UnistorStoreMem::storeEvent(UnistorTss* , CwxMsgBlock*& )
{
    return 0;
}


///�ر��α�
void UnistorStoreMem::closeCursor(UnistorStoreCursor& cursor){
    UnistorStoreMemCursor* pCursor = (UnistorStoreMemCursor*) cursor.m_cursorHandle;
    if (pCursor){
        delete pCursor;
        cursor.m_cursorHandle = NULL;
    }
    if (cursor.m_field){
        UnistorStoreBase::freeField(cursor.m_field);
        cursor.m_field = NULL;
    }
}

///��ʼ�������ݡ�-1���ڲ�����ʧ�ܣ�0���ɹ�
int UnistorStoreMem::exportBegin(UnistorStoreCursor& ,
                                 char const* ,
                                 char const* , 
                                 UnistorSubscribe const& ,
                                 CWX_UINT64& ,
                                 char* szErr2K)
{
    if (szErr2K) strcpy(szErr2K, "Not support");
    return -1;
}
///��ȡ���ݡ�-1��ʧ�ܣ�0��������1����ȡһ����2��skip����Ϊ0
int UnistorStoreMem::exportNext(UnistorTss* tss,
                                UnistorStoreCursor& ,
                                char const*& ,
                                CWX_UINT16& ,
                                char const*& ,
                                CWX_UINT32& ,
                                bool& ,
                                CWX_UINT32& ,
                                CWX_UINT32& ,
                                CWX_UINT16& ,
                                char const*& ,
                                CWX_UINT32& )
{
    strcpy(tss->m_szBuf2K, "Not support");
    return -1;
}

///������������
void UnistorStoreMem::exportEnd(UnistorStoreCursor& cursor){
    UnistorStoreMemCursor* pCursor = (UnistorStoreMemCursor*) cursor.m_cursorHandle;
    if (pCursor){
        delete pCursor;
        cursor.m_cursorHandle = NULL;
    }
    if (cursor.m_cacheCursor){
        delete cursor.m_cacheCursor;
        cursor.m_cacheCursor = NULL;
    }
}

///��鶩�ĸ�ʽ�Ƿ�Ϸ�
bool UnistorStoreMem::isValidSubscribe(UnistorSubscribe const& subscribe,///<���Ķ���
                                       char* szErr2K ///<���Ϸ�ʱ�Ĵ�����Ϣ
                                       )
{
    if (!subscribe.m_bAll && (UnistorSubscribe::SUBSCRIBE_MODE_KEY == subscribe.m_uiMode)){
        map<string, string>::const_iterator iter = subscribe.m_key.m_keys.begin();
        while(iter != subscribe.m_key.m_keys.end()){
            if (iter->second.length()){
                if (iter->second > iter->first){
                    if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Subscribe's begin key[%s] is more than end key[%s]", iter->first.c_str(), iter->second.c_str());
                    return false;
                }
            }
            iter++;
        }
    }
    return true;
}



int UnistorStoreMem::commit(char* szErr2K){
    CWX_INFO(("Begin commit write cache........"));
    int ret =  _commit(szErr2K);
    CWX_INFO(("End commit write cache........"));
    return ret;
}


///�رգ�0���ɹ���-1��ʧ��
int UnistorStoreMem::close(){
    if (m_memCache){
        delete m_memCache;
        m_memCache = NULL;
    }
    return UnistorStoreBase::close();
}

///checkpoint
void UnistorStoreMem::checkpoint(UnistorTss*)
{
    if (m_bValid){
        CWX_INFO(("Begin check expire key............"));
        m_memCache->checkExpire(m_ttExpireClock, UnistorConfigMem::UNISTOR_CONFIG_BATCH_REMOVE_NUM);
        CWX_INFO(("End check expire key."));
    }
}

///ʧȥͬ��֪ͨ�������洦������ֵ��0��û�б仯��1������ͬ����-1��ʧ��
int UnistorStoreMem::lostSync(){
    if (m_binLogMgr){
        m_binLogMgr->removeAllBinlog();
    }
    if (m_memCache){
        m_memCache->reset();
    }
    return 0;
}


///commit��0���ɹ���-1��ʧ��
int UnistorStoreMem::_commit(char* szErr2K){
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    getBinLogMgr()->commit(false, szErr2K);
    m_uiUncommitBinlogNum = 0;
    m_uiLastCommitSecond = m_ttExpireClock;
    return 0;
}



//��ȡϵͳkey��1���ɹ���0�������ڣ�-1��ʧ��;
int UnistorStoreMem::_getSysKey(UnistorTss* , ///<�߳�tss����
                                char const* key, ///<Ҫ��ȡ��key
                                CWX_UINT16 unKeyLen, ///<key�ĳ���
                                char* szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
                                CWX_UINT32& uiLen ///<szData���ݵ��ֽ���
                                )
{
    string strValue(key, unKeyLen);
    if (1 == getSysKey(key, unKeyLen, szData, uiLen)) return 1;
    if (strValue == UNISTOR_KEY_SID){
        CwxCommon::toString(m_ullStoreSid, szData, 10);
    }else{
        return 0;
    }
    uiLen = strlen(szData);
    return 1;
}


//0:�ɹ���-1��ʧ��
int UnistorStoreMem::_setKey(char const* szKey,
                             CWX_UINT16 unKeyLen,
                             char const* szData,
                             CWX_UINT32 uiLen,
                             CWX_UINT32* uiExpire,
                             char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    m_memCache->insert(szKey, unKeyLen, szData, uiLen, uiExpire);
    m_uiUncommitBinlogNum++;
    if (isNeedCommit()){
        if (0 != _commit(szErr2K)) return -1;
    }
    return 0;
}


//0:�����ڣ�1����ȡ��-1��ʧ��
int UnistorStoreMem::_getKey(char const* szKey,
                             CWX_UINT16 unKeyLen,
                             char* szData,
                             CWX_UINT32& uiLen,
                             CWX_UINT32& uiExpire,
                             char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = m_memCache->fetch(szKey, unKeyLen, szData, uiLen, uiExpire, m_ttExpireClock, true);
    if (-1 == ret){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Data buf size[%u] is too small.", uiLen);
    }
    return ret;
}


//0:�ɹ���-1��ʧ��
int UnistorStoreMem::_delKey(char const* szKey,
                             CWX_UINT16 unKeyLen,
                             char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    m_memCache->remove(szKey, unKeyLen, 0);
    return 0;
}
