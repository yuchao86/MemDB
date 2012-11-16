#include "UnistorStoreEmpty.h"

#define  EMPTY_MIN_READ_CACHE_MSIZE   128
#define  EMPTY_MAX_READ_CACHE_MSIZE   (64 * 1024)

#define  EMPTY_MIN_WRITE_CACHE_MSIZE   32
#define  EMPTY_MAX_WRITE_CACHE_MSIZE   1024

#define  EMPTY_MIN_READ_CACHE_KEY_NUM   100000
#define  EMPTY_MAX_READ_CACHE_KEY_NUM   800000000


extern "C"{
	UnistorStoreBase* unistor_create_engine()
	{
		return new UnistorStoreEmpty();
	}
}

//0:成功；-1：失败
int UnistorStoreEmpty::parseConf(){
    //TODO 解析unistor.cnf中[empty]部分的配置
    //TODO 输出配置信息
	return 0;
}


//加载配置文件.-1:failure, 0:success
int UnistorStoreEmpty::init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc,
                            UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<获取系统信息的function
                            void* pApp, ///<UnistorApp对象
                          UnistorConfig const* config)
{
	m_bValid = false;
	strcpy(m_szErrMsg, "Not init");
    ///调用基类的init
    if (0 != UnistorStoreBase::init(msgPipeFunc, getSysInfoFunc, pApp, config)) return -1;
    m_uiWriteCacheMSize = m_config->getCommon().m_uiWriteCacheMByte;
    if (m_uiWriteCacheMSize < EMPTY_MIN_WRITE_CACHE_MSIZE) m_uiWriteCacheMSize = EMPTY_MIN_WRITE_CACHE_MSIZE;
    if (m_uiWriteCacheMSize > EMPTY_MAX_WRITE_CACHE_MSIZE) m_uiWriteCacheMSize = EMPTY_MAX_WRITE_CACHE_MSIZE;

    m_uiReadCacheMSize = m_config->getCommon().m_uiReadCacheMByte;
    if (m_uiReadCacheMSize < EMPTY_MIN_READ_CACHE_MSIZE) m_uiReadCacheMSize = EMPTY_MIN_READ_CACHE_MSIZE;
    if (m_uiReadCacheMSize > EMPTY_MAX_READ_CACHE_MSIZE) m_uiReadCacheMSize = EMPTY_MAX_READ_CACHE_MSIZE;

    m_uiReadCacheItemNum = m_config->getCommon().m_uiReadCacheMaxKeyNum;
    if (m_uiReadCacheItemNum < EMPTY_MIN_READ_CACHE_KEY_NUM) m_uiReadCacheItemNum = EMPTY_MIN_READ_CACHE_KEY_NUM;
    if (m_uiReadCacheItemNum > EMPTY_MAX_READ_CACHE_KEY_NUM) m_uiReadCacheItemNum = EMPTY_MAX_READ_CACHE_KEY_NUM;
    //parse conf
    if (0 != parseConf()) return -1;
    //启动cache
    if (0 != startCache(m_uiWriteCacheMSize,
        m_uiReadCacheMSize,
        m_uiReadCacheItemNum,
        UnistorStoreEmpty::cacheWriteBegin,
        UnistorStoreEmpty::cacheWrite,
        UnistorStoreEmpty::cacheWriteEnd,
        this,
        UnistorStoreEmpty::keyStoreCmpEqual,
        UnistorStoreEmpty::keyStoreCmpLess,
        UnistorStoreEmpty::keyStoreHash,
        1.2,
        m_szErrMsg))
    {
        CWX_ERROR(("Failure to start cache, err=%s", m_szErrMsg));
        return -1;
    }

    m_strEngine = m_config->getCommon().m_strStoreType;
	m_uiUncommitBinlogNum = 0;
    m_uiLastCommitSecond = 0;
    m_ullStoreSid = 0;

    m_bValid = true;
	//TODO 初始化empty 引擎
	//恢复binlog
	if (0 != restore(m_ullStoreSid)){
		m_bValid = false;
		return -1;
	}
	m_szErrMsg[0] = 0x00;
	return 0;
}

///开始写的函数，返回值：0，成功；-1：失败
int UnistorStoreEmpty::cacheWriteBegin(void* context, char* szErr2K)
{
    UnistorStoreEmpty* pEmpty = (UnistorStoreEmpty*)context;
    if (!pEmpty->m_bValid){
        if (szErr2K) strcpy(szErr2K, pEmpty->m_szErrMsg);
        return -1;
    }
    CWX_INFO(("Begin commit....................."));
    //flush binlog first
    if (0 != pEmpty->flushBinlog(pEmpty->m_szErrMsg)){
        if (szErr2K) strcpy(szErr2K, pEmpty->m_szErrMsg);
        return -1;
    }
    //TODO 引擎准备保存dirty数据
    return 0;
}
///写数据，返回值：0，成功；-1：失败
int UnistorStoreEmpty::cacheWrite(void* context,
                                char const* szKey,
                                CWX_UINT16 unKeyLen,
                                char const* szData,
                                CWX_UINT32 uiDataLen,
                                bool bDel,
                                CWX_UINT32 ttOldExpire,
                                char* szStoreKeyBuf,
                                CWX_UINT16 unKeyBufLen,
                                char* szErr2K)
{
    UnistorStoreEmpty* pEmpty = (UnistorStoreEmpty*)context;
    if (!pEmpty->m_bValid){
        if (szErr2K) strcpy(szErr2K, pEmpty->m_szErrMsg);
        return -1;
    }
    UnistorStoreExpireKey* pKey=NULL;
    memcpy(szStoreKeyBuf, szKey, unKeyLen);
    if (bDel){
        if (0 != pEmpty->_delEmptyKey(szStoreKeyBuf, unKeyLen, unKeyBufLen, szErr2K)) return -1;
        if (pEmpty->m_bEnableExpire){
            pKey = (UnistorStoreExpireKey*)szStoreKeyBuf;
            if (ttOldExpire){///删除旧key
                pKey->m_ttExpire = ttOldExpire;
                memcpy(pKey->m_key, szKey, unKeyLen);
                if (0 != pEmpty->_delEmptyKey(szStoreKeyBuf, sizeof(UnistorStoreExpireKey) + unKeyLen, unKeyBufLen, szErr2K)) return -1;
            }
        }
    }else{
        if (0 != pEmpty->_setEmptyKey(szStoreKeyBuf, unKeyLen, unKeyBufLen, szData, uiDataLen, szErr2K)) return -1;
        if (pEmpty->m_bEnableExpire){
            pKey = (UnistorStoreExpireKey*)szStoreKeyBuf;
            CWX_UINT32 uiVersion=0;
            CWX_UINT32 ttNewExpire = 0;
            pEmpty->getKvVersion(szData, uiDataLen, ttNewExpire, uiVersion);
            if (ttOldExpire != ttNewExpire){
                if (ttOldExpire){///删除旧key
                    pKey->m_ttExpire = ttOldExpire;
                    memcpy(pKey->m_key, szKey, unKeyLen);
                    if (0 != pEmpty->_delEmptyKey(szStoreKeyBuf, sizeof(UnistorStoreExpireKey) + unKeyLen, unKeyBufLen, szErr2K)) return -1;
                }
                pKey->m_ttExpire = ttNewExpire;
                memcpy(pKey->m_key, szKey, unKeyLen);
                if (0 != pEmpty->_setEmptyKey(szStoreKeyBuf, sizeof(UnistorStoreExpireKey) + unKeyLen, unKeyBufLen, "", 0, szErr2K)) return -1;
            }
        }
    }
    return 0;
}

