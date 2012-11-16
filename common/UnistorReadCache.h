#ifndef __UNISTOR_READ_CACHE_H__
#define __UNISTOR_READ_CACHE_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

///读cache的数据
struct UnistorReadCacheItem{
public:
    ///构造函数
    UnistorReadCacheItem(){
        m_prev = NULL;
        m_next = NULL;
        m_uiDataLen = 0;
        m_uiCapacity = 0;
        m_unKeyLen = 0;
    }
    ///析构函数
    ~UnistorReadCacheItem(){
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
        return m_uiCapacity + sizeof(UnistorReadCacheItem);
    }
    ///比较两个key是否相等
    bool operator == (UnistorReadCacheItem const& item) const{
        return m_fnEqual(m_szBuf, m_unKeyLen, item.m_szBuf, item.m_unKeyLen);
    }
    ///获取key的hash值
    inline size_t hash() const{
        return m_fnHash(m_szBuf, m_unKeyLen);
    }
public:
    ///获取容量
    inline static CWX_UINT32 calBufCapacity(CWX_UINT16 unKeyLen,///<key的长度
        CWX_UINT32 uiDataLen, ///<data的长度
        CWX_UINT16 unAlign ///<对齐字节数
        )
    {
        if (0 == unAlign)  return uiDataLen + unKeyLen;
        CWX_UINT32 uiCapacity = (unKeyLen + uiDataLen + unAlign - 1)/unAlign;
        uiCapacity *= unAlign;
        return uiCapacity;
    }
private:
    friend class UnistorReadCache;
private:
    UnistorReadCacheItem*  m_prev; ///前一个
    UnistorReadCacheItem*  m_next; ///下一个
    CWX_UINT32             m_uiDataLen;///<data的长度
    CWX_UINT32             m_uiCapacity;///<空间的容量
    CWX_UINT16             m_unKeyLen; ///<key的长度
    char                   m_szBuf[0]; ///<数据的指针

    static UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key相等的比较函数
    static UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<key小于的比较函数
    static UNISTOR_KEY_HASH_FN          m_fnHash; ///<key的hash值的计算函数
}__attribute__ ((__packed__));


///读cache对象，为LRU cache
class UnistorReadCache{
private:
    typedef hash_map<UnistorReadCacheItem*, UnistorReadCacheItem*, CwxPointHash<UnistorReadCacheItem>, CwxPointEqual<UnistorReadCacheItem> > _MAP;///<key的索引类型
    typedef _MAP::iterator _MAP_ITERATOR;///<key的索引iterator
private:
    UnistorReadCacheItem*    m_chain_head; ///<lru key's chain head
    UnistorReadCacheItem*    m_chain_tail;///<lru key's chain tail
    CWX_UINT32 const         m_max_cache_num; ///<cache key的最多数量
    unsigned long int        m_size; ///<used size
    CWX_UINT32               m_count; ///<cached key count
    unsigned long int        m_max_size; ///<max key size
    _MAP                     m_index; ///<key's index
    CwxRwLock*               m_lock; ///<lru cache's lock
    CWX_UINT16 const         m_unAlign; ///<align值
    UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key相等的比较函数
    UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<key小于的比较函数
    UNISTOR_KEY_HASH_FN          m_fnHash; ///<key的hash值的计算函数
public:
    /**
    @brief 构造函数，设置LRU CACHE的内存及KEY的可能最大数量。
    @param [in] size LRU CACHE的内存总量
    @param [in] count LRU CACHE的KEY的可能最大值
    @param [in] bLock 是否多线程安全，内部进行锁同步
    */
    UnistorReadCache(unsigned long int size, ///<空间大小
        CWX_UINT32 count, ///<key的数量
        UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key相等比较函数
        UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less的比较函数
        UNISTOR_KEY_HASH_FN    fnHash, ///<key的hash函数
        CWX_UINT16 unAlign=32, ///<对齐字节数
        CwxRwLock* lock=NULL ///<锁
        ):m_max_cache_num(count), m_max_size(size), m_index((CWX_UINT32)(count*1.2)),m_unAlign(unAlign)
    {
        m_chain_head = NULL;
        m_chain_tail = NULL;
        m_size = 0;
        m_count =0;
        m_lock = lock;
        UnistorReadCacheItem::m_fnEqual = m_fnEqual = fnEqual;
        UnistorReadCacheItem::m_fnLess = m_fnLess = fnLess;
        UnistorReadCacheItem::m_fnHash = m_fnHash = fnHash;        
    }
    ///析构函数
    ~UnistorReadCache(){
        while(m_chain_head){
            m_chain_tail = m_chain_head->m_next;
            free(m_chain_head);
            m_chain_head = m_chain_tail;
        }
    }
public:
    ///获取内存使用量
    inline unsigned long int size( void ){
        CwxReadLockGuard<CwxRwLock> lock(m_lock);
        return m_size; 
    }

