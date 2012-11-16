#include "UnistorStoreMemCache.h"

UNISTOR_KEY_CMP_EQUAL_FN UnistorStoreMemCacheItem::m_fnEqual = NULL; ///<key��ȵıȽϺ���
UNISTOR_KEY_CMP_LESS_FN UnistorStoreMemCacheItem::m_fnLess = NULL; ///<keyС�ڵıȽϺ���
UNISTOR_KEY_HASH_FN UnistorStoreMemCacheItem::m_fnHash = NULL; ///<key��hashֵ�ļ��㺯��

UnistorStoreMemCache::UnistorStoreMemCache(unsigned long int size, ///<�ռ��С
                   CWX_UINT32 count, ///<key������
                   CWX_UINT32 maxPerCheckNum, ///<ÿ�μ��key���������
                   UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key��ȱȽϺ���
                   UNISTOR_KEY_CMP_LESS_FN   fnLess, ///<key less�ıȽϺ���
                   UNISTOR_KEY_HASH_FN    fnHash, ///<key��hash����
                   float  fBucketRate ///<Ͱ�ı���
                   ):m_bucket_num((CWX_UINT32)((count *fBucketRate + 1)>count?(count *fBucketRate + 1):count)),m_max_cache_num(count)
{
    m_max_size = size;
    if (m_max_size % UNISTOR_STORE_MEM_CACHE_SLOT_SIZE){
        m_max_size += (UNISTOR_STORE_MEM_CACHE_SLOT_SIZE - (m_max_size %UNISTOR_STORE_MEM_CACHE_SLOT_SIZE -1));
    }
    m_hashArr = NULL;
    m_cacheBufArr = NULL;
    m_cacheBufArrSize = m_max_size/UNISTOR_STORE_MEM_CACHE_SLOT_SIZE;
    if (!m_cacheBufArrSize) m_cacheBufArrSize =1;
    m_cacheBufArrIndex = 0;
    m_cacheBufArrPos = 0;
    m_chainArr = NULL;

    if (!maxPerCheckNum) maxPerCheckNum = 1000;
    m_uiMaxPerCheckExpireNum = maxPerCheckNum;
    m_uiCurCheckSlotIndex = 0;
    m_uiCurCheckSlotPos = 0;
    m_arrExpiredItem = NULL;
    m_uiExpireNum = 0;

    for (CWX_UINT32 i=0; i<UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM; i++){
        m_chainMutexArr[i] = NULL;
    }
    m_usedSize = 0;
    m_usedCapacity = 0;
    m_usedDataSize = 0;
    m_freeSize = 0;
    m_freeCapacity = 0;
    m_cachedKeyCount=0;
    m_cachedItemCount = 0;
    m_freeItemCount=0;
    UnistorStoreMemCacheItem::m_fnEqual = m_fnEqual = fnEqual;
    UnistorStoreMemCacheItem::m_fnLess = m_fnLess = fnLess;
    UnistorStoreMemCacheItem::m_fnHash = m_fnHash = fnHash;        
}

///��ʼ��cache������ֵ��0���ɹ���-1��ʧ��
int UnistorStoreMemCache::init(char* szErr2K){
    free();
    {
        CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
        ///����hash bucket
        m_hashArr = new UnistorStoreMemCacheItem*[m_bucket_num];
        if (!m_hashArr){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc hash arr, bucket number:%u", m_bucket_num);
            return -1;
        }
        memset(m_hashArr, 0x00, sizeof(UnistorStoreMemCacheItem*)*m_bucket_num);
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
            m_cacheBufArr[i] = new char[UNISTOR_STORE_MEM_CACHE_SLOT_SIZE];
            if (!m_cacheBufArr[i]){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc cache slot, no:%u, size=%u", i, UNISTOR_STORE_MEM_CACHE_SLOT_SIZE);
                return -1;
            }
            memset(m_cacheBufArr[i], 0x00, sizeof(UNISTOR_STORE_MEM_CACHE_SLOT_SIZE));
        }
        ///������������
        CWX_UINT32 uiMaxPinChainNum = UnistorStoreMemCacheItem::calMaxIndexNum(UNISTOR_STORE_MEM_CACHE_MAX_ITEM_SIZE);
        m_chainArr = new UnistorStoreMemCacheItemPin[uiMaxPinChainNum];
        ///������ʱ��������
        m_arrExpiredItem = new UnistorStoreMemCacheItem*[m_uiMaxPerCheckExpireNum];
        m_uiExpireNum = 0;
        m_uiCurCheckSlotIndex = 0;
        m_uiCurCheckSlotPos = 0;
        ///�����������
        for (CWX_UINT32 i=0; i<UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM; i++){
            m_chainMutexArr[i] = new CwxMutexLock();
        }
    }
    return 0;
}

