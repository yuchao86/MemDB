#ifndef __UNISTOR_STORE_MEM_CACHE_H__
#define __UNISTOR_STORE_MEM_CACHE_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"


///��cache������
struct UnistorStoreMemCacheItem{
public:
    enum{
        UNISTOR_MEM_ALIGN_SIZE = 16, ///<�ռ�����С
        UNISTOR_MEM_ALIGN_BIT  = 4,  ///<�ռ�����bit
        UNISTOR_MEM_ALIGN_MASK = 0xF ///<�ռ�����mask
    };
    enum{
        UNISTOR_MEM_ITEM_STATE_UNMALLOC = 0, ///<itemδ����״̬�����µ��ڴ�ȫ��û�з���
        UNISTOR_MEM_ITEM_STATE_FREE = 1, ///<item���ڿ���״̬
        UNISTOR_MEM_ITEM_STATE_USED = 2 ///<item����ʹ��״̬
    };
public:
    ///���캯��
    UnistorStoreMemCacheItem(){
        m_prev = NULL;
        m_next = NULL;
        m_hashNext = NULL;
        m_uiDataLen = 0;
        m_unKeyLen = 0;
        m_uiExpire = 0;
        m_uiKeyIndex = 0;
        m_ucState = UNISTOR_MEM_ITEM_STATE_UNMALLOC;
    }
    ///��������
    ~UnistorStoreMemCacheItem(){
    }
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
    inline CWX_UINT32 getExpire() const{
        return m_uiExpire;
    }
    ///���ó�ʱʱ��
    inline void setExpire(CWX_UINT32 uiExpire){
        m_uiExpire = uiExpire;
    }
    ///��ȡ�Ƿ�����ʹ��
    inline bool isUsed() const{
        return UNISTOR_MEM_ITEM_STATE_USED == m_ucState;
    }
    ///�Ƿ���δ����״̬
    inline bool isUnmalloc() const{
        return UNISTOR_MEM_ITEM_STATE_UNMALLOC == m_ucState;
    }
    ///�Ƿ��ڿ���״̬
    inline bool isFree() const{
        return UNISTOR_MEM_ITEM_STATE_FREE == m_ucState;
    }
    ///����ʹ�ñ��
    inline void setState(CWX_UINT8 ucState){
        m_ucState = ucState;
    }
    ///��ȡkey�Ĵ�С
    inline CWX_UINT32 size() const{
        return (m_uiKeyIndex<<UNISTOR_MEM_ALIGN_BIT);
    }
    ///��ȡ����
    inline CWX_UINT32 capacity() const{
        return (m_uiKeyIndex<<UNISTOR_MEM_ALIGN_BIT) - sizeof(UnistorStoreMemCacheItem);
    }
    ///��ȡkey��index
    inline CWX_UINT32 index() const{
        return m_uiKeyIndex;
    }
public:
    ///��ȡ����
    inline static CWX_UINT32 calKeySize(CWX_UINT16 unKeyLen,///<key�ĳ���
        CWX_UINT32 uiDataLen ///<data�ĳ���
        )
    {
        CWX_UINT32 uiCapacity = unKeyLen + uiDataLen + sizeof(UnistorStoreMemCacheItem);
        if (uiCapacity & UNISTOR_MEM_ALIGN_MASK){
            uiCapacity >>=UNISTOR_MEM_ALIGN_BIT;
            uiCapacity++;
            uiCapacity <<=UNISTOR_MEM_ALIGN_BIT;
        }
        return uiCapacity;
    }
    ///��ȡ����������index
    inline static CWX_UINT32 calKeyIndex(CWX_UINT16 unKeyLen,///<key�ĳ���
        CWX_UINT32 uiDataLen ///<data�ĳ���
        )
    {
        CWX_UINT32 uiCapacity = unKeyLen + uiDataLen + sizeof(UnistorStoreMemCacheItem);
        if (uiCapacity & UNISTOR_MEM_ALIGN_MASK){
            return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT)+1;
        }
        return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT);
    }
    ///��ȡ����Ҫ�ķ�������
    inline static CWX_UINT32 calMaxIndexNum(CWX_UINT32 uiMaxKeyValue){
        CWX_UINT32 uiCapacity = uiMaxKeyValue + sizeof(UnistorStoreMemCacheItem);
        if (uiCapacity & UNISTOR_MEM_ALIGN_MASK){
            return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT)+1;
        }
        return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT);
    }
