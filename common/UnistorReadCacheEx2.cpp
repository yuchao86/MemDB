#include "UnistorReadCacheEx2.h"

UNISTOR_KEY_CMP_EQUAL_FN UnistorReadCacheItemEx2::m_fnEqual = NULL; ///<key��ȵıȽϺ���
UNISTOR_KEY_CMP_LESS_FN UnistorReadCacheItemEx2::m_fnLess = NULL; ///<keyС�ڵıȽϺ���
UNISTOR_KEY_HASH_FN UnistorReadCacheItemEx2::m_fnHash = NULL; ///<key��hashֵ�ļ��㺯��

UnistorReadCacheEx2::UnistorReadCacheEx2(unsigned long int size, ///<�ռ��С
                   CWX_UINT32 count, ///<key������
                   UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key��ȱȽϺ���
                   UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less�ıȽϺ���
                   UNISTOR_KEY_HASH_FN    fnHash, ///<key��hash����
                   CwxRwLock* rwLock, ///<��д��
                   float  fBucketRate ///<Ͱ�ı���
                   ):m_bucket_num((CWX_UINT32)((count *fBucketRate + 1)>count?(count *fBucketRate + 1):count)),m_max_cache_num(count)
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
    m_chainArr = NULL;
    m_usedSize = 0;
    m_usedCapacity = 0;
    m_usedDataSize = 0;
    m_freeSize = 0;
    m_freeCapacity = 0;
    m_cachedKeyCount=0;
    m_cachedItemCount = 0;
    m_freeItemCount=0;
    m_rwLock = rwLock;
    for (CWX_UINT32 i=0; i<UNISTOR_READ_CACHE_CHAIN_LOCK_NUM; i++){
        m_chainMutexArr[i] = NULL;
    }
    UnistorReadCacheItemEx2::m_fnEqual = m_fnEqual = fnEqual;
    UnistorReadCacheItemEx2::m_fnLess = m_fnLess = fnLess;
    UnistorReadCacheItemEx2::m_fnHash = m_fnHash = fnHash;        
}
///��ʼ��cache������ֵ��0���ɹ���-1��ʧ��
int UnistorReadCacheEx2::init(char* szErr2K){
    clear();
    {
        CwxWriteLockGuard<CwxRwLock>  lock(m_rwLock);
        ///����hash bucket
        m_hashArr = new UnistorReadCacheItemEx2*[m_bucket_num];
        if (!m_hashArr){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc hash arr, bucket number:%u", m_bucket_num);
            return -1;
        }
        memset(m_hashArr, 0x00, sizeof(UnistorReadCacheItemEx2*)*m_bucket_num);
        ///����ռ�
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
        ///������������
        CWX_UINT32 uiMaxPinChainNum = UnistorReadCacheItemEx2::calMaxIndexNum(UNISTOR_READ_CACHE_MAX_ITEM_SIZE);
        m_chainArr = new UnistorReadCacheItemPinEx2[uiMaxPinChainNum];
        ///�����������
        for (i=0; i<UNISTOR_READ_CACHE_CHAIN_LOCK_NUM; i++){
            m_chainMutexArr[i] = new CwxMutexLock();            
        }
    }
    return 0;
}

