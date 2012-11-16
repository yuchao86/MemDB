#include "UnistorStoreBase.h"

UNISTOR_KEY_GROUP_FN UnistorStoreBase::m_fnKeyStoreGroup=NULL; 
UNISTOR_KEY_GROUP_FN UnistorStoreBase::m_fnKeyAsciiGroup=NULL;
UNISTOR_KEY_CMP_LESS_FN  UnistorStoreBase::m_fnKeyAsciiLess=NULL;

UnistorStoreCursor::~UnistorStoreCursor(){
    if (m_cacheCursor){
        delete m_cacheCursor;
        m_cacheCursor = NULL;
    }
    if (m_field){
        UnistorStoreBase::freeField(m_field);
        m_field = NULL;
    }
}
//加载配置文件.-1:failure, 0:success
int UnistorStoreBase::init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc,
                           UNISTOR_GET_SYS_INFO_FN getSysInfoFunc,
                           void* pApp,
                           UnistorConfig const* config)
{
    m_pMsgPipeFunc =  msgPipeFunc;
    m_pGetSysInfoFunc = getSysInfoFunc;
    m_pApp = pApp;
    if (0 != close()) return -1;
    m_config = config;
    m_uiDefExpire = m_config->getCommon().m_uiDefExpire;
    //创建binlog
    CWX_UINT64 ullBinLogSize = m_config->getBinlog().m_uiBinLogMSize;
    ullBinLogSize *= 1024 * 1024;

    m_binLogMgr = new CwxBinLogMgr(m_config->getBinlog().m_strBinlogPath.c_str(),
        m_config->getBinlog().m_strBinlogPrex.c_str(),
        ullBinLogSize,
        m_config->getBinlog().m_uiFlushNum,
        m_config->getBinlog().m_uiFlushSecond,
        m_config->getBinlog().m_bDelOutdayLogFile);
    if (0 != m_binLogMgr->init(m_config->getBinlog().m_uiMgrFileNum,
        m_config->getBinlog().m_bCache, m_szErrMsg))
    {
        m_bValid = false;
        CWX_ERROR(("Failure to init binlog, errno=%s", m_szErrMsg));
        return -1;
    }
    if (m_cache) delete (m_cache);
    m_cache = NULL;
    return 0;
}

///启动cache
int UnistorStoreBase::startCache(CWX_UINT32 uiWriteCacheMBtye, ///<write cache的大小，若为0表示没有write cache
               CWX_UINT32 uiReadCacheMByte, ///<读cache的大小，若为0表示没有read cache
               CWX_UINT32 uiReadMaxCacheKeyNum, ///<读cache的最大cache条目
               UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite, ///<写的开始函数
               UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite, ///<写函数
               UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite, ///<写结束函数
               void*     context, ///<函数的context
               UNISTOR_KEY_CMP_EQUAL_FN  fnEqual,
               UNISTOR_KEY_CMP_LESS_FN   fnLess,
               UNISTOR_KEY_HASH_FN       fnHash,
               float     fBucketRate, ///<read cache's bucket rate
               char*     szErr2K
               )
{
    if (m_cache) delete (m_cache);
    m_cache = NULL;
    //初始化cache
    m_cache = new UnistorCache(uiWriteCacheMBtye,
        uiReadCacheMByte,
        uiReadMaxCacheKeyNum,
        fnEqual,
        fnLess,
        fnHash);
    if (0 != m_cache->init(fnBeginWrite,
        fnWrite,
        fnEndWrite,
        context,
        fBucketRate,
        szErr2K))
    {
        CWX_ERROR(("Failure to init UnistorCache. err=%s", szErr2K?szErr2K:""));
        return -1;
    }
    return 0;
}

///获取数据。-1：失败；0：结束；1：获取一个
int UnistorStoreBase::nextCache(UnistorStoreCursor& cursor, char* szErr2K){
    int ret = 0;
    bool bDel = false;
    CWX_UINT16 unKeyLen = 0;
    do{
        do{
            if (cursor.m_bBegin && cursor.m_unBeginKeyLen && cursor.m_cacheCursor->m_bCacheFirst){///如果获取第一个
                cursor.m_cacheCursor->m_bCacheFirst = false;
                cursor.m_cacheCursor->m_uiCacheDataLen = UNISTOR_MAX_KV_SIZE;
                ret = getCache()->getWriteKey(cursor.m_beginKey,
                    cursor.m_unBeginKeyLen,
                    cursor.m_cacheCursor->m_szCacheData,
                    cursor.m_cacheCursor->m_uiCacheDataLen,
                    bDel);
                if (1 == ret){
                    ///设置key
                    cursor.m_cacheCursor->m_unCacheKeyLen = cursor.m_unBeginKeyLen; 
                    memcpy(cursor.m_cacheCursor->m_szCacheKey, cursor.m_beginKey, cursor.m_cacheCursor->m_unCacheKeyLen);
                    cursor.m_cacheCursor->m_szCacheKey[cursor.m_cacheCursor->m_unCacheKeyLen] = 0x00;
                    break;
                }else if (-1 == ret){
                    if (szErr2K) CwxCommon::snprintf(szErr2K, 2048, "Buf size[%u] is too small.", UNISTOR_MAX_KV_SIZE);
                    return -1;
                }
                cursor.m_cacheCursor->m_unCacheKeyLen = cursor.m_unBeginKeyLen;
                memcpy(cursor.m_cacheCursor->m_szCacheKey, cursor.m_beginKey, cursor.m_unBeginKeyLen);
            }else if(cursor.m_cacheCursor->m_bCacheFirst){
                cursor.m_cacheCursor->m_bCacheFirst = false;
                if (cursor.m_unBeginKeyLen){
                    cursor.m_cacheCursor->m_unCacheKeyLen = cursor.m_unBeginKeyLen;
                    memcpy(cursor.m_cacheCursor->m_szCacheKey, cursor.m_beginKey, cursor.m_unBeginKeyLen);
                }else{
                    cursor.m_cacheCursor->m_szCacheKey[0]=0x00;
                    cursor.m_cacheCursor->m_unCacheKeyLen = 0;
                }
            }
            ///此时，或者是第一次获取，当前key不存在或者取下一个
            unKeyLen = UNISTOR_MAX_KEY_SIZE;
            cursor.m_cacheCursor->m_uiCacheDataLen = UNISTOR_MAX_KV_SIZE;
            ret = getCache()->nextWriteKey(cursor.m_cacheCursor->m_szCacheKey,
                cursor.m_cacheCursor->m_unCacheKeyLen,
                cursor.m_asc,
                cursor.m_cacheCursor->m_szCacheKey,
                unKeyLen,
                cursor.m_cacheCursor->m_szCacheData,
                cursor.m_cacheCursor->m_uiCacheDataLen,
                bDel);
            if (0 == ret) return 0;
            if (-1 == ret){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2048, "Buf size[%u] is too small.", cursor.m_cacheCursor->m_uiCacheDataLen);
                return -1;
            }
            cursor.m_cacheCursor->m_unCacheKeyLen = unKeyLen;
            break;
        }while(1);
        cursor.m_cacheCursor->m_bCacheDel = bDel;
        if (cursor.m_unEndKeyLen){
            int ret = m_fnKeyAsciiLess(cursor.m_cacheCursor->m_szCacheKey, cursor.m_cacheCursor->m_unCacheKeyLen, cursor.m_endKey, cursor.m_unEndKeyLen);
            if (0 == ret) return 0;
            if (cursor.m_asc){
                if (0<ret) return 0;
            }else{
                if (0>ret) return 0;
            }
        }
        return 1;
    }while(1);
    return 1;
}