private:
    friend class UnistorStoreMemCache;
private:
    UnistorStoreMemCacheItem*  m_prev; ///<LRU�����ǰһ��
    UnistorStoreMemCacheItem*  m_next; ///<LRU�������һ��
    UnistorStoreMemCacheItem*  m_hashNext; ///<hash�������һ��
    CWX_UINT32                m_uiDataLen;///<data�ĳ���
    CWX_UINT32                m_uiExpire; ///<��ʱ��ʱ��
    CWX_UINT32                m_uiKeyIndex; ///<key������
    CWX_UINT16                m_unKeyLen; ///<key�ĳ���
    CWX_UINT8                 m_ucState; ///<�Ƿ���ʹ��
    char                      m_szBuf[0]; ///<���ݵ�ָ��
    static UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key��ȵıȽϺ���
    static UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<keyС�ڵıȽϺ���
    static UNISTOR_KEY_HASH_FN          m_fnHash; ///<key��hashֵ�ļ��㺯��
}__attribute__ ((__packed__));


class UnistorStoreMemCacheItemPin{
public:
    UnistorStoreMemCacheItemPin(){
        memset(this, 0x00, sizeof(UnistorStoreMemCacheItemPin));
    }
public:
    UnistorStoreMemCacheItem*     m_usedHead; ///<ʹ�õ�ͷ
    UnistorStoreMemCacheItem*     m_usedTail; ///<ʹ�õ�β
    UnistorStoreMemCacheItem*     m_freeHead; ///<���е�ͷ
    CWX_UINT32                    m_uiUsedNum; ///<ʹ�õ�����
    CWX_UINT32                    m_uiFreeNum; ///<���е�����
};

///��cache����ΪLRU cache
class UnistorStoreMemCache{
private:
    enum{
        UNISTOR_STORE_MEM_CACHE_SLOT_SIZE = 1024 * 1024 * 256, ///<�ڴ�۵Ĵ�С
        UNISTOR_STORE_MEM_CACHE_MAX_ITEM_SIZE = UNISTOR_MAX_KV_SIZE, ///<����һ�����ݴ�С
        UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM = 4096  ///<������������
    };
    ///hash���ҵķ��ض���
    class UnistorStoreMemCacheIter{
    public:
        UnistorStoreMemCacheIter(){
            m_pFind = NULL;
            m_pPrev = NULL;
            m_uiHash = 0;
        }
    public:
        UnistorStoreMemCacheItem*   m_pFind; ///<���ֵ�ֵ
        UnistorStoreMemCacheItem*   m_pPrev; ///<ǰһ��ֵ
        CWX_UINT32                  m_uiHash; ///<hashֵ
    };
public:
    /**
    @brief ���캯��������LRU CACHE���ڴ漰KEY�Ŀ������������
    @param [in] size LRU CACHE���ڴ�����
    @param [in] count LRU CACHE��KEY�Ŀ������ֵ
    @param [in] bLock �Ƿ���̰߳�ȫ���ڲ�������ͬ��
    */
    UnistorStoreMemCache(unsigned long int size, ///<�ռ��С
        CWX_UINT32 count, ///<key������
        CWX_UINT32 maxPerCheckNum, ///<ÿ�μ��key���������
        UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key��ȱȽϺ���
        UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less�ıȽϺ���
        UNISTOR_KEY_HASH_FN    fnHash, ///<key��hash����
        float  fBucketRate=1.2 ///<Ͱ�ı���
        );
    ///��������
    ~UnistorStoreMemCache(){
        free();
    }
public:
    ///��ʼ��cache������ֵ��0���ɹ���-1��ʧ��
    int init(char* szErr2K);

