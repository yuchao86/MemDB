#include "UnistorCache.h"

///初始化write item的引擎比较函数为空
UNISTOR_KEY_CMP_EQUAL_FN UnistorWriteCacheItem::m_fnEqual= NULL;     ///<key相等的比较函数
UNISTOR_KEY_CMP_LESS_FN  UnistorWriteCacheItem::m_fnLess= NULL;   ///<key小于的比较函数

///初始化化；0：成功；-1：失败
int UnistorWriteCache::init(UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite,
         UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite,
         UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite,
         void*     context,
         char* )
{
    CwxReadLockGuard<CwxRwLock>  lock(m_rwLock);
    if (m_buf) delete [] m_buf;
    m_buf = new char[m_uiCacheByteSize];
    if (!m_buf) return -1;
    m_bFirstHalf = true;
    m_uiPos = 0;
    m_uiCommitBeginPos = 0;
    m_uiCommitEndPos = 0;
    m_ullCommitSid = 0;
    m_pCommitUserData = NULL;
    m_pMutex = new pthread_mutex_t;
    ::pthread_mutex_init(m_pMutex, NULL);
    m_pCommitThreadWaitCond = new pthread_cond_t;
    ::pthread_cond_init(m_pCommitThreadWaitCond, NULL);
    m_pWriteThreadWaitCond = new pthread_cond_t;
    ::pthread_cond_init(m_pWriteThreadWaitCond, NULL);
    m_bCommitThreadWait = false;
    m_bWriteThreadWait = false;
    m_fnBeginWrite = fnBeginWrite;
    m_fnWrite = fnWrite;
    m_fnEndWrite = fnEndWrite;
    m_context = context; 
    UnistorWriteCacheItem::m_fnEqual = m_fnEqual;
    UnistorWriteCacheItem::m_fnLess = m_fnLess;
    m_keyIndex = new set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >;
    return 0;
}