void UnistorStoreMemCache::insert(char const* szKey,
            CWX_UINT16 unKeyLen,
            char const* szData,
            CWX_UINT32 uiDataLen,
            CWX_UINT32* uiExpire)
{
    ///key�ĳ��Ȳ���Ϊ0
    if (!unKeyLen) return;
    CWX_UINT32 uiOldExpire = 0;
    CWX_UINT32 uiIndex = 0;
    char key_buf[UnistorStoreMemCacheItem::calKeySize(unKeyLen , 0)];
    UnistorStoreMemCacheItem* key = (UnistorStoreMemCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorStoreMemCacheIter iter;

    CWX_UINT32 uiKeySize = UnistorStoreMemCacheItem::calKeySize(unKeyLen , uiDataLen);
    ///��key�Ŀռ��С����slot����cache
    if (uiKeySize > UNISTOR_STORE_MEM_CACHE_MAX_ITEM_SIZE) return;

    iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
        ///���key����
        if(_findKey(key, iter)){
            if (uiKeySize == iter.m_pFind->size()){///����ͬһ��slot
                ///�޸Ŀռ�
                m_usedDataSize -= iter.m_pFind->getDataLen();
                m_usedDataSize += uiDataLen;
                iter.m_pFind->m_uiDataLen = uiDataLen;
                if (uiExpire) iter.m_pFind->m_uiExpire = *uiExpire;
                if (uiDataLen){
                    memcpy((char*)iter.m_pFind->getData(), szData, uiDataLen);  
                }
                uiIndex = iter.m_pFind->index();
                _touch(iter.m_pFind, uiIndex);
                return;
            }
            uiOldExpire = iter.m_pFind->m_uiExpire;
            ///���ڲ�ͬ��slot�������������ʱӵ��hash��������������ͳ����
            _remove(iter);
        }
        ///��������
        uiIndex = UnistorStoreMemCacheItem::calKeyIndex(unKeyLen , uiDataLen);
        ///����ʹ�ÿ��е�key
        if (m_chainArr[uiIndex].m_freeHead){
            key = m_chainArr[uiIndex].m_freeHead;
            m_chainArr[uiIndex].m_freeHead = m_chainArr[uiIndex].m_freeHead->m_next;
            ///�޸Ŀռ�
            m_chainArr[uiIndex].m_uiFreeNum--;
            m_freeSize -= uiKeySize;
            m_freeCapacity -= (uiKeySize - sizeof(UnistorStoreMemCacheItem));
            m_freeItemCount --;
        }else if ((m_cacheBufArrIndex + 1 < m_cacheBufArrSize) ||  ///�����ڴ����key
            (m_cacheBufArrPos + uiKeySize < UNISTOR_STORE_MEM_CACHE_SLOT_SIZE))
        {
            if (m_cacheBufArrPos + uiKeySize > UNISTOR_STORE_MEM_CACHE_SLOT_SIZE){///<ʹ����slot
                m_cacheBufArrIndex ++;
                m_cacheBufArrPos = 0;
            }
            ///�����¿ռ�
            key = (UnistorStoreMemCacheItem*)(m_cacheBufArr[m_cacheBufArrIndex] + m_cacheBufArrPos);
            m_cacheBufArrPos += uiKeySize;
        }else if (m_chainArr[uiIndex].m_usedTail){///�ͷ����пռ�
            key = m_chainArr[uiIndex].m_usedTail;
            iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
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
            m_usedCapacity -= uiKeySize - sizeof(UnistorStoreMemCacheItem);
            m_usedDataSize -= key->m_uiDataLen + key->m_unKeyLen;
            m_cachedKeyCount--;
            m_cachedItemCount--;
        }else{ ///û�пռ���ã���cache
            return ;
        }
        ///copy����
        key->m_uiKeyIndex = uiIndex; ///<����key��index
        key->m_unKeyLen = unKeyLen;
        key->m_uiDataLen = uiDataLen;
        key->m_ucState = UnistorStoreMemCacheItem::UNISTOR_MEM_ITEM_STATE_USED;
        key->m_uiExpire = uiExpire?*uiExpire:uiOldExpire;
        if (unKeyLen)  memcpy((char*)key->getKey(), szKey, unKeyLen);
        if (uiDataLen) memcpy((char*)key->getData(), szData, uiDataLen);
        _addKey(uiIndex, uiKeySize, key);
    }
}

