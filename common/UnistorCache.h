#ifndef __UNISTOR_CACHE_H__
#define __UNISTOR_CACHE_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "UnistorReadCacheEx2.h"
#include <pthread.h>
///write cache的key item对象，自己不分配内存，使用cache buf
struct UnistorWriteCacheItem{
public:
    ///构造函数
    UnistorWriteCacheItem(){
        m_uiDataLen = 0;
        m_uiCapacity = 0;
        m_ttOldExpire = 0;
        m_unKeyLen = 0;
        m_bDel = false;
    };
public:
    ///获取key
    inline char const* getKey() const{
        return m_szBuf;
    }
    ///获取data
    inline char const* getData() const{
        return m_szBuf + m_unKeyLen;
    }
    ///获取key的长度
    inline CWX_UINT16 getKeyLen() const{
        return m_unKeyLen;
    }
    ///获取data的长度
    inline CWX_UINT32 getDataLen() const{
        return m_uiDataLen;
    }
    ///获取超时时间
    inline CWX_UINT32 getOldExpire() const{
        return m_ttOldExpire;
    }
    ///获取空间大小
    inline CWX_UINT32 getCapacity() const{
        return m_uiCapacity;
    }
    ///是否删除
    inline bool isDel() const{
        return m_bDel;
    }
    ///获取key的大小
    inline CWX_UINT32 size() const{
        return m_uiCapacity + sizeof(UnistorWriteCacheItem);
    }
    ///相等比较，比较采用存储引擎的key的比较函数
    bool operator == (UnistorWriteCacheItem const& item) const{
        return m_fnEqual(m_szBuf, m_unKeyLen, item.m_szBuf, item.m_unKeyLen);
    }
    ///小于比较，比较采用存储引擎的key的比较函数
    bool operator < (UnistorWriteCacheItem const& item) const{
        return m_fnLess(m_szBuf, m_unKeyLen, item.m_szBuf, item.m_unKeyLen)<0?true:false;
    }
public:
    ///获取容量
    inline static CWX_UINT32 calBufCapacity(CWX_UINT16 unKeyLen,///<key的长度
        CWX_UINT32 uiDataLen, ///<data的长度
        CWX_UINT16 unAlign ///<对齐字节数
        )
    {
        CWX_UINT32 uiCapacity = 0;
        if (0 == unAlign){
            uiCapacity = uiDataLen + unKeyLen;
        }else{
            uiCapacity = (unKeyLen + uiDataLen +  unAlign - 1)/unAlign;
            uiCapacity *= unAlign;
        }
        if (uiCapacity < 8) uiCapacity = 8;
        //必须按照8对齐
        CWX_UINT16 unMod = (sizeof(UnistorWriteCacheItem) + uiCapacity)%8;
        if (unMod){
            uiCapacity += (8 - unMod);
        }
        return uiCapacity;
    }
private:
    friend class UnistorWriteCache;
private:
    CWX_UINT32                m_uiDataLen;  ///<data的长度
    CWX_UINT32                m_uiCapacity; ///<空间的容量
    CWX_UINT32                m_ttOldExpire; ///<旧的超时时间
    CWX_UINT16                m_unKeyLen;   ///<key的长度
    bool                      m_bDel;      ///<key的长度
    char                      m_szBuf[0];     ///<key的数据
    static UNISTOR_KEY_CMP_EQUAL_FN m_fnEqual; ///<存储引擎的key相等的比较函数
    static UNISTOR_KEY_CMP_LESS_FN  m_fnLess;   ///<存储引擎的key小于的比较函数

}__attribute__ ((__packed__));;


