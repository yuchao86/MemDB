#ifndef __CWX_LRU_CACHE_H__
#define __CWX_LRU_CACHE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxLruCache.h
@brief LRU Cache��ģ���ࡣ
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxGlobalMacro.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "CwxTypePoolEx.h"

CWINUX_BEGIN_NAMESPACE

/**
@class CwxLruCacheKeyData
@brief LRU cache�����ݶ����塣
*/
template<typename KEY, typename DATA>
class CwxLruCacheKeyData
{
public:
    CwxLruCacheKeyData(KEY key, DATA data)
        :m_prev(NULL),m_next(NULL),m_key(key), m_data(data)
    {
    }
public:
    CwxLruCacheKeyData<KEY, DATA>*     m_prev;///<����LRU cache�����ǰ��ָ��
    CwxLruCacheKeyData<KEY, DATA>*     m_next;///<����LRU cache����ĺ���ָ��
    KEY                              m_key; ///<LRU cache��KEY
    DATA                             m_data;///<LRU cache��DATA
};

/**
@class CwxLruCacheKeyMethod
@brief ʵ��LRU cache��KEY��hash, equal�����ݴ�С����ĺ�������
*/
template <typename KEY, typename DAT, typename HASH, typename EQUAL, typename KSIZE, typename DSIZE>
class CwxLruCacheKeyMethod
{
public:
    ///KEY��hash���󷽷�
    size_t operator()(KEY const* item) const 
    {
        return m_hash(*item);
    }
    ///KEY��equal���󷽷�
    bool operator()(KEY const* item1, KEY const* item2) const
    {
        return m_equal(*item1, *item2);
    }
    ///key��data��sizeof����
    size_t size(CwxLruCacheKeyData<KEY, DATA> const& item) const
    {
        return m_ksize(item.m_key) + m_dsize(item.m_data) + 2 * sizeof(void*);
    }
    ///key��data��sizeof����
    size_t size(KEY const& key, DATA const& data) const
    {
        return m_ksize(key) + m_dsize(data) + 2 * sizeof(void*);
    }
private:
    HASH  m_hash;///<key��hash������
    EQUAL m_equal;///<key��equal������
    KSIZE m_ksize;///<key��sizeof������
    DSIZE m_dsize;///<data��sizeof������
};


/**
@class CwxLruCache
@brief LRU cache��ģ���ࡣ����ģ������ĺ������£�
KEY: lru cache��Key����
DATA��lru cache��Data��������
HASH��KEY hash-code����ķ�������Ĭ��Ϊhash\<KEY\>
EQUAL�� KEY ��ȱȽϵķ�������Ĭ��Ϊequal_to\<KEY\>
KSIZE��key���ڴ��С�ļ��㷽������Ĭ��ΪCwxSize(KEY)
DSIZE��data���ڴ��С�ļ��㷽������Ĭ��ΪCwxSize(KEY)
*/