void UnistorStoreMemCache::remove( char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT32 uiExpire){
    char key_buf[UnistorStoreMemCacheItem::calKeySize(unKeyLen , 0)];
    UnistorStoreMemCacheItem* key = (UnistorStoreMemCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorStoreMemCacheIter iter;
    iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
        if (_findKey(key, iter) && (!uiExpire || (iter.m_pFind->m_uiExpire == uiExpire))){
            _remove(iter);
        }
    }
}

//return */0�������ڣ�1����ȡ��-1���ռ䲻��
int UnistorStoreMemCache::fetch(char const* szKey,
                 CWX_UINT16 unKeyLen,
                 char* szData,
                 CWX_UINT32& uiDataLen,
                 CWX_UINT32& uiExpire,
                 CWX_UINT32  uiCurExpireTime,
                 bool bTouch)
{
    char key_buf[UnistorStoreMemCacheItem::calKeySize(unKeyLen , 0)];
    UnistorStoreMemCacheItem* key = (UnistorStoreMemCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorStoreMemCacheIter iter;
    iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
        if (!_findKey(key, iter)) return 0;
        if (uiDataLen < iter.m_pFind->getDataLen()) return -1;
        if (iter.m_pFind->m_uiExpire && (iter.m_pFind->m_uiExpire <= uiCurExpireTime)){///<��ʱ
            return 0;
        }
        memcpy(szData, iter.m_pFind->getData(), iter.m_pFind->getDataLen());
        uiDataLen = iter.m_pFind->getDataLen();
        uiExpire = iter.m_pFind->m_uiExpire;
        if(bTouch) {
            CWX_UINT32 uiIndex = iter.m_pFind->index();
            CwxMutexGuard<CwxMutexLock> lock(m_chainMutexArr[uiIndex%UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM]);
            _touch(iter.m_pFind, uiIndex);
        }
        return 1;
    }
}    


void UnistorStoreMemCache::touch(char const* szKey, ///<key
                  CWX_UINT16 unKeyLen ///<key�ĳ���
                  )
{
    char key_buf[UnistorStoreMemCacheItem::calKeySize(unKeyLen , 0)];
    UnistorStoreMemCacheItem* key = (UnistorStoreMemCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorStoreMemCacheIter iter;
    iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
        if (_findKey(key, iter)){
            CWX_UINT32 uiIndex = iter.m_pFind->index();
            CwxMutexGuard<CwxMutexLock> lock(m_chainMutexArr[uiIndex%UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM]);
            _touch(iter.m_pFind, uiIndex);
        }
    }
}

bool UnistorStoreMemCache::exist(char const* szKey, ///<key
                  CWX_UINT16 unKeyLen ///<key�ĳ���
                  )
{
    char key_buf[UnistorStoreMemCacheItem::calKeySize(unKeyLen, 0)];
    UnistorStoreMemCacheItem* key = (UnistorStoreMemCacheItem*)key_buf;
    memcpy(key->m_szBuf, szKey, unKeyLen);
    key->m_unKeyLen = unKeyLen;
    UnistorStoreMemCacheIter iter;
    iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(key->getKey(), key->getKeyLen())%m_bucket_num;
    {
        CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
        return _findKey(key, iter);
    }
}