class UnistorCache;
///写cache，只支持一个写线程，多个读线程
///写cache采用连续的内存空间，分为上、下半区。
///当上半区写满的时候，使用下半区写，dirty thread会被通知将上半区的数据写入disk，依次轮换
class UnistorWriteCache{
public:
    enum{
        UNISTOR_WRITE_CACHE_MIN_SIZE = 1024 * 1024 * 32 ///<写cache的最小大小，为64M
    };
public:
    ///构造函数
    UnistorWriteCache(CWX_UINT32 uiCacheSize, ///<写cache的大小
        UNISTOR_KEY_CMP_EQUAL_FN fnEqual, ///<key相等的比较函数
        UNISTOR_KEY_CMP_LESS_FN  fnLess,  ///<key小于的比较函数
        UNISTOR_KEY_HASH_FN      fnHash, ///<key的hash函数
        CwxRwLock* lock=NULL, ///<锁对象。若为NULL表示不加锁
        CWX_UINT16 unAlign = 64 ///<空间对齐字节数
        ):m_unAlign(unAlign)
    {
        m_buf = NULL;
        m_uiCacheByteSize = uiCacheSize;
        if (m_uiCacheByteSize < UNISTOR_WRITE_CACHE_MIN_SIZE) m_uiCacheByteSize = UNISTOR_WRITE_CACHE_MIN_SIZE;
        ///内存K对齐
        if (m_uiCacheByteSize%1024){
            m_uiCacheByteSize -= (m_uiCacheByteSize%1024);
            m_uiCacheByteSize += 1024;
        }
        ///上下半区一定8字节对齐
        m_uiSplitHalfPos = m_uiCacheByteSize/2;
        m_bFirstHalf = true;
        m_uiPos = 0;
        m_uiCommitBeginPos = 0;
        m_uiCommitEndPos = 0;
        m_ullCommitSid = 0;
        m_pCommitUserData = NULL;
        m_ullPrevCommitSid = 0;
        m_pMutex = NULL;
        m_pCommitThreadWaitCond = NULL;
        m_pWriteThreadWaitCond = NULL;
        m_bCommitThreadWait = false;
        m_bWriteThreadWait = false;
        m_keyIndex = NULL;
        m_fnBeginWrite = NULL;
        m_fnWrite = NULL;
        m_fnEndWrite = NULL;
        m_rwLock = lock;
        m_fnEqual = fnEqual;
        m_fnLess = fnLess;
        m_fnHash = fnHash;
    }
    ///析构函数
    ~UnistorWriteCache(){
        if (m_buf) delete [] m_buf;
        if (m_pMutex){
            ::pthread_mutex_destroy(m_pMutex);
            delete m_pMutex;
            m_pMutex = NULL;
        }
        if (m_pCommitThreadWaitCond){
            delete m_pCommitThreadWaitCond;
            m_pCommitThreadWaitCond = NULL;
        }
        if (m_pWriteThreadWaitCond){
            delete m_pWriteThreadWaitCond;
            m_pWriteThreadWaitCond = NULL;
        }
        if (m_keyIndex) {
            delete m_keyIndex;
            m_keyIndex = NULL;
        }
    }
public:

    ///初始化写cache。返回值：0：成功；-1：失败
    int init(UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite, ///<dirty数据开始flush的通知函数
        UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite, ////<dirty key的写入通知函数
        UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite, ///<dirty数据写入完毕的通知函数
        void*     context,  ///<环境信息。此为存储引擎的指针
        char* szErr2K=NULL
        );
    
    ///更新一个key。返回值：1：成功；0：cache满了，需要commit
    int updateKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        char const* szData, ///<data
        CWX_UINT32 uiDataLen, ///<data的长度
        CWX_UINT32 uiOldExpire, ///<更新前key的expire值，此为了维护expire的索引
        bool& bInWriteCache ///<数据是否在write cache中存在
        );

    ///删除一个key。返回值：1：成功；0：cache满了，需要写入
    int delKey(char const* szKey, ///<key的名字
        CWX_UINT32 unKeyLen, ///<key的长度
        CWX_UINT32 uiOldExpire, ///<更新前key的expire值，此为了维护expire的索引
        bool& bInWriteCache ///<数据是否在write cache中存在
        );

    ///获取一个key。返回值：1：获取一个；0：不存在；-1：buf空间太小
    int getKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData,  ///<返回data的数据buf
        CWX_UINT32& uiDataLen, ///<传入数据buf的空间大小，返回data的大小
        bool& bDel  ///<key是否已经删除
        );

    ///获取下一个写cache中的key。返回值：1：获取一个；0：不存在；-1：buf空间太小
    int nextKey(char const* szBeginKey, ///<开始key。若为NULL表示不指定
        CWX_UINT16 unBeginKeyLen, ///<开始key的长度
        char* szKey,  ///<返回key的buf
        CWX_UINT16& unKeyLen, ///<传入key buf大小，返回key的长度
        char* szData, ///<返回data的buf
        CWX_UINT32& uiDataLen, ///<传入data buf的大小，返回data的长度
        bool& bDel ///<key是否已经删除
        );

    ///获取前一个写cache中的key。返回值：1：获取一个；0：不存在；-1：buf空间太小
    int prevKey(char const* szBeginKey,///<开始key。若为NULL表示不指定
        CWX_UINT16 unBeginKeyLen,///<开始key的长度
        char* szKey,///<返回key的buf
        CWX_UINT16& unKeyLen,///<传入key buf大小，返回key的长度
        char* szData,///<返回data的buf
        CWX_UINT32& uiDataLen,///<传入data buf的大小，返回data的长度
        bool& bDel///<key是否已经删除
        );

    ///提交数据；返回值：0：成功；-1：失败
    int commit(CWX_UINT64 ullSid, ///<commit的binlog的sid值。
        void* userData  ///<用户的其他数据
        );

    ///唤醒commit线程
    void wakeupCommitThread(){
        if (m_pMutex){
            ::pthread_mutex_lock(m_pMutex);
            ::pthread_cond_broadcast(m_pCommitThreadWaitCond);
            ::pthread_mutex_unlock(m_pMutex);
        }
    }
    
    ///获取前一次提交的sid
    inline CWX_UINT64 getPrevCommitSid() const{
        return m_ullPrevCommitSid;
    }

