#ifndef __UNISTOR_READ_CACHE_EX2_H__
#define __UNISTOR_READ_CACHE_EX2_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"


///��cache������
struct UnistorReadCacheItemEx2{
public:
    enum{
        UNISTOR_MEM_ALIGN_SIZE = 16, ///<�ռ�����С
        UNISTOR_MEM_ALIGN_BIT  = 4,  ///<�ռ�����bit
        UNISTOR_MEM_ALIGN_MASK = 0xF ///<�ռ�����mask
    };
public:
    ///���캯��
    UnistorReadCacheItemEx2(){
        m_prev = NULL;
        m_next = NULL;
        m_hashNext = NULL;
        m_uiDataLen = 0;
        m_unKeyLen = 0;
    }
    ///��������
    ~UnistorReadCacheItemEx2(){
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
    ///��ȡkey�Ĵ�С
    inline CWX_UINT32 size() const{
        return calKeySize(m_unKeyLen, m_uiDataLen);
    }
    ///��ȡ����
    inline CWX_UINT32 capacity() const{
        return calKeySize(m_unKeyLen, m_uiDataLen) - sizeof(UnistorReadCacheItemEx2);
    }
    ///��ȡkey��index
    inline CWX_UINT32 index() const{
        return calKeyIndex(m_unKeyLen, m_uiDataLen);
    }
public:
    ///��ȡ����
    inline static CWX_UINT32 calKeySize(CWX_UINT16 unKeyLen,///<key�ĳ���
        CWX_UINT32 uiDataLen ///<data�ĳ���
        )
    {
        CWX_UINT32 uiCapacity = unKeyLen + uiDataLen + sizeof(UnistorReadCacheItemEx2);
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
        CWX_UINT32 uiCapacity = unKeyLen + uiDataLen + sizeof(UnistorReadCacheItemEx2);
        if (uiCapacity & UNISTOR_MEM_ALIGN_MASK){
            return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT)+1;
        }
        return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT);
    }
    ///��ȡ����Ҫ�ķ�������
    inline static CWX_UINT32 calMaxIndexNum(CWX_UINT32 uiMaxKeyValue){
        CWX_UINT32 uiCapacity = uiMaxKeyValue + sizeof(UnistorReadCacheItemEx2);
        if (uiCapacity & UNISTOR_MEM_ALIGN_MASK){
            return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT)+1;
        }
        return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT);
    }
private:
    friend class UnistorReadCacheEx2;
private:
    UnistorReadCacheItemEx2*  m_prev; ///<LRU�����ǰһ��
    UnistorReadCacheItemEx2*  m_next; ///<LRU�������һ��
    UnistorReadCacheItemEx2*  m_hashNext; ///<hash�������һ��
    CWX_UINT32                m_uiDataLen;///<data�ĳ���
    CWX_UINT16                m_unKeyLen; ///<key�ĳ���
    char                      m_szBuf[0]; ///<���ݵ�ָ��

    static UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key��ȵıȽϺ���
    static UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<keyС�ڵıȽϺ���
    static UNISTOR_KEY_HASH_FN          m_fnHash; ///<key��hashֵ�ļ��㺯��
}__attribute__ ((__packed__));


class UnistorReadCacheItemPinEx2{
public:
    UnistorReadCacheItemPinEx2(){
        memset(this, 0x00, sizeof(UnistorReadCacheItemPinEx2));
    }
public:
    UnistorReadCacheItemEx2*     m_usedHead; ///<ʹ�õ�ͷ
    UnistorReadCacheItemEx2*     m_usedTail; ///<ʹ�õ�β
    UnistorReadCacheItemEx2*     m_freeHead; ///<���е�ͷ
    CWX_UINT32                   m_uiUsedNum; ///<ʹ�õ�����
    CWX_UINT32                   m_uiFreeNum; ///<���е�����
};

