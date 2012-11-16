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
///write cache��key item�����Լ��������ڴ棬ʹ��cache buf
struct UnistorWriteCacheItem{
public:
    ///���캯��
    UnistorWriteCacheItem(){
        m_uiDataLen = 0;
        m_uiCapacity = 0;
        m_ttOldExpire = 0;
        m_unKeyLen = 0;
        m_bDel = false;
    };
public:
    ///��ȡkey
    inline char const* getKey() const{
        return m_szBuf;
    }
    ///��ȡdata
    inline char const* getData() const{
        return m_szBuf + m_unKeyLen;
    }
    ///��ȡkey�ĳ���
    inline CWX_UINT16 getKeyLen() const{
        return m_unKeyLen;
    }
    ///��ȡdata�ĳ���
    inline CWX_UINT32 getDataLen() const{
        return m_uiDataLen;
    }
    ///��ȡ��ʱʱ��
    inline CWX_UINT32 getOldExpire() const{
        return m_ttOldExpire;
    }
    ///��ȡ�ռ��С
    inline CWX_UINT32 getCapacity() const{
        return m_uiCapacity;
    }
    ///�Ƿ�ɾ��
    inline bool isDel() const{
        return m_bDel;
    }
    ///��ȡkey�Ĵ�С
    inline CWX_UINT32 size() const{
        return m_uiCapacity + sizeof(UnistorWriteCacheItem);
    }
    ///��ȱȽϣ��Ƚϲ��ô洢�����key�ıȽϺ���
    bool operator == (UnistorWriteCacheItem const& item) const{
        return m_fnEqual(m_szBuf, m_unKeyLen, item.m_szBuf, item.m_unKeyLen);
    }
    ///С�ڱȽϣ��Ƚϲ��ô洢�����key�ıȽϺ���
    bool operator < (UnistorWriteCacheItem const& item) const{
        return m_fnLess(m_szBuf, m_unKeyLen, item.m_szBuf, item.m_unKeyLen)<0?true:false;
    }
public:
    ///��ȡ����
    inline static CWX_UINT32 calBufCapacity(CWX_UINT16 unKeyLen,///<key�ĳ���
        CWX_UINT32 uiDataLen, ///<data�ĳ���
        CWX_UINT16 unAlign ///<�����ֽ���
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
        //���밴��8����
        CWX_UINT16 unMod = (sizeof(UnistorWriteCacheItem) + uiCapacity)%8;
        if (unMod){
            uiCapacity += (8 - unMod);
        }
        return uiCapacity;
    }
private:
    friend class UnistorWriteCache;
private:
    CWX_UINT32                m_uiDataLen;  ///<data�ĳ���
    CWX_UINT32                m_uiCapacity; ///<�ռ������
    CWX_UINT32                m_ttOldExpire; ///<�ɵĳ�ʱʱ��
    CWX_UINT16                m_unKeyLen;   ///<key�ĳ���
    bool                      m_bDel;      ///<key�ĳ���
    char                      m_szBuf[0];     ///<key������
    static UNISTOR_KEY_CMP_EQUAL_FN m_fnEqual; ///<�洢�����key��ȵıȽϺ���
    static UNISTOR_KEY_CMP_LESS_FN  m_fnLess;   ///<�洢�����keyС�ڵıȽϺ���

}__attribute__ ((__packed__));;