private:
    friend class UnistorCache;

    ///检测key是否处于当前正在写的分区
    inline bool _isInWriteCommitHalf(char const* szKey) const{
        return m_bFirstHalf?((szKey - m_buf) < m_uiSplitHalfPos):
            ((szKey - m_buf) >= m_uiSplitHalfPos);
    }
private:
    CWX_UINT16 const                m_unAlign; ///<对齐的字节数
    char*                           m_buf; ///<写的cache buf
    CWX_UINT32                      m_uiCacheByteSize; ///<写的cache buf的空间大小
    volatile CWX_UINT32             m_uiSplitHalfPos; ///<内存分割点
    volatile bool                   m_bFirstHalf; ///<是否当前使用上半区
    volatile CWX_UINT32             m_uiPos; ///<cache写到的当前位置
    volatile CWX_UINT32             m_uiCommitBeginPos; ///<commit的开始位置
    volatile CWX_UINT32             m_uiCommitEndPos;  ///<commit的结束位置
    volatile CWX_UINT64             m_ullCommitSid; ///<提交的sid
    void*                           m_pCommitUserData; ///<提交其他数据
    volatile CWX_UINT64             m_ullPrevCommitSid; ///<前一次提交的sid
    pthread_mutex_t*	            m_pMutex;       ///<commit写线程的锁
    pthread_cond_t*	                m_pCommitThreadWaitCond; ///<commit线程wait的condition
    pthread_cond_t*	                m_pWriteThreadWaitCond; ///<写线程的wait condition
    bool				            m_bCommitThreadWait; ///<commit线程是否在condition上wait
    bool				            m_bWriteThreadWait;  ///<write线程是否在condition上wait
    UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN          m_fnBeginWrite; ///<开始写的函数
    UNISTOR_WRITE_CACHE_WRITE_WRITE_FN          m_fnWrite; ///<写数据的函数
    UNISTOR_WRITE_CACHE_WRITE_END_FN            m_fnEndWrite; ///<结束写的函数
    void*                                       m_context; ///<函数的context信息
    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >*  m_keyIndex;///<key索引
    CwxRwLock*                      m_rwLock; ///<读写锁，若为NULL表示没有锁
    UNISTOR_KEY_CMP_EQUAL_FN        m_fnEqual; ///<key相等的比较函数
    UNISTOR_KEY_CMP_LESS_FN         m_fnLess; ///<key小于的比较函数
    UNISTOR_KEY_HASH_FN             m_fnHash; ///<key的hash值的计算函数
};

