#ifndef __UNISTOR_READ_CACHE_EX2_H__
#define __UNISTOR_READ_CACHE_EX2_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"


///读cache的数据
struct UnistorReadCacheItemEx2{
public:
    enum{
        UNISTOR_MEM_ALIGN_SIZE = 16, ///<空间对齐大小
        UNISTOR_MEM_ALIGN_BIT  = 4,  ///<空间对齐的bit
        UNISTOR_MEM_ALIGN_MASK = 0xF ///<空间对齐的mask
    };
public:
    ///构造函数
    UnistorReadCacheItemEx2(){
        m_prev = NULL;
        m_next = NULL;
        m_hashNext = NULL;
        m_uiDataLen = 0;
        m_unKeyLen = 0;
    }
    ///析构函数
    ~UnistorReadCacheItemEx2(){
    }
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
    ///获取key的大小
    inline CWX_UINT32 size() const{
        return calKeySize(m_unKeyLen, m_uiDataLen);
    }
    ///获取容量
    inline CWX_UINT32 capacity() const{
        return calKeySize(m_unKeyLen, m_uiDataLen) - sizeof(UnistorReadCacheItemEx2);
    }
    ///获取key的index
    inline CWX_UINT32 index() const{
        return calKeyIndex(m_unKeyLen, m_uiDataLen);
    }
public:
    ///获取容量
    inline static CWX_UINT32 calKeySize(CWX_UINT16 unKeyLen,///<key的长度
        CWX_UINT32 uiDataLen ///<data的长度
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
    ///获取可以所属的index
    inline static CWX_UINT32 calKeyIndex(CWX_UINT16 unKeyLen,///<key的长度
        CWX_UINT32 uiDataLen ///<data的长度
        )
    {
        CWX_UINT32 uiCapacity = unKeyLen + uiDataLen + sizeof(UnistorReadCacheItemEx2);
        if (uiCapacity & UNISTOR_MEM_ALIGN_MASK){
            return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT)+1;
        }
        return (uiCapacity >>=UNISTOR_MEM_ALIGN_BIT);
    }
    ///获取所需要的分组数量
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
    UnistorReadCacheItemEx2*  m_prev; ///<LRU链表的前一个
    UnistorReadCacheItemEx2*  m_next; ///<LRU链表的下一个
    UnistorReadCacheItemEx2*  m_hashNext; ///<hash链表的下一个
    CWX_UINT32                m_uiDataLen;///<data的长度
    CWX_UINT16                m_unKeyLen; ///<key的长度
    char                      m_szBuf[0]; ///<数据的指针

    static UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key相等的比较函数
    static UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<key小于的比较函数
    static UNISTOR_KEY_HASH_FN          m_fnHash; ///<key的hash值的计算函数
}__attribute__ ((__packed__));


class UnistorReadCacheItemPinEx2{
public:
    UnistorReadCacheItemPinEx2(){
        memset(this, 0x00, sizeof(UnistorReadCacheItemPinEx2));
    }
public:
    UnistorReadCacheItemEx2*     m_usedHead; ///<使用的头
    UnistorReadCacheItemEx2*     m_usedTail; ///<使用的尾
    UnistorReadCacheItemEx2*     m_freeHead; ///<空闲的头
    CWX_UINT32                   m_uiUsedNum; ///<使用的数量
    CWX_UINT32                   m_uiFreeNum; ///<空闲的数量
};