///���CACHE    
void UnistorStoreMemCache::free( void ){
    CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
    CWX_UINT32 i=0;
    if (m_hashArr){
        delete [] m_hashArr;
        m_hashArr = NULL;
    }
    if (m_cacheBufArr){
        for (i=0; i<m_cacheBufArrSize; i++){
            if (m_cacheBufArr[i]) delete [] m_cacheBufArr[i];
        }
        delete [] m_cacheBufArr;
        m_cacheBufArr = NULL;
    }
    if (m_chainArr){
        delete [] m_chainArr;
        m_chainArr = NULL;
    }


    m_uiCurCheckSlotIndex = 0;
    m_uiCurCheckSlotPos = 0;
    if (m_arrExpiredItem) delete [] m_arrExpiredItem;
    m_arrExpiredItem = NULL;
    m_uiExpireNum = 0;

    for (i=0; i<UNISTOR_STORE_MEM_CACHE_CHAIN_LOCK_NUM; i++){
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

///���cache
void UnistorStoreMemCache::reset(void){
    CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
    CWX_UINT32 i=0;
    if (m_hashArr){
        memset(m_hashArr, 0x00, sizeof(UnistorStoreMemCacheItem*) * m_bucket_num);
    }
    if (m_chainArr){
        CWX_UINT32 uiMaxPinChainNum = UnistorStoreMemCacheItem::calMaxIndexNum(UNISTOR_STORE_MEM_CACHE_MAX_ITEM_SIZE);
        memset(m_chainArr, 0x00, sizeof(UnistorStoreMemCacheItemPin) * uiMaxPinChainNum);
    }
    if (m_cacheBufArr){
        for (i=0; i<m_cacheBufArrSize; i++){
            memset(m_cacheBufArr[i], 0x00, UNISTOR_STORE_MEM_CACHE_SLOT_SIZE);
        }
    }
    m_cacheBufArrIndex = 0;
    m_cacheBufArrPos = 0;

    if (m_arrExpiredItem){
        memset(m_arrExpiredItem, 0x00, sizeof(UnistorStoreMemCacheItem*) * m_uiMaxPerCheckExpireNum);
    }
    m_uiCurCheckSlotIndex = 0;
    m_uiCurCheckSlotPos = 0;
    m_uiExpireNum = 0;

    m_usedSize = 0;
    m_usedCapacity = 0;
    m_usedDataSize = 0;
    m_freeSize = 0;
    m_freeCapacity = 0;
    m_cachedKeyCount=0;
    m_cachedItemCount = 0;
    m_freeItemCount=0;
}

///��ⳬʱ��Ԫ�أ�key��expireʱ�䳬��uiCurTime��ȫ��ʧЧ
void UnistorStoreMemCache::checkExpire(CWX_UINT32 uiCurTime, CWX_UINT32 uiBatchNum){
    CWX_UINT32 uiSlot=m_uiCurCheckSlotIndex;
    CWX_UINT32 uiPos=m_uiCurCheckSlotPos;
    CWX_UINT32 uiTotalSlot = m_cacheBufArrIndex + 1;
    CWX_UINT32 uiCheckNum = 0;
    UnistorStoreMemCacheItem* key=NULL;
    m_uiExpireNum = 0;
    for (; uiSlot<uiTotalSlot; uiSlot++){
        while(UNISTOR_STORE_MEM_CACHE_SLOT_SIZE >= uiPos + sizeof(UnistorStoreMemCacheItem)){
            key = (UnistorStoreMemCacheItem*)(m_cacheBufArr[uiSlot] + uiPos);
            if (UnistorStoreMemCacheItem::UNISTOR_MEM_ITEM_STATE_UNMALLOC == key->m_ucState){
                break;///<ʣ��Ŀռ�û�з���
            }else if (UnistorStoreMemCacheItem::UNISTOR_MEM_ITEM_STATE_USED == key->m_ucState){
                if (key->m_uiExpire && (key->m_uiExpire < uiCurTime)){///<���Ƴ�ʱ
                    m_arrExpiredItem[m_uiExpireNum++] = key;
                }
            }
            uiCheckNum++;
            uiPos += key->size();
            ///����Ѿ������ָ������������break��
            if (uiCheckNum >= m_uiMaxPerCheckExpireNum) break;
        }
        ///����Ѿ������ָ������������break��
        if (uiCheckNum >= m_uiMaxPerCheckExpireNum) break;
        uiPos = 0;///<��ͷ��ʼ
    }
    ///����Ѿ����ˣ���break
    if (uiCheckNum < m_uiMaxPerCheckExpireNum){
        ///���ּ��ȫ�����
        CWX_ASSERT(uiSlot == uiTotalSlot);
        m_uiCurCheckSlotIndex = 0;
        m_uiCurCheckSlotPos = 0;
    }else{
        m_uiCurCheckSlotIndex = uiSlot;
        m_uiCurCheckSlotPos = uiPos;
    }
    CWX_UINT32 uiIndex = 0;
    UnistorStoreMemCacheIter iter;
    while(uiIndex < m_uiExpireNum){
        if (uiIndex + uiBatchNum > m_uiExpireNum){
            uiBatchNum = m_uiExpireNum - uiIndex;
        }
        {
            ///��ȡд��
            CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
            for (CWX_UINT32 i=0; i<uiBatchNum; i++){
                iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(m_arrExpiredItem[uiIndex]->getKey(), m_arrExpiredItem[uiIndex]->getKeyLen())%m_bucket_num;
                if (_findKey(m_arrExpiredItem[uiIndex], iter) && m_arrExpiredItem[uiIndex]->m_uiExpire && (m_arrExpiredItem[uiIndex]->m_uiExpire < uiCurTime)){
                    _remove(iter);
                }
                uiIndex++;
            } 

        }
    }
    m_uiExpireNum = 0;

}

///��������remove����
void UnistorStoreMemCache::_remove(UnistorStoreMemCacheIter const& iter){
    ///��hash��ɾ��key
    _removeKey(iter);
    iter.m_pFind->m_hashNext = NULL;
    iter.m_pFind->m_ucState = UnistorStoreMemCacheItem::UNISTOR_MEM_ITEM_STATE_FREE;
    ///��LRU������ɾ��key
    UnistorStoreMemCacheItem* data = iter.m_pFind;
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
void UnistorStoreMemCache::_touch(UnistorStoreMemCacheItem* data, CWX_UINT32 uiIndex){
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
void UnistorStoreMemCache::_addFreeList(CWX_UINT32 uiIndex, UnistorStoreMemCacheItem* item){
    ///����ŵ�free list��
    item->m_next = m_chainArr[uiIndex].m_freeHead;
    item->m_prev = NULL;
    item->m_hashNext = NULL;
    if (m_chainArr[uiIndex].m_freeHead){
        m_chainArr[uiIndex].m_freeHead->m_prev = item;
    }
    m_chainArr[uiIndex].m_freeHead = item;
}

void UnistorStoreMemCache::_addKey(CWX_UINT32 uiIndex, CWX_UINT32 uiSize, UnistorStoreMemCacheItem* item){
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
    m_usedCapacity += (uiSize - sizeof(UnistorStoreMemCacheItem));
    m_usedDataSize += item->m_uiDataLen + item->m_unKeyLen;
    m_cachedKeyCount ++;
    m_cachedItemCount ++;
    ///��ӵ�hash
    uiIndex = UnistorStoreMemCacheItem::m_fnHash(item->getKey(), item->getKeyLen())%m_bucket_num;
    item->m_hashNext = m_hashArr[uiIndex];
    m_hashArr[uiIndex] = item;
}


///��ȡkey
bool UnistorStoreMemCache::_findKey(UnistorStoreMemCacheItem const* item, UnistorStoreMemCacheIter& iter){
//    iter.m_uiHash = UnistorStoreMemCacheItem::m_fnHash(item->getKey(), item->getKeyLen())%m_bucket_num;
    iter.m_pFind = m_hashArr[iter.m_uiHash];
    iter.m_pPrev = NULL;
    while(iter.m_pFind){
        if (UnistorStoreMemCacheItem::m_fnEqual(item->getKey(),
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
