#include "UnistorReadCacheEx.h"

UNISTOR_KEY_CMP_EQUAL_FN UnistorReadCacheItemEx::m_fnEqual = NULL; ///<key相等的比较函数
UNISTOR_KEY_CMP_LESS_FN UnistorReadCacheItemEx::m_fnLess = NULL; ///<key小于的比较函数
UNISTOR_KEY_HASH_FN UnistorReadCacheItemEx::m_fnHash = NULL; ///<key的hash值的计算函数

UnistorReadCacheEx::UnistorReadCacheEx(unsigned long int size, ///<空间大小
                   CWX_UINT32 count, ///<key的数量
                   UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key相等比较函数
                   UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less的比较函数
                   UNISTOR_KEY_HASH_FN    fnHash, ///<key的hash函数
                   CWX_UINT16 unAlign, ///<对齐字节数
                   CwxRwLock* rwLock, ///<读写锁
                   CwxMutexLock* mutex, ///<排他锁
                   float  fBucketRate ///<桶的比率
                   ):m_bucket_num((CWX_UINT32)((count *fBucketRate + 1)>count?(count *fBucketRate + 1):count)),
                   m_max_cache_num(count),
                   m_unAlign(unAlign)
{
    m_max_size = size;
    if (m_max_size % UNISTOR_READ_CACHE_SLOT_SIZE){
        m_max_size += (UNISTOR_READ_CACHE_SLOT_SIZE - (m_max_size %UNISTOR_READ_CACHE_SLOT_SIZE -1));
    }
    m_hashArr = NULL;
    m_cacheBufArr = NULL;
    m_cacheBufArrSize = m_max_size/UNISTOR_READ_CACHE_SLOT_SIZE;
    if (!m_cacheBufArrSize) m_cacheBufArrSize =1;
    m_cacheBufArrIndex = 0;
    m_cacheBufArrPos = 0;
    m_freeHead = NULL;
    m_freeTail = NULL;
    m_chain_head = NULL;
    m_chain_tail = NULL;
    m_usedSize = 0;
    m_usedCapacity = 0;
    m_usedDataSize = 0;
    m_freeSize = 0;
    m_freeCapacity = 0;
    m_cachedKeyCount=0;
    m_cachedItemCount = 0;
    m_freeItemCount=0;
    m_rwLock = rwLock;
    m_mutex = mutex;
    UnistorReadCacheItemEx::m_fnEqual = m_fnEqual = fnEqual;
    UnistorReadCacheItemEx::m_fnLess = m_fnLess = fnLess;
    UnistorReadCacheItemEx::m_fnHash = m_fnHash = fnHash;        
}
///初始化cache。返回值：0：成功，-1：失败
int UnistorReadCacheEx::init(char* szErr2K){
    clear();
    {
        CwxWriteLockGuard<CwxRwLock>  lock(m_rwLock);
        m_hashArr = new UnistorReadCacheItemEx*[m_bucket_num];
        if (!m_hashArr){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc hash arr, bucket number:%u", m_bucket_num);
            return -1;
        }
        memset(m_hashArr, 0x00, sizeof(UnistorReadCacheItemEx*)*m_bucket_num);
        m_cacheBufArr = new char*[m_cacheBufArrSize];
        if (!m_cacheBufArr){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc cache slot array, number:%u", m_cacheBufArrSize);
            return -1;
        }
        CWX_UINT32 i=0;
        for (i=0; i<m_cacheBufArrSize; i++){
            m_cacheBufArr[i] = NULL;
        }
        for (i=0; i<m_cacheBufArrSize; i++){
            m_cacheBufArr[i] = new char[UNISTOR_READ_CACHE_SLOT_SIZE];
            if (!m_cacheBufArr[i]){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc cache slot, no:%u, size=%u", i, UNISTOR_READ_CACHE_SLOT_SIZE);
                return -1;
            }
        }

    }
    return 0;
}