///关闭，0：成功；-1：失败
int UnistorStoreBase::close(){
    if (m_binLogMgr){
        delete m_binLogMgr;
        m_binLogMgr = NULL;
    }
    m_config = NULL; ///<系统配置
    m_bValid = false;
    strcpy(m_szErrMsg, "No init.");
    return 0;
}

///从binlog中恢复数据。0：成功；-1：失败
int UnistorStoreBase::restore(CWX_UINT64 ullSid){
    CWX_ASSERT(m_bValid);
    int ret = 0;
    CwxBinLogCursor* cursor = getBinLogMgr()->createCurser(ullSid);
    UnistorTss* pTss = new UnistorTss();
    pTss->init();
    CWX_UINT32 uiDataLen=0;
    char* pBuf = NULL;
    bool bSeek = false;

    CwxKeyValueItemEx item;
    CwxKeyValueItemEx const* pItem = NULL;
    CWX_UINT32 uiType = 0;
    CWX_UINT32 uiVersion = 0;
    CWX_UINT32 uiRestoreNum = 0;

    CWX_INFO(("Begin restore store from binlog, start sid=%s", CwxCommon::toString(ullSid, pTss->m_szBuf2K, 10)));
    while(true){
        if (bSeek){
            ret = getBinLogMgr()->next(cursor);
        }else{
            ret = getBinLogMgr()->seek(cursor, ullSid);
            bSeek = true;
        }
        if (-1 == ret){
            m_bValid = false;
            strcpy(m_szErrMsg, cursor->getErrMsg());
            CWX_ERROR(("Failure to seek cursor, err=%s", m_szErrMsg));
            break;
        }else if (0 == ret){
            break;
        }

        //fetch
        uiDataLen = cursor->getHeader().getLogLen();
        ///准备data读取的buf
        pBuf = getBuf(uiDataLen);        
        ///读取data
        ret = getBinLogMgr()->fetch(cursor, pBuf, uiDataLen);
        if (-1 == ret){//读取失败
            m_bValid = false;
            strcpy(m_szErrMsg, cursor->getErrMsg());
            CWX_ERROR(("Failure to fetch data, err:%s", m_szErrMsg));
            break;
        }
        if (!pTss->m_pEngineReader->unpack(pBuf, uiDataLen, false, true))
        {
            CWX_ERROR(("Failure to parase binlog ,err:%s", pTss->m_pEngineReader->getErrMsg()));
            continue;
        }
        //get data
        if (!(pItem = pTss->m_pEngineReader->getKey(UNISTOR_KEY_D)))
        {
            CWX_ERROR(("Can't find key[%s] in binlog", UNISTOR_KEY_D));
            continue;
        }
        item.m_szData = pItem->m_szData;
        item.m_uiDataLen = pItem->m_uiDataLen;
        item.m_bKeyValue = pItem->m_bKeyValue;
        //get type
        if (!pTss->m_pEngineReader->getKey(UNISTOR_KEY_TYPE, uiType)){
            CWX_ERROR(("Can't find key[%s] in binlog", UNISTOR_KEY_TYPE));
            continue;
        }
        //get version
        if (!pTss->m_pEngineReader->getKey(UNISTOR_KEY_V, uiVersion)){
            CWX_ERROR(("Can't find key[%s] in binlog", UNISTOR_KEY_V));
            continue;
        }
        if (0 != syncMasterBinlog(pTss,
            pTss->m_pReader,
            cursor->getHeader().getSid(),
            cursor->getHeader().getDatetime(),
            cursor->getHeader().getGroup(),
            uiType,
            item,
            uiVersion,
            true))
        {
            CWX_ERROR(("Failure to restore binlog , type=%u  err=%s", uiType, pTss->m_szBuf2K));
        }
        uiRestoreNum++;
        if (uiRestoreNum % 10000 == 0){
            CWX_INFO(("Starting check point for restore record:%u ......", uiRestoreNum));
            checkpoint(pTss);
            CWX_INFO(("End check point............"));
            ///给父进程附送信号
            CWX_INFO(("send heatbeat to parent proc:%d", ::getppid()));
            if (1 == ::getppid()){
                CWX_INFO(("Exit for no parent."));
                break;
            }
            ::kill(::getppid(), SIGHUP);
        }
    }

    if (!m_bValid){
        strcpy(m_szErrMsg, pTss->m_szBuf2K);
        CWX_ERROR((m_szErrMsg));
    }else{
        CWX_INFO(("End restore store from binlog, end sid=%s", CwxCommon::toString(cursor->getHeader().getSid()==0?ullSid:cursor->getHeader().getSid(), pTss->m_szBuf2K, 10)));
    }
    delete pTss;
    getBinLogMgr()->destoryCurser(cursor);
    if (m_bValid){ ///double commit保证数据全部写入
        //commit 数据
        if (0 != commit(NULL)){
            m_bValid = false;
            return -1;
        }
        //commit 数据
        if (0 != commit(NULL)){
            m_bValid = false;
            return -1;
        }
    }
    return m_bValid?0:-1;
}