///1：成功；0：cache满了，需要写入
int UnistorWriteCache::updateKey(char const* szKey,
                                 CWX_UINT16 unKeyLen,
                                 char const* szData,
                                 CWX_UINT32 uiDataLen,
                                 CWX_UINT32 uiOldExpire,
                                 bool& bInWriteCache)
{
    CwxWriteLockGuard<CwxRwLock>  lock(m_rwLock);
    char key_buf[UnistorWriteCacheItem::calBufCapacity(unKeyLen, 0, 0) + sizeof(UnistorWriteCacheItem)];
    UnistorWriteCacheItem* key = (UnistorWriteCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    bInWriteCache = false;
    CWX_UINT32 const uiCapacity = UnistorWriteCacheItem::calBufCapacity(unKeyLen, uiDataLen, m_unAlign);
    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator key_iter = m_keyIndex->find(key);
    if (key_iter != m_keyIndex->end()){///如果key在write cache中存在
        bInWriteCache = true;
        if (((*key_iter)->m_uiCapacity >= uiCapacity)  ///空间足够
            && _isInWriteCommitHalf((char*)(*key_iter))) ///而且所在半区不是commit区间
        {
            ///若空间足够，则key无需删除，直接覆盖就可以了。
            memcpy((*key_iter)->m_szBuf + (*key_iter)->m_unKeyLen, szData, uiDataLen);
            (*key_iter)->m_uiDataLen = uiDataLen;
            (*key_iter)->m_bDel = false;
            return 1;
        }else{///若空间不够，则需要重新分配空间。此需要记下最初的Expire时间，以便支持expire的引擎处理expire索引
            uiOldExpire = (*key_iter)->m_ttOldExpire;
        }
    }
    if (m_bFirstHalf){///使用上半区
        if (m_uiSplitHalfPos - m_uiPos < uiCapacity + sizeof(UnistorWriteCacheItem)){///没有足够的空间
            return 0; ///需要执行commit
        }
    }else{///使用下半区
        if (m_uiCacheByteSize - m_uiPos < uiCapacity + sizeof(UnistorWriteCacheItem)){
            return 0;///需要执行commit
        }
    }
    ///无需commit，但需要为key分配新的空间
    UnistorWriteCacheItem* pItem = (UnistorWriteCacheItem*)(m_buf + m_uiPos);
    pItem->m_unKeyLen = unKeyLen;
    pItem->m_uiDataLen = uiDataLen;
    pItem->m_uiCapacity = uiCapacity;
    pItem->m_bDel = false;
    pItem->m_ttOldExpire = uiOldExpire;
    memcpy(pItem->m_szBuf, szKey, unKeyLen);
    memcpy(pItem->m_szBuf + unKeyLen, szData, uiDataLen);
    ///如果key已经存在，则必须先删除key，否则无法加入到索引中。
    if (key_iter != m_keyIndex->end()) m_keyIndex->erase(key_iter);
    m_keyIndex->insert(pItem);
    m_uiPos += pItem->size();
    return 1;
}

///1：成功；0：cache满了，需要写入
int UnistorWriteCache::delKey(char const* szKey,
                              CWX_UINT32 unKeyLen,
                              CWX_UINT32 uiOldExpire,
                              bool& bInWriteCache)
{
    CwxWriteLockGuard<CwxRwLock>  lock(m_rwLock);
    char key_buf[UnistorWriteCacheItem::calBufCapacity(unKeyLen, 0, 0) + sizeof(UnistorWriteCacheItem)];
    UnistorWriteCacheItem* key = (UnistorWriteCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    bInWriteCache = false;
    CWX_UINT32  const uiCapacity = UnistorWriteCacheItem::calBufCapacity(unKeyLen, 0, m_unAlign);

    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator key_iter = m_keyIndex->find(key);
    if (key_iter != m_keyIndex->end()){ ///如果key存在
        bInWriteCache = true;
        if (_isInWriteCommitHalf((char*)(*key_iter))){///如果key位于非commit的半区，则直接标记key删除即可
            (*key_iter)->m_bDel = true;
            return 1;
        }else{///需要重新分配空间。此需要记下最初的Expire时间，以便支持expire的引擎处理expire索引
            uiOldExpire = (*key_iter)->m_ttOldExpire;
        }
    }
    if (m_bFirstHalf){///使用上半区
        if (m_uiSplitHalfPos - m_uiPos < uiCapacity + sizeof(UnistorWriteCacheItem)){
            return 0;///没有足够的空间
        }
    }else{
        if (m_uiCacheByteSize - m_uiPos < uiCapacity + sizeof(UnistorWriteCacheItem)){
            return 0;///没有足够的空间
        }
    }
    ///为删除的key分配新空间
    UnistorWriteCacheItem* pItem = (UnistorWriteCacheItem*)(m_buf + m_uiPos);
    pItem->m_unKeyLen = unKeyLen;
    pItem->m_uiDataLen = 0;
    pItem->m_uiCapacity = uiCapacity;
    pItem->m_bDel = true;
    pItem->m_ttOldExpire = uiOldExpire;
    memcpy(pItem->m_szBuf, szKey, unKeyLen);
    ///若key已经在索引中存在，则需要删除否则无法添加
    if (key_iter != m_keyIndex->end()) m_keyIndex->erase(key_iter);
    m_keyIndex->insert(pItem);
    m_uiPos += pItem->size();
    return 1;
}

///1：获取一个；0：不存在；-1：buf空间太小
int UnistorWriteCache::getKey(char const* szKey,
                              CWX_UINT16 unKeyLen,
                              char* szData,
                              CWX_UINT32& uiDataLen,
                              bool& bDel)
{
    CwxReadLockGuard<CwxRwLock>  lock(m_rwLock);
    char key_buf[UnistorWriteCacheItem::calBufCapacity(unKeyLen, 0, 0) + sizeof(UnistorWriteCacheItem)];
    UnistorWriteCacheItem* key = (UnistorWriteCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;

    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator key_iter = m_keyIndex->find(key);
    if (key_iter != m_keyIndex->end()){///如果存在则拷贝内容
        UnistorWriteCacheItem* pItem = *key_iter;
        if (uiDataLen < pItem->m_uiDataLen) return -1;
        memcpy(szData, pItem->m_szBuf + pItem->m_unKeyLen, pItem->m_uiDataLen);
        uiDataLen = pItem->m_uiDataLen;
        bDel = pItem->m_bDel;
        return 1;
    }
    return 0;
}

///获取下一个写cache中的key。1：获取一个；0：不存在；-1：buf空间太小
int UnistorWriteCache::nextKey(char const* szBeginKey,
                               CWX_UINT16 unBeginKeyLen,
                               char* szKey,
                               CWX_UINT16& unKeyLen,
                               char* szData,
                               CWX_UINT32& uiDataLen,
                               bool& bDel)
{
    CwxReadLockGuard<CwxRwLock>  lock(m_rwLock);

    char key_buf[UnistorWriteCacheItem::calBufCapacity(unBeginKeyLen, 0, 0) + sizeof(UnistorWriteCacheItem)];
    UnistorWriteCacheItem* key = (UnistorWriteCacheItem*)key_buf;
    if (szBeginKey) memcpy(key->m_szBuf, szBeginKey, unBeginKeyLen);
    key->m_unKeyLen = unBeginKeyLen;

    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator key_iter;
    UnistorWriteCacheItem* pItem = NULL;
    if (szBeginKey && unBeginKeyLen){///如果指定开始的key，则从>key的位置开始
        key_iter = m_keyIndex->upper_bound(key);
    }else{///否则从开始的位置开始
        key_iter = m_keyIndex->begin();
    }
    if (key_iter != m_keyIndex->end()){///<获取key
        pItem = *key_iter;
        if (uiDataLen <pItem->m_uiDataLen) return -1;
        memcpy(szKey, pItem->m_szBuf, pItem->m_unKeyLen);
        unKeyLen = pItem->m_unKeyLen;
        memcpy(szData, pItem->m_szBuf + pItem->m_unKeyLen, pItem->m_uiDataLen);
        uiDataLen = pItem->m_uiDataLen;
        bDel = pItem->m_bDel;
        return 1;
    }
    return 0;
}

///获取前一个写cache中的key。1：获取一个；0：不存在；-1：buf空间太小
int UnistorWriteCache::prevKey(char const* szBeginKey,
                                                  CWX_UINT16 unBeginKeyLen,
                                                  char* szKey,
                                                  CWX_UINT16& unKeyLen,
                                                  char* szData,
                                                  CWX_UINT32& uiDataLen,
                                                  bool& bDel)
{

    CwxReadLockGuard<CwxRwLock>  lock(m_rwLock);
    char key_buf[UnistorWriteCacheItem::calBufCapacity(unBeginKeyLen, 0, 0) + sizeof(UnistorWriteCacheItem)];
    UnistorWriteCacheItem* key = (UnistorWriteCacheItem*)key_buf;
    if (szBeginKey) memcpy(key->m_szBuf, szBeginKey, unBeginKeyLen);
    key->m_unKeyLen = unBeginKeyLen;

    UnistorWriteCacheItem* pItem = NULL;
    ///不存在key
    if (!m_keyIndex->size()) return 0;

    if (szBeginKey && unBeginKeyLen){///如果指定了开始key
        set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator key_iter;
        key_iter = m_keyIndex->lower_bound(key);///从>=key的位置开始
        if (key_iter == m_keyIndex->begin()){///<所有的key都不比当前的key小，没有值
            return 0;
        }
        if (key_iter != m_keyIndex->end()){///如果不是结束则取前一个，而且一定有前一个
            key_iter--;
            pItem = *key_iter;
        }else{///说明所有的key都比当前的key小
            set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::reverse_iterator key_iter;
            key_iter = m_keyIndex->rbegin();
            if (key_iter != m_keyIndex->rend()){
                pItem = *key_iter;
            }
        }
    }else{///否则从最后一个获取
        set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::reverse_iterator key_iter;
        key_iter = m_keyIndex->rbegin();
        if (key_iter != m_keyIndex->rend()){
            pItem = *key_iter;
        }
    }
    if (pItem){///如果存在，则返回
        if (uiDataLen < pItem->m_uiDataLen) return -1;
        memcpy(szKey, pItem->m_szBuf, pItem->m_unKeyLen);
        unKeyLen = pItem->m_unKeyLen;
        memcpy(szData, pItem->m_szBuf + pItem->m_unKeyLen, pItem->m_uiDataLen);
        uiDataLen = pItem->m_uiDataLen;
        bDel = pItem->m_bDel;
        return 1;
    }
    return 0;
}

///提交数据；0：成功；-1：失败
int UnistorWriteCache::commit(CWX_UINT64 ullSid, void* userData){
    ::pthread_mutex_lock(m_pMutex); ///获取guard的mutex
    do{
        if (m_ullCommitSid){///上一次commit还没有完成，必须等到
            m_bWriteThreadWait = true;
            ::pthread_cond_wait(m_pWriteThreadWaitCond, m_pMutex);
            m_bWriteThreadWait = false;
        }
        ///开始提交
        CWX_ASSERT(!m_ullCommitSid);
        m_ullCommitSid = ullSid;
        m_pCommitUserData = userData;
        m_uiCommitBeginPos = m_bFirstHalf?0:m_uiSplitHalfPos;
        m_uiCommitEndPos = m_uiPos;
        CWX_ASSERT(m_uiCommitBeginPos<=m_uiCommitEndPos);
        if (m_bFirstHalf){
            m_bFirstHalf = false;
            m_uiPos = m_uiSplitHalfPos;
        }else{
            m_bFirstHalf = true;
            m_uiPos = 0;
        }
        ///通知commit线程
        if(m_bCommitThreadWait) ::pthread_cond_signal(m_pCommitThreadWaitCond);
    } while(0);
    ::pthread_mutex_unlock(m_pMutex);
    return 0;
}


void* UnistorCache::commitThreadMain(void* cache)
{
    char szKeyBuf[UNISTOR_MAX_KEY_SIZE + 1];
    UnistorCache* pCache=(UnistorCache*)cache;
    ::pthread_mutex_lock(pCache->m_writeCache->m_pMutex);
    CWX_UINT32 uiPos = 0;
    UnistorWriteCacheItem* pItem = NULL;
    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >  index;///<key索引
    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator iter;
    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >::iterator iter_1;
    while(!pCache->m_bExit && pCache->m_bValid){
        if (pCache->m_writeCache->m_ullCommitSid){///有数据需要提交
            ///建立需要写的数据的map
            uiPos = pCache->m_writeCache->m_uiCommitBeginPos;
            while(uiPos<pCache->m_writeCache->m_uiCommitEndPos){
                pItem = (UnistorWriteCacheItem*)(pCache->m_writeCache->m_buf + uiPos);
                if (pItem->getCapacity() != UnistorWriteCacheItem::calBufCapacity(pItem->getKeyLen(),
                    pItem->getDataLen(), pCache->m_writeCache->m_unAlign))
                {
                    CwxCommon::snprintf(pCache->m_szErr2K, 2048,  "Write cache memory error, item's capacity[%u] is not right. key_len:%u, data_len:%u, align:%u",
                        pItem->getCapacity(), pItem->getKeyLen(), pItem->getDataLen(), pCache->m_writeCache->m_unAlign);
                    pCache->m_bValid = false;
                    break;

                }
                index.insert(pItem);
                uiPos += pItem->size();
                CWX_ASSERT(uiPos <= pCache->m_writeCache->m_uiCommitEndPos);
            }
            if (pCache->m_bValid && index.size()){///如果有需要提交的数据
                if (0 != pCache->m_writeCache->m_fnBeginWrite(pCache->m_writeCache->m_context, pCache->m_szErr2K)){
                    pCache->m_bValid = false;
                    break;
                }
                iter = index.begin();
                while(iter != index.end()){
                    pItem = *iter;
                    if (0 != pCache->m_writeCache->m_fnWrite(pCache->m_writeCache->m_context,
                        pItem->getKey(),
                        pItem->getKeyLen(),
                        pItem->getData(),
                        pItem->getDataLen(),
                        pItem->isDel(),
                        pItem->getOldExpire(),
                        szKeyBuf,
                        UNISTOR_MAX_KEY_SIZE,
                        pCache->m_szErr2K))
                    {
                        pCache->m_bValid = false;
                        break;
                    }
                    iter++;
                }

                if (0 != pCache->m_writeCache->m_fnEndWrite(pCache->m_writeCache->m_context,
                    pCache->m_writeCache->m_ullCommitSid,
                    pCache->m_writeCache->m_pCommitUserData,
                    pCache->m_szErr2K))
                {
                    pCache->m_bValid = false;
                    break;
                }
                ///将key从写cache中删除
                {
                    ///由于要修改write cache的索引，必须获取写锁。
                    CwxWriteLockGuard<CwxRwLock>  lock(&pCache->m_writeCacheRwLock);
                    iter = index.begin();
                    while(iter != index.end()){
                        iter_1 = pCache->m_writeCache->m_keyIndex->find(*iter);
                        CWX_ASSERT(iter_1 != pCache->m_writeCache->m_keyIndex->end());
                        if (*iter_1 == *iter){
                            pCache->m_writeCache->m_keyIndex->erase(iter_1);
                        }
                        iter++;
                    }
                }
                index.clear();
                pCache->m_writeCache->m_ullCommitSid = 0;
                pCache->m_writeCache->m_pCommitUserData = NULL;
                pCache->m_writeCache->m_uiCommitBeginPos = 0;
                pCache->m_writeCache->m_uiCommitEndPos = 0;
            }
            pCache->m_writeCache->m_ullPrevCommitSid = pCache->m_writeCache->m_ullCommitSid;
        }
        pCache->m_writeCache->m_ullCommitSid = 0;
        pCache->m_writeCache->m_pCommitUserData = NULL;
        pCache->m_writeCache->m_uiCommitBeginPos = 0;
        pCache->m_writeCache->m_uiCommitEndPos = 0;
        if (pCache->m_writeCache->m_bWriteThreadWait){
            ::pthread_cond_signal(pCache->m_writeCache->m_pWriteThreadWaitCond);
        }
        pCache->m_writeCache->m_bCommitThreadWait = true;
        ::pthread_cond_wait(pCache->m_writeCache->m_pCommitThreadWaitCond, pCache->m_writeCache->m_pMutex);
        pCache->m_writeCache->m_bCommitThreadWait = false;

    }
    ::pthread_mutex_unlock(pCache->m_writeCache->m_pMutex);
    return NULL;
}

///初始化化；0：成功；-1：失败
int UnistorCache::init(UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite,
                       UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite,
                       UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite,
                       void*     context,
                       float     fBucketRate,
                       char*      szErr2K)
{
    m_writeThreadId = ::pthread_self();
    if (m_commitThreadId){
        m_bExit = true;
        if (m_writeCache){
            m_writeCache->wakeupCommitThread();
            ::pthread_join(m_commitThreadId, NULL);
        }
        m_commitThreadId = 0;
    }
    if (m_writeCache) delete m_writeCache;
    m_writeCache = NULL;
    if (m_readCache) delete m_readCache;
    m_readCache = NULL;
    m_bExit = false;

    if (m_uiWriteCacheByte){
        m_writeCache = new UnistorWriteCache(m_uiWriteCacheByte,
            m_fnEqual,
            m_fnLess,
            m_fnHash,
            &m_writeCacheRwLock,
            32);
        if (0 != m_writeCache->init(fnBeginWrite,
            fnWrite,
            fnEndWrite,
            context,
            szErr2K))
        {
            return -1;
        }
    }
    if (m_ullReadCacheByte){
        m_readCache = new UnistorReadCacheEx2(m_ullReadCacheByte,
            m_uiReadCacheKeyNum,
            m_fnEqual,
            m_fnLess,
            m_fnHash,
            &m_readCacheRwLock,
            fBucketRate);
        if (0 != m_readCache->init(szErr2K)) return -1;
    }
    //启动commit线程
    if (m_writeCache){
        if(0 != ::pthread_create(&m_commitThreadId,
            NULL,
            UnistorCache::commitThreadMain,
            this))
        {
            m_bValid = false;
            strcpy(m_szErr2K, "failed to start commit thread");
            return -1;
        }
    }
    m_bValid = true;
    m_szErr2K[0] = 0x00;
    return 0;
}

///更新key。1：成功；0：cache满了，需要写入；-1：cache错误，-2：没有写cache
int UnistorCache::updateKey(char const* szKey,
                            CWX_UINT16 unKeyLen,
                            char const* szData,
                            CWX_UINT32 uiDataLen,
                            CWX_UINT32 uiOldExpire,
                            bool  bCache,
                            bool& bInWriteCache)
{
    if (m_writeCache){
        if (!m_bValid) return -1;
        ///首先更新写cache
        int ret = m_writeCache->updateKey(szKey, unKeyLen, szData, uiDataLen, uiOldExpire, bInWriteCache);
        ///更新读cache
        if ((1 == ret) && m_readCache){
            if (bCache){
                m_readCache->insert(szKey, unKeyLen, szData, uiDataLen, false);
            }else{
                m_readCache->remove(szKey, unKeyLen);
            }
        }
        return ret;
    }
    return -2;
}

///1：成功；0：cache满了，需要写入；-1：cache错误
int UnistorCache::delKey(char const* szKey,
                         CWX_UINT32 unKeyLen,
                         CWX_UINT32 uiOldExpire,
                         bool& bInWriteCache)
{
    if (!m_bValid) return -1;
    if (m_writeCache){
        if (0 == m_writeCache->delKey(szKey, unKeyLen, uiOldExpire, bInWriteCache)) return 0;
    }
    if (m_readCache) m_readCache->remove(szKey, unKeyLen);
    return 1;
}

///cache key
void UnistorCache::cacheKey(char const* szKey,
                            CWX_UINT16 unKeyLen,
                            char const* szData,
                            CWX_UINT32 uiDataLen,
                            bool bNotExist)
{
    if (m_readCache){
        m_readCache->insert(szKey, unKeyLen, szData, uiDataLen, bNotExist);
    }
}

///1：获取一个；0：不存在；-1：buf空间太小
int UnistorCache::getKey(char const* szKey,
                         CWX_UINT16 unKeyLen,
                         char* szData,
                         CWX_UINT32& uiDataLen,
                         bool& bDel,
                         bool& bReadCache)
{
    int ret = 0;
    bReadCache = false;
    if (m_writeCache){
        ret = m_writeCache->getKey(szKey, unKeyLen, szData, uiDataLen, bDel);
    }
    if ((0 == ret) && m_readCache){
        ret = m_readCache->fetch(szKey, unKeyLen, szData, uiDataLen, true);
        if (1 == ret) bReadCache = true;
        bDel = false;
    }
    return ret;
}

///从写cache获取一个key。1：获取一个；0：不存在；-1：buf空间太小
int UnistorCache::getWriteKey(char const* szKey,
                              CWX_UINT16 unKeyLen,
                              char* szData,
                              CWX_UINT32& uiDataLen,
                              bool& bDel)
{
    if (m_writeCache){
        return m_writeCache->getKey(szKey, unKeyLen, szData, uiDataLen, bDel);
    }
    return 0;
}


///获取下一个写cache中的key。1：获取一个；0：不存在；-1：buf空间太小
int UnistorCache::nextWriteKey(char const* szBeginKey,
                               CWX_UINT16 unBeginKeyLen,
                               bool bAsc,
                               char* szKey,
                               CWX_UINT16& unKeyLen,
                               char* szData,
                               CWX_UINT32& uiDataLen,
                               bool& bDel)
{
    if (m_writeCache){
        if (bAsc){
            return m_writeCache->nextKey(szBeginKey, unBeginKeyLen, szKey, unKeyLen, szData, uiDataLen, bDel);
        }
        return m_writeCache->prevKey(szBeginKey, unBeginKeyLen, szKey, unKeyLen, szData, uiDataLen, bDel);
    }
    return 0;
}
//-1: failure, 0: success
int UnistorCache::commit(CWX_UINT64 ullSid, void* userData, char* szErr2K)
{///必须不能lock m_rwLock
    if (m_writeCache){
        if (!m_bValid){
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
        if (0 != m_writeCache->commit(ullSid, userData)){
            strcpy(m_szErr2K, "Failure to commit the write cache.");
            m_bValid = false;
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
    }
    return 0;
}