class UnistorCache;
///дcache��ֻ֧��һ��д�̣߳�������߳�
///дcache�����������ڴ�ռ䣬��Ϊ�ϡ��°�����
///���ϰ���д����ʱ��ʹ���°���д��dirty thread�ᱻ֪ͨ���ϰ���������д��disk�������ֻ�
class UnistorWriteCache{
public:
    enum{
        UNISTOR_WRITE_CACHE_MIN_SIZE = 1024 * 1024 * 32 ///<дcache����С��С��Ϊ64M
    };
public:
    ///���캯��
    UnistorWriteCache(CWX_UINT32 uiCacheSize, ///<дcache�Ĵ�С
        UNISTOR_KEY_CMP_EQUAL_FN fnEqual, ///<key��ȵıȽϺ���
        UNISTOR_KEY_CMP_LESS_FN  fnLess,  ///<keyС�ڵıȽϺ���
        UNISTOR_KEY_HASH_FN      fnHash, ///<key��hash����
        CwxRwLock* lock=NULL, ///<��������ΪNULL��ʾ������
        CWX_UINT16 unAlign = 64 ///<�ռ�����ֽ���
        ):m_unAlign(unAlign)
    {
        m_buf = NULL;
        m_uiCacheByteSize = uiCacheSize;
        if (m_uiCacheByteSize < UNISTOR_WRITE_CACHE_MIN_SIZE) m_uiCacheByteSize = UNISTOR_WRITE_CACHE_MIN_SIZE;
        ///�ڴ�K����
        if (m_uiCacheByteSize%1024){
            m_uiCacheByteSize -= (m_uiCacheByteSize%1024);
            m_uiCacheByteSize += 1024;
        }
        ///���°���һ��8�ֽڶ���
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
    ///��������
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

    ///��ʼ��дcache������ֵ��0���ɹ���-1��ʧ��
    int init(UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite, ///<dirty���ݿ�ʼflush��֪ͨ����
        UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite, ////<dirty key��д��֪ͨ����
        UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite, ///<dirty����д����ϵ�֪ͨ����
        void*     context,  ///<������Ϣ����Ϊ�洢�����ָ��
        char* szErr2K=NULL
        );
    
    ///����һ��key������ֵ��1���ɹ���0��cache���ˣ���Ҫcommit
    int updateKey(char const* szKey, ///<key������
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char const* szData, ///<data
        CWX_UINT32 uiDataLen, ///<data�ĳ���
        CWX_UINT32 uiOldExpire, ///<����ǰkey��expireֵ����Ϊ��ά��expire������
        bool& bInWriteCache ///<�����Ƿ���write cache�д���
        );

    ///ɾ��һ��key������ֵ��1���ɹ���0��cache���ˣ���Ҫд��
    int delKey(char const* szKey, ///<key������
        CWX_UINT32 unKeyLen, ///<key�ĳ���
        CWX_UINT32 uiOldExpire, ///<����ǰkey��expireֵ����Ϊ��ά��expire������
        bool& bInWriteCache ///<�����Ƿ���write cache�д���
        );

    ///��ȡһ��key������ֵ��1����ȡһ����0�������ڣ�-1��buf�ռ�̫С
    int getKey(char const* szKey, ///<key������
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData,  ///<����data������buf
        CWX_UINT32& uiDataLen, ///<��������buf�Ŀռ��С������data�Ĵ�С
        bool& bDel  ///<key�Ƿ��Ѿ�ɾ��
        );

    ///��ȡ��һ��дcache�е�key������ֵ��1����ȡһ����0�������ڣ�-1��buf�ռ�̫С
    int nextKey(char const* szBeginKey, ///<��ʼkey����ΪNULL��ʾ��ָ��
        CWX_UINT16 unBeginKeyLen, ///<��ʼkey�ĳ���
        char* szKey,  ///<����key��buf
        CWX_UINT16& unKeyLen, ///<����key buf��С������key�ĳ���
        char* szData, ///<����data��buf
        CWX_UINT32& uiDataLen, ///<����data buf�Ĵ�С������data�ĳ���
        bool& bDel ///<key�Ƿ��Ѿ�ɾ��
        );

    ///��ȡǰһ��дcache�е�key������ֵ��1����ȡһ����0�������ڣ�-1��buf�ռ�̫С
    int prevKey(char const* szBeginKey,///<��ʼkey����ΪNULL��ʾ��ָ��
        CWX_UINT16 unBeginKeyLen,///<��ʼkey�ĳ���
        char* szKey,///<����key��buf
        CWX_UINT16& unKeyLen,///<����key buf��С������key�ĳ���
        char* szData,///<����data��buf
        CWX_UINT32& uiDataLen,///<����data buf�Ĵ�С������data�ĳ���
        bool& bDel///<key�Ƿ��Ѿ�ɾ��
        );

    ///�ύ���ݣ�����ֵ��0���ɹ���-1��ʧ��
    int commit(CWX_UINT64 ullSid, ///<commit��binlog��sidֵ��
        void* userData  ///<�û�����������
        );

    ///����commit�߳�
    void wakeupCommitThread(){
        if (m_pMutex){
            ::pthread_mutex_lock(m_pMutex);
            ::pthread_cond_broadcast(m_pCommitThreadWaitCond);
            ::pthread_mutex_unlock(m_pMutex);
        }
    }
    
    ///��ȡǰһ���ύ��sid
    inline CWX_UINT64 getPrevCommitSid() const{
        return m_ullPrevCommitSid;
    }

private:
    friend class UnistorCache;

    ///���key�Ƿ��ڵ�ǰ����д�ķ���
    inline bool _isInWriteCommitHalf(char const* szKey) const{
        return m_bFirstHalf?((szKey - m_buf) < m_uiSplitHalfPos):
            ((szKey - m_buf) >= m_uiSplitHalfPos);
    }
private:
    CWX_UINT16 const                m_unAlign; ///<������ֽ���
    char*                           m_buf; ///<д��cache buf
    CWX_UINT32                      m_uiCacheByteSize; ///<д��cache buf�Ŀռ��С
    volatile CWX_UINT32             m_uiSplitHalfPos; ///<�ڴ�ָ��
    volatile bool                   m_bFirstHalf; ///<�Ƿ�ǰʹ���ϰ���
    volatile CWX_UINT32             m_uiPos; ///<cacheд���ĵ�ǰλ��
    volatile CWX_UINT32             m_uiCommitBeginPos; ///<commit�Ŀ�ʼλ��
    volatile CWX_UINT32             m_uiCommitEndPos;  ///<commit�Ľ���λ��
    volatile CWX_UINT64             m_ullCommitSid; ///<�ύ��sid
    void*                           m_pCommitUserData; ///<�ύ��������
    volatile CWX_UINT64             m_ullPrevCommitSid; ///<ǰһ���ύ��sid
    pthread_mutex_t*	            m_pMutex;       ///<commitд�̵߳���
    pthread_cond_t*	                m_pCommitThreadWaitCond; ///<commit�߳�wait��condition
    pthread_cond_t*	                m_pWriteThreadWaitCond; ///<д�̵߳�wait condition
    bool				            m_bCommitThreadWait; ///<commit�߳��Ƿ���condition��wait
    bool				            m_bWriteThreadWait;  ///<write�߳��Ƿ���condition��wait
    UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN          m_fnBeginWrite; ///<��ʼд�ĺ���
    UNISTOR_WRITE_CACHE_WRITE_WRITE_FN          m_fnWrite; ///<д���ݵĺ���
    UNISTOR_WRITE_CACHE_WRITE_END_FN            m_fnEndWrite; ///<����д�ĺ���
    void*                                       m_context; ///<������context��Ϣ
    set<UnistorWriteCacheItem*, CwxPointLess<UnistorWriteCacheItem> >*  m_keyIndex;///<key����
    CwxRwLock*                      m_rwLock; ///<��д������ΪNULL��ʾû����
    UNISTOR_KEY_CMP_EQUAL_FN        m_fnEqual; ///<key��ȵıȽϺ���
    UNISTOR_KEY_CMP_LESS_FN         m_fnLess; ///<keyС�ڵıȽϺ���
    UNISTOR_KEY_HASH_FN             m_fnHash; ///<key��hashֵ�ļ��㺯��
};

///cacheϵͳ��ֻ֧��һ��д�̣߳�������߳�
class UnistorCache{
public:
    ///���캯��
    UnistorCache(CWX_UINT32 uiWriteCacheMBtye, ///<write cache�Ĵ�С����Ϊ0��ʾû��write cache
        CWX_UINT32 uiReadCacheMByte, ///<��cache�Ĵ�С����Ϊ0��ʾû��read cache
        CWX_UINT32 uiReadMaxCacheKeyNum, ///<��cache�����cache��Ŀ
        UNISTOR_KEY_CMP_EQUAL_FN fnEqual, ///<key��ȵıȽϺ���
        UNISTOR_KEY_CMP_LESS_FN  fnLess,  ///<keyС�ڵıȽϺ���
        UNISTOR_KEY_HASH_FN    fnHash     ///<key��hashֵ�ļ��㺯��
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
    ///��������
    ~UnistorCache(){
        m_bExit = true;
        if (m_commitThreadId){
            if (m_writeCache){
                ///����commit�̲߳��ȴ����˳�
                m_writeCache->wakeupCommitThread();
                ::pthread_join(m_commitThreadId, NULL);
            }
            m_commitThreadId = 0;
        }
        if (m_writeCache) delete m_writeCache;
        if (m_readCache) delete m_readCache;
    }
public:
    ///��ʼ������0���ɹ���-1��ʧ��
    int init(UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite,  ///<dirty���ݿ�ʼflush��֪ͨ����
        UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite,///<dirty key��д��֪ͨ����
        UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite,///<dirty����д����ϵ�֪ͨ����
        void*     context, ///<������Ϣ����Ϊ�洢�����ָ��
        float     fBucketRate=1.2, ///<read cache��hash bucket�ı���
        char*      szErr2K=NULL ///<������Ϣ 
        );

    ///����key������ֵ��1���ɹ���0��cache���ˣ���Ҫд�룻-1��cache����-2��û��дcache
    int updateKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char const* szData, ///<data
        CWX_UINT32 uiDataLen, ///<data�Ĵ���
        CWX_UINT32 uiOldExpire, ///<key����ǰexpireֵ�����ڿ���expire����
        bool  bCache, ///<�Ƿ�������read cache��cache��
        bool& bInWriteCache ///<�����Ƿ���write cache�д���
        );