///同步master的binlog.0：成功；-1：失败
int UnistorStoreBase::syncMasterBinlog(UnistorTss* tss,
                                       CwxPackageReaderEx* reader,
                                       CWX_UINT64 ullSid,
                                       CWX_UINT32 ttTimestamp,
                                       CWX_UINT32 uiGroup,
                                       CWX_UINT32 uiType,
                                       CwxKeyValueItemEx const& value,
                                       CWX_UINT32 uiVersion,
                                       bool bRestore)
{
    int ret = 0;
    CwxKeyValueItemEx const* key=NULL;
    CwxKeyValueItemEx const* field = NULL;
    CwxKeyValueItemEx const* extra = NULL;
    CWX_UINT32		 uiExpire = 0;
    CWX_INT64        llMax = 0;
    CWX_INT64        llMin = 0;
    CWX_INT64        llValue = 0;
    CWX_INT64        result = 0;
    CwxKeyValueItemEx const* data = NULL;
    CWX_UINT32       uiSign=0;
    char const*      szUser=NULL;
    char const*      szPasswd=NULL;
    CWX_INT64		 num=0;
    CWX_UINT32       uiOldVersion=0;
    bool             bCache=true;
    bool             bReadCached=false;
    bool             bWriteCached=false;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    bool bRet = reader->unpack(value.m_szData, value.m_uiDataLen, false);
    if (!bRet){
        strcpy(tss->m_szBuf2K, reader->getErrMsg());
        return -1;
    }
    ///无条件的写binlog
    if (!bRestore){
        tss->m_pEngineWriter->beginPack();
        if (!tss->m_pEngineWriter->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), value.m_szData, value.m_uiDataLen, true)){
            strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
            return -1;
        }
        if (!tss->m_pEngineWriter->addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
            strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
            return -1;
        }
        if (!tss->m_pEngineWriter->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            strcpy(tss->m_szBuf2K, tss->m_pEngineWriter->getErrMsg());
            return -1;
        }
        tss->m_pEngineWriter->pack();
        
        ///如果sid为0，属于从新编号sid，此看做是restore
        if (!ullSid) bRestore = true;

        if (0 != appendBinlog(ullSid,
            ttTimestamp,
            uiGroup,
            tss->m_pEngineWriter->getMsg(),
            tss->m_pEngineWriter->getMsgSize(),
            tss->m_szBuf2K))
        {
            m_bValid = false;
            return -1;
        }
    }

    switch(uiType){
    case UnistorPoco::MSG_TYPE_RECV_ADD:
        ret = UnistorPoco::parseRecvAdd(reader,
            key,
            field,
            extra,
            data,
            uiExpire,
            uiSign,
            uiOldVersion,
            bCache,
            szUser,
            szPasswd,
            tss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != ret){
            ret = -1;
            break;
        }
        ret = syncAddKey(tss,
            *key,
            field,
            extra,
            *data,
            uiSign,
            uiVersion,
            bCache,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
        tss->m_ullStatsAddNum++;
        if (-1 != ret){
            if (bReadCached) tss->m_ullStatsAddReadCacheNum++;
            if (bWriteCached) tss->m_ullStatsAddWriteCacheNum++;
        }
        if (1 == ret){
            ret = 0;
        }else if(0 == ret){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Not find key [%s]", key->m_szData);
            ret = -1;
        }else{
            ret = -1;
        }
        break;
    case UnistorPoco::MSG_TYPE_RECV_SET:
        ret = UnistorPoco::parseRecvSet(reader,
            key,
            field,
            extra,
            data,
            uiSign,
            uiExpire,
            uiOldVersion,
            bCache,
            szUser,
            szPasswd,
            tss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != ret){
            ret = -1;
            break;
        }
        ret = syncSetKey(tss,
            *key,
            field,
            extra,
            *data,
            uiSign,
            uiVersion,
            bCache,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
        tss->m_ullStatsSetNum++;
        if (-1 != ret){
            if (bReadCached) tss->m_ullStatsSetReadCacheNum++;
            if (bWriteCached) tss->m_ullStatsSetWriteCacheNum++;
        }
        if (1 == ret){
            ret = 0;
        }else if(0 == ret){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Not find key [%s]", key->m_szData);
            ret = -1;
        }else{
            ret = -1;
        }
        break;
    case UnistorPoco::MSG_TYPE_RECV_UPDATE:
        ret = UnistorPoco::parseRecvUpdate(reader,
            key,
            field,
            extra,
            data,
            uiSign,
            uiExpire,
            uiOldVersion,
            szUser,
            szPasswd,
            tss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != ret){
            ret = -1;
            break;
        }
        ret = syncUpdateKey(tss,
            *key,
            field,
            extra,
            *data,
            uiSign,
            uiVersion,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore); ///无条件更新
        tss->m_ullStatsUpdateNum++;
        if (-1 != ret){
            if (bReadCached) tss->m_ullStatsUpdateReadCacheNum++;
            if (bWriteCached) tss->m_ullStatsUpdateWriteCacheNum++;
        }
        if (1 == ret){
            ret = 0;
        }else if(0 == ret){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Not find key [%s]", key->m_szData);
            ret = -1;
        }else{
            ret = -1;
        }
        break;
    case UnistorPoco::MSG_TYPE_RECV_INC:
        ret = UnistorPoco::parseRecvInc(reader,
            key,
            field,
            extra,
            num,
            result,
            llMax,
            llMin,
            uiExpire,
            uiSign,
            uiOldVersion,
            szUser,
            szPasswd,
            tss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != ret){
            ret = -1;
            break;
        }
        ret = syncIncKey(tss,
            *key,
            field,
            extra,
            num,
            result,
            0,
            0,
            uiSign,
            llValue,
            uiVersion,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore); ///无条件更新
        tss->m_ullStatsIncNum++;
        if (-1 != ret){
            if (bReadCached) tss->m_ullStatsIncReadCacheNum++;
            if (bWriteCached) tss->m_ullStatsIncWriteCacheNum++;
        }
        if (1 == ret){
            ret = 0;
        }else if(0 == ret){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Not find key [%s]", key->m_szData);
            ret = -1;
        }else{
            ret = -1;
        }
        break;
    case UnistorPoco::MSG_TYPE_RECV_DEL:
        ret = UnistorPoco::parseRecvDel(reader,
            key,
            field,
            extra,
            uiOldVersion,
            szUser,
            szPasswd,
            tss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != ret){
            ret = -1;
            break;
        }
        ret = syncDelKey(tss,
            *key,
            field,
            extra,
            uiVersion,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore); ///无条件删除
        tss->m_ullStatsDelNum++;
        if (-1 != ret){
            if (bReadCached) tss->m_ullStatsDelReadCacheNum++;
            if (bWriteCached) tss->m_ullStatsDelWriteCacheNum++;
        }
        if (1 == ret){
            ret = 0;
        }else if(0 == ret){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Not find key [%s]", key->m_szData);
            ret = -1;
        }else{
            ret = -1;
        }
        break;
    case UnistorPoco::MSG_TYPE_TIMESTAMP:
        m_ttExpireClock = ttTimestamp; ///<设置超时的clock
        ret = 0;
        break;
    case UnistorPoco::MSG_TYPE_RECV_IMPORT:
        ret = UnistorPoco::parseRecvImport(reader,
            key,
            extra,
            data,
            uiExpire,
            uiOldVersion,
            bCache,
            szUser,
            szPasswd,
            tss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != ret){
            ret = -1;
            break;
        }
        ret = syncImportKey(tss,
            *key,
            extra,
            *data,
            uiVersion,
            bCache,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
        tss->m_ullStatsImportNum++;
        if (-1 != ret){
            if (bReadCached) tss->m_ullStatsImportReadCacheNum++;
            if (bWriteCached) tss->m_ullStatsImportWriteCacheNum++;
        }
        if (1 == ret){
            ret = 0;
        }else{
            ret = -1;
        }
        break;
    default:
        ret = -1;
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Unknown binlog type:%d", uiType);
        break;
    }
    return ret;
}

///添加Add key binlog.0：成功；-1：失败
int UnistorStoreBase::appendTimeStampBinlog(CwxPackageWriterEx& writer,
                                      CWX_UINT32 ttNow,
                                      char* szErr2K)
{
    writer.beginPack();
    if (!writer.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), "", 0, false))
    {
        if (szErr2K) strcpy(szErr2K, writer.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_TIMESTAMP;
    if (!writer.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiVersion = 0;
    if (!writer.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer.getErrMsg());
        return -1;
    }
    writer.pack();
    CWX_UINT64 ullSid = 0;
    int ret = appendBinlog(ullSid, ttNow, 0, writer.getMsg(), writer.getMsgSize(), szErr2K);
    if (0 == ret){
        m_ullStoreSid = ullSid;
    }
    return ret;
}


///添加Add key binlog.0：成功；-1：失败
int UnistorStoreBase::appendAddBinlog(CwxPackageWriterEx& writer,
                                      CwxPackageWriterEx& writer1,
                                      CWX_UINT32 uiGroup,
                                      CwxKeyValueItemEx const& key,
                                      CwxKeyValueItemEx const* field,
                                      CwxKeyValueItemEx const* extra,
                                      CwxKeyValueItemEx const& data,
                                      CWX_UINT32    uiExpire,
                                      CWX_UINT32    uiSign,
                                      CWX_UINT32    uiVersion,
                                      bool          bCache,
                                      char*			szErr2K)
{
    writer.beginPack();
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvAdd(&writer,
        key,
        field,
        extra,
        data,
        uiExpire,
        uiSign,
        0,
        bCache,
        NULL,
        NULL,
        szErr2K))
    {
        return -1;
    }
    writer.pack();
    writer1.beginPack();
    if (!writer1.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), writer.getMsg(), writer.getMsgSize(), true))
    {
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_RECV_ADD;
    if (!writer1.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    if (!writer1.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    writer1.pack();
    CWX_UINT64 ullSid = 0;
    return appendBinlog(ullSid, time(NULL), uiGroup, writer1.getMsg(), writer1.getMsgSize(), szErr2K);
}

///添加set key binlog.0：成功；-1：失败
int UnistorStoreBase::appendSetBinlog(CwxPackageWriterEx& writer,
                                      CwxPackageWriterEx& writer1,
                                      CWX_UINT32 uiGroup,
                                      CwxKeyValueItemEx const& key,
                                      CwxKeyValueItemEx const* field,
                                      CwxKeyValueItemEx const* extra,
                                      CwxKeyValueItemEx const& data,
                                      CWX_UINT32    uiExpire,
                                      CWX_UINT32    uiSign,
                                      CWX_UINT32    uiVersion,
                                      bool          bCache,
                                      char*			 szErr2K)
{
    writer.beginPack();
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvSet(&writer,
        key,
        field,
        extra,
        data,
        uiSign,
        uiExpire,
        0,
        bCache,
        NULL,
        NULL,
        szErr2K))
    {
        return -1;
    }
    writer.pack();
    writer1.beginPack();
    if (!writer1.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), writer.getMsg(), writer.getMsgSize(), true))
    {
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_RECV_SET;
    if (!writer1.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    if (!writer1.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    writer1.pack();
    CWX_UINT64 ullSid = 0;
    return appendBinlog(ullSid, time(NULL), uiGroup, writer1.getMsg(), writer1.getMsgSize(), szErr2K);
}

///添加update key binlog.0：成功；-1：失败
int UnistorStoreBase::appendUpdateBinlog(CwxPackageWriterEx& writer,
                                         CwxPackageWriterEx& writer1,
                                         CWX_UINT32 uiGroup,
                                         CwxKeyValueItemEx const& key,
                                         CwxKeyValueItemEx const* field,
                                         CwxKeyValueItemEx const* extra,
                                         CwxKeyValueItemEx const& data,
                                         CWX_UINT32       uiExpire,
                                         CWX_UINT32 uiSign,
                                         CWX_UINT32       uiVersion,
                                         char*			  szErr2K)
{
    writer.beginPack();
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvUpdate(&writer,
        key,
        field,
        extra,
        data,
        uiSign,
        uiExpire))
    {
        return -1;
    }
    writer.pack();
    writer1.beginPack();
    if (!writer1.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), writer.getMsg(), writer.getMsgSize(), true))
    {
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_RECV_UPDATE;
    if (!writer1.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    if (!writer1.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    writer1.pack();
    CWX_UINT64 ullSid = 0;
    return appendBinlog(ullSid, time(NULL), uiGroup, writer1.getMsg(), writer1.getMsgSize(), szErr2K);
}

///添加inc key binlog.0：成功；-1：失败
int UnistorStoreBase::appendIncBinlog(CwxPackageWriterEx& writer,
                                      CwxPackageWriterEx& writer1,
                                      CWX_UINT32 uiGroup,
                                      CwxKeyValueItemEx const& key,
                                      CwxKeyValueItemEx const* field,
                                      CwxKeyValueItemEx const* extra,
                                      CWX_INT64	       num,
                                      CWX_INT64        result,
                                      CWX_INT64        llMax,
                                      CWX_INT64        llMin,
                                      CWX_UINT32    uiExpire,
                                      CWX_UINT32       uiSign,
                                      CWX_UINT32       uiVersion,
                                      char*			 szErr2K)
{
    writer.beginPack();
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvInc(&writer,
        key,
        field,
        extra,
        num,
        result,
        llMax,
        llMin,
        uiExpire,
        uiSign))
    {
        return -1;
    }
    writer.pack();
    writer1.beginPack();
    if (!writer1.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), writer.getMsg(), writer.getMsgSize(), true))
    {
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_RECV_INC;
    if (!writer1.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    if (!writer1.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    writer1.pack();
    CWX_UINT64 ullSid = 0;
    return appendBinlog(ullSid, time(NULL),
        uiGroup,
        writer1.getMsg(),
        writer1.getMsgSize(),
        szErr2K);
}

///添加del key binlog.0：成功；-1：失败
int UnistorStoreBase::appendDelBinlog(CwxPackageWriterEx& writer,
                                      CwxPackageWriterEx& writer1,
                                      CWX_UINT32        uiGroup,
                                      CwxKeyValueItemEx const& key,
                                      CwxKeyValueItemEx const* field,
                                      CwxKeyValueItemEx const* extra,
                                      CWX_UINT32       uiVersion,
                                      char*			 szErr2K)
{
    writer.beginPack();
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvDel(&writer,
        key,
        field,
        extra))
    {
        return -1;
    }
    writer.pack();
    writer1.beginPack();
    if (!writer1.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), writer.getMsg(), writer.getMsgSize(), true))
    {
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_RECV_DEL;
    if (!writer1.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    if (!writer1.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    writer1.pack();
    CWX_UINT64 ullSid = 0;
    return appendBinlog(ullSid, time(NULL), uiGroup, writer1.getMsg(), writer1.getMsgSize(), szErr2K);
}

///添加import key binlog.0：成功；-1：失败
int UnistorStoreBase::appendImportBinlog(CwxPackageWriterEx& writer,
                                      CwxPackageWriterEx& writer1,
                                      CWX_UINT32 uiGroup,
                                      CwxKeyValueItemEx const& key,
                                      CwxKeyValueItemEx const* extra,
                                      CwxKeyValueItemEx const& data,
                                      CWX_UINT32    uiExpire,
                                      CWX_UINT32    uiVersion,
                                      bool          bCache,
                                      char*			 szErr2K)
{
    writer.beginPack();
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvImport(&writer,
        key,
        extra,
        data,
        uiExpire,
        0,
        bCache,
        NULL,
        NULL,
        szErr2K))
    {
        return -1;
    }
    writer.pack();
    writer1.beginPack();
    if (!writer1.addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), writer.getMsg(), writer.getMsgSize(), true))
    {
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    CWX_UINT32 uiType = UnistorPoco::MSG_TYPE_RECV_SET;
    if (!writer1.addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), uiType)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    if (!writer1.addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
        if (szErr2K) strcpy(szErr2K, writer1.getErrMsg());
        return -1;
    }
    writer1.pack();
    CWX_UINT64 ullSid = 0;
    return appendBinlog(ullSid, time(NULL), uiGroup, writer1.getMsg(), writer1.getMsgSize(), szErr2K);
}