void UnistorReadCacheEx2::insert(char const* szKey,
            CWX_UINT16 unKeyLen,
            char const* szData,
            CWX_UINT32 uiDataLen,
            bool bNotExist)
{
    ///key�ĳ��Ȳ���Ϊ0
    if (!unKeyLen) return;

    char key_buf[UnistorReadCacheItemEx2::calKeySize(unKeyLen , 0)];
    UnistorReadCacheItemEx2* key = (UnistorReadCacheItemEx2*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;

    CWX_UINT32 uiKeySize = UnistorReadCacheItemEx2::calKeySize(unKeyLen , uiDataLen);
    ///��key�Ŀռ��С����slot����cache
    if (uiKeySize > UNISTOR_READ_CACHE_MAX_ITEM_SIZE) return;
    iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxWriteLockGuard<CwxRwLock> lock(this->m_rwLock);
        ///���key����
        if(_findKey(key, iter)){
            if (bNotExist) return;
            if (uiKeySize == iter.m_pFind->size()){///����ͬһ��slot
                ///�޸Ŀռ�
                m_usedDataSize -= iter.m_pFind->getDataLen();
                m_usedDataSize += uiDataLen;
                iter.m_pFind->m_uiDataLen = uiDataLen;
                if (uiDataLen){
                    memcpy((char*)iter.m_pFind->getData(), szData, uiDataLen);  
                }
                CWX_UINT32 uiIndex = iter.m_pFind->index();
                _touch(iter.m_pFind, uiIndex);
                return;
            }
            ///���ڲ�ͬ��slot���������
            _remove(iter);
        }

        CWX_UINT32 uiIndex = UnistorReadCacheItemEx2::calKeyIndex(unKeyLen , uiDataLen);
        ///����ʹ�ÿ��е�key
        if (m_chainArr[uiIndex].m_freeHead){
            key = m_chainArr[uiIndex].m_freeHead;
            m_chainArr[uiIndex].m_freeHead = m_chainArr[uiIndex].m_freeHead->m_next;
            ///�޸Ŀռ�
            m_chainArr[uiIndex].m_uiFreeNum--;
            m_freeSize -= uiKeySize;
            m_freeCapacity -= (uiKeySize - sizeof(UnistorReadCacheItemEx2));
            m_freeItemCount --;
        }else if ((m_cacheBufArrIndex + 1 < m_cacheBufArrSize) ||  ///�����ڴ����key
            (m_cacheBufArrPos + uiKeySize < UNISTOR_READ_CACHE_SLOT_SIZE))
        {
            if (m_cacheBufArrPos + uiKeySize > UNISTOR_READ_CACHE_SLOT_SIZE){///<ʹ����slot
                m_cacheBufArrIndex ++;
                m_cacheBufArrPos = 0;
            }
            ///�����¿ռ�
            key = (UnistorReadCacheItemEx2*)(m_cacheBufArr[m_cacheBufArrIndex] + m_cacheBufArrPos);
            m_cacheBufArrPos += uiKeySize;
        }else if (m_chainArr[uiIndex].m_usedTail){///�ͷ����пռ�
            key = m_chainArr[uiIndex].m_usedTail;
            iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
            _findKey(key, iter);
            if (iter.m_pFind != key){
                CWX_ASSERT(0);
            }
            ///��hash��ɾ��key
            _removeKey(iter);
            m_chainArr[uiIndex].m_usedTail = m_chainArr[uiIndex].m_usedTail->m_prev;
            if (!m_chainArr[uiIndex].m_usedTail){
                m_chainArr[uiIndex].m_usedHead = NULL;
            }else{
                m_chainArr[uiIndex].m_usedTail->m_next = NULL;
            }
            m_chainArr[uiIndex].m_uiUsedNum--;
            m_usedSize -= uiKeySize;
            m_usedCapacity -= uiKeySize - sizeof(UnistorReadCacheItemEx2);
            m_usedDataSize -= key->m_uiDataLen + key->m_unKeyLen;
            m_cachedKeyCount--;
            m_cachedItemCount--;
        }else{ ///û�пռ���ã���cache
            return ;
        }
        ///copy����
        key->m_unKeyLen = unKeyLen;
        key->m_uiDataLen = uiDataLen;
        if (unKeyLen)  memcpy((char*)key->getKey(), szKey, unKeyLen);
        if (uiDataLen) memcpy((char*)key->getData(), szData, uiDataLen);
        _addKey(uiIndex, uiKeySize, key);
    }
}

void UnistorReadCacheEx2::remove( char const* szKey, CWX_UINT16 unKeyLen){
    char key_buf[UnistorReadCacheItemEx2::calKeySize(unKeyLen , 0)];
    UnistorReadCacheItemEx2* key = (UnistorReadCacheItemEx2*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxWriteLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (_findKey(key, iter)){
            _remove(iter);
        }
    }
}

