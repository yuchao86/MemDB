#ifndef __UNISTOR_READ_CACHE_H__
#define __UNISTOR_READ_CACHE_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

///��cache������
struct UnistorReadCacheItem{
public:
    ///���캯��
    UnistorReadCacheItem(){
        m_prev = NULL;
        m_next = NULL;
        m_uiDataLen = 0;
        m_uiCapacity = 0;
        m_unKeyLen = 0;
    }
    ///��������
    ~UnistorReadCacheItem(){
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
        return m_uiCapacity + sizeof(UnistorReadCacheItem);
    }
    ///�Ƚ�����key�Ƿ����
    bool operator == (UnistorReadCacheItem const& item) const{
        return m_fnEqual(m_szBuf, m_unKeyLen, item.m_szBuf, item.m_unKeyLen);
    }
    ///��ȡkey��hashֵ
    inline size_t hash() const{
        return m_fnHash(m_szBuf, m_unKeyLen);
    }
public:
    ///��ȡ����
    inline static CWX_UINT32 calBufCapacity(CWX_UINT16 unKeyLen,///<key�ĳ���
        CWX_UINT32 uiDataLen, ///<data�ĳ���
        CWX_UINT16 unAlign ///<�����ֽ���
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
    UnistorReadCacheItem*  m_prev; ///ǰһ��
    UnistorReadCacheItem*  m_next; ///��һ��
    CWX_UINT32             m_uiDataLen;///<data�ĳ���
    CWX_UINT32             m_uiCapacity;///<�ռ������
    CWX_UINT16             m_unKeyLen; ///<key�ĳ���
    char                   m_szBuf[0]; ///<���ݵ�ָ��

    static UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key��ȵıȽϺ���
    static UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<keyС�ڵıȽϺ���
    static UNISTOR_KEY_HASH_FN          m_fnHash; ///<key��hashֵ�ļ��㺯��
}__attribute__ ((__packed__));


///��cache����ΪLRU cache
class UnistorReadCache{
private:
    typedef hash_map<UnistorReadCacheItem*, UnistorReadCacheItem*, CwxPointHash<UnistorReadCacheItem>, CwxPointEqual<UnistorReadCacheItem> > _MAP;///<key����������
    typedef _MAP::iterator _MAP_ITERATOR;///<key������iterator
private:
    UnistorReadCacheItem*    m_chain_head; ///<lru key's chain head
    UnistorReadCacheItem*    m_chain_tail;///<lru key's chain tail
    CWX_UINT32 const         m_max_cache_num; ///<cache key���������
    unsigned long int        m_size; ///<used size
    CWX_UINT32               m_count; ///<cached key count
    unsigned long int        m_max_size; ///<max key size
    _MAP                     m_index; ///<key's index
    CwxRwLock*               m_lock; ///<lru cache's lock
    CWX_UINT16 const         m_unAlign; ///<alignֵ
    UNISTOR_KEY_CMP_EQUAL_FN     m_fnEqual; ///<key��ȵıȽϺ���
    UNISTOR_KEY_CMP_LESS_FN      m_fnLess; ///<keyС�ڵıȽϺ���
    UNISTOR_KEY_HASH_FN          m_fnHash; ///<key��hashֵ�ļ��㺯��
public:
    /**
    @brief ���캯��������LRU CACHE���ڴ漰KEY�Ŀ������������
    @param [in] size LRU CACHE���ڴ�����
    @param [in] count LRU CACHE��KEY�Ŀ������ֵ
    @param [in] bLock �Ƿ���̰߳�ȫ���ڲ�������ͬ��
    */
    UnistorReadCache(unsigned long int size, ///<�ռ��С
        CWX_UINT32 count, ///<key������
        UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key��ȱȽϺ���
        UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less�ıȽϺ���
        UNISTOR_KEY_HASH_FN    fnHash, ///<key��hash����
        CWX_UINT16 unAlign=32, ///<�����ֽ���
        CwxRwLock* lock=NULL ///<��
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
    ///��������
    ~UnistorReadCache(){
        while(m_chain_head){
            m_chain_tail = m_chain_head->m_next;
            free(m_chain_head);
            m_chain_head = m_chain_tail;
        }
    }
public:
    ///��ȡ�ڴ�ʹ����
    inline unsigned long int size( void ){
        CwxReadLockGuard<CwxRwLock> lock(m_lock);
        return m_size; 
    }

    ///��ȡcache��key������
    inline CWX_UINT32 count( void ){
        CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
        return m_count; 
    }
    
    ///��ȡ�����ж��������
    inline CWX_UINT32 long mapSize(){
        CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
        return m_index.size(); 
    }
    
    ///��ȡ����ʹ���ڴ������
    inline unsigned long int maxSize( void ){
        CwxReadLockGuard<CwxRwLock> lock(this->m_lock);
        return m_max_size;
    }
    
    ///���CACHE    
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

    ///���key��CACHE���Ƿ���ڡ�����ֵ��true�����ڣ�false��������
    inline bool exist(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key�ĳ���
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

    ///��key�Ƶ�LRU cache�Ŀ�ʼλ�ã���ֹ����
    inline void touch(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key�ĳ���
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
    @brief ��ȡһ��KEY��data��
    @param [in] szKey Ҫ��ȡkye
    @param [in] bTouch ��szKey���ڣ��Ƿ񽫴�szKey���Ƶ�LRU CACHE��ͷ��
    @return NULL��szKey�����ڣ�����ΪszKey��data
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
    @brief ��LRU CACHE�в���һ��KEY��
    @param [in] szKey Ҫ�����KEY
    @param [in] unKeyLen Ҫ�����KEY�ĳ���
    @param [in] szData Ҫ����Key��data��
    @param [in] uiDataLen Ҫ����Key��data�ĳ��ȡ�
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

    ///��LRU cache��ɾ��һ��szKey
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

    ///��������touch����
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

    ///��������remove����
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