///解析以[\n]分隔的多个字段
int UnistorStoreBase::parseMultiField(char const* szValue, UnistorKeyField*& field){
    int num = 0;
    if (szValue){
        UnistorKeyField* field_tmp = NULL;
        list<string> fields;
        CwxCommon::split(string(szValue), fields, '\n');
        list<string>::iterator iter = fields.begin();
        while(iter != fields.end()){
            CwxCommon::trim(*iter);
            if (iter->length()){
                if (!field_tmp){
                    num++;
                    field_tmp = field = new UnistorKeyField();
                }else{
                    num++;
                    field_tmp->m_next = new UnistorKeyField();
                    field_tmp = field_tmp->m_next;
                }
                field_tmp->m_key = *iter;
            }
            iter++;
        }
    }
    return num;
}


///释放key
void UnistorStoreBase::freeField(UnistorKeyField*& field){
    UnistorKeyField* next;
    while(field){
        next = field->m_next;
        delete field;
        field = next;
    }
    field = NULL;
}

///对add key的新、旧field进行归并。-1：失败；0：存在；1：成功
int UnistorStoreBase::mergeAddKeyField(CwxPackageWriterEx* writer1,
                                       CwxPackageReaderEx* reader1,
                                       CwxPackageReaderEx* reader2,
                                       char const* key,
                                       CwxKeyValueItemEx const* field,
                                       char const* szOldData,
                                       CWX_UINT32 uiOldDataLen,
                                       bool bOldKeyValue,
                                       char const* szNewData,
                                       CWX_UINT32 uiNewDataLen,
                                       bool bNewKeyValue,
                                       CWX_UINT32& uiFieldNum,
                                       char* szErr2K)
{
    CWX_UINT32 i=0;
    CwxKeyValueItemEx const* pItem=NULL;
    ///旧值一定是kv结构
    CWX_ASSERT(bOldKeyValue);
    uiFieldNum = 0;
    if (!reader1->unpack(szOldData, uiOldDataLen, false, true)){
        strcpy(szErr2K, reader1->getErrMsg());
        return -1;
    }
    if (field){
        writer1->beginPack();
        for (i=0; i<reader1->getKeyNum(); i++){
            pItem = reader1->getKey(i);
            if ((pItem->m_unKeyLen == field->m_uiDataLen) && (memcmp(pItem->m_szKey, field->m_szData, pItem->m_unKeyLen)==0)){
                CwxCommon::snprintf(szErr2K, 2047, "Key[%s]'s field [%s] exist.", key, field->m_szData);
                return 0;
            }
            if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen,  pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
        }
        if (!writer1->addKeyValue(field->m_szData, field->m_uiDataLen, szNewData, uiNewDataLen, bNewKeyValue)){
            strcpy(szErr2K, writer1->getErrMsg());
            return -1;
        }
        writer1->pack();
        uiFieldNum = reader1->getKeyNum() + 1;
        return 1;
    }
    CWX_ASSERT(bOldKeyValue);
    if (!reader2->unpack(szNewData, uiNewDataLen)){
        strcpy(szErr2K, reader2->getErrMsg());
        return -1;
    }
    writer1->beginPack();
    for (i=0; i<reader1->getKeyNum(); i++){
        pItem = reader1->getKey(i);
        if (reader2->getKey(pItem->m_szKey)){
            CwxCommon::snprintf(szErr2K, 2047, "Key[%s]'s field [%s] exist.", key, pItem->m_szKey);
            return 0;
        }
        if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
            strcpy(szErr2K, writer1->getErrMsg());
            return -1;
        }
    }
    for (i=0; i<reader2->getKeyNum(); i++){
        pItem = reader2->getKey(i);
        if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
            strcpy(szErr2K, writer1->getErrMsg());
            return -1;
        }
    }
    writer1->pack();
    uiFieldNum = reader1->getKeyNum() + reader2->getKeyNum();
    return 1;
}