    ///ɾ��key������ֵ��1���ɹ���0��cache���ˣ���Ҫд�룻-1��cache����
    int delKey(char const* szKey,///<key������
        CWX_UINT32 unKeyLen, ///<key�ĳ���
        CWX_UINT32 uiOldExpire, ///<key����ǰexpireֵ�����ڿ���expire����
        bool& bInWriteCache ///<�����Ƿ���write cache�д���
        );

    ///cacheһ��key��
    void cacheKey(char const* szKey, ///<key������
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char const* szData, ///<key��data
        CWX_UINT32 uiDataLen, ///<data�ĳ���
        bool bNotExist  ///<�Ƿ�ֻ�е�key�����ڵ�ʱ�����cache���˷�ֹ�����������ˡ�д����
        );

    ///��ȡһ��key������дcache����cache������ֵ��1����ȡһ����0�������ڣ�-1��buf�ռ�̫С
    int getKey(char const* szKey, ///<key������
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData, ///<data��buf
        CWX_UINT32& uiDataLen, ///<����data buf�Ĵ�С������data�Ĵ�С
        bool& bDel, ///<key�Ƿ��Ѿ�ɾ��
        bool& bReadCache ///<�Ƿ���read cache�д���
        );

    ///��дcache��ȡһ��key������ֵ��1����ȡһ����0�������ڣ�-1��buf�ռ�̫С
    int getWriteKey(char const* szKey, ///<key������
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData, ///<data��buf
        CWX_UINT32& uiDataLen, ///<����data buf�Ĵ�С������data�Ĵ�С
        bool& bDel ///<key�Ƿ��Ѿ�ɾ��
        );