///读cache对象，为LRU cache
class UnistorReadCacheEx2{
private:
    enum{
        UNISTOR_READ_CACHE_SLOT_SIZE = 1024 * 1024 * 256, ///<内存槽的大小
        UNISTOR_READ_CACHE_MAX_ITEM_SIZE = UNISTOR_MAX_KV_SIZE, ///<最大的一条数据大小
        UNISTOR_READ_CACHE_CHAIN_LOCK_NUM = 4096  ///<链表锁的数量

    };
    ///hash查找的返回对象
    class UnistorReadCacheExIter{
    public:
        UnistorReadCacheExIter(){
            m_pFind = NULL;
            m_pPrev = NULL;
            m_uiHash = 0;
        }
    public:
        UnistorReadCacheItemEx2*   m_pFind; ///<发现的值
        UnistorReadCacheItemEx2*   m_pPrev; ///<前一个值
        CWX_UINT32                m_uiHash; ///<hash值
    };
public:
    /**
    @brief 构造函数，设置LRU CACHE的内存及KEY的可能最大数量。
    @param [in] size LRU CACHE的内存总量
    @param [in] count LRU CACHE的KEY的可能最大值
    @param [in] bLock 是否多线程安全，内部进行锁同步
    */
    UnistorReadCacheEx2(unsigned long int size, ///<空间大小
        CWX_UINT32 count, ///<key的数量
        UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key相等比较函数
        UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less的比较函数
        UNISTOR_KEY_HASH_FN    fnHash, ///<key的hash函数
        CwxRwLock* rwLock=NULL, ///<读写锁
        float  fBucketRate=1.2 ///<桶的比率
        );
    ///析构函数
    ~UnistorReadCacheEx2(){
        clear();
    }
public:
    ///初始化cache。返回值：0：成功，-1：失败
    int init(char* szErr2K);

    ///检查key在CACHE中是否存在。返回值：true：存在；false：不存在
    bool exist(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        );

    ///将key移到LRU cache的开始位置，防止换出
    void touch(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        );

    /**
    @brief 获取一个KEY的data。
    @param [in] szKey 要获取key
    @param [in] unKeyLen key的长度
    @param [in] bTouch 若szKey存在，是否将此szKey的移到LRU CACHE的头。
    @return NULL：szKey不存在；否则为szKey的data
    */
    UnistorReadCacheItemEx2* fetch(char const* szKey,
        CWX_UINT16 unKeyLen,
        bool bTouch = true);

    /**
    @brief 获取一个KEY的data。
    @param [in] szKey 要获取key
    @param [in] unKeyLen key的长度
    @param [in] szData 返回数据
    @param [in out] uiDataLen 传入szData的大小，返回实际的大小
    @param [in] bTouch 若szKey存在，是否将此szKey的移到LRU CACHE的头。
    @return 0：不存在；1：获取；-1：空间不足
    */
    int fetch(char const* szKey,
        CWX_UINT16 unKeyLen,
        char* szData,
        CWX_UINT32& uiDataLen,
        bool bTouch = true);
    /**
    @brief 往LRU CACHE中插入一个KEY。
    @param [in] szKey 要插入的KEY
    @param [in] unKeyLen 要插入的KEY的长度
    @param [in] szData 要插入Key的data。
    @param [in] uiDataLen 要插入Key的data的长度。
    @param [in] bNotExist 若为true则插入的key必须不存在，否则可以存在。
    @return void
    */
    void insert(char const* szKey,
        CWX_UINT16 unKeyLen,
        char const* szData,
        CWX_UINT32 uiDataLen,
        bool bNotExist);

    ///从LRU cache中删除一个szKey
    void remove( char const* szKey, CWX_UINT16 unKeyLen);


    ///获取cache数据使用的内存空间大小
    inline unsigned long int getUsedSize() const{
        return m_usedSize; ///<使用的空间大小
    }

    ///获取cache数据的数据空间大小
    inline unsigned long int getUsedCapacity() const{
        return m_usedCapacity; ///<使用的容量大小
    }

    ///获取cache数据的数据大小
    inline unsigned long int getUsedDataSize() const{
        return m_usedDataSize; ///<使用有效数据的大小
    }

    ///获取free的内存空间大小
    inline unsigned long int getFreeSize() const{
        return m_freeSize; ///<空闲的空间
    }

    ///获取free的数据空间容量大小
    inline unsigned long int getFreeCapacity() const{
        return m_freeCapacity; ///<空闲的容量
    }