    ///���key��CACHE���Ƿ���ڡ�����ֵ��true�����ڣ�false��������
    bool exist(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key�ĳ���
        );

    ///��key�Ƶ�LRU cache�Ŀ�ʼλ�ã���ֹ����
    void touch(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key�ĳ���
        );

    /**
    @brief ��ȡһ��KEY��data��
    @param [in] szKey Ҫ��ȡkey
    @param [in] unKeyLen key�ĳ���
    @param [in] szData ��������
    @param [in out] uiDataLen ����szData�Ĵ�С������ʵ�ʵĴ�С
    @param [out] uiExpire key��ʧЧʱ��
    @param [in] uiCurExpireTime ��ǰ��ʧЧʱ��
    @param [in] bTouch ��szKey���ڣ��Ƿ񽫴�szKey���Ƶ�LRU CACHE��ͷ��
    @return 0�������ڣ�1����ȡ��-1���ռ䲻��
    */
    int fetch(char const* szKey,
        CWX_UINT16 unKeyLen,
        char* szData,
        CWX_UINT32& uiDataLen,
        CWX_UINT32& uiExpire,
        CWX_UINT32  uiCurExpireTime,
        bool bTouch = true);
    /**
    @brief ��LRU CACHE�в���һ��KEY��
    @param [in] szKey Ҫ�����KEY
    @param [in] unKeyLen Ҫ�����KEY�ĳ���
    @param [in] szData Ҫ����Key��data��
    @param [in] uiDataLen Ҫ����Key��data�ĳ��ȡ�
    @param [in] uiExpire ��ʱʱ�䣬��expire��ΪNULL��ʾ���ó�ʱʱ�䣬����������key��ʾû�г�ʱʱ�䡢�����ڱ�ʾ���޸ġ�
    @return void
    */
    void insert(char const* szKey,
        CWX_UINT16 unKeyLen,
        char const* szData,
        CWX_UINT32 uiDataLen,
        CWX_UINT32* uiExpire=NULL);

    ///��LRU cache��ɾ��һ��szKey����ָ����expire����ֻ��expire��ȵ�ʱ���ɾ��
    void remove( char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT32 uiExpire=0);

    ///��ⳬʱ��Ԫ�أ�key��expireʱ�䳬��uiCurTime��ȫ��ʧЧ
    void checkExpire(CWX_UINT32 uiCurTime, CWX_UINT32 uiBatchNum);

    ///��ȡcache����ʹ�õ��ڴ�ռ��С
    inline unsigned long int getUsedSize() const{
        return m_usedSize; ///<ʹ�õĿռ��С
    }

    ///��ȡcache���ݵ����ݿռ��С
    inline unsigned long int getUsedCapacity() const{
        return m_usedCapacity; ///<ʹ�õ�������С
    }

    ///��ȡcache���ݵ����ݴ�С
    inline unsigned long int getUsedDataSize() const{
        return m_usedDataSize; ///<ʹ����Ч���ݵĴ�С
    }

    ///��ȡfree���ڴ�ռ��С
    inline unsigned long int getFreeSize() const{
        return m_freeSize; ///<���еĿռ�
    }

    ///��ȡfree�����ݿռ�������С
    inline unsigned long int getFreeCapacity() const{
        return m_freeCapacity; ///<���е�����
    }

    ///��ȡcache��key������
    inline CWX_UINT32 getCachedKeyCount() const{
        return m_cachedKeyCount; ///<cached key count
    }

    ///��ȡcacheʹ�õ��ڴ������
    inline unsigned long int getCachedItemCount() const{
        return m_cachedItemCount; ///<cacheʹ�õ�item����
    }

    ///��ȡ���е��ڴ������
    inline unsigned long int getFreeItemCount() const{
        return m_freeItemCount; ///<���е�item����
    }