int UnistorStoreBase::mergeSetKeyField(CwxPackageWriterEx* writer1,
                                       CwxPackageReaderEx* reader1,
                                       CwxPackageReaderEx* reader2,
                                       char const* ,
                                       CwxKeyValueItemEx const* field,
                                       char const* szOldData,
                                       CWX_UINT32 uiOldDataLen,
                                       bool ,
                                       char const* szNewData,
                                       CWX_UINT32 uiNewDataLen,
                                       bool bNewKeyValue,
                                       CWX_UINT32& uiFieldNum,
                                       char* szErr2K)
{
    CWX_UINT32 i=0;
    CwxKeyValueItemEx const* pItem=NULL;
    if (!reader1->unpack(szOldData, uiOldDataLen, false, true)){
        strcpy(szErr2K, reader1->getErrMsg());
        return -1;
    }
    uiFieldNum = 0;
    if (field){///set一个新
        bool bAddField = false;
        writer1->beginPack();
        for (i=0; i<reader1->getKeyNum(); i++){
            pItem = reader1->getKey(i);
            if ((pItem->m_unKeyLen == field->m_uiDataLen) && (memcmp(pItem->m_szKey, field->m_szData, pItem->m_unKeyLen)==0)){
                if (!writer1->addKeyValue(field->m_szData, field->m_uiDataLen, szNewData, uiNewDataLen, bNewKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                bAddField = true;
                uiFieldNum++;
            }else{
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                uiFieldNum++;
            }
        }
        if (!bAddField){///
            if (!writer1->addKeyValue(field->m_szData, field->m_uiDataLen, szNewData, uiNewDataLen, bNewKeyValue)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
            uiFieldNum++;
        }
        writer1->pack();
        return 1;
    }else{
        if (!reader2->unpack(szNewData, uiNewDataLen)){
            strcpy(szErr2K, reader2->getErrMsg());
            return -1;
        }
        writer1->beginPack();
        for (i=0; i<reader2->getKeyNum(); i++){
            pItem = reader2->getKey(i);
            if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
            uiFieldNum++;
        }
        for (i=0; i<reader1->getKeyNum(); i++){
            pItem = reader1->getKey(i);
            if (!reader2->getKey(pItem->m_szKey)){
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                uiFieldNum++;
            }
        }
        writer1->pack();
    }
    return 1;
}

///对update key的新、旧field进行归并。-1：失败；0：不存在；1：成功
int UnistorStoreBase::mergeUpdateKeyField(CwxPackageWriterEx* writer1,
                                          CwxPackageReaderEx* reader1,
                                          CwxPackageReaderEx* reader2,
                                          char const* key,
                                          CwxKeyValueItemEx const* field,
                                          char const* szOldData,
                                          CWX_UINT32 uiOldDataLen,
                                          bool bOldKeyValue,
                                          char const* szNewData,
                                          CWX_UINT32 uiNewDataLen,
                                          bool bNewKeyValue,
                                          CWX_UINT32& uiFieldNum,
                                          bool bAppend,
                                          char* szErr2K)
{
    CWX_UINT32 i=0;
    CwxKeyValueItemEx const* pItem=NULL;
    ///旧值一定是kv结构
    CWX_ASSERT(bOldKeyValue);
    if (!reader1->unpack(szOldData, uiOldDataLen, false, true)){
        strcpy(szErr2K, reader1->getErrMsg());
        return -1;
    }
    uiFieldNum = 0;
    if (field){///update新key
        bool bAdd=false;
        if (!reader1->getKey(field->m_szData) && !bAppend){
            CwxCommon::snprintf(szErr2K, 2047, "Key[%s]'s field [%s] doesn't exist.", key, field->m_szData);
            return 0;
        }
        writer1->beginPack();
        for (i=0; i<reader1->getKeyNum(); i++){
            pItem = reader1->getKey(i);
            if ((pItem->m_unKeyLen != field->m_uiDataLen) || (memcmp(pItem->m_szKey, field->m_szData, pItem->m_unKeyLen)!=0)){
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                bAdd = true;
                uiFieldNum++;
            }else{
                if (!writer1->addKeyValue(field->m_szData, field->m_uiDataLen, szNewData, uiNewDataLen, bNewKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                uiFieldNum++;
            }
        }
        if (!bAdd){
            if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
            uiFieldNum++;
        }
        writer1->pack();
        return 1;
    }else{
        if (!reader2->unpack(szNewData, uiNewDataLen)){
            strcpy(szErr2K, reader2->getErrMsg());
            return -1;
        }
        if (!bAppend){
            for (i=0; i<reader2->getKeyNum(); i++){
                if (!reader1->getKey(reader2->getKey(i)->m_szKey)){
                    CwxCommon::snprintf(szErr2K, 2047, "Key[%s]'s field [%s] doesn't exist.", key, reader2->getKey(i)->m_szKey);
                    return 0;
                }
            }
        }
        writer1->beginPack();
        for (i=0; i<reader2->getKeyNum(); i++){
            pItem = reader2->getKey(i);
            if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
            uiFieldNum++;
        }
        for (i=0; i<reader1->getKeyNum(); i++){
            pItem = reader1->getKey(i);
            if (!reader2->getKey(pItem->m_szKey)){
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                uiFieldNum++;
            }
        }
        writer1->pack();
    }
    return 1;
}

///对int key的更新。-2：key超出边界；-1：失败；0：不存在；1：成功;
int UnistorStoreBase::mergeIncKeyField(CwxPackageWriterEx* writer1,
                                       CwxPackageReaderEx* reader1,
                                       char const* ,
                                       CwxKeyValueItemEx const* field,
                                       char const* szOldData,
                                       CWX_UINT32 uiOldDataLen,
                                       bool bOldKeyValue,
                                       CWX_INT64  num,
                                       CWX_INT64* result,
                                       CWX_INT64  llMax,
                                       CWX_INT64  llMin,
                                       CWX_INT64& llValue,
                                       char* szBuf,
                                       CWX_UINT32& uiBufLen,
                                       bool& bKeyValue,
                                       CWX_UINT32 uiSign,
                                       char* szErr2K)
{
    CWX_UINT32 i=0;
    CwxKeyValueItemEx const* pItem=NULL;
    char szValue[64];
    if (bOldKeyValue){//Key的旧值对象
        if (!field){//如果key不是对象，则key自身为计数器
            if (0 == uiSign){///计数器必须存在
                strcpy(szErr2K, "Key is not counter.");
                return 0;
            }
            if (!result){
                llValue = num;
                if ((num>0) && llMax && (llValue > llMax)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is more than the max[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMax, szBuf, 0));
                    return -2;
                }else if ((num<0) && llMin && (llValue < llMin)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is less than the min[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMin, szBuf, 0));
                    return -2;
                }
            }else{
                llValue = *result;
            }
            CwxCommon::toString(llValue, szBuf, 0);
            uiBufLen = strlen(szBuf);
            bKeyValue = false;
            return 1;
        }
        ///指定了field
        if (!reader1->unpack(szOldData, uiOldDataLen, false, true)){
            strcpy(szErr2K, reader1->getErrMsg());
            return -1;
        }
        if (0 == uiSign){
            if (!reader1->getKey(field->m_szData)){
                strcpy(szErr2K, "Field counter doesn't exist..");
                return 0;
            }
        }
        bool bFindField = false;
        writer1->beginPack();
        for (i=0; i<reader1->getKeyNum(); i++){
            pItem = reader1->getKey(i);
            if ((pItem->m_unKeyLen == field->m_uiDataLen) && (memcmp(pItem->m_szKey, field->m_szData, pItem->m_unKeyLen)==0)){
                if (!result){
                    llValue = strtoll(pItem->m_szData, NULL, 0);
                    llValue += num;
                    if ((num>0) && llMax && (llValue > llMax)){
                        CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is more than the max[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMax, szBuf, 0));
                        return -2;
                    }else if ((num<0) && llMin && (llValue < llMin)){
                        CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is less than the min[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMin, szBuf, 0));
                        return -2;
                    }
                }else{
                    llValue = *result;
                }
                CwxCommon::toString((CWX_INT64)llValue, szBuf, 0);
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, szBuf, strlen(szBuf), false)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
                bFindField = true;
            }else{
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
            }
        }
        if (!bFindField){
            if (!result){
                llValue = num;
                if ((num>0) && llMax && (llValue > llMax)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is more than the max[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMax, szBuf, 0));
                    return -2;
                }else if ((num<0) && llMin && (llValue < llMin)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is less than the min[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMin, szBuf, 0));
                    return -2;
                }
            }else{
                llValue = *result;
            }
            if (!writer1->addKeyValue(field->m_szData, field->m_uiDataLen,  num)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
        }
        writer1->pack();
        uiBufLen = writer1->getMsgSize();
        memcpy(szBuf, writer1->getMsg(), uiBufLen);
        bKeyValue = true;
        return 1;
    }else{//key的旧值不是对象
        if (!field){//新值也不是对象
            if (!result){
                if (uiOldDataLen > 63) uiOldDataLen=63;
                memcpy(szValue, szOldData, uiOldDataLen);
                szValue[uiOldDataLen] = 0x00;
                llValue = strtoll(szValue, NULL, 0);
                llValue += num;
                if ((num>0) && llMax && (llValue > llMax)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is more than the max[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMax, szBuf, 0));
                    return -2;
                }else if ((num<0) && llMin && (llValue < llMin)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is less than the min[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMin, szBuf, 0));
                    return -2;
                }
            }else{
                llValue = *result;
            }
            CwxCommon::toString(llValue, szBuf, 0);
            uiBufLen = strlen(szBuf);
            bKeyValue = false;
            return 1;
        }else{
            if (0 == uiSign){
                strcpy(szErr2K, "Field counter doesn't exist..");
                return 0;
            }
            if (!result){
                llValue = num;
                if ((num>0) && llMax && (llValue > llMax)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is more than the max[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMax, szBuf, 0));
                    return -2;
                }else if ((num<0) && llMin && (llValue < llMin)){
                    CwxCommon::snprintf(szErr2K, 2047, "Couter[%s] is less than the min[%s]", CwxCommon::toString(llValue, szValue, 0), CwxCommon::toString(llMin, szBuf, 0));
                    return -2;
                }
            }else{
                llValue = *result;
            }
            CwxCommon::toString(llValue, szBuf, 0);
            writer1->beginPack();
            if (!writer1->addKeyValue(field->m_szData, field->m_uiDataLen, szBuf, strlen(szBuf), false)){
                strcpy(szErr2K, writer1->getErrMsg());
                return -1;
            }
            writer1->pack();
            uiBufLen = writer1->getMsgSize();
            memcpy(szBuf, writer1->getMsg(), uiBufLen);
            bKeyValue = true;
        }
    }
    return 1;
}