void UnistorReadCacheEx::insert(char const* szKey,
            CWX_UINT16 unKeyLen,
            char const* szData,
            CWX_UINT32 uiDataLen,
            bool bNotExist)
{
    ///key的长度不能为0
    if (!unKeyLen) return;

    char key_buf[sizeof(UnistorReadCacheItemEx) + UnistorReadCacheItemEx::calBufCapacity(unKeyLen , 0, 0)];
    UnistorReadCacheItemEx* key = (UnistorReadCacheItemEx*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;

    CWX_UINT32 uiKeyCacacity = UnistorReadCacheItemEx::calBufCapacity(unKeyLen , uiDataLen, m_unAlign);
    ///若key的空间大小大于slot，则不cache
    if (uiKeyCacacity + sizeof(UnistorReadCacheItemEx) > UNISTOR_READ_CACHE_MAX_ITEM_SIZE) return;

    {
        CwxWriteLockGuard<CwxRwLock> lock(this->m_rwLock);
        ///如果key存在，首先删除
        if(_findKey(key, iter)){
            if (bNotExist) return;
            _remove(iter);
        }

        ///添加新Key
        ///优先使用free key
        if (m_freeCapacity < uiKeyCacacity){///<没有free的空间
            if ((m_cacheBufArrIndex + 1 < m_cacheBufArrSize) ||
                (m_cacheBufArrPos + uiKeyCacacity + sizeof(UnistorReadCacheItemEx) < UNISTOR_READ_CACHE_SLOT_SIZE))
            {
                if (m_cacheBufArrPos + uiKeyCacacity + sizeof(UnistorReadCacheItemEx) > UNISTOR_READ_CACHE_SLOT_SIZE)
                {///<使用新slot
                    ///首先将剩余的空间加入cache
                    if (m_cacheBufArrPos + uiKeyCacacity + sizeof(UnistorReadCacheItemEx) - UNISTOR_READ_CACHE_SLOT_SIZE > 8192){
                        ///分配新空间
                        UnistorReadCacheItemEx* item = (UnistorReadCacheItemEx*)(m_cacheBufArr[m_cacheBufArrIndex] + m_cacheBufArrPos);
                        item->m_uiCapacity = UNISTOR_READ_CACHE_SLOT_SIZE - m_cacheBufArrPos - sizeof(UnistorReadCacheItemEx); ///<设置item的空间容量
                        item->m_next = NULL;
                        item->m_prev = NULL;
                        item->m_hashNext = NULL;
                        item->m_child = NULL;
                        _addFreeList(item);
                        m_freeSize += item->m_uiCapacity + sizeof(UnistorReadCacheItemEx);
                        m_freeCapacity += item->m_uiCapacity;
                        m_freeItemCount++;
                        m_cacheBufArrPos = UNISTOR_READ_CACHE_SLOT_SIZE; ///<空间已经满了
                    }
                    m_cacheBufArrIndex ++;
                    m_cacheBufArrPos = 0;
                }
                ///分配新空间
                UnistorReadCacheItemEx* item = (UnistorReadCacheItemEx*)(m_cacheBufArr[m_cacheBufArrIndex] + m_cacheBufArrPos);
                item->m_uiCapacity = uiKeyCacacity; ///<设置item的空间容量
                item->m_next = NULL;
                item->m_prev = NULL;
                item->m_hashNext = NULL;
                item->m_child = NULL;
                _addFreeList(item);
                m_freeSize += uiKeyCacacity + sizeof(UnistorReadCacheItemEx);
                m_freeCapacity += uiKeyCacacity;
                m_freeItemCount++;
                m_cacheBufArrPos += uiKeyCacacity + sizeof(UnistorReadCacheItemEx);
            }else{///释放已有空间
                while(m_freeCapacity < uiKeyCacacity){
                    _findKey(m_chain_tail, iter);
                    _remove(iter);
                }
            }
        }
        ///检查key的数量
        while(m_cachedKeyCount >= m_max_cache_num){
            _findKey(m_chain_tail, iter);
            _remove(iter);
        }
        ///添加key
        CWX_UINT16 unKeyPos = 0;
        CWX_UINT32 uiDataPos = 0;
        CWX_UINT32 uiCopyLen = 0;
        key = m_freeHead;
        while(key){
            if (unKeyPos < unKeyLen){
                uiCopyLen = unKeyLen - unKeyPos;
                if (uiCopyLen > key->capacity()){
                    uiCopyLen = key->capacity();
                }
                memcpy(key->m_szBuf, szKey + unKeyPos, uiCopyLen);
                key->m_unKeyLen = uiCopyLen;
                unKeyPos+=uiCopyLen;
                ///拷贝数据
                if ((unKeyPos == unKeyLen) && (key->capacity() >= key->getKeyLen() + uiDataLen)){
                    uiCopyLen = key->capacity() - key->getKeyLen();
                    if (uiCopyLen > uiDataLen) uiCopyLen = uiDataLen;
                    memcpy(key->m_szBuf + key->getKeyLen(), szData, uiCopyLen);
                    key->m_uiDataLen = uiCopyLen;
                    uiDataPos += uiCopyLen;
                }else{
                    key->m_uiDataLen = 0;
                }
            }else{///copy数据
                uiCopyLen = uiDataLen - uiDataPos;
                if (uiCopyLen > key->capacity()) uiCopyLen = key->capacity();
                key->m_unKeyLen = 0;
                memcpy(key->m_szBuf, szData + uiDataPos, uiCopyLen);
                key->m_uiDataLen = uiCopyLen;
                uiDataPos += uiCopyLen;
            }

            ///计算空间
            m_usedSize += key->size();
            m_usedCapacity += key->capacity();
            m_usedDataSize += key->getKeyLen() + key->getDataLen();
            m_freeSize -= key->size();
            m_freeCapacity -= key->capacity();
            m_cachedItemCount ++;
            m_freeItemCount--;
            key->m_next = NULL;
            key->m_prev = NULL;
            key->m_hashNext = NULL;
            ///如果已经完成copy，则跳出
            if ((unKeyPos == unKeyLen)&& (uiDataPos == uiDataLen)) break;
            ///继续使用下一块空间
            key=key->m_child;
        }

        if ((unKeyPos != unKeyLen) || (uiDataPos != uiDataLen)){
            CWX_ASSERT(0);
        }

        UnistorReadCacheItemEx* head = m_freeHead;
        m_freeHead = head->m_child;
        if (!m_freeHead){
            m_freeTail = NULL;
        }else{
            head->m_child = NULL;
        }

        m_cachedKeyCount++;
        head->m_next = m_chain_head;
        head->m_prev = NULL;
        if (m_chain_head == NULL){
            m_chain_head = m_chain_tail = head;
        }else{
            m_chain_head->m_prev = head;
            m_chain_head = head;
        }
        ///add to hash
        _addKey(head);

    }
}

void UnistorReadCacheEx::remove( char const* szKey, CWX_UINT16 unKeyLen){
    char key_buf[sizeof(UnistorReadCacheItemEx) + UnistorReadCacheItemEx::calBufCapacity(unKeyLen , 0, 0)];
    UnistorReadCacheItemEx* key = (UnistorReadCacheItemEx*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (_findKey(key, iter)){
            CwxMutexGuard<CwxMutexLock> lock(this->m_mutex);
            _remove(iter);
        }
    }
}

int UnistorReadCacheEx::fetch(char const* szKey,
                 CWX_UINT16 unKeyLen,
                 char* szData,
                 CWX_UINT32& uiDataLen,
                 bool bTouch)
{
    char key_buf[sizeof(UnistorReadCacheItemEx) + UnistorReadCacheItemEx::calBufCapacity(unKeyLen , 0, 0)];
    UnistorReadCacheItemEx* key = (UnistorReadCacheItemEx*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    CWX_UINT32 pos = 0;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (!_findKey(key, iter)) return 0;
        key = iter.m_pFind;
        while(iter.m_pFind){
            if (uiDataLen < iter.m_pFind->getDataLen()) return -1;
            memcpy(szData + pos, iter.m_pFind->getData(), iter.m_pFind->getDataLen());
            uiDataLen -= iter.m_pFind->getDataLen();
            pos += iter.m_pFind->getDataLen();
            iter.m_pFind = iter.m_pFind->m_child;
        }
        uiDataLen = pos;
        if( bTouch) {
            CwxMutexGuard<CwxMutexLock> lock(this->m_mutex);
            _touch(key);
        }
        return 1;
    }
}    


UnistorReadCacheItemEx* UnistorReadCacheEx::fetch(char const* szKey,
                                     CWX_UINT16 unKeyLen,
                                     bool bTouch)
{
    char key_buf[sizeof(UnistorReadCacheItemEx) + UnistorReadCacheItemEx::calBufCapacity(unKeyLen , 0, 0)];
    UnistorReadCacheItemEx* key = (UnistorReadCacheItemEx*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (!_findKey(key, iter)) return NULL;
        if( bTouch) {
            CwxMutexGuard<CwxMutexLock> lock(this->m_mutex);
            _touch(iter.m_pFind);
        }
        return iter.m_pFind;
    }
}

void UnistorReadCacheEx::touch(char const* szKey, ///<key
                  CWX_UINT16 unKeyLen ///<key的长度
                  )
{
    char key_buf[sizeof(UnistorReadCacheItemEx) + UnistorReadCacheItemEx::calBufCapacity(unKeyLen , 0, 0)];
    UnistorReadCacheItemEx* key = (UnistorReadCacheItemEx*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (_findKey(key, iter)){
            CwxMutexGuard<CwxMutexLock> lock(this->m_mutex);
            _touch(iter.m_pFind);
        }
    }
}

bool UnistorReadCacheEx::exist(char const* szKey, ///<key
                  CWX_UINT16 unKeyLen ///<key的长度
                  )
{
    char key_buf[sizeof(UnistorReadCacheItemEx) + UnistorReadCacheItemEx::calBufCapacity(unKeyLen , 0, 0)];
    UnistorReadCacheItemEx* key = (UnistorReadCacheItemEx*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        return _findKey(key, iter);
    }
}

///清空CACHE    
void UnistorReadCacheEx::clear( void ){
    CwxWriteLockGuard<CwxRwLock>  lock(m_rwLock);
    if (m_hashArr){
        delete [] m_hashArr;
        m_hashArr = NULL;
    }
    if (m_cacheBufArr){
        for (CWX_UINT32 i=0; i<m_cacheBufArrSize; i++){
            if (m_cacheBufArr[i]) delete [] m_cacheBufArr[i];
        }
        delete [] m_cacheBufArr;
        m_cacheBufArr = NULL;
    }
    m_cacheBufArrIndex = 0;
    m_cacheBufArrPos = 0;
    m_freeHead = NULL;
    m_freeTail = NULL;
    m_chain_head = NULL;
    m_chain_tail = NULL;
    m_usedSize = 0;
    m_usedCapacity = 0;
    m_usedDataSize = 0;
    m_freeSize = 0;
    m_freeCapacity = 0;
    m_cachedKeyCount=0;
    m_cachedItemCount = 0;
    m_freeItemCount=0;
}

///copy数据，0：成功；-1：空间不足
int UnistorReadCacheEx::copyData(UnistorReadCacheItemEx const* item, char* szData, CWX_UINT32& uiDataLen){
    CWX_UINT32 pos = 0;
    while(item){
        if (uiDataLen < item->getDataLen()) return -1;
        memcpy(szData + pos, item->getData(), item->getDataLen());
        uiDataLen -= item->getDataLen();
        pos += item->getDataLen();
        item = item->m_child;
    }
    uiDataLen = pos;
    return 0;
}


///不带锁的remove操作
void UnistorReadCacheEx::_remove(UnistorReadCacheExIter const& iter){
    ///从hash中删除key
    _removeKey(iter);
    ///从LRU链表中删除key
    UnistorReadCacheItemEx* data = iter.m_pFind;
    if (data == m_chain_head){
        if (data == m_chain_tail){
            m_chain_head = m_chain_tail = NULL;
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
    ///计算空间
    unsigned long int size = 0;
    unsigned long int dataSize = 0;
    unsigned long int capacity = 0;
    CWX_UINT32 uiItemNum = _getItemCountSize(data, size, dataSize, capacity);
    ///添加的free的空间
    _addFreeList(data);
    ///设置空间
    m_usedSize -= size;
    m_usedCapacity -= capacity;
    m_usedDataSize -= dataSize;
    m_freeSize += size;
    m_freeCapacity += capacity;
    m_cachedKeyCount --;
    m_cachedItemCount -= uiItemNum;
    m_freeItemCount +=uiItemNum;
}

///不带锁的touch操作
void UnistorReadCacheEx::_touch(UnistorReadCacheItemEx* data ){
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


///将item添加到free list，不修改容量空间
void UnistorReadCacheEx::_addFreeList(UnistorReadCacheItemEx* item){
    ///将其放到free list中
    if (m_freeTail){
        m_freeTail->m_child = item;
        m_freeTail = item;
    }else{
        m_freeTail = m_freeHead = item;
    }
    while(m_freeTail){
        m_freeTail->m_hashNext=m_freeTail->m_next=m_freeTail->m_prev=NULL;
        if (m_freeTail->m_child){
            m_freeTail = m_freeTail->m_child;
            continue;
        }
        break;
    }
}

void UnistorReadCacheEx::_addKey(UnistorReadCacheItemEx* item){
    CWX_UINT32 pos = UnistorReadCacheItemEx::m_fnHash(item->getKey(), item->getKeyLen())%m_bucket_num;
    item->m_hashNext = m_hashArr[pos];
    m_hashArr[pos] = item;
}
///获取key
bool UnistorReadCacheEx::_findKey(UnistorReadCacheItemEx const* item, UnistorReadCacheExIter& iter){
    char* szKey = NULL;
    CWX_UINT16 unKeyBuf = 0;
    CWX_UINT16 unKeyLen = 0;
    UnistorReadCacheItemEx* child=NULL;
    iter.m_uiHash = UnistorReadCacheItemEx::m_fnHash(item->getKey(), item->getKeyLen())%m_bucket_num;
    iter.m_pFind = m_hashArr[iter.m_uiHash];
    iter.m_pPrev = NULL;
    while(iter.m_pFind){
        if (!iter.m_pFind->m_child || !iter.m_pFind->m_child->getKeyLen()){
            if (UnistorReadCacheItemEx::m_fnEqual(item->getKey(),
                item->getKeyLen(),
                iter.m_pFind->getKey(),
                iter.m_pFind->getKeyLen()))
            {
                if (szKey) delete [] szKey;
                return true;
            }
        }else{
            child = iter.m_pFind;
            unKeyLen = 0;
            while(child){
                unKeyLen += child->getKeyLen();
                child = child->m_child;
            }
            if (unKeyLen > unKeyBuf){
                if (szKey) delete [] szKey;
                szKey = new char[unKeyLen];
                unKeyBuf = unKeyLen;
            }
            child = iter.m_pFind;
            unKeyLen = 0;
            while(child){
                memcpy(szKey + unKeyLen, child->getKey(), child->getKeyLen());
                unKeyLen += child->getKeyLen();
                child = child->m_child;
            }
            if (UnistorReadCacheItemEx::m_fnEqual(item->getKey(),
                item->getKeyLen(),
                szKey,
                unKeyLen))
            {
                if (szKey) delete [] szKey;
                return true;
            }
        }
        iter.m_pPrev = iter.m_pFind;
        iter.m_pFind = iter.m_pFind->m_hashNext;
    }
    iter.m_pFind = NULL;
    iter.m_pPrev = NULL;
    if (szKey) delete [] szKey;
    return false;
}