    ///获取cache的key的数量
    inline CWX_UINT32 count( void ){
        CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
        return m_count; 
    }
    
    ///获取索引中对象的数量
    inline CWX_UINT32 long mapSize(){
        CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
        return m_index.size(); 
    }
    
    ///获取最大可使用内存的数量
    inline unsigned long int maxSize( void ){
        CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
        return m_max_size;
    }
    
    ///清空CACHE    
    void clear( void ){
        CwxWriteLockGuard<CwxRwLock> lock(this->m_lock);
        m_index.clear();
        while(m_chain_head){
            m_chain_tail = m_chain_head->m_next;
            free(m_chain_head);
            m_chain_head = m_chain_tail;
        }
        m_chain_head = NULL;
        m_chain_tail = NULL;
        m_size = 0;
        m_count =0;
    }

    ///检查key在CACHE中是否存在。返回值：true：存在；false：不存在
    inline bool exist(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        )
    {
        char key_buf[sizeof(UnistorReadCacheItem) + UnistorReadCacheItem::calBufCapacity(unKeyLen , 0, 0)];
        UnistorReadCacheItem* key = (UnistorReadCacheItem*)key_buf;
        memcpy(key->m_szBuf, szKey, unKeyLen);
        key->m_unKeyLen = unKeyLen;
        {
            CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
            return m_index.find(key) != m_index.end();
        }
    }

    ///将key移到LRU cache的开始位置，防止换出
    inline void touch(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        )
    {
        char key_buf[sizeof(UnistorReadCacheItem) + UnistorReadCacheItem::calBufCapacity(unKeyLen , 0, 0)];
        UnistorReadCacheItem* key = (UnistorReadCacheItem*)key_buf;
        memcpy(key->m_szBuf, szKey, unKeyLen);
        key->m_unKeyLen = unKeyLen;
        {
            CwxWriteLockGuard<CwxRwLock> lock(this->m_lock);
            _MAP_ITERATOR miter = m_index.find(key);
            if( miter == m_index.end() ) return ;
            _touch(miter->second);
        }
    }

    /**
    @brief 获取一个KEY的data。
    @param [in] szKey 要获取kye
    @param [in] bTouch 若szKey存在，是否将此szKey的移到LRU CACHE的头。
    @return NULL：szKey不存在；否则为szKey的data
    */
    inline UnistorReadCacheItem* fetch(char const* szKey,
        CWX_UINT16 unKeyLen,
        bool bTouch = true)
    {
        char key_buf[sizeof(UnistorReadCacheItem) + UnistorReadCacheItem::calBufCapacity(unKeyLen , 0, 0)];
        UnistorReadCacheItem* key = (UnistorReadCacheItem*)key_buf;
        memcpy(key->m_szBuf, szKey, unKeyLen);
        key->m_unKeyLen = unKeyLen;
        {
            CwxWriteLockGuard<CwxRwLock> lock(this->m_lock);
            _MAP_ITERATOR miter = m_index.find(key);
            if( miter == m_index.end() ) return NULL;
            if( bTouch) _touch( miter->second );
            return miter->second;
        }
    }    