///提交数据，返回值：0，成功；-1：失败
int UnistorStoreEmpty::cacheWriteEnd(void* context, CWX_UINT64 ullSid, void* , char* szErr2K)
{
    UnistorStoreEmpty* pEmpty = (UnistorStoreEmpty*)context;
    if (!pEmpty->m_bValid){
        if (szErr2K) strcpy(szErr2K, pEmpty->m_szErrMsg);
        return -1;
    }
    //commit the empty
    if (0 != pEmpty->_updateSysInfo(ullSid, szErr2K)){
        return -1;
    }
    //TODO: 提交数据
    CWX_INFO(("End commit....................."));
    return 0;
}


///检测是否存在key；1：存在；0：不存在；-1：失败
int UnistorStoreEmpty::isExist(UnistorTss* tss,
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
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true, tss->m_szBuf2K);
    uiFieldNum = 0;
    if (1 == ret){
        ret = unpackFields(*tss->m_pEngineReader, szBuf, uiBufLen, ttOldExpire, uiVersion);
        if (-1 == ret){
            strcpy(tss->m_szBuf2K, tss->m_pEngineReader->getErrMsg());
            return -1;
        }
        ///检查是否超时
        if (m_config->getCommon().m_bEnableExpire && (ttOldExpire <= m_ttExpireClock)) return 0;
        ///不是Key/Value结构
        if (0 == ret){
            if (field) return 0;
            return 1;
        }
        uiFieldNum = tss->m_pEngineReader->getKeyNum();
        if (field)  return tss->m_pEngineReader->getKey(field->m_szData)?1:0;
        return 1;
    }
    if (0 == ret)  return 0;
    return -1;
}

///添加key，1：成功；0：存在；-1：失败；
int UnistorStoreEmpty::addKey(UnistorTss* tss,
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
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiOldVersion = 0;
    uiFieldNum = 0;
    ///修改sign
    if (uiSign > 2) uiSign = 0;
    int ret = 0;
    bReadCached = false;
    bWriteCached = false;

    if ((0==uiSign) && uiVersion && !m_config->getCommon().m_bEnableExpire){///如果指定了版本的全替换，而且不超时
        ret = 0;
        ttNewExpire = 0;
        uiOldVersion = 0;
    }else{
        ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, bCache, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiOldVersion);
        ///计算超时时间
        if (m_config->getCommon().m_bEnableExpire){
            if((1==ret) && (ttOldExpire<=m_ttExpireClock)) ret = 0;///超时
            if (1 == ret){
                ttNewExpire = ttOldExpire;
            }else{
                ttNewExpire = getNewExpire(uiExpire);
            }
        }else{
            ttNewExpire = 0;
        }
    }
    if (0 == ret){//not exist
        if (1 == uiSign){///添加field，而且key必须存在
            strcpy(tss->m_szBuf2K, "Key doesn't exist, you can't add any field.");
            return -1; ///不能单独添加field
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
        if (!uiVersion) uiVersion = UNISTOR_KEY_START_VERION; ///起始版本
    }else if (1 == ret){//key存在
        if (!uiVersion){
            uiVersion = uiOldVersion + 1;
        }
        if (0 == uiSign){//添加key
            strcpy(tss->m_szBuf2K, "Key exists.");
            return 0; //key存在。
        }
        bool bOldKv = isKvData(szBuf, uiBufLen);
        if (!bOldKv){///原来数据不是key/value结构，则无论如何不能add key或add field
            strcpy(tss->m_szBuf2K, "Key is not key/value, can't add field.");
            return 0; //key存在。
        }
        if (!field && !data.m_bKeyValue){
            strcpy(tss->m_szBuf2K, "The added content is not field.");
            return 0; //key存在。
        }
        //此时，key的新、旧value一定为key/value结构
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

    if (0 != appendAddBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        data,
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
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, bCache, tss->m_szBuf2K)) return -1;
    return 1;
}
                            
///添加key，1：成功；0：存在；-1：失败；
int UnistorStoreEmpty::syncAddKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* field,
                                CwxKeyValueItemEx const* extra,
                                CwxKeyValueItemEx const& data,
                                CWX_UINT32 uiSign,///<not use
                                CWX_UINT32 uiVersion,
                                bool bCache,
                                CWX_UINT32 uiExpire,
                                CWX_UINT64 ullSid,
                                bool& bReadCached, ///<数据是否在read cache中
                                bool& bWriteCached, ///<数据是否在write cache中
                                bool  bRestore)
{
    return syncSetKey(tss, key, field, extra, data, uiSign, uiVersion, bCache, uiExpire, ullSid, bReadCached, bWriteCached, bRestore);
}

///set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
int UnistorStoreEmpty::setKey(UnistorTss* tss,
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
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiOldVersion = 0;
	CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
	char* szBuf = tss->getBuf(uiBufLen);
    int ret = 0;
    bReadCached = false;
    bWriteCached = false;

    if ((0==uiSign) && uiVersion && !m_config->getCommon().m_bEnableExpire){///如果指定了版本的全替换，而且不超时
        ret = 0;
        ttNewExpire = 0;
        uiOldVersion = 0;
    }else{
        ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, bCache, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiOldVersion);
        ///计算超时时间
        if (m_config->getCommon().m_bEnableExpire){
            if((1==ret) && (ttOldExpire<=m_ttExpireClock)) ret = 0;///超时
            if (1 == ret){
                ttNewExpire = ttOldExpire;
            }else{
                ttNewExpire = getNewExpire(uiExpire);
            }
        }else{
            ttNewExpire = 0;
        }
    }
    uiFieldNum = 0;
    if (uiSign > 1) uiSign = 0;
	if ((0 == ret/*旧值不存在*/) ||
		(0 == uiSign)/*替换整个key*/)
    {
        if (!uiVersion){
            if (0 == ret){
                uiVersion = UNISTOR_KEY_START_VERION; ///起始版本
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
	}else{//key存在而且不是全部替换
        if (!isKvData(szBuf, uiBufLen)){
            strcpy(tss->m_szBuf2K, "Key is key/value.");
            return -1;
        }
        if (!field && !data.m_bKeyValue){
            strcpy(tss->m_szBuf2K, "The set content is key/value.");
            return -1;
        }
        if (!uiVersion) uiVersion = uiOldVersion + 1;
        //此时，key的旧、新value一定为key/value结构
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
    if (0 != appendSetBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        data,
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
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
	if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, bCache, tss->m_szBuf2K)) return -1;
	return 1;
}

///import key，1：成功；-1：失败；
int UnistorStoreEmpty::importKey(UnistorTss* tss, ///<tss对象
                               CwxKeyValueItemEx const& key, ///<添加的key
                               CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
                               CwxKeyValueItemEx const& data, ///<添加key或field的数据
                               CWX_UINT32& uiVersion, ///<若大于0，则设置修改后的key为此版本
                               bool& bReadCached, ///<数据是否在read cache中
                               bool& bWriteCached, ///<数据是否在write cache中
                               bool bCache, ///<是否将key放到读cache
                               CWX_UINT32 uiExpire ///<若创建key，而且指定了uiExpire则设置key的超时时间
                               )
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bool bNewKeyValue = false;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiOldVersion = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    bReadCached = false;
    bWriteCached = false;

    char* szBuf = tss->getBuf(uiBufLen);
    int ret = 0;
    if (m_config->getCommon().m_bEnableExpire){
        ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, false, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiOldVersion);
        ttNewExpire = uiExpire;
    }else{
        ttNewExpire = 0;
    }
    if (!uiVersion) uiVersion = UNISTOR_KEY_START_VERION; ///起始版本
    if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
        return -1;
    }
    uiBufLen = data.m_uiDataLen;
    memcpy(szBuf, data.m_szData, uiBufLen);
    bNewKeyValue = data.m_bKeyValue;

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
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, bCache, tss->m_szBuf2K)) return -1;
    return 1;

}