    ///获取cache的key的数量
    inline CWX_UINT32 getCachedKeyCount() const{
        return m_cachedKeyCount; ///<cached key count
    }

    ///获取cache使用的内存块数量
    inline unsigned long int getCachedItemCount() const{
        return m_cachedItemCount; ///<cache使用的item数量
    }

    ///获取空闲的内存块数量
    inline unsigned long int getFreeItemCount() const{
        return m_freeItemCount; ///<空闲的item数量
    }

    ///获取最大可使用内存的数量
    inline unsigned long int maxSize( void ) const{
        return m_max_size;
    }

    ///获取cache最大key的数量
    inline CWX_UINT32 getMaxCacheKeyNum() const{
        return  m_max_cache_num; ///<cache key的最多数量
    }

    ///清空CACHE    
    void clear( void );

private:

    ///不带锁的touch操作
    void _touch(UnistorReadCacheItemEx2* data, CWX_UINT32 uiIndex);

    ///将item添加到free list，不修改容量空间
    void _addFreeList(CWX_UINT32 uiIndex, UnistorReadCacheItemEx2* item);

    ///不带锁的remove操作
    void _remove(UnistorReadCacheExIter const& iter);
 
    ///删除key
    inline void _removeKey(UnistorReadCacheExIter const& iter){
        if (iter.m_pPrev){
            CWX_ASSERT(iter.m_pPrev->m_hashNext == iter.m_pFind);
            iter.m_pPrev->m_hashNext = iter.m_pFind->m_hashNext;
        }else{
            CWX_ASSERT(m_hashArr[iter.m_uiHash] == iter.m_pFind);
            m_hashArr[iter.m_uiHash] = iter.m_pFind->m_hashNext;
        }
    }
    
    ///添加key
    void _addKey(CWX_UINT32 uiIndex, CWX_UINT32 uiSize, UnistorReadCacheItemEx2* item);

    ///获取key
    bool _findKey(UnistorReadCacheItemEx2 const* item, UnistorReadCacheExIter& iter);
private:
    UnistorReadCacheItemEx2**    m_hashArr; ///<hash的数组
    UnistorReadCacheItemPinEx2*  m_chainArr; ///<链表的数组
    CWX_UINT32 const          m_bucket_num; ///<hash桶的数量
    CWX_UINT32 const          m_max_cache_num; ///<cache key的最多数量
    unsigned long int         m_max_size; ///<max key size
    char**                    m_cacheBufArr; ///<cache buf的数组
    CWX_UINT32                m_cacheBufArrSize; ///<cache buf数组的大小
    CWX_UINT32                m_cacheBufArrIndex; ///<当前分配内存的数组的索引
    CWX_UINT32                m_cacheBufArrPos;  ///<当前分配buf的pos

    volatile unsigned long int  m_usedSize; ///<使用的空间大小
    volatile unsigned long int  m_usedCapacity; ///<使用的容量大小
    volatile unsigned long int  m_usedDataSize; ///<使用有效数据的大小

    volatile unsigned long int  m_freeSize; ///<空闲的空间
    volatile unsigned long int  m_freeCapacity; ///<空闲的容量

    volatile CWX_UINT32         m_cachedKeyCount; ///<cached key count
    volatile unsigned long int  m_cachedItemCount; ///<cache使用的item数量
    volatile unsigned long int  m_freeItemCount; ///<空闲的item数量

    CwxRwLock*                  m_rwLock; ///<lru cache 读写锁
    CwxMutexLock*               m_chainMutexArr[UNISTOR_READ_CACHE_CHAIN_LOCK_NUM]; ///<lru cache 的排他锁
    UNISTOR_KEY_CMP_EQUAL_FN    m_fnEqual; ///<key相等的比较函数
    UNISTOR_KEY_CMP_LESS_FN     m_fnLess; ///<key小于的比较函数
    UNISTOR_KEY_HASH_FN         m_fnHash; ///<key的hash值的计算函数


};

#endif