///cache系统，只支持一个写线程，多个读线程
class UnistorCache{
public:
    ///构造函数
    UnistorCache(CWX_UINT32 uiWriteCacheMBtye, ///<write cache的大小，若为0表示没有write cache
        CWX_UINT32 uiReadCacheMByte, ///<读cache的大小，若为0表示没有read cache
        CWX_UINT32 uiReadMaxCacheKeyNum, ///<读cache的最大cache条目
        UNISTOR_KEY_CMP_EQUAL_FN fnEqual, ///<key相等的比较函数
        UNISTOR_KEY_CMP_LESS_FN  fnLess,  ///<key小于的比较函数
        UNISTOR_KEY_HASH_FN    fnHash     ///<key的hash值的计算函数
        )
    {
        m_uiWriteCacheByte = uiWriteCacheMBtye;
        if (m_uiWriteCacheByte > 2048 * 1024) m_uiWriteCacheByte = 2048 * 1024;
        m_uiWriteCacheByte *= 1024 * 1024;
        m_ullReadCacheByte = uiReadCacheMByte;
        m_ullReadCacheByte *= 1024 * 1024;
        m_uiReadCacheKeyNum = uiReadMaxCacheKeyNum;
        m_writeCache = NULL;
        m_readCache = NULL;
        m_bExit = false;
        m_writeThreadId = 0;
        m_commitThreadId = 0;
        m_bValid = true;
        m_szErr2K[0] = 0x00;
        m_fnEqual = fnEqual;
        m_fnLess = fnLess;
        m_fnHash = fnHash;
    }
    ///析构函数
    ~UnistorCache(){
        m_bExit = true;
        if (m_commitThreadId){
            if (m_writeCache){
                ///唤醒commit线程并等待其退出
                m_writeCache->wakeupCommitThread();
                ::pthread_join(m_commitThreadId, NULL);
            }
            m_commitThreadId = 0;
        }
        if (m_writeCache) delete m_writeCache;
        if (m_readCache) delete m_readCache;
    }
public:
    ///初始化化；0：成功；-1：失败
    int init(UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite,  ///<dirty数据开始flush的通知函数
        UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite,///<dirty key的写入通知函数
        UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite,///<dirty数据写入完毕的通知函数
        void*     context, ///<环境信息。此为存储引擎的指针
        float     fBucketRate=1.2, ///<read cache的hash bucket的比率
        char*      szErr2K=NULL ///<错误信息 
        );