///sync import key，1：成功；-1：错误。
int UnistorStoreEmpty::syncImportKey(UnistorTss* tss, ///<线程的tss数据
                                     CwxKeyValueItemEx const& key, ///<set的key
                                     CwxKeyValueItemEx const* , ///<存储引擎的extra数据
                                     CwxKeyValueItemEx const& data, ///<set的数据
                                     CWX_UINT32 uiVersion, ///<set的key 版本号
                                     bool bCache,    ///<是否将key放到读cache
                                     CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
                                     CWX_UINT64 ullSid, ///<操作对应的binlog的sid
                                     bool& bReadCached, ///<数据是否在read cache中
                                     bool& bWriteCached, ///<数据是否在write cache中
                                     bool  bRestore ///<是否从binlog恢复的数据
                                   )
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;
    bReadCached = false;
    bWriteCached = false;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = 0;
    if (m_config->getCommon().m_bEnableExpire){
        ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, false, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
        ttNewExpire = uiExpire;
    }else{
        ttNewExpire = 0;
    }

    if (1 == ret){
        if (bRestore && (uiKeyVersion >= uiVersion)){
            if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
            return 1; ///已经添加
        }
    }
    if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
        return -1;
    }
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;

    uiBufLen = data.m_uiDataLen;
    memcpy(szBuf, data.m_szData, uiBufLen);
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, data.m_bKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, bCache, tss->m_szBuf2K)) return -1;
    return 1;
}


///set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
int UnistorStoreEmpty::syncSetKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* field,
                                CwxKeyValueItemEx const* ,
                                CwxKeyValueItemEx const& data,
                                CWX_UINT32 uiSign,
                                CWX_UINT32 uiVersion,
                                bool bCache,
                                CWX_UINT32 uiExpire,
                                CWX_UINT64 ullSid,
                                bool& bReadCached,
                                bool& bWriteCached,
                                bool  bRestore)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    CWX_UINT32 uiFieldNum = 0;
    bool bNewKeyValue = false;

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion;
    bReadCached = false;
    bWriteCached = false;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = 0;
    if ((0==uiSign) && !bRestore && !m_config->getCommon().m_bEnableExpire){///如果指定了版本的全替换，而且不超时
        ret = 0;
        ttNewExpire = 0;
        uiKeyVersion = 0;
    }else{
        ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, bCache, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
        ///计算超时时间
        if (m_config->getCommon().m_bEnableExpire){
            if((1==ret) && (ttOldExpire<=m_ttExpireClock)) ret = 0;///超时
            if (1 == ret){
                ttNewExpire = ttOldExpire;
            }else{
                ttNewExpire = getNewExpire(uiExpire);
            }
        }else{
            ttNewExpire = 0;
        }
    }

    if ((0 == ret/*旧值不存在*/) ||
        (!isKvData(szBuf, uiBufLen))/*旧值不是kv，直接替换*/ ||
        (!field && !data.m_uiDataLen)/*新值不是key/value*/)
    {
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
        }else{
            if (data.m_uiDataLen > UNISTOR_MAX_DATA_SIZE - getKvDataSignLen()){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key/value is too big, max is [%u], now[%u]", UNISTOR_MAX_DATA_SIZE - getKvDataSignLen(), data.m_uiDataLen);
                return -1;
            }
            uiBufLen = data.m_uiDataLen;
            memcpy(szBuf, data.m_szData, uiBufLen);
            bNewKeyValue = data.m_bKeyValue;
        }
    }else{//key存在，此是属于两部分的merge
        if (bRestore){
            if (uiKeyVersion >= uiVersion){
                m_ullStoreSid = ullSid;
                return 1; ///已经添加
            }
        }
        //此时，key的旧、新value一定为key/value结构
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
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, bCache, tss->m_szBuf2K)) return -1;
    return 1;

}

///update key，1：成功；0：不存在；-1：失败；-2：版本错误
int UnistorStoreEmpty::updateKey(UnistorTss* tss,
                               CwxKeyValueItemEx const& key,
                               CwxKeyValueItemEx const* field,
                               CwxKeyValueItemEx const* extra,
                               CwxKeyValueItemEx const& data,
                               CWX_UINT32 uiSign,
                               CWX_UINT32& uiVersion,
                               CWX_UINT32& uiFieldNum,
                               bool& bReadCached, ///<数据是否在read cache中
                               bool& bWriteCached, ///<数据是否在write cache中
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
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion=0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;

    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
    ///计算超时时间
    if (m_config->getCommon().m_bEnableExpire){
        if((1==ret) && (ttOldExpire<=m_ttExpireClock)) ret = 0; ///超时
        if (1 == ret){
            ttNewExpire = ttOldExpire;
        }else{
            ttNewExpire = getNewExpire(uiExpire);
        }
    }else{
        ttNewExpire = 0;
    }

    if (0 == ret){//not exist
        strcpy(tss->m_szBuf2K,"Key doesn't exist.");
        return 0; ///不能单独添加field
    }
    //key存在
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
    if (0 == uiSign){///更新整个key
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
    }else{//此时，新、旧值全部为kv
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

    if (0 != appendUpdateBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        data,
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
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, true, tss->m_szBuf2K)) return -1;
    return 1;
}

///update key，1：成功；0：不存在；-1：失败
int UnistorStoreEmpty::syncUpdateKey(UnistorTss* tss,
                                   CwxKeyValueItemEx const& key,
                                   CwxKeyValueItemEx const* field,
                                   CwxKeyValueItemEx const* extra,
                                   CwxKeyValueItemEx const& data,
                                   CWX_UINT32 uiSign,
                                   CWX_UINT32 uiVersion,
                                   CWX_UINT32 uiExpire,
                                   CWX_UINT64 ullSid,
                                   bool& bReadCached,
                                   bool& bWriteCached,
                                   bool  bRestore)
{
    return syncSetKey(tss,
        key,
        field,
        extra,
        data,
        uiSign,
        uiVersion,
        true,
        uiExpire,
        ullSid,
        bReadCached,
        bWriteCached,
        bRestore);
}


///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；-3：超出边界
int UnistorStoreEmpty::incKey(UnistorTss* tss,
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
                            CWX_UINT32  uiExpire)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bool bNewKeyValue=false;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    CWX_UINT32 uiOutBufLen = UNISTOR_MAX_KV_SIZE;
    bool bKeyValue = false;
    if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
    if (1 == ret){
        if (uiVersion && (uiKeyVersion != uiVersion)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%s] is not same with input version[%s].",
                uiKeyVersion,
                uiVersion);
            return -2; ///版本错误
        }
        uiVersion ++;
    }else{
        uiVersion = UNISTOR_KEY_START_VERION; 
    }
    ///计算超时时间
    if (m_config->getCommon().m_bEnableExpire){
        if((1==ret) && (ttOldExpire<=m_ttExpireClock)) ret = 0; ///超时
        if (1 == ret){
            ttNewExpire = ttOldExpire;
        }else{
            ttNewExpire = getNewExpire(uiExpire);
        }
    }else{
        ttNewExpire = 0;
    }

    if (uiSign > 2) uiSign = 0;

    if (0 == ret){//not exist
        if (2 != uiSign){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s] doesn't exist.", key.m_szData);
            return 0; ///不能单独添加field
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

    if (0 != appendIncBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        num,
        llValue,
        llMax,
        llMin,
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

    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, true, tss->m_szBuf2K))  return -1;
    return 1;
}