    /**
    @brief 往LRU CACHE中插入一个KEY。
    @param [in] szKey 要插入的KEY
    @param [in] unKeyLen 要插入的KEY的长度
    @param [in] szData 要插入Key的data。
    @param [in] uiDataLen 要插入Key的data的长度。
    @return void
    */
    inline void insert(char const* szKey,
        CWX_UINT16 unKeyLen,
        char const* szData,
        CWX_UINT32 uiDataLen)
    {
        CwxWriteLockGuard<CwxRwLock> lock(this->m_lock);
        char key_buf[sizeof(UnistorReadCacheItem) + UnistorReadCacheItem::calBufCapacity(unKeyLen , 0, 0)];
        UnistorReadCacheItem* key = (UnistorReadCacheItem*)key_buf;
        memcpy(key->m_szBuf, szKey, unKeyLen);
        key->m_unKeyLen = unKeyLen;
        _MAP_ITERATOR miter = this->m_index.find(key);
        CWX_UINT32 uiCacacity = UnistorReadCacheItem::calBufCapacity(unKeyLen , uiDataLen, m_unAlign);
        if( miter != m_index.end()){
            if (miter->second->m_uiCapacity >= uiCacacity){
                memcpy(miter->second->m_szBuf + miter->second->m_unKeyLen, szData, uiDataLen);
                miter->second->m_uiDataLen = uiDataLen;
                _touch(miter->second);
                return;
            }
            _remove(miter);
        }

        UnistorReadCacheItem* item = (UnistorReadCacheItem*)malloc(sizeof(UnistorReadCacheItem) + uiCacacity);
        memcpy(item->m_szBuf, szKey, unKeyLen);
        memcpy(item->m_szBuf + unKeyLen, szData, uiDataLen);
        item->m_unKeyLen = unKeyLen;
        item->m_uiDataLen = uiDataLen;
        item->m_uiCapacity = uiCacacity;
        // Check to see if we need to remove an element due to exceeding max_size
        while((m_size + item->size() > m_max_size) || (m_count > m_max_cache_num)){
            // Remove the last element.
            miter = this->m_index.find(m_chain_tail);
            this->_remove( miter);
        }
        m_size += item->size();
        m_count ++;        
        //add to list
        item->m_prev = NULL;
        item->m_next = m_chain_head;
        if (m_chain_head == NULL){
            m_chain_head = m_chain_tail = item;
        }else{
            m_chain_head->m_prev = item;
            m_chain_head = item;
        }
        //add to map
        this->m_index[item] = item;
    }

    ///从LRU cache中删除一个szKey
    inline void remove( char const* szKey, CWX_UINT16 unKeyLen){
        char key_buf[sizeof(UnistorReadCacheItem) + UnistorReadCacheItem::calBufCapacity(unKeyLen , 0, 0)];
        UnistorReadCacheItem* key = (UnistorReadCacheItem*)key_buf;
        memcpy(key->m_szBuf, szKey, unKeyLen);
        key->m_unKeyLen = unKeyLen;
        CwxWriteLockGuard<CwxRwLock> lock(this->m_lock);
        _MAP_ITERATOR miter = m_index.find(key);
        if( miter == m_index.end() ) return;
        _remove(miter);
    }
private:

    ///不带锁的touch操作
    inline void _touch(UnistorReadCacheItem* data ){
        if (data->m_prev == NULL) //the head
            return;
        if (data->m_next == NULL){// the tail
            m_chain_tail = data->m_prev;
            m_chain_tail->m_next = NULL;
        }else{
            data->m_prev->m_next = data->m_next;
            data->m_next->m_prev = data->m_prev;
        }
        data->m_prev = NULL;
        data->m_next = m_chain_head;
        m_chain_head->m_prev = data;
        m_chain_head = data;
    }

    ///不带锁的remove操作
    inline void _remove(_MAP_ITERATOR& miter )
    {
        UnistorReadCacheItem* data = miter->second;
        m_index.erase(miter);
        this->m_size -= data->size();
        this->m_count --;
        if (data == m_chain_head){
            if (data == m_chain_tail){
                m_chain_head = m_chain_tail = NULL;
                this->m_size = 0;
                this->m_count = 0;
            }else{
                m_chain_head = data->m_next;
                m_chain_head->m_prev = NULL;
            }
        }else if (data == m_chain_tail){
            m_chain_tail = data->m_prev;
            m_chain_tail->m_next = NULL;
        }else{
            data->m_prev->m_next = data->m_next;
            data->m_next->m_prev = data->m_prev;
        }
        free(data);
    }
};

#endif