    ///��ȡ����ʹ���ڴ������
    inline unsigned long int maxSize( void ) const{
        return m_max_size;
    }

    ///��ȡcache���key������
    inline CWX_UINT32 getMaxCacheKeyNum() const{
        return  m_max_cache_num; ///<cache key���������
    }

    ///�ͷ�CACHE    
    void free( void );
    ///���cache
    void reset(void);
private:

    ///��������touch����
    void _touch(UnistorStoreMemCacheItem* data, CWX_UINT32 uiIndex);

    ///��item��ӵ�free list�����޸������ռ�
    void _addFreeList(CWX_UINT32 uiIndex, UnistorStoreMemCacheItem* item);

    ///��������remove����
    void _remove(UnistorStoreMemCacheIter const& iter);
 
    ///ɾ��key
    inline void _removeKey(UnistorStoreMemCacheIter const& iter){
        if (iter.m_pPrev){
            CWX_ASSERT(iter.m_pPrev->m_hashNext == iter.m_pFind);
            iter.m_pPrev->m_hashNext = iter.m_pFind->m_hashNext;
        }else{
            CWX_ASSERT(m_hashArr[iter.m_uiHash] == iter.m_pFind);
            m_hashArr[iter.m_uiHash] = iter.m_pFind->m_hashNext;
        }
    }
    
    ///���key
    void _addKey(CWX_UINT32 uiIndex, CWX_UINT32 uiSize, UnistorStoreMemCacheItem* item);

    ///��ȡkey
    bool _findKey(UnistorStoreMemCacheItem const* item, UnistorStoreMemCacheIter& iter);
private:
    UnistorStoreMemCacheItem**    m_hashArr; ///<hash������
    UnistorStoreMemCacheItemPin*  m_chainArr; ///<���������
    CWX_UINT32 const          m_bucket_num; ///<hashͰ������
    CWX_UINT32 const          m_max_cache_num; ///<cache key���������
    unsigned long int         m_max_size; ///<max key size
    char**                    m_cacheBufArr; ///<cache buf������
    CWX_UINT32                m_cacheBufArrSize; ///<cache buf����Ĵ�С
    volatile CWX_UINT32       m_cacheBufArrIndex; ///<��ǰ�����ڴ�����������
    volatile CWX_UINT32       m_cacheBufArrPos;  ///<��ǰ����buf��pos

    CWX_UINT32                 m_uiMaxPerCheckExpireNum; ///<ÿ�μ�ⳬʱ��item������
    CWX_UINT32                 m_uiCurCheckSlotIndex; ///<��ǰ����slot������
    CWX_UINT32                 m_uiCurCheckSlotPos; ///<��ǰ����slot��pos
    UnistorStoreMemCacheItem** m_arrExpiredItem;   ///<�Ѿ���ʱ��item������
    CWX_UINT32                 m_uiExpireNum; ///<���μ����ĳ�ʱitem������

    volatile unsigned long int  m_usedSize; ///<ʹ�õĿռ��С
    volatile unsigned long int  m_usedCapacity; ///<ʹ�õ�������С
    volatile unsigned long int  m_usedDataSize; ///<ʹ����Ч���ݵĴ�С

    volatile unsigned long int  m_freeSize; ///<���еĿռ�
    volatile unsigned long int  m_freeCapacity; ///<���е�����

    volatile CWX_UINT32         m_cachedKeyCount; ///<cached key count
    volatile unsigned long int  m_cachedItemCount; ///<cacheʹ�õ�item����
    volatile unsigned long int  m_freeItemCount; ///<���е�item����

    CwxRwLock                    m_rwLock; ///<lru cache ��д��
    CwxMutexLock*                m_chainMutexArr[UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM]; ///<lru cache ��������
    UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key��ȵıȽϺ���
    UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<keyС�ڵıȽϺ���
    UNISTOR_KEY_HASH_FN          m_fnHash; ///<key��hashֵ�ļ��㺯��


};

#endif