///inc key，1：成功；0：不存在；-1：失败；
int UnistorStoreEmpty::syncIncKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* field,
                                CwxKeyValueItemEx const* ,
                                CWX_INT64 num,  ///<inc的数值，可以为负值
                                CWX_INT64 result,  ///<inc的数值，可以为负值
                                CWX_INT64  llMax,
                                CWX_INT64  llMin,
                                CWX_UINT32 ,
                                CWX_INT64& llValue,
                                CWX_UINT32 uiVersion,
                                CWX_UINT32 uiExpire,
                                CWX_UINT64 ullSid,
                                bool& bReadCached,
                                bool& bWriteCached,
                                bool  bRestore)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bool bNewKeyValue=false;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 ttNewExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    bReadCached = false;
    bWriteCached = false;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    CWX_UINT32 uiOutBufLen = UNISTOR_MAX_KV_SIZE;
    bool bKeyValue = false;
    if (1 == ret) getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
    ///计算超时时间
    if (m_config->getCommon().m_bEnableExpire){
        if((1==ret) && (ttOldExpire<=m_ttExpireClock)) ret = 0; ///超时
        if (1 == ret){
            ttNewExpire = ttOldExpire;
        }else{
            ttNewExpire = getNewExpire(uiExpire);
        }
    }else{
        ttNewExpire = 0;
    }

    if (0 == ret){//not exist
        if (field){
            tss->m_pEngineWriter->beginPack();
            tss->m_pEngineWriter->addKeyValue(field->m_szData, field->m_uiDataLen, num);
            tss->m_pEngineWriter->pack();
            memcpy(szBuf, tss->m_pEngineWriter->getMsg(), tss->m_pEngineWriter->getMsgSize());
            uiBufLen = tss->m_pEngineWriter->getMsgSize();
            bNewKeyValue = true;
        }else{
            CwxCommon::toString((CWX_INT64)num, szBuf, 0);
            uiBufLen = strlen(szBuf);
            bNewKeyValue=false;
        }
    }else{
        if (bRestore){
            if (uiKeyVersion >= uiVersion){
                m_ullStoreSid = ullSid;
                return 1;
            }
        }
        bOldKv = isKvData(szBuf, uiBufLen);
        ret = mergeIncKeyField(tss->m_pEngineWriter,
            tss->m_pEngineReader,
            key.m_szData,
            field,
            szBuf,
            uiBufLen - getKvDataSignLen(),
            bOldKv,
            num,
            &result,
            llMax,
            llMin,
            llValue,
            szBuf,
            uiOutBufLen,
            bKeyValue,
            2,
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
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    setKvDataSign(szBuf, uiBufLen, ttNewExpire, uiVersion, bNewKeyValue);
    if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, true, tss->m_szBuf2K)) return -1;
    return 1;
}