int UnistorReadCacheEx2::fetch(char const* szKey,
                 CWX_UINT16 unKeyLen,
                 char* szData,
                 CWX_UINT32& uiDataLen,
                 bool bTouch)
{
    char key_buf[UnistorReadCacheItemEx2::calKeySize(unKeyLen , 0)];
    UnistorReadCacheItemEx2* key = (UnistorReadCacheItemEx2*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (!_findKey(key, iter)) return 0;
        if (uiDataLen < iter.m_pFind->getDataLen()) return -1;
        memcpy(szData, iter.m_pFind->getData(), iter.m_pFind->getDataLen());
        uiDataLen = iter.m_pFind->getDataLen();
        if(bTouch) {
            CWX_UINT32 uiIndex = iter.m_pFind->index();
            CwxMutexGuard<CwxMutexLock> lock(m_chainMutexArr[uiIndex%UNISTOR_READ_CACHE_CHAIN_LOCK_NUM]);
            _touch(iter.m_pFind, uiIndex);
        }
        return 1;
    }
}    

UnistorReadCacheItemEx2* UnistorReadCacheEx2::fetch(char const* szKey,
                                     CWX_UINT16 unKeyLen,
                                     bool bTouch)
{
    char key_buf[UnistorReadCacheItemEx2::calKeySize(unKeyLen , 0)];
    UnistorReadCacheItemEx2* key = (UnistorReadCacheItemEx2*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (!_findKey(key, iter)) return NULL;
        if( bTouch) {
            CWX_UINT32 uiIndex = iter.m_pFind->index();
            CwxMutexGuard<CwxMutexLock> lock(m_chainMutexArr[uiIndex%UNISTOR_READ_CACHE_CHAIN_LOCK_NUM]);
            _touch(iter.m_pFind, uiIndex);
        }
        return iter.m_pFind;
    }
}

void UnistorReadCacheEx2::touch(char const* szKey, ///<key
                  CWX_UINT16 unKeyLen ///<key�ĳ���
                  )
{
    char key_buf[UnistorReadCacheItemEx2::calKeySize(unKeyLen , 0)];
    UnistorReadCacheItemEx2* key = (UnistorReadCacheItemEx2*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        if (_findKey(key, iter)){
            CWX_UINT32 uiIndex = iter.m_pFind->index();
            CwxMutexGuard<CwxMutexLock> lock(m_chainMutexArr[uiIndex%UNISTOR_READ_CACHE_CHAIN_LOCK_NUM]);
            _touch(iter.m_pFind, uiIndex);
        }
    }
}

bool UnistorReadCacheEx2::exist(char const* szKey, ///<key
                  CWX_UINT16 unKeyLen ///<key�ĳ���
                  )
{
    char key_buf[UnistorReadCacheItemEx2::calKeySize(unKeyLen, 0)];
    UnistorReadCacheItemEx2* key = (UnistorReadCacheItemEx2*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorReadCacheExIter iter;
    iter.m_uiHash = UnistorReadCacheItemEx2::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(this->m_rwLock);
        return _findKey(key, iter);
    }
}

///���CACHE    
void UnistorReadCacheEx2::clear( void ){
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
    if (m_chainArr){
        delete [] m_chainArr;
        m_chainArr = NULL;
    }
    for (CWX_UINT32 i=0; i<UNISTOR_READ_CACHE_CHAIN_LOCK_NUM; i++){
        if (m_chainMutexArr[i]) delete m_chainMutexArr[i];
        m_chainMutexArr[i] = NULL;
    }

    m_cacheBufArrIndex = 0;
    m_cacheBufArrPos = 0;
    m_usedSize = 0;
    m_usedCapacity = 0;
    m_usedDataSize = 0;
    m_freeSize = 0;
    m_freeCapacity = 0;
    m_cachedKeyCount=0;
    m_cachedItemCount = 0;
    m_freeItemCount=0;
}