///对delete  key的field。-1：失败；1：成功
int UnistorStoreBase::mergeRemoveKeyField(CwxPackageWriterEx* writer1,
                                          CwxPackageReaderEx* reader1,
                                          char const* ,
                                          CwxKeyValueItemEx const* field,
                                          char const* szOldData,
                                          CWX_UINT32 uiOldDataLen,
                                          bool ,
                                          CWX_UINT32& uiFieldNum,
                                          char* szErr2K)
{
    CWX_UINT32 i=0;
    CwxKeyValueItemEx const* pItem=NULL;
    UnistorKeyField* pField = NULL;
    UnistorKeyField* pTmp = NULL;

    parseMultiField(field->m_szData, pField);

    uiFieldNum = 0;
    if (!reader1->unpack(szOldData, uiOldDataLen, false, true)){
        strcpy(szErr2K, reader1->getErrMsg());
        freeField(pField);
        return -1;
    }
    uiFieldNum = reader1->getKeyNum();

    writer1->beginPack();
    for (i=0; i<reader1->getKeyNum(); i++){
        pItem = reader1->getKey(i);
        pTmp = pField;
        while(pTmp){
            if ((pItem->m_unKeyLen != pTmp->m_key.length()) || (memcmp(pItem->m_szKey, pTmp->m_key.c_str(), pTmp->m_key.length())!=0)){
                if (!writer1->addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue)){
                    freeField(pField);
                    strcpy(szErr2K, writer1->getErrMsg());
                    return -1;
                }
            }else{
                uiFieldNum--;
            }
            pTmp = pTmp->m_next;
        }
    }
    writer1->pack();
    freeField(pField);
    return 1;
}