///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；
int UnistorStoreEmpty::delKey(UnistorTss* tss,
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
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;
    bReadCached = false;
    bWriteCached = false;

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true,  tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    if (0 == ret){//not exist
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s] doesn't exist.", key.m_szData);
        return 0;
    }else{
        getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
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
        if (field){//删除一个field
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
    if (0 != appendDelBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        uiVersion,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();
    if (!field){
        uiFieldNum = 0;
        if (0 != _delKey(key.m_szData, key.m_uiDataLen, ttOldExpire, bWriteCached, tss->m_szBuf2K)) return -1;
    }else{
        setKvDataSign(szBuf, uiBufLen, ttOldExpire, uiVersion, bNewKeyValue);
        if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, true, tss->m_szBuf2K)) return -1;
    }
    return 1;
}
///inc key，1：成功；0：不存在；-1：失败；
int UnistorStoreEmpty::syncDelKey(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* field,
                                CwxKeyValueItemEx const* ,
                                CWX_UINT32 uiVersion,
                                CWX_UINT64 ullSid,
                                bool& bReadCached, ///<数据是否在read cache中
                                bool& bWriteCached, ///<数据是否在write cache中
                                bool  bRestore)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    CWX_UINT32 uiFieldNum = 0;
    bool bNewKeyValue=false;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 uiKeyVersion = 0;
    bReadCached = false;
    bWriteCached = false;

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true,  tss->m_szBuf2K);
    if (-1 == ret) return -1;
    bool bOldKv = false;
    if (0 == ret){//not exist
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key[%s] doesn't exist.", key.m_szData);
        return 0;
    }else{
        getKvVersion(szBuf, uiBufLen, ttOldExpire, uiKeyVersion);
        if (bRestore){
            if (uiKeyVersion >= uiVersion){
                m_ullStoreSid = ullSid;
                return 1;
            }
        }
        bOldKv = isKvData(szBuf, uiBufLen);
        if (field){//删除一个field
            if (!bOldKv){
                return 1;
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
        //否则，删除整个key
    }
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    if (!field){
        if (0 != _delKey(key.m_szData, key.m_uiDataLen, ttOldExpire, bWriteCached, tss->m_szBuf2K)) return -1;
    }else{
        setKvDataSign(szBuf, uiBufLen, ttOldExpire, uiVersion, bNewKeyValue);
        if (0 != _setKey(key.m_szData, key.m_uiDataLen, szBuf, uiBufLen, ttOldExpire, bWriteCached, true, tss->m_szBuf2K)) return -1;
    }
    return 1;
}

///获取key, 1：成功；0：不存在；-1：失败;
int UnistorStoreEmpty::get(UnistorTss* tss,
                         CwxKeyValueItemEx const& key,
                         CwxKeyValueItemEx const* field,
                         CwxKeyValueItemEx const* ,
                         char const*& szData,
                         CWX_UINT32& uiLen,
                         bool& bKeyValue,
                         CWX_UINT32& uiVersion,
                         CWX_UINT32& uiFieldNum,
                         bool& bReadCached, ///<数据是否在read cache中
                         CWX_UINT8 ucKeyInfo)
{
    uiLen = UNISTOR_MAX_KV_SIZE;
    int ret = 0;
    bReadCached = false;
    uiFieldNum = 0;
    szData = tss->getBuf(uiLen);

    if (ucKeyInfo > 2) ucKeyInfo = 0;
    if (ucKeyInfo > 2) ucKeyInfo = 0;
    if (2 == ucKeyInfo){
        bKeyValue = false;
        uiFieldNum = 0;
        uiVersion = 0;
        return _getSysKey(tss, key.m_szData, key.m_uiDataLen, (char*)szData, uiLen);
    }

	ret = _getKey(key.m_szData, key.m_uiDataLen, (char*)szData, uiLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true, tss->m_szBuf2K);
	if (1 == ret){//key存在
        CWX_UINT32 ttOldExpire = 0;
        getKvVersion(szData, uiLen, ttOldExpire, uiVersion);
        if (m_config->getCommon().m_bEnableExpire && (ttOldExpire<=m_ttExpireClock)) return 0;
		bKeyValue = isKvData(szData, uiLen);
		if (uiLen) uiLen -= getKvDataSignLen();
        if (bKeyValue){
            uiFieldNum = CwxPackageEx::getKeyValueNum(szData, uiLen);
        }
        UnistorKeyField* fieldKey = NULL;
        if (ucKeyInfo){
            uiLen = sprintf((char*)szData,"%u,%u,%u,%u", ttOldExpire, uiVersion, uiLen, bKeyValue?1:0);
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

///获取key, 1：成功；0：不存在；-1：失败;
int UnistorStoreEmpty::gets(UnistorTss* tss,
                 list<pair<char const*, CWX_UINT16> > const& keys,
                 CwxKeyValueItemEx const* field,
                 CwxKeyValueItemEx const* ,
                 char const*& szData,
                 CWX_UINT32& uiLen,
                 CWX_UINT32& uiReadCacheNum, ///<在read cache中的数量
                 CWX_UINT32& uiExistNum, ///<存在的key的数量
                 CWX_UINT8 ucKeyInfo)
{
    int ret = 0;
    CWX_UINT32 uiVersion = 0;
    CWX_UINT32 ttOldExpire = 0;
    bool bReadCache = false;
    bool bKeyValue = false;
    UnistorKeyField* fieldKey = NULL;
    list<pair<char const*, CWX_UINT16> >::const_iterator iter = keys.begin();
    szData = tss->getBuf(UNISTOR_MAX_KV_SIZE);
    if (field) UnistorStoreBase::parseMultiField(field->m_szData, fieldKey);
    uiReadCacheNum = 0;
    uiExistNum = 0;
    tss->m_pEngineWriter->beginPack();
    if (ucKeyInfo > 2) ucKeyInfo = 0;
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
                if (1 == ret){//key存在
                    tss->m_pEngineWriter->addKeyValue(iter->first,iter->second, szData, uiLen, false);
                }else{
                    tss->m_pEngineWriter->addKeyValue(iter->first, iter->second, "", 0, false);
                }
            }else{
                ret = _getKey(iter->first, iter->second, (char*)szData, uiLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCache, true, tss->m_szBuf2K);
                if (bReadCache) uiReadCacheNum++;
                if (1 == ret){//key存在
                    uiExistNum++;
                    getKvVersion(szData, uiLen, ttOldExpire, uiVersion);
                    if (m_config->getCommon().m_bEnableExpire && (ttOldExpire<=m_ttExpireClock)){///timeout
                        break;
                    }else{
                        bKeyValue = isKvData(szData, uiLen);
                        if (uiLen) uiLen -= getKvDataSignLen();
                    }
                    if (ucKeyInfo){
                        uiLen = sprintf((char*)szData,"%u,%u,%u,%u", ttOldExpire, uiVersion, uiLen, bKeyValue?1:0);
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


///建立游标。-1：内部错误失败；0：不支持；1：成功
int UnistorStoreEmpty::createCursor(UnistorStoreCursor& cursor,
                                    char const* szBeginKey,
                                    char const* szEndKey,
                                    CwxKeyValueItemEx const* field,
                                    CwxKeyValueItemEx const*,
                                    char* )
{
    closeCursor(cursor);
    ///设置开始的key
    if (szBeginKey){
        cursor.m_unBeginKeyLen = strlen(szBeginKey);
        if (cursor.m_unBeginKeyLen > UNISTOR_MAX_KEY_SIZE) cursor.m_unBeginKeyLen = UNISTOR_MAX_KEY_SIZE;
        memcpy(cursor.m_beginKey, szBeginKey, cursor.m_unBeginKeyLen);
    }else{
        cursor.m_unBeginKeyLen = 0;
    }
    ///设置结束的key
    if (szEndKey){
        cursor.m_unEndKeyLen = strlen(szEndKey);
        if (cursor.m_unEndKeyLen > UNISTOR_MAX_KEY_SIZE) cursor.m_unEndKeyLen = UNISTOR_MAX_KEY_SIZE;
        memcpy(cursor.m_endKey, szEndKey, cursor.m_unEndKeyLen);
    }else{
        cursor.m_unEndKeyLen = 0;
    }
    ///创建cache cursor
    cursor.m_cacheCursor = new UnistorStoreCacheCursor();
    UnistorStoreEmptyCursor* pCursor =  new UnistorStoreEmptyCursor();
    if (field){
        UnistorStoreBase::parseMultiField(field->m_szData, cursor.m_field);
    }
    //TODO:创建引擎的cursor
    pCursor->m_bFirst = true;
    cursor.m_cursorHandle = pCursor;
    return 1;
}

///获取数据。-1：失败；0：结束；1：获取一个
int UnistorStoreEmpty::next(UnistorTss* tss,
                          UnistorStoreCursor& cursor,
                          char const*& szKey,
                          CWX_UINT16& unKeyLen,
                          char const*& szData,
                          CWX_UINT32& uiDataLen,
                          bool& bKeyValue,
                          CWX_UINT32& uiVersion,
                          bool bKeyInfo)
{
    UnistorStoreEmptyCursor* pEmptyCursor=(UnistorStoreEmptyCursor*)cursor.m_cursorHandle;
    int ret = 0;
    bool bUserCache = false;
    CWX_UINT32 ttExpire=0;
    uiDataLen = UNISTOR_MAX_KV_SIZE;
    szData = tss->getBuf(uiDataLen);
    do{
        if (pEmptyCursor->m_bStoreMore && !pEmptyCursor->m_bStoreValue){///获取store的值
            pEmptyCursor->m_unStoreKeyLen = UNISTOR_MAX_KEY_SIZE;
            pEmptyCursor->m_uiStoreDataLen = UNISTOR_MAX_KV_SIZE;
            ret = _nextEmpty(cursor, tss->m_szBuf2K);
            if (-1 == ret) return -1;
            if (0 == ret){
                pEmptyCursor->m_bStoreMore = false;
            }else{
                pEmptyCursor->m_bStoreValue = true;
            }
        }
        if (cursor.m_cacheCursor->m_bCacheMore && !cursor.m_cacheCursor->m_bCacheValue){///获取cache的值
            ret = nextCache(cursor, tss->m_szBuf2K);
            if (-1 == ret) return -1;
            if (0 == ret){
                cursor.m_cacheCursor->m_bCacheMore = false;
            }else{
                cursor.m_cacheCursor->m_bCacheValue = true;
            }
        }
        if (!pEmptyCursor->m_bStoreMore && !cursor.m_cacheCursor->m_bCacheMore) return 0; ///没有数据

        bUserCache = false;
        if (cursor.m_cacheCursor->m_bCacheMore){
            if (!pEmptyCursor->m_bStoreMore){
                bUserCache = true;
            }else{
                if (cursor.m_asc){
                    ret = keyStoreCmpLess(cursor.m_cacheCursor->m_szCacheKey,
                        cursor.m_cacheCursor->m_unCacheKeyLen,
                        pEmptyCursor->m_szStoreKey,
                        pEmptyCursor->m_unStoreKeyLen);
                    if (ret < 0){
                        bUserCache = true;
                    }else if (0 == ret){
                        bUserCache = true;
                        pEmptyCursor->m_bStoreValue = false; ///store中的值作废，因为重复
                    }
                }else{
                    ret = keyStoreCmpLess(cursor.m_cacheCursor->m_szCacheKey,
                        cursor.m_cacheCursor->m_unCacheKeyLen,
                        pEmptyCursor->m_szStoreKey,
                        pEmptyCursor->m_unStoreKeyLen);
                    if (ret > 0){
                        bUserCache = true;
                    }else if (0 == ret){
                        bUserCache = true;
                        pEmptyCursor->m_bStoreValue = false;///store中的值作废，因为重复
                    }
                }
            }
        }
        ///检查是否超时
        if (bUserCache){
            if (cursor.m_cacheCursor->m_bCacheDel){
                cursor.m_cacheCursor->m_bCacheValue = false;
                continue; ///继续查找
            }
            getKvVersion(cursor.m_cacheCursor->m_szCacheData, cursor.m_cacheCursor->m_uiCacheDataLen, ttExpire, uiVersion);
            if (m_config->getCommon().m_bEnableExpire && (ttExpire<=m_ttExpireClock)){
                cursor.m_cacheCursor->m_bCacheValue = false;
                continue;
            }
        }else{
            getKvVersion(pEmptyCursor->m_szStoreData, pEmptyCursor->m_uiStoreDataLen, ttExpire, uiVersion);
            if (m_config->getCommon().m_bEnableExpire && (ttExpire<=m_ttExpireClock)){
                pEmptyCursor->m_bStoreValue = false;
                continue;
            }
        }
        break;
    }while(1);
    if (bUserCache){
        cursor.m_cacheCursor->m_bCacheValue = false;
        szKey = cursor.m_cacheCursor->m_szCacheKey;
        unKeyLen = cursor.m_cacheCursor->m_unCacheKeyLen;
        bKeyValue = isKvData(cursor.m_cacheCursor->m_szCacheData, cursor.m_cacheCursor->m_uiCacheDataLen);
        uiDataLen = cursor.m_cacheCursor->m_uiCacheDataLen - getKvDataSignLen();
        if (bKeyInfo){
            uiDataLen = sprintf((char*)szData, "%u,%u,%u,%u", ttExpire, uiVersion, uiDataLen, bKeyValue?1:0);
            bKeyValue = false;
        }else if (!cursor.m_field){
            szData = cursor.m_cacheCursor->m_szCacheData;
        }else if(!bKeyValue){
            szData="";
            uiDataLen = 0;
            bKeyValue = false;
        }else{
            ret = UnistorStoreBase::pickField(*tss->m_pEngineReader,
                *tss->m_pEngineWriter,
                cursor.m_field,
                cursor.m_cacheCursor->m_szCacheData,
                uiDataLen,
                tss->m_szBuf2K);
            if (ret == UNISTOR_ERR_SUCCESS){
                uiDataLen = tss->m_pEngineWriter->getMsgSize();
                szData = tss->m_pEngineWriter->getMsg();
                bKeyValue = true;
            }else{
                szData="";
                uiDataLen = 0;
                bKeyValue = true;
            }
        }
    }else{
        pEmptyCursor->m_bStoreValue = false;
        szKey=pEmptyCursor->m_szStoreKey;
        unKeyLen = pEmptyCursor->m_unStoreKeyLen;
        bKeyValue = isKvData(pEmptyCursor->m_szStoreData, pEmptyCursor->m_uiStoreDataLen);
        uiDataLen =pEmptyCursor->m_uiStoreDataLen-getKvDataSignLen();
        if (bKeyInfo){
            uiDataLen = sprintf((char*)szData,"%u,%u,%u,%u", ttExpire, uiVersion, uiDataLen, bKeyValue?1:0);
            bKeyValue = false;
        }else if (!cursor.m_field){
            szData=pEmptyCursor->m_szStoreData;
        }else if(!bKeyValue){
            szData="";
            uiDataLen = 0;
            bKeyValue = true;
        }else{
            ret = UnistorStoreBase::pickField(*tss->m_pEngineReader,
                *tss->m_pEngineWriter,
                cursor.m_field,
                pEmptyCursor->m_szStoreData,
                uiDataLen,
                tss->m_szBuf2K);
            if (ret == UNISTOR_ERR_SUCCESS){
                uiDataLen = tss->m_pEngineWriter->getMsgSize();
                szData = tss->m_pEngineWriter->getMsg();
                bKeyValue = true;
            }else{
                szData="";
                uiDataLen = 0;
                bKeyValue = true;
            }
        }
    }
    ((char*)szKey)[unKeyLen]=0x00;
    return 1;
}
///获取数据。-1：失败；0：结束；1：获取一个
int UnistorStoreEmpty::_nextEmpty(UnistorStoreCursor& cursor, char* ){
    UnistorStoreEmptyCursor* pCursor = (UnistorStoreEmptyCursor*)cursor.m_cursorHandle;
    pCursor = NULL;
    return 0;
}


///消息相关的event处理；0：成功；-1：失败
int UnistorStoreEmpty::storeEvent(UnistorTss* tss, CwxMsgBlock*& msg){
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (EVENT_STORE_COMMIT == msg->event().getEvent()){
        return _dealCommitEvent(tss, msg);
    }else if (EVENT_STORE_DEL_EXPIRE == msg->event().getEvent()){
        return _dealExpireEvent(tss, msg);
    }else if (EVENT_STORE_DEL_EXPIRE_REPLY == msg->event().getEvent()){
        return _dealExpireReplyEvent(tss, msg);
    }
    m_bValid = false;
    ///未知的消息类型
    CwxCommon::snprintf(m_szErrMsg, 2047, "Unknown event type:%u", msg->event().getEvent());
    strcpy(tss->m_szBuf2K, m_szErrMsg);
    return -1;
}


///关闭游标
void UnistorStoreEmpty::closeCursor(UnistorStoreCursor& cursor){
	UnistorStoreEmptyCursor* pCursor = (UnistorStoreEmptyCursor*) cursor.m_cursorHandle;
    if (pCursor){
        delete pCursor;
        cursor.m_cursorHandle = NULL;
    }
    if (cursor.m_cacheCursor){
        delete cursor.m_cacheCursor;
        cursor.m_cacheCursor = NULL;
    }
    if (cursor.m_field){
        UnistorStoreBase::freeField(cursor.m_field);
        cursor.m_field = NULL;
    }
}

///开始导出数据。-1：内部错误失败；0：成功
int UnistorStoreEmpty::exportBegin(UnistorStoreCursor& cursor,
                                 char const* ,
                                 char const* ,
                                 UnistorSubscribe const& scribe,
                                 CWX_UINT64& ullSid,
                                 char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    exportEnd(cursor);
    ///创建cache cursor
    cursor.m_cacheCursor = new UnistorStoreCacheCursor();

    UnistorStoreEmptyCursor* pCursor =  new UnistorStoreEmptyCursor();
    //TODO:创建存储引擎的cursor
    cursor.m_scribe = scribe;
    if (!cursor.m_scribe.m_bAll &&
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_MOD) &&
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_RANGE) &&
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_KEY))
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Invalid scribe mode:%u", cursor.m_scribe.m_uiMode);
        delete pCursor;
        return -1;
    }
    pCursor->m_bFirst = true;
    cursor.m_cursorHandle = pCursor;
    ullSid = getCache()->getPrevCommitSid();
    return 0;

}
///获取数据。-1：失败；0：结束；1：获取一个；2：skip数量为0
int UnistorStoreEmpty::exportNext(UnistorTss* tss,
                                UnistorStoreCursor& cursor,
                                char const*& szKey,
                                CWX_UINT16& unKeyLen,
                                char const*& szData,
                                CWX_UINT32& uiDataLen,
                                bool& bKeyValue,
                                CWX_UINT32& uiVersion,
                                CWX_UINT32& uiExpire,
                                CWX_UINT16& unSkipNum,
                                char const*& szExtra,
                                CWX_UINT32& uiExtraLen)
{
    int ret = 0;
    szExtra = NULL;
    uiExtraLen = 0;
    if (cursor.m_scribe.m_bAll || 
        (cursor.m_scribe.m_uiMode == UnistorSubscribe::SUBSCRIBE_MODE_MOD) ||
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_RANGE))
    {
        ret = _exportNext(tss, cursor, szKey, unKeyLen, szData, uiDataLen, bKeyValue, uiVersion, uiExpire, unSkipNum);
    }else if (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_KEY){
        ret = _exportKeyNext(tss, cursor, szKey, unKeyLen, szData, uiDataLen, bKeyValue, uiVersion, uiExpire, unSkipNum);
    }else{
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Invalid scribe mode:%u", cursor.m_scribe.m_uiMode);
        return -1;
    }
    return ret;
}

///结束导出数据
void UnistorStoreEmpty::exportEnd(UnistorStoreCursor& cursor){
    UnistorStoreEmptyCursor* pCursor = (UnistorStoreEmptyCursor*) cursor.m_cursorHandle;
    if (pCursor){
        delete pCursor;
        cursor.m_cursorHandle = NULL;
    }
    if (cursor.m_cacheCursor){
        delete cursor.m_cacheCursor;
        cursor.m_cacheCursor = NULL;
    }
}

///检查订阅格式是否合法
bool UnistorStoreEmpty::isValidSubscribe(UnistorSubscribe const& subscribe,///<订阅对象
                                       char* szErr2K ///<不合法时的错误消息
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

int UnistorStoreEmpty::commit(char* szErr2K){
    CWX_INFO(("Begin commit write cache........"));
    int ret =  _commit(szErr2K);
    CWX_INFO(("End commit write cache........"));
    return ret;

}


///关闭，0：成功；-1：失败
int UnistorStoreEmpty::close(){
    if (getCache())getCache()->stop();

    if (m_exKey){
        for (CWX_UINT32 i=0; i<UNISTOR_PER_FETCH_EXPIRE_KEY_NUM; i++){
            if (m_exKey[i].second) free(m_exKey[i].second);
        }
        delete [] m_exKey;
        m_exKey = NULL;
    }
    m_unExKeyNum = 0;
    m_unExKeyPos = 0;
    if (m_exFreeMsg.begin() != m_exFreeMsg.end()){
        list<CwxMsgBlock*>::iterator iter = m_exFreeMsg.begin();
        while(iter != m_exFreeMsg.end()){
            CwxMsgBlockAlloc::free(*iter);
            iter++;
        }
        m_exFreeMsg.clear();
    }
	return UnistorStoreBase::close();
}

///checkpoint
void UnistorStoreEmpty::checkpoint(UnistorTss* )
{
}

///commit。0：成功；-1：失败
int UnistorStoreEmpty::_commit(char* szErr2K){
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    if (0 != getCache()->commit(m_ullStoreSid, NULL, m_szErrMsg)){
        m_bValid = false;
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    m_uiUncommitBinlogNum = 0;
    m_uiLastCommitSecond = m_ttExpireClock;
    ///给checkpoint线程发送commit消息
    if (m_pMsgPipeFunc){
        CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
        msg->event().setEvent(EVENT_STORE_COMMIT);
        if (0 != m_pMsgPipeFunc(m_pApp, msg, false, szErr2K)){
            CwxMsgBlockAlloc::free(msg);
        }
    }
    return 0;
}

//0:成功；-1：失败
int UnistorStoreEmpty::_updateSysInfo(CWX_UINT64 ullSid, char* szErr2K){
	if (!m_bValid){
		if (szErr2K) strcpy(szErr2K, m_szErrMsg);
		return -1;
	}
	//sid
	char szSid[64];
	char szKey[128];
	CwxCommon::toString(ullSid, szSid, 10);
    CWX_INFO(("Set empty sid valus is %s", szSid));
    strcpy(szKey, UNISTOR_KEY_SID);
    if (0 != _setEmptyKey(szKey, strlen(szKey), 128, szSid, strlen(szSid), szErr2K)){
        return -1;
    }
    m_ullStoreSid = ullSid;
	return 0;
}


//获取系统key。1：成功；0：不存在；-1：失败;
int UnistorStoreEmpty::_getSysKey(UnistorTss* , ///<线程tss对象
                                  char const* key, ///<要获取的key
                                  CWX_UINT16 unKeyLen, ///<key的长度
                                  char* szData, ///<若存在，则返回数据。内存有存储引擎分配
                                  CWX_UINT32& uiLen  ///<szData数据的字节数
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

//0:成功；-1：成功
int UnistorStoreEmpty::_loadSysInfo(char* ){
    ///获取UNISTOR_KEY_SID，回退1000万的binlog
    m_ullStoreSid = m_binLogMgr->getMaxSid()>10000000?m_binLogMgr->getMaxSid()-10000000:0;
    return 0;
}

//0:成功；-1：失败
int UnistorStoreEmpty::_setKey(char const* szKey,
                               CWX_UINT16 unKeyLen,
                               char const* szData,
                               CWX_UINT32 uiLen,
                               CWX_UINT32 ttOldExpire,
                               bool& bWriteCache,
                               bool bCache,
                               char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = getCache()->updateKey(szKey, unKeyLen, szData, uiLen, ttOldExpire, bCache, bWriteCache);
    if (0 == ret){
        if (-1 == _commit(szErr2K)) return -1;
        ret = getCache()->updateKey(szKey, unKeyLen, szData, uiLen, ttOldExpire, bCache, bWriteCache);
    };
    if (-1 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, getCache()->getErrMsg());
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }else if (0 == ret){
        m_bValid = true;
        strcpy(m_szErrMsg, "UnistorCache::updateKey can't return 0 after commit");
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }else if (-2 == ret){
        CWX_ASSERT(0);
    }
    m_uiUncommitBinlogNum++;
    if (isNeedCommit()){
        if (0 != _commit(szErr2K)) return -1;
    }
    return 0;
}


//0:不存在；1：获取；-1：失败
int UnistorStoreEmpty::_getKey(char const* szKey,
                               CWX_UINT16 unKeyLen,
                               char* szData,
                               CWX_UINT32& uiLen,
                               char* szStoreKeyBuf,
                               CWX_UINT16 unKeyBufLen,
                               bool& isCached,
                               bool bCache,
                               char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = 0;
    bool bDel = false;
    ret = getCache()->getKey(szKey, unKeyLen, szData, uiLen, bDel, isCached);
    if (-1 == ret){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Data buf size[%u] is too small.", uiLen);
        return -1;
    }else if (1 == ret){
        isCached = true;
        return bDel?0:1;
    }
    isCached = false;
    ret =  _getEmptyKey(szKey, unKeyLen, szData, uiLen, szStoreKeyBuf, unKeyBufLen, szErr2K);
    //cache数据
    if ((1 == ret) && bCache){
        getCache()->cacheKey(szKey, unKeyLen, szData, uiLen, true);
    }
    return ret;
}


//0:成功；-1：失败
int UnistorStoreEmpty::_delKey(char const* szKey,
                             CWX_UINT16 unKeyLen,
                             CWX_UINT32 ttOldExpire,
                             bool& bWriteCache,
                             char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = getCache()->delKey(szKey, unKeyLen, ttOldExpire, bWriteCache);
    if (0 == ret){
        if (-1 == _commit(szErr2K)) return -1;
        ret = getCache()->delKey(szKey, unKeyLen, ttOldExpire, bWriteCache);
    };
    if (-1 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, getCache()->getErrMsg());
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }else if (0 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, "UnistorCache::updateKey can't return 0 after commit");
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }
    m_uiUncommitBinlogNum++;
    if (isNeedCommit()){
        if (0 != _commit(szErr2K)) return -1;
    }
    return 0;
}


//0:成功；-1：失败
int UnistorStoreEmpty::_setEmptyKey(char const* ,
                                CWX_UINT16 ,
                                CWX_UINT16 ,
                                char const* ,
                                CWX_UINT32 ,
                                char* szErr2K)
{
	if (!m_bValid){
		if (szErr2K) strcpy(szErr2K, m_szErrMsg);
		return -1;
	}
	return 0;
}


//0:不存在；1：获取；-1：失败
int UnistorStoreEmpty::_getEmptyKey(char const* ,
                                CWX_UINT16 ,
                                char* ,
                                CWX_UINT32& ,
                                char* ,
                                CWX_UINT16 ,
                                char* szErr2K)
{
	if (!m_bValid){
		if (szErr2K) strcpy(szErr2K, m_szErrMsg);
		return -1;
	}
	return 0;
}

//0:成功；-1：失败
int UnistorStoreEmpty::_delEmptyKey(char const* ,
                                CWX_UINT16 ,
                                CWX_UINT16 ,
                                char* szErr2K)
{
	if (!m_bValid){
		if (szErr2K) strcpy(szErr2K, m_szErrMsg);
		return -1;
	}
    return 0;
}


//处理commit事件。0：成功；-1：失败
int UnistorStoreEmpty::_dealCommitEvent(UnistorTss* tss, CwxMsgBlock*& )
{
    CWX_UINT32 i=0;
    if (!m_config->getCommon().m_bEnableExpire) return 0; ///<无需检测超时
    if (m_unExKeyPos < m_unExKeyNum) return 0; ///<还有未处理完的超时
    if (!m_exKey){
        m_exKey = new pair<CWX_UINT32, UnistorStoreExpireKey*>[UNISTOR_PER_FETCH_EXPIRE_KEY_NUM];
        for (i=0; i<UNISTOR_PER_FETCH_EXPIRE_KEY_NUM; i++){
            m_exKey[i].second = (UnistorStoreExpireKey*)malloc(sizeof(UnistorStoreExpireKey) + UNISTOR_MAX_KEY_SIZE);
            m_exKey[i].first = 0;
        }
        m_unExKeyNum = 0;
        for (i=0; i<m_config->getCommon().m_uiExpireConcurrent; i++){
            m_exFreeMsg.push_back(CwxMsgBlockAlloc::malloc(UNISTOR_MAX_KEY_SIZE));
        }
    }
    int ret =_loadExpireData(tss, false); 
    if (-1 == ret) return -1;
    if (0 == ret) return 0;
    return _sendExpireData(tss);
}

//加载超时的数据。0：没有新数据；1：获取了数据；-1：失败
int UnistorStoreEmpty::_loadExpireData(UnistorTss* , bool bJustContinue){
    if (bJustContinue){///不重新加载
        if (m_unExKeyNum < UNISTOR_PER_FETCH_EXPIRE_KEY_NUM) return 0;
    }
    CWX_INFO(("End load expired key.........."));
    return 0;
}

int UnistorStoreEmpty::_sendExpireData(UnistorTss* tss){
    int iRet = 0;
    CwxMsgBlock* msg=NULL;
    while(m_exFreeMsg.begin() != m_exFreeMsg.end()){
        if (m_unExKeyPos >= m_unExKeyNum){
            iRet = _loadExpireData(tss, true);
            if (-1 == iRet) return -1;
            if (0 == iRet) break;
        }
        msg = *m_exFreeMsg.begin();
        msg->reset();
        memcpy(msg->wr_ptr(), m_exKey[m_unExKeyPos].second->m_key, m_exKey[m_unExKeyPos].first);
        msg->wr_ptr()[m_exKey[m_unExKeyPos].first]=0x00;
        msg->wr_ptr(m_exKey[m_unExKeyPos].first);
        msg->event().setEvent(EVENT_STORE_DEL_EXPIRE);
        msg->event().setTimestamp(m_exKey[m_unExKeyPos].second->m_ttExpire);
        if (0 != m_pMsgPipeFunc(m_pApp, msg, true, tss->m_szBuf2K)){
            m_bValid = false;
            CWX_ERROR(("Failure to send expired key to write thread. err=%s", tss->m_szBuf2K));
            strcpy(m_szErrMsg, tss->m_szBuf2K);
            return -1;
        }
        m_exFreeMsg.pop_front();
        m_unExKeyPos++;
    }
    return 0;
}

//处理expire事件。0：成功；-1：失败
int UnistorStoreEmpty::_dealExpireEvent(UnistorTss* tss, CwxMsgBlock*& msg)
{
    CWX_UINT32 uiVersion = 0;
    CWX_UINT32 ttOldExpire = 0;
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    bool bReadCache = false;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(msg->rd_ptr(), msg->length(), szBuf, uiBufLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCache, false, tss->m_szBuf2K);
    if (-1 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        CWX_ERROR(("Failure to get expire key, err:%s", tss->m_szBuf2K));
        return -1;
    }
    if (1 == ret){
        getKvVersion(szBuf, uiBufLen, ttOldExpire, uiVersion);
        if (ttOldExpire == msg->event().getTimestamp()){///同一个key
            if (0 != _delKey(msg->rd_ptr(), msg->length(), ttOldExpire, bReadCache, tss->m_szBuf2K)) return -1;
        }
    }
    msg->event().setEvent(EVENT_STORE_DEL_EXPIRE_REPLY);
    if (0 != m_pMsgPipeFunc(m_pApp, msg, false, tss->m_szBuf2K)){
        m_bValid = false;
        CWX_ERROR(("Failure to send expired key to write thread. err=%s", tss->m_szBuf2K));
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    msg = NULL;
    return 0;
}

//处理expire事件的回复。0：成功；-1：失败
int UnistorStoreEmpty::_dealExpireReplyEvent(UnistorTss* tss, CwxMsgBlock*& msg){
    m_exFreeMsg.push_back(msg);
    msg = NULL;
    return _sendExpireData(tss);
}

//导出mod模式的订阅。-1：失败；0：结束；1：获取一个；2：skip数量为0
int UnistorStoreEmpty::_exportNext(UnistorTss* ,
                                 UnistorStoreCursor& cursor,
                                 char const*& ,
                                 CWX_UINT16& ,
                                 char const*& ,
                                 CWX_UINT32& ,
                                 bool& ,
                                 CWX_UINT32& ,
                                 CWX_UINT32& ,
                                 CWX_UINT16& )
{
    UnistorStoreEmptyCursor* pCursor = (UnistorStoreEmptyCursor*)cursor.m_cursorHandle;
    pCursor = NULL;
    return 0;
}


//导出key模式的订阅。-1：失败；0：结束；1：获取一个；2：skip数量为0
int UnistorStoreEmpty::_exportKeyNext(UnistorTss* ,
                                    UnistorStoreCursor& cursor,
                                    char const*& ,
                                    CWX_UINT16& ,
                                    char const*& ,
                                    CWX_UINT32& ,
                                    bool& ,
                                    CWX_UINT32& ,
                                    CWX_UINT32& ,
                                    CWX_UINT16& )
{
    UnistorStoreEmptyCursor* pCursor = (UnistorStoreEmptyCursor*)cursor.m_cursorHandle;
    pCursor = NULL;
    return 0;
}

bool UnistorStoreEmpty::_exportKeyInit(string const& strKeyBegin,
                                     string& strBegin,
                                     string& strEnd,
                                     UnistorSubscribeKey const& keys)
{
    map<string,string>::const_iterator iter =  keys.m_keys.begin();
    while(iter != keys.m_keys.end()){
        if ((keyStoreCmpLess(strKeyBegin.c_str(), strKeyBegin.size(), iter->second.c_str(), iter->second.size())<0)
            || (!iter->second.size()))
        {
            ///设置begin
            if (keyStoreCmpLess(strKeyBegin.c_str(), strKeyBegin.size(), iter->first.c_str(), iter->first.size()) <= 0){
                strBegin.assign(iter->first.c_str(), iter->first.size());
            }else{
                strBegin.assign(strKeyBegin.c_str(), strKeyBegin.size());
            }
            ///设置end
            strEnd.assign(iter->second.c_str(), iter->second.size());
            return true;
        }
        iter++;
    }
    return false;
}