    ///更新key。返回值：1：成功；0：cache满了，需要写入；-1：cache错误，-2：没有写cache
    int updateKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key的长度
        char const* szData, ///<data
        CWX_UINT32 uiDataLen, ///<data的床单
        CWX_UINT32 uiOldExpire, ///<key的先前expire值，用于控制expire索引
        bool  bCache, ///<是否将数据在read cache中cache。
        bool& bInWriteCache ///<数据是否在write cache中存在
        );

    ///删除key。返回值：1：成功；0：cache满了，需要写入；-1：cache错误
    int delKey(char const* szKey,///<key的名字
        CWX_UINT32 unKeyLen, ///<key的长度
        CWX_UINT32 uiOldExpire, ///<key的先前expire值，用于控制expire索引
        bool& bInWriteCache ///<数据是否在write cache中存在
        );

    ///cache一个key。
    void cacheKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        char const* szData, ///<key的data
        CWX_UINT32 uiDataLen, ///<data的长度
        bool bNotExist  ///<是否只有当key不存在的时候才能cache。此防止【读】覆盖了【写】。
        );

    ///获取一个key，包括写cache、读cache。返回值：1：获取一个；0：不存在；-1：buf空间太小
    int getKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<data的buf
        CWX_UINT32& uiDataLen, ///<传入data buf的大小，返回data的大小
        bool& bDel, ///<key是否已经删除
        bool& bReadCache ///<是否在read cache中存在
        );

    ///从写cache获取一个key。返回值：1：获取一个；0：不存在；-1：buf空间太小
    int getWriteKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<data的buf
        CWX_UINT32& uiDataLen, ///<传入data buf的大小，返回data的大小
        bool& bDel ///<key是否已经删除
        );

    ///获取下一个写cache中的key。返回值：1：获取一个；0：不存在；-1：buf空间太小
    int nextWriteKey(char const* szBeginKey, ///<开始key。若为NULL则从开始开始
        CWX_UINT16 unBeginKeyLen, ///<开始key的长度
        bool bAsc, ///<是否升序
        char* szKey, ///<返回key的buf
        CWX_UINT16& unKeyLen, ///<传入key的buf大小，返回key的大小
        char* szData, ///<返回data的buf
        CWX_UINT32& uiDataLen, ///<传入data的buf大小，返回data的大小
        bool& bDel ///<key是否已经删除
        );

    //commit write cache中的数据。返回值：-1: failure, 0: success
    int commit(CWX_UINT64 ullSid, ///<commit时的当前binlog sid值
        void* userData, ///<用户的其他数据
        char* szErr2K=NULL ///<commit失败时的错误消息
        );

    ///设置write thread的id
    void setWriteThread(pthread_t const& writer = ::pthread_self()){
        m_writeThreadId = writer;
    }

    ///是否开启write cache
    inline bool isEnableWriteCache() const{
        return m_writeCache?true:false;
    }
    
    ///是否开始read cache
    inline bool isEnableReadCache() const{
        return m_readCache?true:false;
    }
    
    ///是否有效
    inline bool isValid() const{
        return m_bValid;
    }
    
    ///获取前一次commit的sid
    inline CWX_UINT64 getPrevCommitSid() const{
        return m_writeCache?m_writeCache->getPrevCommitSid():0;
    }
    
    ///获取错误信息
    inline char const* getErrMsg() const{
        return m_szErr2K;
    }
    
    ///停止cache。其本质是停止dirty 数据的commit线程。
    inline void stop(){
        m_bExit = true;
        if (m_commitThreadId){
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
    }
    
    ///获取写cache的key的数量
    inline CWX_UINT32 getWriteCacheKeyNum() const{
        return m_writeCache?m_writeCache->m_keyIndex->size():0;
    }
    
    ///获取当前写cache占用的空间大小
    inline CWX_UINT32 getWriteCacheUsedSize() const{
        if (m_writeCache){
            if (m_writeCache->m_bFirstHalf){
                return m_writeCache->m_uiPos;
            }else{
                return m_writeCache->m_uiPos - m_writeCache->m_uiSplitHalfPos;
            }
        }
        return 0;
    }

    ///获取cache数据使用的内存空间大小
    inline unsigned long int getUsedSize() const{
        return m_readCache?m_readCache->getUsedSize():0;
    }

    ///获取cache数据的数据空间大小
    inline unsigned long int getUsedCapacity() const{
        return m_readCache?m_readCache->getUsedCapacity():0;
    }

    ///获取cache数据的数据大小
    inline unsigned long int getUsedDataSize() const{
        return m_readCache?m_readCache->getUsedDataSize():0;
    }

    ///获取free的内存空间大小
    inline unsigned long int getFreeSize() const{
        return m_readCache?m_readCache->getFreeSize():0;
    }

    ///获取free的数据空间容量大小
    inline unsigned long int getFreeCapacity() const{
        return m_readCache?m_readCache->getFreeCapacity():0;
    }

    ///获取cache的key的数量
    inline CWX_UINT32 getCachedKeyCount() const{
        return m_readCache?m_readCache->getCachedKeyCount():0;
    }

    ///获取cache使用的内存块数量
    inline unsigned long int getCachedItemCount() const{
        return m_readCache?m_readCache->getCachedItemCount():0;
    }

    ///获取空闲的内存块数量
    inline unsigned long int getFreeItemCount() const{
        return m_readCache?m_readCache->getFreeItemCount():0;
    }

    ///获取最大可使用内存的数量
    inline unsigned long int maxSize( void ) const{
        return m_readCache?m_readCache->maxSize():0;
    }
    ///获取cache最大key的数量
    inline CWX_UINT32 getMaxCacheKeyNum() const{
        return m_readCache?m_readCache->getMaxCacheKeyNum():0;
    }

private:
    ///dirty数据的commit线程的main function
    static void* commitThreadMain(void* cache /*UnistorCache对象*/);
private:
    CWX_UINT32                      m_uiWriteCacheByte; ///<write cache的空间大小
    CWX_UINT64                      m_ullReadCacheByte; ///<read cache的空间大小
    CWX_UINT32                      m_uiReadCacheKeyNum; ///<read cache的key的数量
    UnistorWriteCache*              m_writeCache;  ///<写cache
    UnistorReadCacheEx2*            m_readCache; ///<读cache            
    CwxRwLock                       m_writeCacheRwLock; ///<写cache的读写锁
    CwxRwLock                       m_readCacheRwLock; ///<读cache的读写锁
    volatile bool                   m_bExit; ///<是否推出
    pthread_t                       m_writeThreadId; ///<写线程的线程id
    pthread_t                       m_commitThreadId; ///<dirty数据的commit线程的线程id
    volatile bool                   m_bValid; ///<cache是否有效
    char                            m_szErr2K[2048]; ///<cache出错时的错误消息
    UNISTOR_KEY_CMP_EQUAL_FN        m_fnEqual; ///<存储引擎的key相等的比较函数
    UNISTOR_KEY_CMP_LESS_FN         m_fnLess; ///<存储引擎的key小于的比较函数
    UNISTOR_KEY_HASH_FN             m_fnHash; ///<存储引擎的key的hash值的计算函数
};

#endif