///��cache����ΪLRU cache
class UnistorReadCacheEx2{
private:
    enum{
        UNISTOR_READ_CACHE_SLOT_SIZE = 1024 * 1024 * 256, ///<�ڴ�۵Ĵ�С
        UNISTOR_READ_CACHE_MAX_ITEM_SIZE = UNISTOR_MAX_KV_SIZE, ///<����һ�����ݴ�С
        UNISTOR_READ_CACHE_CHAIN_LOCK_NUM = 4096  ///<������������

    };
    ///hash���ҵķ��ض���
    class UnistorReadCacheExIter{
    public:
        UnistorReadCacheExIter(){
            m_pFind = NULL;
            m_pPrev = NULL;
            m_uiHash = 0;
        }
    public:
        UnistorReadCacheItemEx2*   m_pFind; ///<���ֵ�ֵ
        UnistorReadCacheItemEx2*   m_pPrev; ///<ǰһ��ֵ
        CWX_UINT32                m_uiHash; ///<hashֵ
    };
public:
    /**
    @brief ���캯��������LRU CACHE���ڴ漰KEY�Ŀ������������
    @param [in] size LRU CACHE���ڴ�����
    @param [in] count LRU CACHE��KEY�Ŀ������ֵ
    @param [in] bLock �Ƿ���̰߳�ȫ���ڲ�������ͬ��
    */
    UnistorReadCacheEx2(unsigned long int size, ///<�ռ��С
        CWX_UINT32 count, ///<key������
        UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key��ȱȽϺ���
        UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less�ıȽϺ���
        UNISTOR_KEY_HASH_FN    fnHash, ///<key��hash����
        CwxRwLock* rwLock=NULL, ///<��д��
        float  fBucketRate=1.2 ///<Ͱ�ı���
        );
    ///��������
    ~UnistorReadCacheEx2(){
        clear();
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
    @param [in] bTouch ��szKey���ڣ��Ƿ񽫴�szKey���Ƶ�LRU CACHE��ͷ��
    @return NULL��szKey�����ڣ�����ΪszKey��data
    */
    UnistorReadCacheItemEx2* fetch(char const* szKey,
        CWX_UINT16 unKeyLen,
        bool bTouch = true);

    /**
    @brief ��ȡһ��KEY��data��
    @param [in] szKey Ҫ��ȡkey
    @param [in] unKeyLen key�ĳ���
    @param [in] szData ��������
    @param [in out] uiDataLen ����szData�Ĵ�С������ʵ�ʵĴ�С
    @param [in] bTouch ��szKey���ڣ��Ƿ񽫴�szKey���Ƶ�LRU CACHE��ͷ��
    @return 0�������ڣ�1����ȡ��-1���ռ䲻��
    */
    int fetch(char const* szKey,
        CWX_UINT16 unKeyLen,
        char* szData,
        CWX_UINT32& uiDataLen,
        bool bTouch = true);
    /**
    @brief ��LRU CACHE�в���һ��KEY��
    @param [in] szKey Ҫ�����KEY
    @param [in] unKeyLen Ҫ�����KEY�ĳ���
    @param [in] szData Ҫ����Key��data��
    @param [in] uiDataLen Ҫ����Key��data�ĳ��ȡ�
    @param [in] bNotExist ��Ϊtrue������key���벻���ڣ�������Դ��ڡ�
    @return void
    */
    void insert(char const* szKey,
        CWX_UINT16 unKeyLen,
        char const* szData,
        CWX_UINT32 uiDataLen,
        bool bNotExist);

    ///��LRU cache��ɾ��һ��szKey
    void remove( char const* szKey, CWX_UINT16 unKeyLen);


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

    ///���CACHE    
    void clear( void );

private:

    ///��������touch����
    void _touch(UnistorReadCacheItemEx2* data, CWX_UINT32 uiIndex);

    ///��item��ӵ�free list�����޸������ռ�
    void _addFreeList(CWX_UINT32 uiIndex, UnistorReadCacheItemEx2* item);

    ///��������remove����
    void _remove(UnistorReadCacheExIter const& iter);
 
    ///ɾ��key
    inline void _removeKey(UnistorReadCacheExIter const& iter){
        if (iter.m_pPrev){
            CWX_ASSERT(iter.m_pPrev->m_hashNext == iter.m_pFind);
            iter.m_pPrev->m_hashNext = iter.m_pFind->m_hashNext;
        }else{
            CWX_ASSERT(m_hashArr[iter.m_uiHash] == iter.m_pFind);
            m_hashArr[iter.m_uiHash] = iter.m_pFind->m_hashNext;
        }
    }
    
    ///���key
    void _addKey(CWX_UINT32 uiIndex, CWX_UINT32 uiSize, UnistorReadCacheItemEx2* item);

    ///��ȡkey
    bool _findKey(UnistorReadCacheItemEx2 const* item, UnistorReadCacheExIter& iter);
private:
    UnistorReadCacheItemEx2**    m_hashArr; ///<hash������
    UnistorReadCacheItemPinEx2*  m_chainArr; ///<���������
    CWX_UINT32 const          m_bucket_num; ///<hashͰ������
    CWX_UINT32 const          m_max_cache_num; ///<cache key���������
    unsigned long int         m_max_size; ///<max key size
    char**                    m_cacheBufArr; ///<cache buf������
    CWX_UINT32                m_cacheBufArrSize; ///<cache buf����Ĵ�С
    CWX_UINT32                m_cacheBufArrIndex; ///<��ǰ�����ڴ�����������
    CWX_UINT32                m_cacheBufArrPos;  ///<��ǰ����buf��pos

    volatile unsigned long int  m_usedSize; ///<ʹ�õĿռ��С
    volatile unsigned long int  m_usedCapacity; ///<ʹ�õ�������С
    volatile unsigned long int  m_usedDataSize; ///<ʹ����Ч���ݵĴ�С

    volatile unsigned long int  m_freeSize; ///<���еĿռ�
    volatile unsigned long int  m_freeCapacity; ///<���е�����

    volatile CWX_UINT32         m_cachedKeyCount; ///<cached key count
    volatile unsigned long int  m_cachedItemCount; ///<cacheʹ�õ�item����
    volatile unsigned long int  m_freeItemCount; ///<���е�item����

    CwxRwLock*                  m_rwLock; ///<lru cache ��д��
    CwxMutexLock*               m_chainMutexArr[UNISTOR_READ_CACHE_CHAIN_LOCK_NUM]; ///<lru cache ��������
    UNISTOR_KEY_CMP_EQUAL_FN    m_fnEqual; ///<key��ȵıȽϺ���
    UNISTOR_KEY_CMP_LESS_FN     m_fnLess; ///<keyС�ڵıȽϺ���
    UNISTOR_KEY_HASH_FN         m_fnHash; ///<key��hashֵ�ļ��㺯��


};

#endif