    ///��ȡ��һ��дcache�е�key������ֵ��1����ȡһ����0�������ڣ�-1��buf�ռ�̫С
    int nextWriteKey(char const* szBeginKey, ///<��ʼkey����ΪNULL��ӿ�ʼ��ʼ
        CWX_UINT16 unBeginKeyLen, ///<��ʼkey�ĳ���
        bool bAsc, ///<�Ƿ�����
        char* szKey, ///<����key��buf
        CWX_UINT16& unKeyLen, ///<����key��buf��С������key�Ĵ�С
        char* szData, ///<����data��buf
        CWX_UINT32& uiDataLen, ///<����data��buf��С������data�Ĵ�С
        bool& bDel ///<key�Ƿ��Ѿ�ɾ��
        );

    //commit write cache�е����ݡ�����ֵ��-1: failure, 0: success
    int commit(CWX_UINT64 ullSid, ///<commitʱ�ĵ�ǰbinlog sidֵ
        void* userData, ///<�û�����������
        char* szErr2K=NULL ///<commitʧ��ʱ�Ĵ�����Ϣ
        );

    ///����write thread��id
    void setWriteThread(pthread_t const& writer = ::pthread_self()){
        m_writeThreadId = writer;
    }

    ///�Ƿ���write cache
    inline bool isEnableWriteCache() const{
        return m_writeCache?true:false;
    }
    