///��������remove����
void UnistorReadCacheEx2::_remove(UnistorReadCacheExIter const& iter){
    ///��hash��ɾ��key
    _removeKey(iter);
    iter.m_pFind->m_hashNext = NULL;
    ///��LRU������ɾ��key
    UnistorReadCacheItemEx2* data = iter.m_pFind;
    CWX_UINT32 uiIndex = data->index();
    if (data == m_chainArr[uiIndex].m_usedHead){
        if (data == m_chainArr[uiIndex].m_usedTail){
            m_chainArr[uiIndex].m_usedHead = m_chainArr[uiIndex].m_usedTail = NULL;
        }else{
            m_chainArr[uiIndex].m_usedHead  = data->m_next;
            m_chainArr[uiIndex].m_usedHead->m_prev = NULL;
        }
    }else if (data == m_chainArr[uiIndex].m_usedTail){
        m_chainArr[uiIndex].m_usedTail = data->m_prev;
        m_chainArr[uiIndex].m_usedTail->m_next = NULL;
    }else{
        data->m_prev->m_next = data->m_next;
        data->m_next->m_prev = data->m_prev;
    }
    ///����ռ�
    unsigned long int size = data->size();
    unsigned long int dataSize = data->m_uiDataLen + data->m_unKeyLen;
    unsigned long int capacity = data->capacity();
    ///��ӵ�free�Ŀռ�
    _addFreeList(uiIndex, data);
    ///���ÿռ�
    m_usedSize -= size;
    m_usedCapacity -= capacity;
    m_usedDataSize -= dataSize;
    m_freeSize += size;
    m_freeCapacity += capacity;
    m_cachedKeyCount --;
    m_cachedItemCount --;
    m_freeItemCount ++;
}

///��������touch����
void UnistorReadCacheEx2::_touch(UnistorReadCacheItemEx2* data, CWX_UINT32 uiIndex){
    if (data->m_prev == NULL) //the head
        return;
    if (data->m_next == NULL){// the tail
        m_chainArr[uiIndex].m_usedTail = data->m_prev;
        m_chainArr[uiIndex].m_usedTail->m_next = NULL;
    }else{
        data->m_prev->m_next = data->m_next;
        data->m_next->m_prev = data->m_prev;
    }
    data->m_prev = NULL;
    data->m_next = m_chainArr[uiIndex].m_usedHead;
    m_chainArr[uiIndex].m_usedHead->m_prev = data;
    m_chainArr[uiIndex].m_usedHead = data;
}


///��item��ӵ�free list�����޸������ռ�
void UnistorReadCacheEx2::_addFreeList(CWX_UINT32 uiIndex, UnistorReadCacheItemEx2* item){
    ///����ŵ�free list��
    item->m_next = m_chainArr[uiIndex].m_freeHead;
    item->m_prev = NULL;
    item->m_hashNext = NULL;
    if (m_chainArr[uiIndex].m_freeHead){
        m_chainArr[uiIndex].m_freeHead->m_prev = item;
    }
    m_chainArr[uiIndex].m_freeHead = item;
}

void UnistorReadCacheEx2::_addKey(CWX_UINT32 uiIndex, CWX_UINT32 uiSize, UnistorReadCacheItemEx2* item){
    ///��ӵ�slot
    item->m_hashNext = NULL;
    item->m_prev = NULL;
    item->m_next = m_chainArr[uiIndex].m_usedHead;
    if (!m_chainArr[uiIndex].m_usedHead){
        m_chainArr[uiIndex].m_usedHead = m_chainArr[uiIndex].m_usedTail = item;
    }else{
        m_chainArr[uiIndex].m_usedHead->m_prev = item;
        m_chainArr[uiIndex].m_usedHead = item;
    }
    m_chainArr[uiIndex].m_uiUsedNum++;
    m_usedSize += uiSize;
    m_usedCapacity += (uiSize - sizeof(UnistorReadCacheItemEx2));
    m_usedDataSize += item->m_uiDataLen + item->m_unKeyLen;
    m_cachedKeyCount ++;
    m_cachedItemCount ++;
    ///��ӵ�hash
    uiIndex = UnistorReadCacheItemEx2::m_fnHash(item->getKey(), item->getKeyLen())%m_bucket_num;
    item->m_hashNext = m_hashArr[uiIndex];
    m_hashArr[uiIndex] = item;
}


///��ȡkey
bool UnistorReadCacheEx2::_findKey(UnistorReadCacheItemEx2 const* item, UnistorReadCacheExIter& iter){
    iter.m_pFind = m_hashArr[iter.m_uiHash];
    iter.m_pPrev = NULL;
    while(iter.m_pFind){
        if (UnistorReadCacheItemEx2::m_fnEqual(item->getKey(),
            item->getKeyLen(),
            iter.m_pFind->getKey(),
            iter.m_pFind->getKeyLen())) return true;
        iter.m_pPrev = iter.m_pFind;
        iter.m_pFind = iter.m_pFind->m_hashNext;
    }
    iter.m_pFind = NULL;
    iter.m_pPrev = NULL;
    return false;
}