template<typename KEY, typename DATA, typename HASH=hash<KEY>, typename EQUAL=equal_to<KEY>, typename KSIZE=CwxSize<KEY>, typename DSIZE=CwxSize<DATA> >
class CwxLruCache
{
public:
    typedef CwxLruCacheKeyData<KEY, DATA>  _CACHE_DATA; ///< cache��������
    typedef CwxLruCacheKeyMethod<KEY, DATA, HASH, EQUAL, KSIZE, DSIZE> _FUNC; ///<��������
    typedef hash_map<KEY const*, _CACHE_DATA*, _FUNC, _FUNC> _MAP;///<key����������
    typedef typename _MAP::iterator _MAP_ITERATOR;///<key������iterator
private:
    _CACHE_DATA*    m_chain_head; ///<lru key's chain head
    _CACHE_DATA*    m_chain_tail;///<lru key's chain tail
    unsigned long int       m_size; ///<used size
    CWX_UINT32      m_count; ///<cached key count
    unsigned long int      m_max_size; ///<max key size
    CwxTypePoolEx<_CACHE_DATA> m_kv_pool;///<mem pool
    _MAP            m_index; ///<key's index
    CwxMutexLock* 	 m_lock; ///<lru cache's lock
    _FUNC           m_func; ///<key's function object

public:
    /**
    @brief ���캯��������LRU CACHE���ڴ漰KEY�Ŀ������������
    @param [in] size LRU CACHE���ڴ�����
    @param [in] count LRU CACHE��KEY�Ŀ������ֵ
    @param [in] bLock �Ƿ���̰߳�ȫ���ڲ�������ͬ��
    */
    CwxLruCache(unsigned long int size, CWX_UINT32 count, bool bLock=true)
        :m_max_size( size ), m_kv_pool(count/20), m_index(count * 1.2)
    {
        m_chain_head = NULL;
        m_chain_tail = NULL;
        m_size = 0;
        m_count =0;
        if (bLock)
        {
            m_lock = new CwxMutexLock();
        }
        else
        {
            m_lock = NULL;
        }
    }
    ///��������
    ~CwxLruCache()
    {
        this->clear();
        if (m_lock)  delete m_lock;
        m_lock = NULL;
    }
public:
    ///��ȡ�ڴ�ʹ����
    inline unsigned long int size( void ) const
    {
        CwxMutexGuard lock(m_lock);
        return m_size; 
    }
    ///��ȡcache��key������
    inline CWX_UINT32 count( void ) const
    {
        CwxMutexGuard lock(this->m_lock);
        return m_count; 
    }
    ///��ȡ�ڴ���ж��������
    inline CWX_UINT32 poolCount() const
    {
        CwxMutexGuard lock(this->m_lock);
        return m_kv_pool.size(); 
    }
    ///��ȡ�����ж��������
    inline CWX_UINT32 long mapSize()
    {
        CwxMutexGuard lock(this->m_lock);
        return m_index.size(); 
    }
    ///��ȡ����ʹ���ڴ������
    inline unsigned long int maxSize( void ) const 
    {
        CwxMutexGuard lock(this->m_lock);
        return m_max_size;
    }
    ///���CACHE    
    void clear( void )
    {
        CwxMutexGuard lock(this->m_lock);
        m_index.clear();
        m_kv_pool.clear();
        m_chain_head = NULL;
        m_chain_tail = NULL;
        m_size = 0;
        m_count =0;
    }
    ///���key��CACHE���Ƿ����
    inline bool exist(KEY const&key ) const 
    {
        CwxMutexGuard lock(this->m_lock);
        return m_index.find( &key ) != m_index.end();
    }
    ///��key�Ƶ�LRU cache�Ŀ�ʼλ�ã���ֹ����
    inline void touch(KEY const &key )
    {
        CwxMutexGuard lock(this->m_lock);
        _MAP_ITERATOR miter = m_index.find( &key );
        if( miter == m_index.end() ) return ;
        _touch(miter->second);
    }
    /**
    @brief ��ȡһ��KEY��data��
    @param [in] key Ҫ��ȡDATA��KEY
    @param [in] bTouch ��KEY���ڣ��Ƿ񽫴�KEY���Ƶ�LRU CACHE��ͷ��
    @return NULL��KEY�����ڣ�����ΪKEY��data
    */
    inline DATA* fetch(KEY const &key, bool bTouch = true )
    {
        CwxMutexGuard lock(this->m_lock);
        _MAP_ITERATOR miter = m_index.find( &key );
        if( miter == m_index.end() )  return NULL;
        if( bTouch) _touch( miter->second );
        return miter->second;
    }    
    /**
    @brief ��LRU CACHE�в���һ��KEY��
    @param [in] key Ҫ�����KEY
    @param [in] data Ҫ����Key��data��
    @return void
    */
    inline void insert(KEY const &key, DATA &data )
    {
        size_t size=0;
        CwxMutexGuard lock(this->m_lock);
        _CACHE_DATA* pData = NULL;
        _MAP_ITERATOR miter = this->m_index.find( &key );
        if( miter != m_index.end() )
        {
            size = m_func.size(*miter->second);
            m_size =(m_size>size)?m_size-size:0;
            miter->second->m_data = data;
            miter->second->m_key = key;
            m_size += m_func.size(*miter->second);
            _touch(miter->second);
            return;
        }
        // Check to see if we need to remove an element due to exceeding max_size
        unsigned long size = m_func.size(key, data);
        while(m_size + size > m_max_size)
        {
            // Remove the last element.
            pData = m_chain_tail;
            miter = this->index_.find(&pData->m_key);
            this->_remove( miter);
            m_kv_pool.free(pData);
        }
        pData = m_kv_pool.malloc(_CACHE_DATA(key, data));
        m_size += m_func.size(key, data);
        m_count ++;        
        //add to list
        pData->m_prev = NULL;
        pData->m_next = m_chain_head;
        if (m_chain_head == NULL)
        {
            m_chain_head = m_chain_tail = pData;
        }
        else
        {
            m_chain_head->m_prev = pData;
            m_chain_head = pData;
        }
        //add to map
        this->m_index[&pData->m_key] = pData;
    }
    ///��LRU cache��ɾ��һ��KEY
    inline void remove( KEY const &key )
    {
        CwxMutexGuard lock(this->m_lock);
        _MAP_ITERATOR miter = m_index.find( &key );
        if( miter == index_.end() ) return;
        _CACHE_DATA* pData = miter->second;
        _remove(miter);
        m_kv_pool.free(pData);
    }
private:
    ///��������touch����
    inline void _touch(_CACHE_DATA* data )
    {
        if (data->m_prev == NULL) //the head
            return;
        if (data->m_next == NULL)
        {// the tail
            m_chain_tail = data->m_prev;
            m_chain_tail->m_next = NULL;
        }
        else
        {
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
        size_t size = 0;
        _CACHE_DATA* data = miter->second;
        m_index.erase(miter);
        size = m_func.size(data->m_key, data->m_data);
        this->m_size -= m_size>size?m_size-size:0;
        this->m_count --;
        if (data == m_chain_head)
        {
            if (data == m_chain_tail)
            {
                m_chain_head = m_chain_tail = NULL;
                this->m_size = 0;
                this->m_count = 0;
            }
            else
            {
                m_chain_head = data->m_next;
                m_chain_head->m_prev = NULL;
            }
        }
        else if (data == m_chain_tail)
        {
            m_chain_tail = data->m_prev;
            m_chain_tail->m_next = NULL;
        }else
        {
            data->m_prev->m_next = data->m_next;
            data->m_next->m_prev = data->m_prev;
        }
        data->m_prev=data->m_next = NULL;
    }

};


CWINUX_END_NAMESPACE
#include "CwxPost.h"
#endif