    ///�Ƿ�ʼread cache
    inline bool isEnableReadCache() const{
        return m_readCache?true:false;
    }
    
    ///�Ƿ���Ч
    inline bool isValid() const{
        return m_bValid;
    }
    
    ///��ȡǰһ��commit��sid
    inline CWX_UINT64 getPrevCommitSid() const{
        return m_writeCache?m_writeCache->getPrevCommitSid():0;
    }
    
    ///��ȡ������Ϣ
    inline char const* getErrMsg() const{
        return m_szErr2K;
    }
    
    ///ֹͣcache���䱾����ֹͣdirty ���ݵ�commit�̡߳�
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
    
    ///��ȡдcache��key������
    inline CWX_UINT32 getWriteCacheKeyNum() const{
        return m_writeCache?m_writeCache->m_keyIndex->size():0;
    }
    
    ///��ȡ��ǰдcacheռ�õĿռ��С
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

    ///��ȡcache����ʹ�õ��ڴ�ռ��С
    inline unsigned long int getUsedSize() const{
        return m_readCache?m_readCache->getUsedSize():0;
    }

    ///��ȡcache���ݵ����ݿռ��С
    inline unsigned long int getUsedCapacity() const{
        return m_readCache?m_readCache->getUsedCapacity():0;
    }

    ///��ȡcache���ݵ����ݴ�С
    inline unsigned long int getUsedDataSize() const{
        return m_readCache?m_readCache->getUsedDataSize():0;
    }

    ///��ȡfree���ڴ�ռ��С
    inline unsigned long int getFreeSize() const{
        return m_readCache?m_readCache->getFreeSize():0;
    }

    ///��ȡfree�����ݿռ�������С
    inline unsigned long int getFreeCapacity() const{
        return m_readCache?m_readCache->getFreeCapacity():0;
    }

    ///��ȡcache��key������
    inline CWX_UINT32 getCachedKeyCount() const{
        return m_readCache?m_readCache->getCachedKeyCount():0;
    }

    ///��ȡcacheʹ�õ��ڴ������
    inline unsigned long int getCachedItemCount() const{
        return m_readCache?m_readCache->getCachedItemCount():0;
    }

    ///��ȡ���е��ڴ������
    inline unsigned long int getFreeItemCount() const{
        return m_readCache?m_readCache->getFreeItemCount():0;
    }

    ///��ȡ����ʹ���ڴ������
    inline unsigned long int maxSize( void ) const{
        return m_readCache?m_readCache->maxSize():0;
    }
    ///��ȡcache���key������
    inline CWX_UINT32 getMaxCacheKeyNum() const{
        return m_readCache?m_readCache->getMaxCacheKeyNum():0;
    }

private:
    ///dirty���ݵ�commit�̵߳�main function
    static void* commitThreadMain(void* cache /*UnistorCache����*/);
private:
    CWX_UINT32                      m_uiWriteCacheByte; ///<write cache�Ŀռ��С
    CWX_UINT64                      m_ullReadCacheByte; ///<read cache�Ŀռ��С
    CWX_UINT32                      m_uiReadCacheKeyNum; ///<read cache��key������
    UnistorWriteCache*              m_writeCache;  ///<дcache
    UnistorReadCacheEx2*            m_readCache; ///<��cache            
    CwxRwLock                       m_writeCacheRwLock; ///<дcache�Ķ�д��
    CwxRwLock                       m_readCacheRwLock; ///<��cache�Ķ�д��
    volatile bool                   m_bExit; ///<�Ƿ��Ƴ�
    pthread_t                       m_writeThreadId; ///<д�̵߳��߳�id
    pthread_t                       m_commitThreadId; ///<dirty���ݵ�commit�̵߳��߳�id
    volatile bool                   m_bValid; ///<cache�Ƿ���Ч
    char                            m_szErr2K[2048]; ///<cache����ʱ�Ĵ�����Ϣ
    UNISTOR_KEY_CMP_EQUAL_FN        m_fnEqual; ///<�洢�����key��ȵıȽϺ���
    UNISTOR_KEY_CMP_LESS_FN         m_fnLess; ///<�洢�����keyС�ڵıȽϺ���
    UNISTOR_KEY_HASH_FN             m_fnHash; ///<�洢�����key��hashֵ�ļ��㺯��
};

#endif