///提取指定的field, UNISTOR_ERR_SUCCESS：成功；其他：错误代码
int UnistorStoreBase::pickField(CwxPackageReaderEx& reader,
                                CwxPackageWriterEx& write,
                                UnistorKeyField const* field,
                                char const* szData,
                                CWX_UINT32 uiDataLen,
                                char* szErr2K)
{
    if (!reader.unpack(szData, uiDataLen, false, true)){
        if (szErr2K) strcpy(szErr2K, reader.getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxKeyValueItemEx const* pItem = NULL;
    UnistorKeyField const* field_item = NULL;
    write.beginPack();
    field_item = field;
    while(field_item){
        pItem = reader.getKey(field_item->m_key.c_str());
        if (pItem){
            write.addKeyValue(pItem->m_szKey, pItem->m_unKeyLen, pItem->m_szData, pItem->m_uiDataLen, pItem->m_bKeyValue);
        }
        field_item = field_item->m_next;
    }
    write.pack();
    return UNISTOR_ERR_SUCCESS;
}

//获取系统key。1：成功；0：不存在；-1：失败;
int UnistorStoreBase::getSysKey(char const* key, ///<要获取的key
              CWX_UINT16 unKeyLen, ///<key的长度
              char* szData, ///<若存在，则返回数据。内存有存储引擎分配
              CWX_UINT32& uiLen  ///<szData数据的字节数
              )
{
    return m_pGetSysInfoFunc(m_pApp, key, unKeyLen, szData, uiLen);
}

