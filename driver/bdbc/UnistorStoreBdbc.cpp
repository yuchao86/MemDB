#include "UnistorStoreBdbc.h"
#include "CwxFile.h"

#define UNISTOR_BDBC_BIT32_MASK  0xFFFFFFFF

extern "C"{
    UnistorStoreBase* unistor_create_engine()
    {
        return new UnistorStoreBdbc();
    }
}

///初始化key的信息，此需要在bdbc引擎init的时候根据配置信息设置为正确的值
CWX_UINT8  UnistorBdbcKey::m_ucKeyType=UnistorBdbcKey::BDBC_KEY_TYPE_UNKNOWN;
CWX_UINT8  UnistorBdbcKey::m_ucBase=0;
bool       UnistorBdbcKey::m_bUpperHex=false;
UNISTOR_KEY_CMP_EQUAL_FN  UnistorBdbcKey::m_storeKeyEqualFn=NULL;
UNISTOR_KEY_CMP_LESS_FN   UnistorBdbcKey::m_storeKeyLessFn=NULL;
UNISTOR_KEY_HASH_FN       UnistorBdbcKey::m_storeKeyHashFn=NULL;
UNISTOR_KEY_GROUP_FN      UnistorBdbcKey::m_storeKeyGroupFn=NULL;
UNISTOR_KEY_CMP_EQUAL_FN  UnistorBdbcKey::m_asciiKeyEqualFn=NULL;
UNISTOR_KEY_CMP_LESS_FN   UnistorBdbcKey::m_asciiKeyLessFn=NULL;
UNISTOR_KEY_GROUP_FN      UnistorBdbcKey::m_asciiKeyGroupFn=NULL;
BDBC_KEY_CMP_LESS_FN      UnistorBdbcKey::m_bdbKeyLessFn=NULL;

/*******************************************************************************
bdb的key的比较函数定义。返回值：0，相等；-1：小于；1：大于
*******************************************************************************/
static int bdb_compare_int32(DB *, const DBT *a, const DBT *b){
    CWX_UINT32 k1= 0, k2= 0;
    memcpy(&k1, a->data, sizeof(CWX_UINT32));
    memcpy(&k2, b->data, sizeof(CWX_UINT32));
    return k1==k2?0:(k1<k2?-1:1);
}

static int bdb_compare_int64(DB *, const DBT *a, const DBT *b){
    CWX_UINT64 k1=0,k2=0;
    memcpy(&k1, a->data, sizeof(CWX_UINT64));
    memcpy(&k2, b->data, sizeof(CWX_UINT64));
    return k1==k2?0:(k1<k2?-1:1);
}

static int bdb_compare_int128(DB *, const DBT *a, const DBT *b){
    CWX_UINT64 k1_1=0, k1_2=0;
    CWX_UINT64 k2_1=0, k2_2=0;
    memcpy(&k1_1, a->data, sizeof(CWX_UINT64));
    memcpy(&k2_1, b->data, sizeof(CWX_UINT64));
    if (k1_1 == k2_1){
        memcpy(&k1_2, ((char*)a->data)+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        memcpy(&k2_2, ((char*)b->data)+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        return k1_2 == k2_2?0:(k1_2 < k2_2?-1:1);
    }
    return k1_1 < k2_1?-1:1;
}
static int bdb_compare_int256(DB *, const DBT *a, const DBT *b){
    CWX_UINT64 k1_1=0, k1_2=0, k1_3=0, k1_4=0;
    CWX_UINT64 k2_1=0, k2_2=0, k2_3=0, k2_4=0;
    memcpy(&k1_1, a->data, sizeof(CWX_UINT64));
    memcpy(&k2_1, b->data, sizeof(CWX_UINT64));
    if (k1_1 == k2_1){
        memcpy(&k1_2, ((char*)a->data)+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        memcpy(&k2_2, ((char*)b->data)+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        if (k1_2 == k2_2){
            memcpy(&k1_3, ((char*)a->data)+sizeof(CWX_UINT64)*2, sizeof(CWX_UINT64));
            memcpy(&k2_3, ((char*)b->data)+sizeof(CWX_UINT64)*2, sizeof(CWX_UINT64));
            if (k1_3 == k2_3){
                memcpy(&k1_4, ((char*)a->data)+sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
                memcpy(&k2_4, ((char*)b->data)+sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
                return k1_4 == k2_4?0:(k1_4 < k2_4?-1:1);
            }
            return k1_3 < k2_3?-1:1;
        }
        return k1_2 < k2_2?-1:1;
    }
    return k1_1 < k2_1?-1:1;
}

static int bdb_compare_char(DB *, const DBT *a, const DBT *b){
    char const* pa= (char const*)a->data;
    char const* pb= (char const*)b->data;
    int ret = memcmp(pa, pb, a->size<b->size?a->size-1:b->size-1);
    if (0 != ret) return ret;
    return a->size == b->size?0:(a->size < b->size?-1:1);
}

/*******************************************************************************
存储key的equal函数。返回值：true：相等；false：不相等
*******************************************************************************/
static bool store_equal_int32(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT32 k1=0, k2=2;
    memcpy(&k1, key1, sizeof(CWX_UINT32));
    memcpy(&k2, key2, sizeof(CWX_UINT32));
    return k1==k2;
}
static bool store_equal_int64(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT64 k1=0, k2=0;
    memcpy(&k1, key1, sizeof(CWX_UINT64));
    memcpy(&k2, key2, sizeof(CWX_UINT64));
    return k1==k2;
}
static bool store_equal_int128(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT64 k1_1=0, k1_2=0, k2_1=0, k2_2=0;
    memcpy(&k1_1, key1, sizeof(CWX_UINT64));
    memcpy(&k2_1, key2, sizeof(CWX_UINT64));
    if (k1_1 == k2_1){
        memcpy(&k1_2, key1+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        memcpy(&k2_2, key2+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        return k1_2 == k2_2;
    }
    return false;
}

static bool store_equal_int256(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT64 k1_1=0, k1_2=0, k1_3=0, k1_4=0, k2_1=0, k2_2=0, k2_3=0, k2_4=0;
    memcpy(&k1_1, key1, sizeof(CWX_UINT64));
    memcpy(&k2_1, key2, sizeof(CWX_UINT64));
    if (k1_1 == k2_1){
        memcpy(&k1_2, key1+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        memcpy(&k2_2, key2+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        if (k1_2 == k2_2){
            memcpy(&k1_3, key1+sizeof(CWX_UINT64)*2, sizeof(CWX_UINT64));
            memcpy(&k2_3, key2+sizeof(CWX_UINT64)*2, sizeof(CWX_UINT64));
            if (k1_3 == k2_3){
                memcpy(&k1_4, key1+sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
                memcpy(&k2_4, key2+sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
                return k1_4 == k2_4;
            }
        }
        return false;
    }
    return false;
}

static bool store_equal_char(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    if (unKey1Len != unKey2Len) return false;
    return memcmp(key1, key2, unKey1Len)==0;
}

/*******************************************************************************
存储key的less函数。返回值：0，相等；-1：小于；1：大于
*******************************************************************************/
static int store_less_int32(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT32 k1= 0, k2= 0;
    memcpy(&k1, key1, sizeof(CWX_UINT32));
    memcpy(&k2, key2, sizeof(CWX_UINT32));
    return k1==k2?0:(k1<k2?-1:1);
}

static int store_less_int64(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16){
    CWX_UINT64 k1=0,k2=0;
    memcpy(&k1, key1, sizeof(CWX_UINT64));
    memcpy(&k2, key2, sizeof(CWX_UINT64));
    return k1==k2?0:(k1<k2?-1:1);
}

static int store_less_int128(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16){
    CWX_UINT64 k1_1=0, k1_2=0;
    CWX_UINT64 k2_1=0, k2_2=0;
    memcpy(&k1_1, key1, sizeof(CWX_UINT64));
    memcpy(&k2_1, key2, sizeof(CWX_UINT64));
    if (k1_1 == k2_1){
        memcpy(&k1_2, key1+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        memcpy(&k2_2, key2+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        return k1_2 == k2_2?0:(k1_2 < k2_2?-1:1);
    }
    return k1_1 < k2_1?-1:1;
}
static int store_less_int256(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16){
    CWX_UINT64 k1_1=0, k1_2=0, k1_3=0, k1_4=0;
    CWX_UINT64 k2_1=0, k2_2=0, k2_3=0, k2_4=0;
    memcpy(&k1_1, key1, sizeof(CWX_UINT64));
    memcpy(&k2_1, key2, sizeof(CWX_UINT64));
    if (k1_1 == k2_1){
        memcpy(&k1_2, key1+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        memcpy(&k2_2, key2+sizeof(CWX_UINT64), sizeof(CWX_UINT64));
        if (k1_2 == k2_2){
            memcpy(&k1_3, key1+sizeof(CWX_UINT64)*2, sizeof(CWX_UINT64));
            memcpy(&k2_3, key2+sizeof(CWX_UINT64)*2, sizeof(CWX_UINT64));
            if (k1_3 == k2_3){
                memcpy(&k1_4, key1+sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
                memcpy(&k2_4, key2+sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
                return k1_4 == k2_4?0:(k1_4 < k2_4?-1:1);
            }
            return k1_3 < k2_3?-1:1;
        }
        return k1_2 < k2_2?-1:1;
    }
    return k1_1 < k2_1?-1:1;
}

static int store_less_char(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    int ret = memcmp(key1, key2, unKey1Len<unKey2Len?unKey1Len:unKey2Len);
    if (0 != ret) return ret;
    return unKey1Len == unKey2Len?0:(unKey1Len < unKey2Len?-1:1);
}

/*******************************************************************************
存储key的hash函数。返回值hash值
*******************************************************************************/
static size_t store_hash_int32(char const* key, CWX_UINT16){
    CWX_UINT32 uiValue = 0;
    memcpy(&uiValue, key, sizeof(CWX_UINT32));
    return uiValue;
}

static size_t store_hash_int64(char const* key, CWX_UINT16 ){
    CWX_UINT64 ullValue = 0;
    memcpy(&ullValue, key, sizeof(CWX_UINT64));
    return (CWX_UINT32)(ullValue&UNISTOR_BDBC_BIT32_MASK);
}

static size_t store_hash_int128(char const* key, CWX_UINT16 ){
    CWX_UINT64 ullValue = 0;
    memcpy(&ullValue, key + sizeof(CWX_UINT64), sizeof(CWX_UINT64));
    return (CWX_UINT32)(ullValue&UNISTOR_BDBC_BIT32_MASK);
}

static size_t store_hash_int256(char const* key, CWX_UINT16){
    CWX_UINT64 ullValue = 0;
    memcpy(&ullValue, key + sizeof(CWX_UINT64)*3, sizeof(CWX_UINT64));
    return (CWX_UINT32)(ullValue&UNISTOR_BDBC_BIT32_MASK);
}

static size_t store_hash_char(char const* key, CWX_UINT16 unKeyLen){
    size_t h = 216613626UL;
    for (CWX_UINT16 i = 0; i < unKeyLen; ++i) {
        h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
        h ^= key[i];
    }
    return h;
}
/*******************************************************************************
存储key的group函数。返回值hash值
*******************************************************************************/
static CWX_UINT32 store_group_int32(char const* key, CWX_UINT16 unKeyLen){
    return store_hash_int32(key, unKeyLen);
}

static CWX_UINT32 store_group_int64(char const* key, CWX_UINT16 unKeyLen){
    return store_hash_int64(key, unKeyLen);
}

static CWX_UINT32 store_group_int128(char const* key, CWX_UINT16 unKeyLen){
    return store_hash_int128(key, unKeyLen);
}

static CWX_UINT32 store_group_int256(char const* key, CWX_UINT16 unKeyLen){
    return store_hash_int256(key, unKeyLen);
}

static CWX_UINT32 store_group_char(char const* key, CWX_UINT16 unKeyLen){
    CWX_UINT32 uiGroup = 0;
    CwxMd5 md5;
    unsigned char szMd5[16];
    md5.update((unsigned char const*)key, unKeyLen);
    md5.final(szMd5);
    memcpy(&uiGroup, szMd5+12, 4);
    return uiGroup;
}

/*******************************************************************************
ascii key的equal函数。返回值：true：相等；false：不相等
*******************************************************************************/
static bool ascii_equal_int32(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT32 k1=strtoul(key1, NULL, 0);
    CWX_UINT32 k2=strtoul(key2, NULL, 0);
    return k1==k2;
}
static bool ascii_equal_int64(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT64 k1=strtoull(key1, NULL, 0);
    CWX_UINT64 k2=strtoull(key2, NULL, 0);
    return k1==k2;
}

static bool ascii_equal_int128(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    char szKey1[65];
    char szKey2[65];
    char const* pKey1 = key1;
    char const* pKey2 = key2;
    if (32 > unKey1Len){
        memcpy(szKey1, BDBC_HEX_64_ZERO, 32 - unKey1Len);
        memcpy(szKey1 + 32 - unKey1Len, key1, unKey1Len);
        pKey1 = szKey1;
    }
    if (32 > unKey2Len){
        memcpy(szKey2, BDBC_HEX_64_ZERO, 32 - unKey2Len);
        memcpy(szKey2 + 32 - unKey2Len, key2, unKey2Len);
        pKey2 = szKey2;
    }
    return strncasecmp(pKey1, pKey2, 32)==0;
}

static bool ascii_equal_int256(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    char szKey1[65];
    char szKey2[65];
    char const* pKey1 = key1;
    char const* pKey2 = key2;
    if (64 > unKey1Len){
        memcpy(szKey1, BDBC_HEX_64_ZERO, 64 - unKey1Len);
        memcpy(szKey1 + 64 - unKey1Len, key1, unKey1Len);
        pKey1 = szKey1;
    }
    if (64 > unKey2Len){
        memcpy(szKey2, BDBC_HEX_64_ZERO, 64 - unKey2Len);
        memcpy(szKey2 + 64 - unKey2Len, key2, unKey2Len);
        pKey2 = szKey2;
    }
    return strncasecmp(pKey1, pKey2, 32)==0;
}

static bool ascii_equal_char(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    if (unKey1Len != unKey2Len) return false;
    return memcmp(key1, key2, unKey1Len)==0;
}

/*******************************************************************************
ascii key的less函数。返回值：0：相等；-1：小于；1：大于
*******************************************************************************/
static int ascii_less_int32(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT32 k1=strtoul(key1, NULL, 0);
    CWX_UINT32 k2=strtoul(key2, NULL, 0);
    return k1==k2?0:(k1<k2?-1:1);
}
static int ascii_less_int64(char const* key1, CWX_UINT16 , char const* key2, CWX_UINT16 ){
    CWX_UINT64 k1=strtoull(key1, NULL, 0);
    CWX_UINT64 k2=strtoull(key2, NULL, 0);
    return k1==k2?0:(k1<k2?-1:1);
}

static int ascii_less_int128(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    char szKey1[65];
    char szKey2[65];
    char const* pKey1 = key1;
    char const* pKey2 = key2;
    if (32 > unKey1Len){
        memcpy(szKey1, BDBC_HEX_64_ZERO, 32 - unKey1Len);
        memcpy(szKey1 + 32 - unKey1Len, key1, unKey1Len);
        pKey1 = szKey1;
    }
    if (32 > unKey2Len){
        memcpy(szKey2, BDBC_HEX_64_ZERO, 32 - unKey2Len);
        memcpy(szKey2 + 32 - unKey2Len, key2, unKey2Len);
        pKey2 = szKey2;
    }
    int ret= strncasecmp(pKey1, pKey2, 32);
    return ret==0?0:(ret<0?-1:1);
}

static int ascii_less_int256(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    char szKey1[65];
    char szKey2[65];
    char const* pKey1 = key1;
    char const* pKey2 = key2;
    if (64 > unKey1Len){
        memcpy(szKey1, BDBC_HEX_64_ZERO, 64 - unKey1Len);
        memcpy(szKey1 + 64 - unKey1Len, key1, unKey1Len);
        pKey1 = szKey1;
    }
    if (64 > unKey2Len){
        memcpy(szKey2, BDBC_HEX_64_ZERO, 64 - unKey2Len);
        memcpy(szKey2 + 64 - unKey2Len, key2, unKey2Len);
        pKey2 = szKey2;
    }
    int ret= strncasecmp(pKey1, pKey2, 32);
    return ret==0?0:(ret<0?-1:1);
}

static int ascii_less_char(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len){
    if (unKey1Len != unKey2Len) return false;
    return memcmp(key1, key2, unKey1Len)==0;
}

/*******************************************************************************
ascii key的group函数。返回值：key的group值
*******************************************************************************/
static CWX_UINT32 ascii_group_int32(char const* key, CWX_UINT16){
    return strtoul(key, NULL, 0);
}
static CWX_UINT32 ascii_group_int64(char const* key, CWX_UINT16){
    return (CWX_UINT32)(strtoull(key, NULL, 0)&UNISTOR_BDBC_BIT32_MASK);
}

static CWX_UINT32 ascii_group_int128(char const* key, CWX_UINT16 unKeyLen){
    if (!key) return 0;
    if (unKeyLen > 32) unKeyLen = 32;
    if (unKeyLen>8){
        return strtoul(key + unKeyLen - 8, NULL, 16) ;
    }
    return strtoul(key, NULL, 16);
}

static CWX_UINT32 ascii_group_int256(char const* key, CWX_UINT16 unKeyLen){
    if (!key) return 0;
    if (unKeyLen > 64) unKeyLen = 64;
    if (unKeyLen>8){
        return strtoul(key + unKeyLen - 8, NULL, 16);
    }
    return strtoul(key, NULL, 16);
}

static CWX_UINT32 ascii_group_char(char const* key, CWX_UINT16 unKeyLen){
    return store_group_char(key, unKeyLen);
}

/******************************************************************************
计数器数据对象
*******************************************************************************/
///将数据空间解析为计数器
bool UnistorBdbcData::decode(unsigned char const* szData,
                               CWX_UINT32 uiDataLen,
                               char* szErr2k)
{
    this->reset();
    if (!uiDataLen){
        if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Invalid counter content.");
        return false;
    }
    m_ucCounterNum = szData[0]; szData++; uiDataLen--;
    ///获取计数器
    for (CWX_UINT32 i=1; i<=m_ucCounterNum; i++){
        if (!uiDataLen){
            if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Invalid counter content.");
            return false;
        }
        m_counter[i].m_ucId = szData[0]; szData++; uiDataLen--;
        szData = (unsigned char*)CwxPackageEx::decodeUint32((unsigned char*)szData,
            uiDataLen,
            m_counter[i].m_uiValue);
        if (!szData){
            if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Failure to encode counter, id=%u", i);
            return false;
        }
        if (!m_counter[i].m_ucId){
            if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Counter id for zero is invalid. The index is %u", i);
            return false;
        }
        m_counterMap[m_counter[i].m_ucId] = i; ///<计数器的索引
    }
    ///获取版本号
    if (!uiDataLen){
        if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Invalid counter content.");
        return false;
    }
    if (!CwxPackageEx::decodeUint32((unsigned char*)szData, uiDataLen, m_uiVersion)){
        if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Failure to encode counter version.");
        return false;
    }
    if (0 != uiDataLen){
        if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "Invalid counter content.");
        return false;
    }
    return true;
}

///将计数器编码为数据空间，返回pack的有效计数器的数量。若失败为-1。
int UnistorBdbcData::encode(unsigned char* szData, CWX_UINT32& uiDataLen, char* szErr2k) const{
    CWX_UINT32 uiEncodeLen=0;
    CWX_UINT32 uiPos = 1;
    CWX_UINT8  ucSkipNum = 0;
    //encode 计数器的数量
    for (CWX_UINT8 i=1; i<=m_ucCounterNum; i++){
        if (uiPos + 6 > uiDataLen){
            if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "No space for encode. data buf len:%s, now length is :%u", uiPos + 6);
            return -1;
        }
        if (m_counter[i].m_uiValue){
            szData[uiPos] = m_counter[i].m_ucId; uiPos++;///设置名字
            CwxPackageEx::encodeUint32(m_counter[i].m_uiValue, szData + uiPos, uiEncodeLen);
            uiPos += uiEncodeLen;
        }else{
            ucSkipNum++;
        }
    }
    szData[0] = m_ucCounterNum - ucSkipNum;
    //encode版本号
    if (uiPos + 6 > uiDataLen){
        if (szErr2k) CwxCommon::snprintf(szErr2k, 2047, "No space for encode. data buf len:%s, now length is :%u", uiPos + 6);
        return -1;
    }
    CwxPackageEx::encodeUint32(m_uiVersion, szData + uiPos, uiEncodeLen);
    uiDataLen = uiPos + uiEncodeLen;
    return szData[0];
}

/******************************************************************************
计数器名字配置对象
*******************************************************************************/

///解析配置信息。返回值：true，成功；false：失败。
bool UnistorStoreBdbcCounterMap::parse(char const* szDef, CWX_UINT32 uiLen, CWX_UINT32 ttTimestamp, char* szErr2K)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    string strDef(szDef, uiLen);
    list<pair<string, string> > values;
    list<pair<string, string> >::iterator iter;
    string strVersion;
    string strValue;
    CWX_UINT8 ucId = 0;
    char* szName = NULL;
    CwxCommon::split(strDef, values, '\n');
    iter = values.begin();
    set<CWX_UINT8> ids;
    map<char const*, CWX_UINT8, CwxCharLess> counters;
    char szBuf[16];
    while(iter != values.end()){
        if (!strVersion.length()){
            strValue = iter->first;
            CwxCommon::trim(strValue);
            if (strValue != BDBC_KEY_VERSION){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Missing [%s]'s information.", BDBC_KEY_VERSION);
                return false;
            }
            strVersion = strValue;
        }else{
            strValue = iter->first;
            CwxCommon::trim(strValue, "\r\n");
            ucId = strtoul(strValue.c_str(), NULL, 10);
            if (!ucId){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Counter id must start from 1. zero is invalid.");
                return false;
            }
            sprintf(szBuf, "%u", ucId);
            if (strValue != szBuf){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Counter id[%s] is not a valid id, it must be [1~255].", strValue.c_str());
                return false;
            }
            if (ids.find(ucId) != ids.end()){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Counter id[%s] is duplicate.", strValue.c_str());
                return false;
            }
            strValue = iter->second;
            CwxCommon::trim(strValue, "\r\n");
            if (!strValue.length()){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Counter id[%u]'s name is empty.", ucId);
                return false;
            }
            if (counters.find(strValue.c_str()) != counters.end()){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Counter name[%s] is duplicate.", strValue.c_str());
                return false;
            }
            ids.insert(ucId);
            szName = new char[strValue.length() + 1];
            strcpy(szName, strValue.c_str());
            counters[szName] = ucId;
        }
        iter++;
    }

    CWX_UINT8 i=0;
    ///清空已有数据
    for(i=0; i<255; i++){
        if (m_idNameMap[i]) delete [] m_idNameMap[i];
        m_idNameMap[i] = NULL;
    }
    m_nameIdMap.clear();
    ///分配新值
    m_ttTimestamp = ttTimestamp;
    m_strDef = strDef;
    m_ullSeq++;
    m_strVersion = strVersion;
    map<char const*, CWX_UINT8, CwxCharLess>::iterator iter_counter = counters.begin();
    while(iter_counter != counters.end()){
        m_idNameMap[iter_counter->second] = (char*)iter_counter->first;
        iter_counter++;
    }
    m_nameIdMap = counters;
    return true;
}

///clone出一个新定义共各个相关线程使用，此需要加锁。
void UnistorStoreBdbcCounterMap::clone(UnistorStoreBdbcCounterMap& def){
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    def.m_ttTimestamp = m_ttTimestamp;
    def.m_ullSeq = m_ullSeq;
    def.m_strDef = m_strDef;
    def.m_strVersion = m_strVersion;
    def.m_nameIdMap.clear();
    for (CWX_UINT16 i=0; i<256; i++){
        if (m_idNameMap[i]){
            if (def.m_idNameMap[i]){
                delete [] def.m_idNameMap[i];
            }
            def.m_idNameMap[i] = new char[strlen(m_idNameMap[i]) + 1];
            strcpy(def.m_idNameMap[i], m_idNameMap[i]);
            def.m_nameIdMap[def.m_idNameMap[i]] = i;
        }else{
            if (def.m_idNameMap[i]){
                delete [] def.m_idNameMap[i];
                def.m_idNameMap[i] = NULL;
            }
        }
    }
}

/******************************************************************************
计数器引擎对象定义
*******************************************************************************/

//0:成功；-1：失败
int UnistorStoreBdbc::_parseConf(){
    string value;
    //get bdbc:env_home
    if (!m_config->getConfFileCnf().getAttr("bdbc", "env_home", value) || !value.length()){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:env_home].");
        return -1;
    }
    if (value.length()){
        m_bdbcConf.m_strEnvPath = value;
        if ('/' != value[value.length()-1]) m_bdbcConf.m_strEnvPath +="/";
    }else{
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:env_home].");
        return -1;
    }

    //get bdbc:db_path
    if (!m_config->getConfFileCnf().getAttr("bdbc", "db_path", value) || !value.length()){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:db_path].");
        return -1;
    }
    if (value.length()){
        m_bdbcConf.m_strDbPath = value;
        if ('/' != value[value.length()-1]) m_bdbcConf.m_strDbPath +="/";
    }else{
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:db_path].");
        return -1;
    }
    //get bdbc:compress
    m_bdbcConf.m_bZip = false;
    if (m_config->getConfFileCnf().getAttr("bdbc", "compress", value)){
        if (value == "yes") m_bdbcConf.m_bZip = true;
    }
    //get bdbc:cache_msize
    if (!m_config->getConfFileCnf().getAttr("bdbc", "cache_msize", value) || !value.length() ){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:cache_msize].");
        return -1;
    }
    m_bdbcConf.m_uiCacheMByte = strtoul(value.c_str(), NULL, 10);
    if (m_bdbcConf.m_uiCacheMByte < 64) m_bdbcConf.m_uiCacheMByte=64;

    m_bdbcConf.m_uiPageKSize = 0;
    if (m_config->getConfFileCnf().getAttr("bdbc", "page_ksize", value)){
        m_bdbcConf.m_uiPageKSize = strtoul(value.c_str(), NULL, 10);
        if (m_bdbcConf.m_uiPageKSize > 64) m_bdbcConf.m_uiPageKSize = 64;
    }
    //get bdbc:key_type
    if (!m_config->getConfFileCnf().getAttr("bdbc", "key_type", value) || !value.length() ){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:key_type].");
        return -1;
    }
    if (value == BDBC_ENGINE_KEY_TYPE_INT32){
        m_bdbcConf.m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT32;
    }else if (value == BDBC_ENGINE_KEY_TYPE_INT64){
        m_bdbcConf.m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT64;
    }else if (value == BDBC_ENGINE_KEY_TYPE_INT128){
        m_bdbcConf.m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT128;
    }else if (value == BDBC_ENGINE_KEY_TYPE_INT256){
        m_bdbcConf.m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT256;
    }else if (value == BDBC_ENGINE_KEY_TYPE_CHAR){
        m_bdbcConf.m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_CHAR;
    }else{
        CwxCommon::snprintf(m_szErrMsg, 2047, "Invalid key type[%s], must be [%s] or [%s] or [%s] or [%s] or [%s].",
            value.c_str(), BDBC_ENGINE_KEY_TYPE_INT32, BDBC_ENGINE_KEY_TYPE_INT64, BDBC_ENGINE_KEY_TYPE_INT128,
            BDBC_ENGINE_KEY_TYPE_INT256, BDBC_ENGINE_KEY_TYPE_CHAR);
        return -1;
    }
    m_bdbcConf.m_strKeyType = value;

    //get bdbc:int32_hex
    m_bdbcConf.m_bInt32Hex = false;
    if (m_config->getConfFileCnf().getAttr("bdbc", "int32_hex", value)){
        if (value == "yes"){
            m_bdbcConf.m_bInt32Hex = true;
        }
    }

    //get bdbc:int64_hex
    m_bdbcConf.m_bInt64Hex = true;
    if (m_config->getConfFileCnf().getAttr("bdbc", "int64_hex", value)){
        if (value == "no"){
            m_bdbcConf.m_bInt64Hex = false;
        }
    }

    //get bdbc:hex_upper
    m_bdbcConf.m_bHexUpper = false;
    if (m_config->getConfFileCnf().getAttr("bdbc", "hex_upper", value)){
        if (value == "yes"){
            m_bdbcConf.m_bHexUpper = true;
        }
    }

    //get bdbc:group_start_time_bit
    if (!m_config->getConfFileCnf().getAttr("bdbc", "group_start_time_bit", value) || !value.length()){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:group_start_time_bit].");
        return -1;
    }
    m_bdbcConf.m_ucGroupStartBit = strtoul(value.c_str(), NULL, 10);
    if (m_bdbcConf.m_ucGroupStartBit > 31){
        CwxCommon::snprintf(m_szErrMsg, 2047, "[bdbc:group_start_time_bit] must be in [0,31]. [%u] is invalid", m_bdbcConf.m_ucGroupStartBit);
        return -1;
    }
    //get bdbc:group_end_time_bit
    if (!m_config->getConfFileCnf().getAttr("bdbc", "group_end_time_bit", value) || !value.length()){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:group_end_time_bit].");
        return -1;
    }
    m_bdbcConf.m_ucGroupEndBit = strtoul(value.c_str(), NULL, 10);
    if (m_bdbcConf.m_ucGroupEndBit > 31){
        CwxCommon::snprintf(m_szErrMsg, 2047, "[bdbc:group_end_time_bit] must be in [0,31]. [%u] is invalid", m_bdbcConf.m_ucGroupEndBit);
        return -1;
    }
    if (m_bdbcConf.m_ucGroupStartBit > m_bdbcConf.m_ucGroupEndBit){
        CwxCommon::snprintf(m_szErrMsg, 2047, "group_end_time_bit[%u] must be more than group_start_time_bit[%u].", m_bdbcConf.m_ucGroupEndBit, m_bdbcConf.m_ucGroupStartBit);
        return -1;
    }

    //get bdbc.counter_def_file
    if (!m_config->getConfFileCnf().getAttr("bdbc", "counter_def_file", value) || !value.length()){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Must set [bdbc:counter_def_file].");
        return -1;
    }
    m_bdbcConf.m_strCounterDefFile = value;
    if (0 != _checkCounterDef(m_szErrMsg)){
        return -1;
    }

    CWX_INFO(("*****************begin bdb conf****************"));
    CWX_INFO(("env_home=%s", m_bdbcConf.m_strEnvPath.c_str()));
    CWX_INFO(("db_path=%s", m_bdbcConf.m_strDbPath.c_str()));
    CWX_INFO(("compress=%s", m_bdbcConf.m_bZip?"yes":"no"));
    CWX_INFO(("cache_msize=%u", m_bdbcConf.m_uiCacheMByte));
    CWX_INFO(("page_ksize=%u", m_bdbcConf.m_uiPageKSize));
    CWX_INFO(("key_type=%s", m_bdbcConf.m_strKeyType.c_str()));
    CWX_INFO(("int32_hex=%u", m_bdbcConf.m_bInt32Hex?"yes":"no"));
    CWX_INFO(("int64_hex=%u", m_bdbcConf.m_bInt64Hex?"yes":"no"));
    CWX_INFO(("hex_upper=%u", m_bdbcConf.m_bHexUpper?"yes":"no"));
    CWX_INFO(("group_start_time_bit=%u", m_bdbcConf.m_ucGroupStartBit));
    CWX_INFO(("group_end_time_bit=%u", m_bdbcConf.m_ucGroupEndBit));
    CWX_INFO(("counter_def_file=%s", m_bdbcConf.m_strCounterDefFile.c_str()));
    CWX_INFO(("memcache_write_cache_msize=%u", m_uiWriteCacheMSize));
    CWX_INFO(("memcache_read_cache_msize=%u", m_uiReadCacheMSize));
    CWX_INFO(("memcache_read_cache_item=%u", m_uiReadCacheItemNum));
    CWX_INFO(("*****************end bdb conf****************"));
    return 0;
}

int UnistorStoreBdbc::_installFunc(){
    if (UnistorBdbcKey::BDBC_KEY_TYPE_INT32 == m_bdbcConf.m_ucKeyType){
        UnistorBdbcKey::m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT32;
        UnistorBdbcKey::m_ucBase = m_bdbcConf.m_bInt32Hex?16:10;
        UnistorBdbcKey::m_bUpperHex = m_bdbcConf.m_bHexUpper;
        UnistorBdbcKey::m_storeKeyEqualFn = store_equal_int32;
        UnistorBdbcKey::m_storeKeyLessFn = store_less_int32;
        UnistorBdbcKey::m_storeKeyHashFn = store_hash_int32;
        UnistorBdbcKey::m_storeKeyGroupFn = store_group_int32;
        UnistorBdbcKey::m_asciiKeyEqualFn = ascii_equal_int32;
        UnistorBdbcKey::m_asciiKeyLessFn = ascii_less_int32;
        UnistorBdbcKey::m_asciiKeyGroupFn = ascii_group_int32;
        UnistorBdbcKey::m_bdbKeyLessFn = bdb_compare_int32;
    }else if (UnistorBdbcKey::BDBC_KEY_TYPE_INT64 == m_bdbcConf.m_ucKeyType){
        UnistorBdbcKey::m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT64;
        UnistorBdbcKey::m_ucBase = m_bdbcConf.m_bInt64Hex?16:10;
        UnistorBdbcKey::m_bUpperHex = m_bdbcConf.m_bHexUpper;
        UnistorBdbcKey::m_storeKeyEqualFn = store_equal_int64;
        UnistorBdbcKey::m_storeKeyLessFn = store_less_int64;
        UnistorBdbcKey::m_storeKeyHashFn = store_hash_int64;
        UnistorBdbcKey::m_storeKeyGroupFn = store_group_int64;
        UnistorBdbcKey::m_asciiKeyEqualFn = ascii_equal_int64;
        UnistorBdbcKey::m_asciiKeyLessFn = ascii_less_int64;
        UnistorBdbcKey::m_asciiKeyGroupFn = ascii_group_int64;
        UnistorBdbcKey::m_bdbKeyLessFn = bdb_compare_int64;
    }else if (UnistorBdbcKey::BDBC_KEY_TYPE_INT128 == m_bdbcConf.m_ucKeyType){
        UnistorBdbcKey::m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT128;
        UnistorBdbcKey::m_ucBase = 16;
        UnistorBdbcKey::m_bUpperHex = m_bdbcConf.m_bHexUpper;
        UnistorBdbcKey::m_storeKeyEqualFn = store_equal_int128;
        UnistorBdbcKey::m_storeKeyLessFn = store_less_int128;
        UnistorBdbcKey::m_storeKeyHashFn = store_hash_int128;
        UnistorBdbcKey::m_storeKeyGroupFn = store_group_int128;
        UnistorBdbcKey::m_asciiKeyEqualFn = ascii_equal_int128;
        UnistorBdbcKey::m_asciiKeyLessFn = ascii_less_int128;
        UnistorBdbcKey::m_asciiKeyGroupFn = ascii_group_int128;
        UnistorBdbcKey::m_bdbKeyLessFn = bdb_compare_int128;
    }else if (UnistorBdbcKey::BDBC_KEY_TYPE_INT256 == m_bdbcConf.m_ucKeyType){
        UnistorBdbcKey::m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_INT256;
        UnistorBdbcKey::m_ucBase = 16;
        UnistorBdbcKey::m_bUpperHex = m_bdbcConf.m_bHexUpper;
        UnistorBdbcKey::m_storeKeyEqualFn = store_equal_int256;
        UnistorBdbcKey::m_storeKeyLessFn = store_less_int256;
        UnistorBdbcKey::m_storeKeyHashFn = store_hash_int256;
        UnistorBdbcKey::m_storeKeyGroupFn = store_group_int256;
        UnistorBdbcKey::m_asciiKeyEqualFn = ascii_equal_int256;
        UnistorBdbcKey::m_asciiKeyLessFn = ascii_less_int256;
        UnistorBdbcKey::m_asciiKeyGroupFn = ascii_group_int256;
        UnistorBdbcKey::m_bdbKeyLessFn = bdb_compare_int256;
    }else if (UnistorBdbcKey::BDBC_KEY_TYPE_CHAR == m_bdbcConf.m_ucKeyType){
        UnistorBdbcKey::m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_CHAR;
        UnistorBdbcKey::m_ucBase = 16;
        UnistorBdbcKey::m_bUpperHex = true;
        UnistorBdbcKey::m_storeKeyEqualFn = store_equal_char;
        UnistorBdbcKey::m_storeKeyLessFn = store_less_char;
        UnistorBdbcKey::m_storeKeyHashFn = store_hash_char;
        UnistorBdbcKey::m_storeKeyGroupFn = store_group_char;
        UnistorBdbcKey::m_asciiKeyEqualFn = ascii_equal_char;
        UnistorBdbcKey::m_asciiKeyLessFn = ascii_less_char;
        UnistorBdbcKey::m_asciiKeyGroupFn = ascii_group_char;
        UnistorBdbcKey::m_bdbKeyLessFn = bdb_compare_char;
    }else{
        CwxCommon::snprintf(m_szErrMsg, 2047, "Invalid key type[%u]", m_bdbcConf.m_ucKeyType);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }
    UnistorStoreBase::m_fnKeyStoreGroup = UnistorBdbcKey::m_storeKeyGroupFn;
    UnistorStoreBase::m_fnKeyAsciiGroup = UnistorBdbcKey::m_asciiKeyGroupFn;
    UnistorStoreBase::m_fnKeyAsciiLess = UnistorBdbcKey::m_asciiKeyLessFn;
    return 0;
}


int UnistorStoreBdbc::_checkSysInfo(){
    string strPathFile;
    strPathFile = m_bdbcConf.m_strDbPath + BDBC_SYS_FILE_NAME;
    char szFileName[512];
    if (CwxFile::isFile(strPathFile.c_str())){
        CWX_INFO(("Sys bdb file[%s] exists.", BDBC_SYS_FILE_NAME));
        for (CWX_UINT8 i=0; i<m_uiGroupNum; i++){
            CwxCommon::snprintf(szFileName, 512, BDBC_DATA_FILE_FORMAT, m_bdbcConf.m_ucGroupStartBit,  m_uiGroupNum, i);
            strPathFile = m_bdbcConf.m_strDbPath + szFileName;
            if (!CwxFile::isFile(strPathFile.c_str())){
                CwxCommon::snprintf(m_szErrMsg, 2047, "Sys bdb file[%s] exists but db file[%s] is missing", BDBC_SYS_FILE_NAME, szFileName);
                CWX_ERROR((m_szErrMsg));
                return -1;
            }
        }
    }else{
        for (CWX_UINT8 i=0; i<m_uiGroupNum; i++){
            CwxCommon::snprintf(szFileName, 512, BDBC_DATA_FILE_FORMAT, m_bdbcConf.m_ucGroupStartBit,  m_uiGroupNum, i);
            strPathFile = m_bdbcConf.m_strDbPath + szFileName;
            if (CwxFile::isFile(strPathFile.c_str())){
                CwxCommon::snprintf(m_szErrMsg, 2047, "Sys bdb file[%s] is missing but db file[%s] exists", BDBC_SYS_FILE_NAME, szFileName);
                CWX_ERROR((m_szErrMsg));
                return -1;
            }
        }
    }
    return 0;
}

///获取计数器的配置信息。0：成功；-1：失败
int UnistorStoreBdbc::_checkCounterDef(char* szErr2K){
    bool bLoad = false;
    CWX_UINT32 ttTimestamp = 0;
    if (!m_pCounterDef){
        m_pCounterDef = new UnistorStoreBdbcCounterMap();
        ttTimestamp = CwxFile::getFileMTime(m_bdbcConf.m_strCounterDefFile.c_str());
        bLoad = true;
    }else{
        ttTimestamp = CwxFile::getFileMTime(m_bdbcConf.m_strCounterDefFile.c_str());
        if (ttTimestamp !=  m_pCounterDef->getTimestamp()){
            bLoad = true;
        }
    }
    if (bLoad){
        CWX_INFO(("Begin to load counter conf file:%s", m_bdbcConf.m_strCounterDefFile.c_str()));
        string strFileContent;
        if (!CwxFile::readTxtFile(m_bdbcConf.m_strCounterDefFile, strFileContent)){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to read counter file:%s, errno=%d", 
                m_bdbcConf.m_strCounterDefFile.c_str(), errno);
            return -1;
        }
        if (!m_pCounterDef->parse(strFileContent.c_str(), strFileContent.length(), ttTimestamp, szErr2K)){
            return -1;
        }
        CWX_INFO(("End to load counter conf file:%s", m_bdbcConf.m_strCounterDefFile.c_str()));
    }
    return 0;
}


//加载配置文件.-1:failure, 0:success
int UnistorStoreBdbc::init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc,
                             UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<获取系统信息的function
                             void* pApp, ///<UnistorApp对象
                             UnistorConfig const* config)
{
    int ret = 0;
    CWX_UINT32 i=0;
    m_bValid = false;
    strcpy(m_szErrMsg, "Not init");
    ///调用父类的初始化
    if (0 != UnistorStoreBase::init(msgPipeFunc, getSysInfoFunc, pApp, config)) return -1;

    ///设置cache内存信息
    m_uiWriteCacheMSize = m_config->getCommon().m_uiWriteCacheMByte;
    if (m_uiWriteCacheMSize < BDBC_MIN_WRITE_CACHE_MSIZE) m_uiWriteCacheMSize = BDBC_MIN_WRITE_CACHE_MSIZE;
    if (m_uiWriteCacheMSize > BDBC_MAX_WRITE_CACHE_MSIZE) m_uiWriteCacheMSize = BDBC_MAX_WRITE_CACHE_MSIZE;

    m_uiReadCacheMSize = m_config->getCommon().m_uiReadCacheMByte;
    if (m_uiReadCacheMSize < BDBC_MIN_READ_CACHE_MSIZE) m_uiReadCacheMSize = BDBC_MIN_READ_CACHE_MSIZE;
    if (m_uiReadCacheMSize > BDBC_MAX_READ_CACHE_MSIZE) m_uiReadCacheMSize = BDBC_MAX_READ_CACHE_MSIZE;

    m_uiReadCacheItemNum = m_config->getCommon().m_uiReadCacheMaxKeyNum;
    if (m_uiReadCacheItemNum < BDBC_MIN_READ_CACHE_KEY_NUM) m_uiReadCacheItemNum = BDBC_MIN_READ_CACHE_KEY_NUM;
    if (m_uiReadCacheItemNum > BDBC_MAX_READ_CACHE_KEY_NUM) m_uiReadCacheItemNum = BDBC_MAX_READ_CACHE_KEY_NUM;

    //parse conf
    if (0 != _parseConf()){
        CWX_ERROR(("Failure to parse conf file, err=%s", m_szErrMsg));
        return -1;
    }
    ///安装引擎函数
    if (0 != _installFunc()){
        CWX_ERROR(("Failure to install engine method, err=%s", m_szErrMsg));
        return -1;
    }
    
    ///设置分组的掩码
    m_uiGroupMask = 0;
    for (CWX_UINT8 bit=m_bdbcConf.m_ucGroupStartBit; bit<=m_bdbcConf.m_ucGroupEndBit; bit++){
        m_uiGroupMask |= (1<<bit);
    }
    m_uiGroupNum = (m_uiGroupMask >> m_bdbcConf.m_ucGroupStartBit) + 1;

    CWX_INFO(("Group num:%u", m_uiGroupNum));

    //启动cache
    if (0 != startCache(m_uiWriteCacheMSize,
        m_uiReadCacheMSize,
        m_uiReadCacheItemNum,
        UnistorStoreBdbc::cacheWriteBegin,
        UnistorStoreBdbc::cacheWrite,
        UnistorStoreBdbc::cacheWriteEnd,
        this,
        UnistorBdbcKey::m_storeKeyEqualFn,
        UnistorBdbcKey::m_storeKeyLessFn,
        UnistorBdbcKey::m_storeKeyHashFn,
        1.2,
        m_szErrMsg))
    {
        CWX_ERROR(("Failure to start cache, err=%s", m_szErrMsg));
        return -1;
    }

    m_strEngine = m_config->getCommon().m_strStoreType;
    m_uiUncommitBinlogNum = 0;
    m_uiLastCommitSecond = 0;
    m_ullStoreSid = 0;

    ///检查环境，若失败则退出
    if (0 != _checkSysInfo()) return -1;

    char szDataName[256];
    m_bValid = true; ///_loadInfo()需要此为true
    do{        
        //打开环境
        if ((ret = ::db_env_create(&m_bdbEnv, 0)) != 0){
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to create bdb env, err-code=%d, err-msg=%s", ret, db_strerror(ret));
            ret = -1; break;
        }
        if ((ret = m_bdbEnv->set_cachesize(m_bdbEnv, m_bdbcConf.m_uiCacheMByte/1024,
            (m_bdbcConf.m_uiCacheMByte%1024)*1024*1024, 0)) != 0)
        {
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to set bdb env cache, err-code=%d, err-msg=%s", ret, db_strerror(ret));
            ret = -1; break;
        }
        m_bdbEnv->set_data_dir(m_bdbEnv, m_bdbcConf.m_strDbPath.c_str());
        m_bdbEnv->set_flags(m_bdbEnv, DB_TXN_NOSYNC, 1);
        m_bdbEnv->set_flags(m_bdbEnv, DB_TXN_WRITE_NOSYNC, 1);

        if ((ret = m_bdbEnv->open(m_bdbEnv, m_bdbcConf.m_strEnvPath.c_str(),
            DB_RECOVER|DB_CREATE |DB_READ_UNCOMMITTED|DB_INIT_LOCK |DB_PRIVATE|DB_INIT_MPOOL | DB_INIT_TXN | DB_THREAD, 0644)) != 0)
        {
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to create bdb env, home:%s, err-code=%d, err-msg=%s",
                m_bdbcConf.m_strEnvPath.c_str(),
                ret,
                db_strerror(ret));
            ret = -1; break;
        }
        ///初始化系统数据库
        if ((ret = db_create(&m_sysDb, m_bdbEnv, 0)) != 0){
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to create sys db, err-code=%d, err-msg=%s",
                ret,
                db_strerror(ret));
            ret = -1; break;
        }
        if (m_bdbcConf.m_uiPageKSize)   m_sysDb->set_pagesize(m_sysDb, m_bdbcConf.m_uiPageKSize * 1024);
        ///初始化数据数据库
        m_bdbs = new DB*[m_uiGroupNum];
        memset(m_bdbs, 0x00, sizeof(DB*) * m_uiGroupNum);
        for (i=0; i<m_uiGroupNum; i++){
            ///初始化数据的datase
            if ((ret = db_create(&m_bdbs[i], m_bdbEnv, 0)) != 0){
                CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to create bdb db, err-code=%d, err-msg=%s",
                    ret,
                    db_strerror(ret));
                ret = -1; break;
            }

            if (m_bdbcConf.m_uiPageKSize) m_bdbs[i]->set_pagesize(m_bdbs[i], m_bdbcConf.m_uiPageKSize * 1024);
            if (m_bdbcConf.m_bZip)  m_bdbs[i]->set_bt_compress(m_bdbs[i], NULL, NULL);
            if (0 != (ret = m_bdbs[i]->set_bt_compare(m_bdbs[i], UnistorBdbcKey::m_bdbKeyLessFn))){
                CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to set bdb's key compare function, err-code:%d, err-msg=%s",
                    ret,
                    db_strerror(ret));
                ret = -1; break;
            }
            ret = 0;
        }
        if (-1 == ret) break;

        //open write thread transaction
        if ((ret = m_bdbEnv->txn_begin(m_bdbEnv, NULL, &m_bdbTxn, DB_READ_UNCOMMITTED)) != 0){
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to begin transaction, err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            ret = -1; break;
        }

        ///打开系统数据库
        if ((ret = m_sysDb->open(m_sysDb, m_bdbTxn, BDBC_SYS_FILE_NAME, NULL,
            DB_BTREE, DB_CREATE|DB_READ_UNCOMMITTED|DB_THREAD, 0644)) != 0)
        {
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to open bdb db[%s%s], err-code=%d, err-msg=%s",
                m_bdbcConf.m_strDbPath.c_str(),
                BDBC_SYS_FILE_NAME,
                ret,
                db_strerror(ret));
            ret = -1; break;
        }

        ///打开数据数据库
        for (i=0; i<m_uiGroupNum; i++){
            CwxCommon::snprintf(szDataName, 255, BDBC_DATA_FILE_FORMAT, m_bdbcConf.m_ucGroupStartBit,  m_uiGroupNum, i);
            /* Open a database with DB_BTREE access method. */
            if ((ret = m_bdbs[i]->open(m_bdbs[i], m_bdbTxn, szDataName, NULL,
                DB_BTREE, DB_CREATE|DB_READ_UNCOMMITTED|DB_THREAD, 0644)) != 0)
            {
                CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to open bdb db[%s%s], err-code=%d, err-msg=%s",
                    m_bdbcConf.m_strDbPath.c_str(),
                    szDataName,
                    ret,
                    db_strerror(ret));
                ret = -1; break;
            }
            ret = 0;
        }
        if (-1 == ret) break;
        //commit write thread transaction
        if ((ret = m_bdbTxn->commit(m_bdbTxn, 0)) != 0){
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to commit transaction, err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            ret = -1; break;
        }
        m_bdbTxn = NULL;
        //open write thread transaction
        if ((ret = m_bdbEnv->txn_begin(m_bdbEnv, NULL, &m_bdbTxn, DB_READ_UNCOMMITTED)) != 0){
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to begin transaction, err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            ret = -1; break;
        }
        if (0 != _loadSysInfo(m_bdbTxn, NULL)){
            ret = -1; break;
        }
        //commit write thread transaction
        if ((ret = m_bdbTxn->commit(m_bdbTxn, 0)) != 0){
            CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to commit transaction, err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            ret = -1; break;
        }
        m_bdbTxn = NULL;
        ///形成空数据
        m_uiEmptyDataLen = 1024;
        if (-1 == m_emptyData.encode((unsigned char*)m_szEmptyData, m_uiEmptyDataLen, m_szErrMsg)){
            ret = -1; break;
        }
        //恢复binlog
        if (0 != restore(m_ullStoreSid)) ret = -1; break;
        ret = 0;
    }while(0);

    if (-1 == ret){
        m_bValid = false;
        CWX_ERROR((m_szErrMsg));
        if (m_bdbTxn){
            m_bdbTxn->abort(m_bdbTxn);
            m_bdbTxn = NULL;
        }
        return -1;
    }

    m_szErrMsg[0] = 0x00;
    return 0;
}

///开始写的函数，返回值：0，成功；-1：失败
int UnistorStoreBdbc::cacheWriteBegin(void* context, char* szErr2K){
    UnistorStoreBdbc* pBdbc = (UnistorStoreBdbc*)context;
    if (!pBdbc->m_bValid){
        if (szErr2K) strcpy(szErr2K, pBdbc->m_szErrMsg);
        return -1;
    }
    CWX_ASSERT(!pBdbc->m_bdbTxn);
    CWX_INFO(("Begin commit....................."));
    //flush binlog first
    if (0 != pBdbc->flushBinlog(pBdbc->m_szErrMsg)){
        if (szErr2K) strcpy(szErr2K, pBdbc->m_szErrMsg);
        return -1;
    }
    int ret = 0;
    //open write thread transaction
    if ((ret = pBdbc->m_bdbEnv->txn_begin(pBdbc->m_bdbEnv,
        NULL,
        &pBdbc->m_bdbTxn,
        DB_READ_UNCOMMITTED)) != 0)
    {
        pBdbc->m_bValid = false;
        CwxCommon::snprintf(pBdbc->m_szErrMsg, 2047, "Failure to begin transaction, err-code:%d, err-msg=%s",
            ret,
            db_strerror(ret));
        CWX_ERROR((pBdbc->m_szErrMsg));
        if (szErr2K) strcpy(szErr2K, pBdbc->m_szErrMsg);
        pBdbc->m_bdbTxn = NULL;
        return -1;
    }
    return 0;
}
///写数据，返回值：0，成功；-1：失败
int UnistorStoreBdbc::cacheWrite(void* context,
                                   char const* szKey,
                                   CWX_UINT16 unKeyLen,
                                   char const* szData,
                                   CWX_UINT32 uiDataLen,
                                   bool bDel,
                                   CWX_UINT32 ,
                                   char* szStoreKeyBuf,
                                   CWX_UINT16 unKeyBufLen,
                                   char* szErr2K)
{
    UnistorStoreBdbc* pBdbc = (UnistorStoreBdbc*)context;
    if (!pBdbc->m_bValid){
        if (szErr2K) strcpy(szErr2K, pBdbc->m_szErrMsg);
        return -1;
    }
    CWX_ASSERT(pBdbc->m_bdbTxn);
    UnistorBdbcKey key(szKey, unKeyLen);
    CWX_UINT32 uiIndex =pBdbc->_getBdbIndex(key);
    memcpy(szStoreKeyBuf, szKey, unKeyLen);
    if (bDel){
        if (0 != pBdbc->_delBdbKey(pBdbc->m_bdbs[uiIndex],
            pBdbc->m_bdbTxn,
            szStoreKeyBuf,
            unKeyLen,
            unKeyBufLen,
            0,
            szErr2K))
        {
            return -1;
        }
    }else{
        if (0 != pBdbc->_setBdbKey(pBdbc->m_bdbs[uiIndex],
            pBdbc->m_bdbTxn,
            szStoreKeyBuf,
            unKeyLen,
            unKeyBufLen,
            szData,
            uiDataLen,
            0,
            szErr2K))
        {
            return -1;
        }
    }
    return 0;
}

///提交数据，返回值：0，成功；-1：失败
int UnistorStoreBdbc::cacheWriteEnd(void* context, CWX_UINT64 ullSid, void* , char* szErr2K){
    int ret = 0;
    UnistorStoreBdbc* pBdbc = (UnistorStoreBdbc*)context;
    if (!pBdbc->m_bValid){
        if (szErr2K) strcpy(szErr2K, pBdbc->m_szErrMsg);
        return -1;
    }
    CWX_ASSERT(pBdbc->m_bdbTxn);
    //commit the bdb
    if (0 != pBdbc->_updateSysInfo(pBdbc->m_bdbTxn, ullSid, szErr2K)) return -1;

    if ((ret = pBdbc->m_bdbTxn->commit(pBdbc->m_bdbTxn, 0)) != 0){
        pBdbc->m_bValid = false;
        CwxCommon::snprintf(pBdbc->m_szErrMsg, 2047, "Failure to commit bdb, err-code:%d, err-msg=%s",
            ret,
            db_strerror(ret));
        CWX_ERROR((pBdbc->m_szErrMsg));
        if (szErr2K) strcpy(szErr2K, pBdbc->m_szErrMsg);
        pBdbc->m_bdbTxn = NULL;
        return -1;
    }
    pBdbc->m_bdbTxn = NULL;
    CWX_INFO(("End commit....................."));
    return 0;
}


///检测是否存在key；1：存在；0：不存在；-1：失败
int UnistorStoreBdbc::isExist(UnistorTss* tss,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* field,
                                CwxKeyValueItemEx const* ,
                                CWX_UINT32& uiVersion,
                                CWX_UINT32& uiFieldNum,
                                bool& bReadCached)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }
    ///获取key的data
    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        true,
        false,
        tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    ///若为空，则表示不存在
    if (pCounterMap->m_bdbcOldData.isEmpty()) return 0;
    uiFieldNum = pCounterMap->m_bdbcOldData.m_ucCounterNum;
    uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion;
    if (field){
        CWX_UINT8 ucId = 0;
        if (!pCounterMap->getId(field->m_szData, ucId)) return 0;
        return pCounterMap->m_bdbcOldData.isExist(ucId)>0;
    }
    return 1;
}


///解析couter=value;couter=value的data。返回值：true，成功；false，失败
bool UnistorStoreBdbc::_parseDataCounter(UnistorBdbcData& bdbcData,///<返回的计数机信息
                                           char const* szData, ///<计数的字符串
                                           CWX_UINT32 uiDataLen, ///<数据长度
                                           UnistorStoreBdbcCounterMap* pCounterMap, ///<线程的counter map对象
                                           bool bSync, ///<是否是同步的数据
                                           char* szErr2K
                                           )
{
    string strValue(szData, uiDataLen);
    list<pair<string, string> > items;
    CwxCommon::split(strValue, items, BDBC_KEY_COUNTER_SPLIT_CHR);
    list<pair<string, string> >::iterator iter=items.begin();
    CWX_UINT8 ucId = 0;
    CWX_UINT32 uiValue=0;
    bdbcData.reset();
    if (bSync){
        while(iter != items.end()){
            ucId = strtoul(iter->first.c_str(), NULL, 0);
            if (!ucId){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "counter id can't be zero.");
                return false;
            }
            if (bdbcData.m_counterMap[ucId]){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "counter[%s] is duplicate.", iter->first.c_str());
                return false;
            }
            uiValue = strtoul(iter->second.c_str(), NULL, 0);
            if (uiValue){
                bdbcData.m_ucCounterNum++;
                bdbcData.m_counter[bdbcData.m_ucCounterNum].m_uiValue = uiValue;
                bdbcData.m_counter[bdbcData.m_ucCounterNum].m_ucId = ucId;
                bdbcData.m_counterMap[ucId] = bdbcData.m_ucCounterNum;
            }
            iter++;
        }
        
    }else{
        while(iter != items.end()){
            if (!pCounterMap->getId(iter->first.c_str(), ucId)){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "counter[%s] doesn't exist.", iter->first.length()?iter->first.c_str():"");
                return false;
            }
            if (bdbcData.m_counterMap[ucId]){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "counter[%s] is duplicate.", iter->first.c_str());
                return false;
            }
            uiValue = strtoul(iter->second.c_str(), NULL, 0);
            if (uiValue){
                bdbcData.m_ucCounterNum++;
                bdbcData.m_counter[bdbcData.m_ucCounterNum].m_uiValue = uiValue;
                bdbcData.m_counter[bdbcData.m_ucCounterNum].m_ucId = ucId;
                bdbcData.m_counterMap[ucId] = bdbcData.m_ucCounterNum;
            }
            iter++;
        }
    }
    return true;
}

///形成 couter_id=value;couter_id=value的data。返回值：true，成功；false，失败
bool UnistorStoreBdbc::_outputSyncCounter(UnistorBdbcData const& bdbcData,///<返回的计数机信息
                        char* szData, ///<计数的字符串
                        CWX_UINT32& uiDataLen, ///<数据长度
                        char* szErr2K ///<若出错，则返回错误原因
                        )
{
    CWX_UINT32 pos = 0;
    CWX_UINT32 uiLen = 0;
    for (CWX_UINT8 i=1; i<=bdbcData.m_ucCounterNum; i++){
        if (uiDataLen - pos <15){
            if (szErr2K) strcpy(szErr2K, "No space to output the counter.");
            return false;
        }
        if (pos){
            uiLen = sprintf(szData + pos, ";%u=%u", bdbcData.m_counter[i].m_ucId, bdbcData.m_counter[i].m_uiValue);
        }else{
            uiLen = sprintf(szData + pos, "%u=%u", bdbcData.m_counter[i].m_ucId, bdbcData.m_counter[i].m_uiValue);
        }
        pos += uiLen;
    }
    uiDataLen = pos;
    return true;
}

///merge计数器，true：成功；false：失败
bool UnistorStoreBdbc::_mergeAddCouter(UnistorBdbcData& oldBdbcData, ///<当前的值
                     UnistorBdbcData const& newBdbcData, ///<新值
                     UnistorStoreBdbcCounterMap* pCounterMap, ///<线程的counter map对象
                     char* szErr2K ///<失败时的错误消息
                     )
{
    for (CWX_UINT8 i=1; i<=newBdbcData.m_ucCounterNum; i++){
        if (oldBdbcData.m_counterMap[newBdbcData.m_counter[i].m_ucId]){
            if (szErr2K) {
                if (pCounterMap->getName(newBdbcData.m_counter[i].m_ucId)){
                    CwxCommon::snprintf(szErr2K, 2047, "Counter[%s] is exist.", pCounterMap->getName(newBdbcData.m_counter[i].m_ucId));
                }else{
                    CwxCommon::snprintf(szErr2K, 2047, "Counter id[%u] is exist.", newBdbcData.m_counter[i].m_ucId);
                }
            }
            return false;
        }
        if (255 == oldBdbcData.m_ucCounterNum){
            if (szErr2K) strcpy(szErr2K, "Counter number is more than the max:255");
            return false;
        }
        oldBdbcData.m_ucCounterNum++;
        oldBdbcData.m_counter[oldBdbcData.m_ucCounterNum] = newBdbcData.m_counter[i];
        oldBdbcData.m_counterMap[oldBdbcData.m_counter[i].m_ucId] = oldBdbcData.m_ucCounterNum;
    }
    return true;
}

///添加key，1：成功；0：存在；-1：失败；
int UnistorStoreBdbc::addKey(UnistorTss* tss,
                               CwxKeyValueItemEx const& key,
                               CwxKeyValueItemEx const* field,
                               CwxKeyValueItemEx const* extra,
                               CwxKeyValueItemEx const& data,
                               CWX_UINT32 uiSign,
                               CWX_UINT32& uiVersion,
                               CWX_UINT32& uiFieldNum,
                               bool& bReadCached,
                               bool& bWriteCached,
                               bool bCache,
                               CWX_UINT32 uiExpire)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    UnistorBdbcData* pBdbcData = NULL;
    bool bExistKey = false;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        strcpy(tss->m_szBuf2K, "Data must be in [[counter:num];[counter:num]...].");
        return -1;
    }
    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }
    if (!_parseDataCounter(pCounterMap->m_bdbcInputData, data.m_szData, data.m_uiDataLen, pCounterMap, false, tss->m_szBuf2K))
        return -1;

    if (pCounterMap->m_bdbcInputData.isEmpty()){
        uiVersion = 0;
        uiFieldNum = 0;
        bReadCached = true;
        bWriteCached = true;
        return 1;
    }


    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;

    if (uiSign > 2) uiSign = 0;
    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);
    if (-1 == ret) return -1;

    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;
    
    if (0 == ret){//not exist
        pBdbcData = &pCounterMap->m_bdbcInputData;
        if (!uiVersion) uiVersion = UNISTOR_KEY_START_VERION; ///起始版本
    }else if (1 == ret){//key存在
        bExistKey = true;
        if (0 == uiSign){//添加key
            strcpy(tss->m_szBuf2K, "Key exists.");
            return 0; //key存在。
        }
        if (!uiVersion){
            uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;
        }
        if (!_mergeAddCouter(pCounterMap->m_bdbcOldData, pCounterMap->m_bdbcInputData, pCounterMap, tss->m_szBuf2K)) return -1;
        pBdbcData = &pCounterMap->m_bdbcOldData;
    }

    ///输出日志
    CwxKeyValueItemEx logData;
    memcpy(&logData, &data, sizeof(data));
    logData.m_uiDataLen = UNISTOR_MAX_KV_SIZE;
    if (!_outputSyncCounter(pCounterMap->m_bdbcInputData, szBuf, logData.m_uiDataLen, tss->m_szBuf2K)){
        return -1;
    }
    logData.m_szData = szBuf;
    if (0 != appendAddBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyStoreGroup((char*)&pCounterMap->m_bdbcKey, sizeof(pCounterMap->m_bdbcKey)),
        key,
        field,
        extra,
        logData,
        uiExpire,
        uiSign,
        uiVersion,
        bCache,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    m_ullStoreSid = getCurSid();
    ///保持数据
    pBdbcData->m_uiVersion = uiVersion;
    uiBufLen = UNISTOR_MAX_KV_SIZE;
    ret = pBdbcData->encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    uiFieldNum = ret;
    if (0 == ret){
        if (bExistKey && (0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K))) return -1;
        return 1;
    }
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, bCache, tss->m_szBuf2K))?-1:1;
}

///同步 add key，1：成功；0：存在；-1：失败；
int UnistorStoreBdbc::syncAddKey(UnistorTss* tss,
                                   CwxKeyValueItemEx const& key,
                                   CwxKeyValueItemEx const* field,
                                   CwxKeyValueItemEx const* extra,
                                   CwxKeyValueItemEx const& data,
                                   CWX_UINT32 uiSign,///<not use
                                   CWX_UINT32 uiVersion,
                                   bool bCache,
                                   CWX_UINT32 uiExpire,
                                   CWX_UINT64 ullSid,
                                   bool& bReadCached, ///<数据是否在read cache中
                                   bool& bWriteCached, ///<数据是否在write cache中
                                   bool  bRestore)
{
    return syncSetKey(tss,
        key,
        field,
        extra,
        data,
        uiSign,
        uiVersion,
        bCache,
        uiExpire,
        ullSid,
        bReadCached,
        bWriteCached,
        bRestore);
}

///merge计数器，true：成功；false：失败
bool UnistorStoreBdbc::_mergeSetCouter(UnistorBdbcData& oldBdbcData, ///<当前的值
                                         UnistorBdbcData const& newBdbcData, ///<新值
                                         char* szErr2K ///<失败时的错误消息
                                         )
{
    for (CWX_UINT8 i=1; i<=newBdbcData.m_ucCounterNum; i++){
        if (oldBdbcData.m_counterMap[i]){///存在
            oldBdbcData.m_counter[oldBdbcData.m_counterMap[i]] = newBdbcData.m_counter[i];
        }else{///不存在
            if (255 == oldBdbcData.m_ucCounterNum){
                if (szErr2K) strcpy(szErr2K, "Counter number is more than the max:255");
                return false;
            }
            oldBdbcData.m_ucCounterNum++;
            oldBdbcData.m_counter[oldBdbcData.m_ucCounterNum] = newBdbcData.m_counter[i];
            oldBdbcData.m_counterMap[oldBdbcData.m_counter[i].m_ucId] = oldBdbcData.m_ucCounterNum;
        }
    }
    return true;
}

///set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
int UnistorStoreBdbc::setKey(UnistorTss* tss,
                               CwxKeyValueItemEx const& key,
                               CwxKeyValueItemEx const* field,
                               CwxKeyValueItemEx const* extra,
                               CwxKeyValueItemEx const& data,
                               CWX_UINT32 uiSign,
                               CWX_UINT32& uiVersion,
                               CWX_UINT32& uiFieldNum,
                               bool& bReadCached,
                               bool& bWriteCached,
                               bool bCache,
                               CWX_UINT32 uiExpire)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    UnistorBdbcData* pBdbcData = NULL;
    bool bExistKey = false;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        strcpy(tss->m_szBuf2K, "Data must be in [[counter:num];[counter:num]...].");
        return -1;
    }
    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    if (!_parseDataCounter(pCounterMap->m_bdbcInputData, data.m_szData, data.m_uiDataLen, pCounterMap, false, tss->m_szBuf2K)) return -1;
    if (uiSign > 1) uiSign = 0;
    if (pCounterMap->m_bdbcInputData.isEmpty() && uiSign){
        uiVersion = 0;
        uiFieldNum = 0;
        bReadCached = true;
        bWriteCached = true;
        return 1;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;

    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);

    if (-1 == ret) return -1;

    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;


    bExistKey = ret?true:false;
    if ((0 == ret/*旧值不存在*/) || (0 == uiSign)/*替换整个key*/){
        if (!uiVersion){
            if (0 == ret){
                uiVersion = UNISTOR_KEY_START_VERION; ///起始版本
            }else{
                uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;
            }
        }
        pBdbcData = &pCounterMap->m_bdbcInputData;
    }else{//key存在而且不是全部替换
        if (!uiVersion) uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;
        if (!_mergeSetCouter(pCounterMap->m_bdbcOldData, pCounterMap->m_bdbcInputData, tss->m_szBuf2K)) return -1;
        pBdbcData = &pCounterMap->m_bdbcOldData;
    }

    ///输出日志
    CwxKeyValueItemEx logData;
    memcpy(&logData, &data, sizeof(data));
    logData.m_uiDataLen = UNISTOR_MAX_KV_SIZE;
    if (!_outputSyncCounter(pCounterMap->m_bdbcInputData, szBuf, logData.m_uiDataLen, tss->m_szBuf2K)){
        return -1;
    }
    logData.m_szData = szBuf;
    if (0 != appendSetBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        logData,
        uiExpire,
        uiSign,
        uiVersion,
        bCache,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }

    m_ullStoreSid = getCurSid();
    pBdbcData->m_uiVersion = uiVersion;
    uiBufLen = UNISTOR_MAX_KV_SIZE;
    ret = pBdbcData->encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    uiFieldNum = ret;
    if (0 == ret){
        if (bExistKey && (0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K))) return -1;
        return 1;
    }
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, bCache, tss->m_szBuf2K))?-1:1;
}

///import key，1：成功；-1：失败；
int UnistorStoreBdbc::importKey(UnistorTss* tss, ///<tss对象
                      CwxKeyValueItemEx const& key, ///<添加的key
                      CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
                      CwxKeyValueItemEx const& data, ///<添加key或field的数据
                      CWX_UINT32& uiVersion, ///<若大于0，则设置修改后的key为此版本
                      bool& bReadCached, ///<数据是否在read cache中
                      bool& bWriteCached, ///<数据是否在write cache中
                      bool bCache, ///<是否将key放到读cache
                      CWX_UINT32 uiExpire ///<若创建key，而且指定了uiExpire则设置key的超时时间
                      )
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }

    bReadCached = false;
    bWriteCached = false;

    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        strcpy(tss->m_szBuf2K, "Data must be in [[counter:num];[counter:num]...].");
        return -1;
    }

    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    if (!_parseDataCounter(pCounterMap->m_bdbcInputData, data.m_szData, data.m_uiDataLen, pCounterMap, false, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcInputData.isEmpty()){
        uiVersion = 0;
        bReadCached = true;
        bWriteCached = true;
        return 1;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    
    ///输出日志
    CwxKeyValueItemEx logData;
    memcpy(&logData, &data, sizeof(data));
    logData.m_uiDataLen = UNISTOR_MAX_KV_SIZE;
    if (!_outputSyncCounter(pCounterMap->m_bdbcInputData, szBuf, logData.m_uiDataLen, tss->m_szBuf2K)){
        return -1;
    }
    logData.m_szData = szBuf;

    if (0 != appendImportBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        extra,
        logData,
        uiExpire,
        uiVersion,
        bCache,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }

    if (!uiVersion) uiVersion = UNISTOR_KEY_START_VERION;
    uiBufLen = UNISTOR_MAX_KV_SIZE;
    pCounterMap->m_bdbcInputData.m_uiVersion = uiVersion;
    if (-1 == pCounterMap->m_bdbcInputData.encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;

    m_ullStoreSid = getCurSid();
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, bCache, tss->m_szBuf2K))?-1:1;

}

///sync import key，1：成功；-1：错误。
int UnistorStoreBdbc::syncImportKey(UnistorTss* tss, ///<线程的tss数据
                                      CwxKeyValueItemEx const& key, ///<set的key
                                      CwxKeyValueItemEx const* , ///<存储引擎的extra数据
                                      CwxKeyValueItemEx const& data, ///<set的数据
                                      CWX_UINT32 uiVersion, ///<set的key 版本号
                                      bool bCache,    ///<是否将key放到读cache
                                      CWX_UINT32 , ///<若创建key，而且指定了uiExpire则设置key的超时时间
                                      CWX_UINT64 ullSid, ///<操作对应的binlog的sid
                                      bool& bReadCached, ///<数据是否在read cache中
                                      bool& bWriteCached, ///<数据是否在write cache中
                                      bool   ///<是否从binlog恢复的数据
                                      )
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    bReadCached = false;
    bWriteCached = false;

    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        strcpy(tss->m_szBuf2K, "Data must be in [[counter:num];[counter:num]...].");
        return -1;
    }

    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    if (!_parseDataCounter(pCounterMap->m_bdbcInputData, data.m_szData, data.m_uiDataLen, pCounterMap, true, tss->m_szBuf2K)) return -1;

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);

    pCounterMap->m_bdbcInputData.m_uiVersion = uiVersion;
    if (-1 == pCounterMap->m_bdbcInputData.encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, bCache, tss->m_szBuf2K))?-1:1;
}

///set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
int UnistorStoreBdbc::syncSetKey(UnistorTss* tss,
                                   CwxKeyValueItemEx const& key,
                                   CwxKeyValueItemEx const* ,
                                   CwxKeyValueItemEx const* ,
                                   CwxKeyValueItemEx const& data,
                                   CWX_UINT32 uiSign,
                                   CWX_UINT32 uiVersion,
                                   bool bCache,
                                   CWX_UINT32 ,
                                   CWX_UINT64 ullSid,
                                   bool& bReadCached, 
                                   bool& bWriteCached,
                                   bool  bRestore)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    UnistorBdbcData* pBdbcData = NULL;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (data.m_bKeyValue){
        strcpy(tss->m_szBuf2K, "Data must be in [[counter:num];[counter:num]...].");
        return -1;
    }
    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    if (!_parseDataCounter(pCounterMap->m_bdbcInputData, data.m_szData, data.m_uiDataLen, pCounterMap, true, tss->m_szBuf2K)) return -1;


    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = 0;
    bReadCached = false;
    bWriteCached = false;

    if (0 != uiSign){
        ret = _getKey(&pCounterMap->m_bdbcKey,
            szBuf,
            uiBufLen,
            tss->m_szStoreKey,
            UNISTOR_MAX_KEY_SIZE,
            bReadCached,
            false,
            false,
            tss->m_szBuf2K);
        if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
        if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;
    }

    if (-1 == ret) return -1;

    if (1 == ret){
        if (bRestore && (pCounterMap->m_bdbcOldData.m_uiVersion >= uiVersion)){
            m_ullStoreSid = ullSid;
            return 1; ///已经添加
        }
        if (!_mergeSetCouter(pCounterMap->m_bdbcOldData, pCounterMap->m_bdbcInputData, tss->m_szBuf2K)) return -1;
        pBdbcData = &pCounterMap->m_bdbcOldData;
    }else{
        pBdbcData = &pCounterMap->m_bdbcInputData;
    }
    if (-1 == ret) return -1;
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    pBdbcData->m_uiVersion = uiVersion;
    uiBufLen = UNISTOR_MAX_KV_SIZE;
    ret = pBdbcData->encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);

    if (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bCache, bWriteCached, tss->m_szBuf2K)) return -1;
    return 1;
}


///update key，1：成功；0：不存在；-1：失败；-2：版本错误
int UnistorStoreBdbc::updateKey(UnistorTss* tss,
                                  CwxKeyValueItemEx const& key,
                                  CwxKeyValueItemEx const* field,
                                  CwxKeyValueItemEx const* extra,
                                  CwxKeyValueItemEx const& data,
                                  CWX_UINT32 uiSign,
                                  CWX_UINT32& uiVersion,
                                  CWX_UINT32& uiFieldNum,
                                  bool& bReadCached,
                                  bool& bWriteCached,
                                  CWX_UINT32 uiExpire)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    UnistorBdbcData* pBdbcData = NULL;
    bool bExistKey = false;    
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    
    if (data.m_bKeyValue){
        strcpy(tss->m_szBuf2K, "Data must be in [[counter:num];[counter:num]...].");
        return -1;
    }

    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    if (!_parseDataCounter(pCounterMap->m_bdbcInputData, data.m_szData, data.m_uiDataLen, pCounterMap, false, tss->m_szBuf2K)){
        return -1;
    }
    if (uiSign > 2) uiSign = 0;
    if (pCounterMap->m_bdbcInputData.isEmpty() && uiSign){
        uiVersion = 0;
        bReadCached = true;
        bWriteCached = true;
        return 1;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;

    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;
    
    bExistKey = ret?true:false;
    if (1 == ret){
        //key存在
        if (uiVersion){
            if (pCounterMap->m_bdbcOldData.m_uiVersion != uiVersion){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%u] is not same with input version[%u].",
                    pCounterMap->m_bdbcOldData.m_uiVersion, uiVersion);
                return -2;
            }
        }
    }

    if ((0 == ret/*旧值不存在*/) ||(0 == uiSign)/*替换整个key*/){
        if (0 == ret){
            uiVersion = UNISTOR_KEY_START_VERION; ///起始版本
        }else{
            uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;
        }
        pBdbcData = &pCounterMap->m_bdbcInputData;
    }else{//key存在而且不是全部替换
        uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;
        if (!_mergeSetCouter(pCounterMap->m_bdbcOldData, pCounterMap->m_bdbcInputData, tss->m_szBuf2K)) return -1;
        pBdbcData = &pCounterMap->m_bdbcOldData;
    }
    ///输出日志
    CwxKeyValueItemEx logData;
    memcpy(&logData, &data, sizeof(data));
    logData.m_uiDataLen = UNISTOR_MAX_KV_SIZE;
    if (!_outputSyncCounter(pCounterMap->m_bdbcInputData, szBuf, logData.m_uiDataLen, tss->m_szBuf2K)){
        return -1;
    }
    logData.m_szData = szBuf;

    if (0 != appendUpdateBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        logData,
        uiExpire,
        uiSign,
        uiVersion,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }

    m_ullStoreSid = getCurSid();
    pBdbcData->m_uiVersion = uiVersion;
    uiBufLen = UNISTOR_MAX_KV_SIZE;
    ret = pBdbcData->encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    uiFieldNum = ret;
    if (0 == ret){
        if (bExistKey && (0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K))) return -1;
        return 1;
    }
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, true, tss->m_szBuf2K))?-1:1;
}

///update key，1：成功；0：不存在；-1：失败
int UnistorStoreBdbc::syncUpdateKey(UnistorTss* tss,
                                      CwxKeyValueItemEx const& key,
                                      CwxKeyValueItemEx const* field,
                                      CwxKeyValueItemEx const* extra,
                                      CwxKeyValueItemEx const& data,
                                      CWX_UINT32 uiSign,
                                      CWX_UINT32 uiVersion,
                                      CWX_UINT32 uiExpire,
                                      CWX_UINT64 ullSid,
                                      bool& bReadCached,
                                      bool& bWriteCached,
                                      bool  bRestore)
{
    return syncSetKey(tss,
        key,
        field,
        extra,
        data,
        uiSign,
        uiVersion,
        true,
        uiExpire,
        ullSid,
        bReadCached,
        bWriteCached,
        bRestore);
}

///形成inc的计数器，1：成功；-1：失败；0：超过范围。
int UnistorStoreBdbc::_mergeIncCouter(UnistorBdbcData& bdbcData, ///<当前的值
                     CWX_INT32 iValue,///<inc的值，可以为负数
                     CWX_INT64 iMax, ///<最大值
                     CWX_INT64 iMin, ///<最小值
                     CWX_UINT8 ucId, ///<计数器的id
                     CWX_INT64& llValue,
                     char* szErr2K ///<失败时的错误消息
                     )
{
    if (!ucId){
        if (szErr2K) strcpy(szErr2K, "Counter id is zero, it must be more than zero.");
        return -1;
    }
    if (iMin<0) iMin=0;
    if (iMax<0) iMax=0;
    if (((CWX_UINT32)iMin) > BDBC_MAX_COUNTER_VALUE) iMin = BDBC_MAX_COUNTER_VALUE;
    if (((CWX_UINT32)iMax) > BDBC_MAX_COUNTER_VALUE) iMax = BDBC_MAX_COUNTER_VALUE;
    if (bdbcData.m_counterMap[ucId]){///存在
        if (iValue > 0){
            if (iMax ){///判断边界
                if ((iValue > iMax) || ((CWX_UINT32)(iMax-iValue) < bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue)){
                    if (szErr2K) strcpy(szErr2K, "Exceed the max value.");
                    return 0;
                }
            }else{
                iMax = BDBC_MAX_COUNTER_VALUE;
            }
            if (bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue > iMax - iValue){
                bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue = iMax;
            }else{
                bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue = bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue + iValue;
            }
        }else{
            iValue = -iValue; ///转变为正值
            if (iMin){///判断边界
                if (((CWX_UINT32)(iMin + iValue) > bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue)){
                    if (szErr2K) strcpy(szErr2K, "Exceed the min value.");
                    return 0;
                }
            }
            if (bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue < (CWX_UINT32)iValue + iMin){
                bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue = iMin;
            }else{
                bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue -= iValue;
            }
        }
    }else{
        if (255 == bdbcData.m_ucCounterNum){
            if (szErr2K) strcpy(szErr2K, "Counter number is more than the max:255");
            return -1;
        }
        bdbcData.m_ucCounterNum++;
        bdbcData.m_counter[bdbcData.m_ucCounterNum].m_ucId = ucId;
        bdbcData.m_counter[bdbcData.m_ucCounterNum].m_uiValue = iValue>0?iValue:0;
        bdbcData.m_counterMap[ucId] = bdbcData.m_ucCounterNum;
    }
    llValue = bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue;
    return 1;
}

///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；-3：超出边界
int UnistorStoreBdbc::incKey(UnistorTss* tss,
                               CwxKeyValueItemEx const& key,
                               CwxKeyValueItemEx const* field,
                               CwxKeyValueItemEx const* extra,
                               CWX_INT64 num,
                               CWX_INT64  llMax,
                               CWX_INT64  llMin,
                               CWX_UINT32  uiSign,
                               CWX_INT64& llValue,
                               CWX_UINT32& uiVersion,
                               bool& bReadCached,
                               bool& bWriteCached,
                               CWX_UINT32 uiExpire)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }

    CWX_UINT8 ucId = 0;

    bool bExistKey = false;
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (!field){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "You must specify the counter to inc");
        return -1;
    }
    if (!pCounterMap->getId(field->m_szData, ucId)){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Counter name[%s] doesn't exist.", field->m_szData);
        return -1;
    }
    llValue = 0;
    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    bReadCached = false;
    bWriteCached = false;
    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);

    if (-1 == ret) return -1;
    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;

    if (uiSign > 2) uiSign = 0;
    bExistKey = ret?true:false;
    if (1 == ret){
        //检查版本
        if (uiVersion){
            if (pCounterMap->m_bdbcOldData.m_uiVersion != uiVersion){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%u] is not same with input version[%u].",
                    pCounterMap->m_bdbcOldData.m_uiVersion, uiVersion);
                return -2;
            }
        }
    }
    if (0 == ret)
        uiVersion = UNISTOR_KEY_START_VERION;
    else
        uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;
    //merge计数器
    ret = _mergeIncCouter(pCounterMap->m_bdbcOldData, num, llMax, llMin,  ucId, llValue, tss->m_szBuf2K);
    if (1 != ret){
        if (-1 == ret) return -1;
        if (0 == ret) return -3;
    }

    if (num){
        CwxKeyValueItemEx logData;
        memcpy(&logData, &field, sizeof(field));
        logData.m_uiDataLen = UNISTOR_MAX_KV_SIZE;
        logData.m_uiDataLen = sprintf(szBuf, "%u", ucId);
        logData.m_szData = szBuf;

        if (0 != appendIncBinlog(*tss->m_pEngineWriter,
            *tss->m_pEngineItemWriter,
            getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
            key,
            &logData,
            extra,
            num,
            llValue,
            llMax,
            llMin,
            uiExpire,
            uiSign,
            uiVersion,
            tss->m_szBuf2K))
        {
            m_bValid = false;
            strcpy(m_szErrMsg, tss->m_szBuf2K);
            return -1;
        }
        m_ullStoreSid = getCurSid();
        pCounterMap->m_bdbcOldData.m_uiVersion = uiVersion;
        uiBufLen = UNISTOR_MAX_KV_SIZE;
        ret = pCounterMap->m_bdbcOldData.encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (0 == ret){
            if (bExistKey &&(0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K))) return -1;
            return 1;
        }
        return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, true, tss->m_szBuf2K))?-1:1;
    }
    return 1;
}

///inc key，1：成功；0：不存在；-1：失败；
int UnistorStoreBdbc::syncIncKey(UnistorTss* tss,
                                   CwxKeyValueItemEx const& key,
                                   CwxKeyValueItemEx const* field,
                                   CwxKeyValueItemEx const* ,
                                   CWX_INT64 ,
                                   CWX_INT64 result,
                                   CWX_INT64  ,
                                   CWX_INT64  ,
                                   CWX_UINT32 ,
                                   CWX_INT64& ,
                                   CWX_UINT32 uiVersion,
                                   CWX_UINT32 ,
                                   CWX_UINT64 ullSid,
                                   bool& bReadCached,
                                   bool& bWriteCached,
                                   bool  bRestore)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    CWX_UINT8 ucId = 0;

    bool bExistKey = false;
    bReadCached = false;
    bWriteCached = false;

    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    if (!field){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "You must specify the field to inc");
        return -1;
    }
    ucId = strtoul(field->m_szData, NULL, 10);
    if (!ucId){
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Counter id[%s] is zero", field->m_szData);
        return -1;
    }

    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);
    if (-1 == ret) return -1;

    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;

    bExistKey = ret?true:false;
    if (1 == ret){
        if (bRestore && (pCounterMap->m_bdbcOldData.m_uiVersion >= uiVersion)){
            m_ullStoreSid = ullSid;
            return 1;
        }
    }
    //set 计数器
    if (pCounterMap->m_bdbcOldData.m_counterMap[ucId]){///存在
        pCounterMap->m_bdbcOldData.m_counter[pCounterMap->m_bdbcOldData.m_counterMap[ucId]].m_uiValue = result;
    }else{///不存在
        if (255 == pCounterMap->m_bdbcOldData.m_ucCounterNum){
            strcpy(tss->m_szBuf2K, "Counter number is more than the max:255");
            return false;
        }
        pCounterMap->m_bdbcOldData.m_ucCounterNum++;
        pCounterMap->m_bdbcOldData.m_counter[pCounterMap->m_bdbcOldData.m_ucCounterNum].m_ucId = ucId;
        pCounterMap->m_bdbcOldData.m_counter[pCounterMap->m_bdbcOldData.m_ucCounterNum].m_uiValue = result;
        pCounterMap->m_bdbcOldData.m_counterMap[ucId] = pCounterMap->m_bdbcOldData.m_ucCounterNum;
    }

    pCounterMap->m_bdbcOldData.m_uiVersion = uiVersion;
    uiBufLen = UNISTOR_MAX_KV_SIZE;
    ret = pCounterMap->m_bdbcOldData.encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    if (0 == ret){
        if (bExistKey && (0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K))) return -1;
        return 1;
    }
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, true, tss->m_szBuf2K))?-1:1;
}


///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；
int UnistorStoreBdbc::delKey(UnistorTss* tss,
                               CwxKeyValueItemEx const& key,
                               CwxKeyValueItemEx const* field,
                               CwxKeyValueItemEx const* extra,
                               CWX_UINT32& uiVersion,
                               CWX_UINT32& uiFieldNum,
                               bool& bReadCached,
                               bool& bWriteCached)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    CWX_UINT8 ucId = 0;
    uiFieldNum = 0;
    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    bReadCached = false;
    bWriteCached = false;

    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;

    if (0 == ret) return 1;
    //检查版本
    if (uiVersion){
        if (pCounterMap->m_bdbcOldData.m_uiVersion != uiVersion){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Key's version[%u] is not same with input version[%u].",
                pCounterMap->m_bdbcOldData.m_uiVersion, uiVersion);
            return -2;
        }
    }

    uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion + 1;

    CwxKeyValueItemEx logData;
    if (!field){
        pCounterMap->m_bdbcOldData.m_ucCounterNum = 0;
        ret = 0;
    }else{
        UnistorKeyField* pField = NULL;
        UnistorKeyField* pTmp = NULL;
        CWX_UINT32 uiPos = 0;
        CWX_UINT32 uiLen = 0;
        parseMultiField(field->m_szData, pField);
        pTmp = pField;
        while(pTmp){
            if (pCounterMap->getId(pTmp->m_key.c_str(), ucId)){
                if (pCounterMap->m_bdbcOldData.m_counterMap[ucId]){
                    pCounterMap->m_bdbcOldData.m_counter[pCounterMap->m_bdbcOldData.m_counterMap[ucId]].m_uiValue = 0;
                    if (uiPos){
                        uiLen = sprintf(szBuf + uiPos, "\n%u", ucId);
                    }else{
                        uiLen = sprintf(szBuf + uiPos, "%u", ucId);
                    }
                    uiPos += uiLen;
                }
            }
            pTmp = pTmp->m_next;
        }
        freeField(pField);
        memcpy(&logData, field, sizeof(logData));
        logData.m_szData = szBuf;
        logData.m_uiDataLen = uiPos;
        field = &logData;
        ret = 1;
    }

    if (0 != appendDelBinlog(*tss->m_pEngineWriter,
        *tss->m_pEngineItemWriter,
        getKeyAsciiGroup(key.m_szData, key.m_uiDataLen),
        key,
        field,
        extra,
        uiVersion,
        tss->m_szBuf2K))
    {
        m_bValid = false;
        strcpy(m_szErrMsg, tss->m_szBuf2K);
        return -1;
    }
    if (0 != ret){
        uiBufLen = UNISTOR_MAX_KV_SIZE;
        pCounterMap->m_bdbcOldData.m_uiVersion = uiVersion;
        ret = pCounterMap->m_bdbcOldData.encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
        if (-1 == ret) return -1;
    }
    uiFieldNum = ret;
    m_ullStoreSid = getCurSid();
    if (0 == ret){
        if (0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K)) return -1;
        return 1;
    }
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bReadCached, true, tss->m_szBuf2K))?-1:1;
}

///inc key，1：成功；0：不存在；-1：失败；
int UnistorStoreBdbc::syncDelKey(UnistorTss* tss,
                                   CwxKeyValueItemEx const& key,
                                   CwxKeyValueItemEx const* field,
                                   CwxKeyValueItemEx const* ,
                                   CWX_UINT32 uiVersion,
                                   CWX_UINT64 ullSid,
                                   bool& bReadCached, ///<数据是否在read cache中
                                   bool& bWriteCached, ///<数据是否在write cache中
                                   bool  bRestore)
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    CWX_UINT8 ucId = 0;

    bReadCached = false;
    bWriteCached = false;

    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    CWX_UINT32 uiBufLen = UNISTOR_MAX_KV_SIZE;
    char* szBuf = tss->getBuf(uiBufLen);
    int ret = _getKey(&pCounterMap->m_bdbcKey,
        szBuf,
        uiBufLen,
        tss->m_szStoreKey,
        UNISTOR_MAX_KEY_SIZE,
        bReadCached,
        false,
        false,
        tss->m_szBuf2K);
    if (-1 == ret) return -1;

    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K)) return -1;
    if (pCounterMap->m_bdbcOldData.isEmpty()) ret = 0;

    if (0 == ret) return 1;

    //检查版本
    if (bRestore && (pCounterMap->m_bdbcOldData.m_uiVersion >= uiVersion)){
        m_ullStoreSid = ullSid;
        return 1;
    }

    if (!field){
        pCounterMap->m_bdbcOldData.m_ucCounterNum = 0;
        ret = 0;
    }else{
        UnistorKeyField* pField = NULL;
        UnistorKeyField* pTmp = NULL;
        parseMultiField(field->m_szData, pField);
        pTmp = pField;
        while(pTmp){
            ucId = strtoul(pTmp->m_key.c_str(), NULL, 0);
            if (ucId && pCounterMap->m_bdbcOldData.m_counterMap[ucId]){
                pCounterMap->m_bdbcOldData.m_counter[pCounterMap->m_bdbcOldData.m_counterMap[ucId]].m_uiValue = 0;
            }
            pTmp = pTmp->m_next;
        }
        freeField(pField);
        uiBufLen = UNISTOR_MAX_KV_SIZE;
        pCounterMap->m_bdbcOldData.m_uiVersion = uiVersion;
        ret = pCounterMap->m_bdbcOldData.encode((unsigned char*)szBuf, uiBufLen, tss->m_szBuf2K);
        if (-1 == ret) return -1;
    }

    if (ullSid > m_ullStoreSid)  m_ullStoreSid = ullSid;
    
    if (0 == ret){
        if (0 != _delKey(&pCounterMap->m_bdbcKey, bWriteCached, tss->m_szBuf2K)) return -1;
        return 1;
    }
    return (0 != _setKey(&pCounterMap->m_bdbcKey, szBuf, uiBufLen, bWriteCached, true, tss->m_szBuf2K))?-1:1;
}

///输出计数的内容，返回新的位置
char* UnistorStoreBdbc::_outputUiCounter(UnistorBdbcData& bdbcData, ///<计数器对象
                                           char const* szField, ///<要输出的计数器，NULL表示全部输出
                                           char* szBuf, ///<输出的buf
                                           CWX_UINT32& uiLen, ///<传入buf的大小，返回剩余buf的大小
                                           UnistorStoreBdbcCounterMap* pCounterMap, ///<线程的counter map对象
                                           char* szErr2K ///<出错时的错误信息
                                           )
{
    CWX_UINT32 pos = 0;
    CWX_UINT8 ucId = 0;
    bool bFirst = true;
    if (szField){
        UnistorKeyField* pField = NULL;
        UnistorKeyField* pTmp = NULL;
        parseMultiField(szField, pField);
        pTmp = pField;
        while(pTmp){
            if (pCounterMap->getId(pTmp->m_key.c_str(), ucId)){
                if (uiLen<pTmp->m_key.length() + 12){
                    if (szErr2K) strcpy(szErr2K, "No space to output counter.");
                    return NULL;
                }
                if (bdbcData.isExist(ucId)){
                    if (bFirst){
                        pos = CwxCommon::snprintf(szBuf, uiLen, "%s=%u", pTmp->m_key.c_str(),
                            bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue);
                        bFirst = false;
                    }else{
                        pos = CwxCommon::snprintf(szBuf, uiLen, ";%s=%u", pTmp->m_key.c_str(),
                            bdbcData.m_counter[bdbcData.m_counterMap[ucId]].m_uiValue);
                    }
                }else{
                    if (bFirst){
                        pos = CwxCommon::snprintf(szBuf, uiLen, "%s=0", pTmp->m_key.c_str());
                        bFirst = false;
                    }else{
                        pos = CwxCommon::snprintf(szBuf, uiLen, ";%s=0", pTmp->m_key.c_str());
                    }
                }
                uiLen -= pos;
                szBuf += pos;
            }
            pTmp=pTmp->m_next;
        }
        freeField(pField);
        szBuf[0] = 0x00;
    }else{
        map<char const*, CWX_UINT8, CwxCharLess>::iterator iter = pCounterMap->m_nameIdMap.begin();
        while(iter != pCounterMap->m_nameIdMap.end()){
            if (uiLen<strlen(iter->first) + 12){
                if (szErr2K) strcpy(szErr2K, "No space to output counter.");
                return NULL;
            }
            if (bdbcData.isExist(iter->second)){
                if (bFirst){
                    pos = CwxCommon::snprintf(szBuf, uiLen, "%s=%u", iter->first,
                        bdbcData.m_counter[bdbcData.m_counterMap[iter->second]].m_uiValue);
                    bFirst = false;
                }else{
                    pos = CwxCommon::snprintf(szBuf, uiLen, ";%s=%u", iter->first,
                        bdbcData.m_counter[bdbcData.m_counterMap[iter->second]].m_uiValue);
                }
            }else{
                if (bFirst){
                    pos = CwxCommon::snprintf(szBuf, uiLen, "%s=0", iter->first);
                    bFirst = false;
                }else{
                    pos = CwxCommon::snprintf(szBuf, uiLen, ";%s=0", iter->first);
                }
            }
            uiLen -= pos;
            szBuf += pos;
            iter++;
        }
        szBuf[0] = 0x00;
    }
    return szBuf;
}

///获取key, 1：成功；0：不存在；-1：失败;
int UnistorStoreBdbc::get(UnistorTss* tss,
                            CwxKeyValueItemEx const& key,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* ,
                            char const*& szData,
                            CWX_UINT32& uiLen,
                            bool& bKeyValue,
                            CWX_UINT32& uiVersion,
                            CWX_UINT32& uiFieldNum,
                            bool& bReadCached, ///<数据是否在read cache中
                            CWX_UINT8 ucKeyInfo)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }
    char* pPos = NULL;
    bReadCached = false;
    uiLen = UNISTOR_MAX_KV_SIZE;
    szData = tss->getBuf(uiLen);

    if (ucKeyInfo > 2) ucKeyInfo = 0;
    if (ucKeyInfo > 2) ucKeyInfo = 0;
    if (2 == ucKeyInfo){
        bKeyValue = false;
        uiFieldNum = 0;
        uiVersion = 0;
        return _getSysKey(tss, key.m_szData, key.m_uiDataLen, (char*)szData, uiLen);
    }

    if (UnistorBdbcKey::isNeedEncode()){
        pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
        pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
        if (!UnistorBdbcKey::encode(key.m_szData, key.m_uiDataLen, tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "[%s] is not a valid key", key.m_szData);
            return -1;
        }
    }else{
        pCounterMap->m_bdbcKey.m_szKey = key.m_szData;
        pCounterMap->m_bdbcKey.m_unKeyLen = key.m_uiDataLen;
    }

    int ret = 0;
    uiFieldNum = 0;
    bKeyValue = false;
    uiLen = UNISTOR_MAX_KV_SIZE;
    ret = _getKey(&pCounterMap->m_bdbcKey, (char*)szData, uiLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCached, true, true, tss->m_szBuf2K);
    if (-1 == ret) return -1;
    if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szData, uiLen, tss->m_szBuf2K)) return -1;
    uiFieldNum = pCounterMap->getCount();
    uiVersion = pCounterMap->m_bdbcOldData.m_uiVersion;
    if (ucKeyInfo){
        uiLen = sprintf((char*)szData,"%u,%u,%u,%u", 0, uiVersion, uiLen, 0);
    }else{
        uiLen = UNISTOR_MAX_KV_SIZE;
        pPos = _outputUiCounter(pCounterMap->m_bdbcOldData,
            field?field->m_szData:NULL,
            (char*)szData,
            uiLen,
            pCounterMap,
            tss->m_szBuf2K);
        if (!pPos) return -1;
        uiLen = pPos - szData;
    }
    return 1;
}


///获取key, 1：成功；0：不存在；-1：失败;
int UnistorStoreBdbc::gets(UnistorTss* tss,
                             list<pair<char const*, CWX_UINT16> > const& keys,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* ,
                             char const*& szData,
                             CWX_UINT32& uiLen,
                             CWX_UINT32& uiReadCacheNum, ///<在read cache中的数量
                             CWX_UINT32& uiExistNum, ///<存在的key的数量
                             CWX_UINT8 ucKeyInfo)
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }

    char* pPos = NULL;
    bool bReadCache = false;
    int ret = 0;
    list<string> counters;
    list<string>::iterator iter_counter;
    uiReadCacheNum = 0;
    uiExistNum = 0;
    bool bSuccess = true;
    list<pair<char const*, CWX_UINT16> >::const_iterator iter = keys.begin();
    szData = tss->getBuf(UNISTOR_MAX_KV_SIZE);
    tss->m_pEngineWriter->beginPack();

    if (ucKeyInfo > 2) ucKeyInfo = 0;
    while(iter != keys.end()){
        if (tss->m_pEngineWriter->getMsgSize() > UNISTOR_MAX_KVS_SIZE){
            CwxCommon::snprintf(tss->m_szBuf2K, 2048, "Output's data size is too big[%u], max is %u", tss->m_pEngineWriter->getMsgSize(), UNISTOR_MAX_KVS_SIZE);
            return -1;
        }
        if (2 == ucKeyInfo){
            uiLen = UNISTOR_MAX_KV_SIZE;
            ret = _getSysKey(tss, iter->first, iter->second, (char*)szData, uiLen);
            if (1 == ret){//key存在
                tss->m_pEngineWriter->addKeyValue(iter->first,iter->second, szData, uiLen, false);
            }else{
                tss->m_pEngineWriter->addKeyValue(iter->first, iter->second, "", 0, false);
            }
        }else{
            CwxCommon::split(string(iter->first), counters, BDBC_KEY_COUNTER_SPLIT_CHR);
            iter_counter = counters.begin();
            while(iter_counter != counters.end()){
                bSuccess = true;
                if (UnistorBdbcKey::isNeedEncode()){
                    pCounterMap->m_bdbcKey.m_szKey = tss->m_szStoreKey;
                    pCounterMap->m_bdbcKey.m_unKeyLen = UNISTOR_MAX_KEY_SIZE;
                    if (!UnistorBdbcKey::encode(iter_counter->c_str(), iter_counter->length(), tss->m_szStoreKey, pCounterMap->m_bdbcKey.m_unKeyLen, tss->m_szBuf2K)){
                        bSuccess = false;
                    }
                }else{
                    pCounterMap->m_bdbcKey.m_szKey = iter_counter->c_str();
                    pCounterMap->m_bdbcKey.m_unKeyLen = iter_counter->length();
                }
                if (bSuccess){
                    uiLen = UNISTOR_MAX_KV_SIZE;
                    ret = _getKey(&pCounterMap->m_bdbcKey, (char*)szData, uiLen, tss->m_szStoreKey, UNISTOR_MAX_KEY_SIZE, bReadCache, true, true, tss->m_szBuf2K);
                    if (bReadCache) uiReadCacheNum++;
                    if (1 == ret){//key存在
                        uiExistNum++;
                        if (!pCounterMap->m_bdbcOldData.decode((unsigned char*)szData, uiLen, tss->m_szBuf2K)){
                            tss->m_pEngineWriter->addKeyValue(iter_counter->c_str(), iter_counter->length(), "", 0, false);
                        }else{
                            if (ucKeyInfo){
                                uiLen = sprintf((char*)szData,"%u,%u,%u,%u", 0, pCounterMap->m_bdbcOldData.m_uiVersion, uiLen, 0);
                            }else{
                                uiLen = UNISTOR_MAX_KV_SIZE;
                                pPos = _outputUiCounter(pCounterMap->m_bdbcOldData,
                                    field?field->m_szData:NULL,
                                    (char*)szData,
                                    uiLen,
                                    pCounterMap,
                                    tss->m_szBuf2K);
                                if (!pPos) return -1;
                                uiLen = pPos - szData;
                            }
                            tss->m_pEngineWriter->addKeyValue(iter_counter->c_str(), iter_counter->length(), szData, uiLen, false);
                        }
                    }else{
                        tss->m_pEngineWriter->addKeyValue(iter_counter->c_str(), iter_counter->length(), "", 0, false);
                    }
                }else{
                    tss->m_pEngineWriter->addKeyValue(iter_counter->c_str(), iter_counter->length(), "", 0, false);
                }
                iter_counter++;
            }
        }
        iter++;
    }
    tss->m_pEngineWriter->pack();
    uiLen = tss->m_pEngineWriter->getMsgSize();
    szData = tss->getBuf(uiLen);
    memcpy((char*)szData, tss->m_pEngineWriter->getMsg(), uiLen);
    return 1;
}


///建立游标。-1：内部错误失败；0：不支持；1：成功
int UnistorStoreBdbc::createCursor(UnistorStoreCursor& cursor,
                                     char const* szBeginKey,
                                     char const* szEndKey,
                                     CwxKeyValueItemEx const* field,
                                     CwxKeyValueItemEx const*,
                                     char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    ///检查关闭cursor
    closeCursor(cursor);

    UnistorBdbcKey key;
    ///设置开始的key
    if (szBeginKey){
        if (UnistorBdbcKey::isNeedEncode()){
            key.m_szKey = cursor.m_beginKey;
            cursor.m_unBeginKeyLen  = UNISTOR_MAX_KEY_SIZE;
            if (!UnistorBdbcKey::encode(szBeginKey, strlen(szBeginKey), cursor.m_beginKey, cursor.m_unBeginKeyLen, szErr2K)){
                return -1;
            }
        }else{
            strcpy(cursor.m_beginKey, szBeginKey);
            cursor.m_unBeginKeyLen = strlen(szBeginKey);
        }
    }else{
        cursor.m_unBeginKeyLen = 0;
    }
    
    ///设置结束的key
    if (szEndKey){
        if (UnistorBdbcKey::isNeedEncode()){
            key.m_szKey = cursor.m_endKey;
            cursor.m_unEndKeyLen  = UNISTOR_MAX_KEY_SIZE;
            if (!UnistorBdbcKey::encode(szEndKey, strlen(szEndKey), cursor.m_endKey, cursor.m_unEndKeyLen, szErr2K)){
                return -1;
            }
        }else{
            strcpy(cursor.m_endKey, szEndKey);
            cursor.m_unEndKeyLen = strlen(szEndKey);
        }
    }else{
        cursor.m_unEndKeyLen = 0;
    }

    ///创建cache cursor
    cursor.m_cacheCursor = new UnistorStoreCacheCursor();

    ///解析field
    if (field) UnistorStoreBase::parseMultiField(field->m_szData, cursor.m_field);

    UnistorStoreBdcCursor* pCursor =  new UnistorStoreBdcCursor();
    pCursor->m_cursor = NULL;
    pCursor->m_bFirst = true;
    cursor.m_cursorHandle = pCursor;
    return 1;
}

///获取数据。-1：失败；0：结束；1：获取一个
int UnistorStoreBdbc::next(UnistorTss* tss,
                             UnistorStoreCursor& ,
                             char const*& ,
                             CWX_UINT16& ,
                             char const*& ,
                             CWX_UINT32& ,
                             bool& ,
                             CWX_UINT32& ,
                             bool )
{
    strcpy(tss->m_szBuf2K, "Not support");
    return -1;
}


///消息相关的event处理；0：成功；-1：失败
int UnistorStoreBdbc::storeEvent(UnistorTss* tss, CwxMsgBlock*& )
{
    if (!m_bValid){
        strcpy(tss->m_szBuf2K, m_szErrMsg);
        return -1;
    }
    return 0;
}


///关闭游标
void UnistorStoreBdbc::closeCursor(UnistorStoreCursor& cursor){
    UnistorStoreBdcCursor* pCursor = (UnistorStoreBdcCursor*) cursor.m_cursorHandle;
    if (pCursor){
        if (pCursor->m_cursor) pCursor->m_cursor->close(pCursor->m_cursor);
        delete pCursor;
        cursor.m_cursorHandle = NULL;
    }
    ///释放内存cursor
    if (cursor.m_cacheCursor){
        delete cursor.m_cacheCursor;
        cursor.m_cacheCursor = NULL;
    }
    ///释放field
    if (cursor.m_field){
        UnistorStoreBase::freeField(cursor.m_field);
        cursor.m_field = NULL;
    }
}

///开始导出数据。-1：内部错误失败；0：成功
int UnistorStoreBdbc::exportBegin(UnistorStoreCursor& cursor,
                                    char const* szStartKey,
                                    char const* szExtra, ///<extra信息
                                    UnistorSubscribe const& scribe,
                                    CWX_UINT64& ullSid,
                                    char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    ///释放cursor
    exportEnd(cursor);
    ///创建cache cursor
    cursor.m_cacheCursor = new UnistorStoreCacheCursor();
    UnistorStoreBdcCursor* pCursor =  new UnistorStoreBdcCursor();
    cursor.m_cursorHandle = pCursor;

    UnistorBdbcKey bdbcKey;
    if (szStartKey){
        if (UnistorBdbcKey::isNeedEncode()){
            pCursor->m_unStoreExportBeginKeyLen  = UNISTOR_MAX_KEY_SIZE;
            if (!UnistorBdbcKey::encode(szStartKey, strlen(szStartKey), pCursor->m_szStoreExportBeginKey, pCursor->m_unStoreExportBeginKeyLen, szErr2K)){
                if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "[%s] is not a valid key.", szStartKey);
                return -1;
            }
        }else{
            strcpy(pCursor->m_szStoreExportBeginKey, szStartKey);
            pCursor->m_unStoreExportBeginKeyLen  = strlen(szStartKey);
        }
    }else{
        pCursor->m_unStoreExportBeginKeyLen = 0;
    }
    
    if (szExtra){
        pCursor->m_uiDbIndex = strtoul(szExtra, NULL, 0);
    }else{
        pCursor->m_uiDbIndex = 0;
    }

    ///设置订阅
    cursor.m_scribe = scribe;

    if (!cursor.m_scribe.m_bAll &&
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_MOD) &&
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_RANGE) &&
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_KEY))
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Invalid scribe mode:%u", cursor.m_scribe.m_uiMode);
        return -1;
    }
    ullSid = getCache()->getPrevCommitSid();
    return 0;

}
///获取数据。-1：失败；0：结束；1：获取一个；2：skip数量为0
int UnistorStoreBdbc::exportNext(UnistorTss* tss,
                                   UnistorStoreCursor& cursor,
                                   char const*& szKey,
                                   CWX_UINT16& unKeyLen,
                                   char const*& szData,
                                   CWX_UINT32& uiDataLen,
                                   bool& bKeyValue,
                                   CWX_UINT32& uiVersion,
                                   CWX_UINT32& uiExpire,
                                   CWX_UINT16& unSkipNum,
                                   char const*& szExtra,
                                   CWX_UINT32& uiExtraLen)
{
    int ret = 0;
    uiExtraLen = 0;
    if (cursor.m_scribe.m_bAll || 
        (cursor.m_scribe.m_uiMode == UnistorSubscribe::SUBSCRIBE_MODE_MOD) ||
        (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_RANGE))
    {
        ret = _exportNext(tss, cursor, szKey, unKeyLen, szData, uiDataLen, bKeyValue, uiVersion, uiExpire, unSkipNum, szExtra, uiExtraLen);
    }else if (cursor.m_scribe.m_uiMode != UnistorSubscribe::SUBSCRIBE_MODE_KEY){
        ret = _exportKeyNext(tss, cursor, szKey, unKeyLen, szData, uiDataLen, bKeyValue, uiVersion, uiExpire, unSkipNum, szExtra, uiExtraLen);
    }else{
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Invalid scribe mode:%u", cursor.m_scribe.m_uiMode);
        return -1;
    }
    return ret;
}

///结束导出数据
void UnistorStoreBdbc::exportEnd(UnistorStoreCursor& cursor){
    UnistorStoreBdcCursor* pCursor = (UnistorStoreBdcCursor*) cursor.m_cursorHandle;
    if (pCursor){
        if (pCursor->m_cursor){
            pCursor->m_cursor->close(pCursor->m_cursor);
        }
        delete pCursor;
        cursor.m_cursorHandle = NULL;
    }
    if (cursor.m_cacheCursor){
        delete cursor.m_cacheCursor;
        cursor.m_cacheCursor = NULL;
    }
}

///检查订阅格式是否合法
bool UnistorStoreBdbc::isValidSubscribe(UnistorSubscribe const& subscribe,///<订阅对象
                                          char* szErr2K ///<不合法时的错误消息
                                          )
{
    char szBuf[UNISTOR_MAX_KEY_SIZE];
    CWX_UINT16 unKeyLen=UNISTOR_MAX_KEY_SIZE;
    if (!subscribe.m_bAll && (UnistorSubscribe::SUBSCRIBE_MODE_KEY == subscribe.m_uiMode)){
        map<string, string>::const_iterator iter = subscribe.m_key.m_keys.begin();
        while(iter != subscribe.m_key.m_keys.end()){
            if (iter->first.length()){
                if (UnistorBdbcKey::isNeedEncode()){
                    unKeyLen=UNISTOR_MAX_KEY_SIZE;
                    if (!UnistorBdbcKey::decode(iter->first.c_str(), iter->first.length(), szBuf, unKeyLen, szErr2K)){
                        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Subscribe's begin key[%s] is not a valid key", iter->first.c_str());
                        return false;
                    }
                }
            }
            if (iter->second.length()){
                if (!UnistorBdbcKey::decode(iter->second.c_str(), iter->second.length(), szBuf, unKeyLen, szErr2K)){
                    if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Subscribe's begin key[%s] is not a valid key", iter->first.c_str());
                    return false;
                }
            }

            if (iter->first.length() && iter->second.length()){
                if (UnistorBdbcKey::m_storeKeyLessFn(iter->first.c_str(), iter->first.length(), iter->second.c_str(), iter->second.length())>0){
                    if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Subscribe's begin key[%s] is more than end key[%s]", iter->first.c_str(), iter->second.c_str());
                    return false;
                }
            }
        }
        iter++;
    }
    return true;
}


int UnistorStoreBdbc::commit(char* szErr2K){
    CWX_INFO(("Begin commit write cache........"));
    int ret =  _commit(szErr2K);
    CWX_INFO(("End commit write cache........"));
    return ret;
}


///关闭，0：成功；-1：失败
int UnistorStoreBdbc::close(){
    int ret = 0;
    if (getCache())getCache()->stop();
    
    if (m_bdbTxn){
        if ((ret = m_bdbTxn->commit(m_bdbTxn,0)) != 0){
            CWX_ERROR(("Failure to commit bdb, err-code=%d, err=%s", ret, db_strerror(ret)));
        }
        m_bdbTxn = NULL;
    }

    if (m_bdbs){
        for (CWX_UINT32 i=0; i<m_uiGroupNum; i++){
            if (m_bdbs[i]){
                if ((ret = m_bdbs[i]->close(m_bdbs[i], 0)) != 0){
                    CWX_ERROR(("Failure to close bdb db, err-code=%d, err=%s", ret, db_strerror(ret)));
                }
            }
        }
        delete [] m_bdbs;
        m_bdbs = NULL;
    }

    if (m_sysDb){
        if ((ret = m_sysDb->close(m_sysDb, 0)) != 0){
            CWX_ERROR(("Failure to close bdb db, err-code=%d, err=%s", ret, db_strerror(ret)));
        }
        m_sysDb = NULL;
    }

    if (m_bdbEnv){
        if ((ret = m_bdbEnv->close(m_bdbEnv, 0)) != 0) {
            fprintf(stderr, "Failure to close bdb env: err-code=%d, err=%s", ret, db_strerror(ret));
        }
        m_bdbEnv = NULL;
    }

    if (m_pCounterDef){
        delete m_pCounterDef;
        m_pCounterDef = NULL;
    }

    return UnistorStoreBase::close();
}

///checkpoint
void UnistorStoreBdbc::checkpoint(UnistorTss* tss)
{
    if (m_bdbEnv && m_bValid){
        m_bdbEnv->txn_checkpoint(m_bdbEnv, 0, 0, 0);
        char **list = NULL;
        m_bdbEnv->log_archive(m_bdbEnv, &list, DB_ARCH_REMOVE);
        if (list != NULL)  free(list);
        ///检测配置文件
        if (0 != _checkCounterDef(tss->m_szBuf2K)){
            CWX_ERROR(("Failure to check conf file. err=%s",  tss->m_szBuf2K));
        }
    }
}

///commit。0：成功；-1：失败
int UnistorStoreBdbc::_commit(char* szErr2K){
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    if (0 != getCache()->commit(m_ullStoreSid, NULL, m_szErrMsg)){
        m_bValid = false;
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    m_uiUncommitBinlogNum = 0;
    m_uiLastCommitSecond = m_ttExpireClock;
    ///给checkpoint线程发送commit消息
    if (m_pMsgPipeFunc){
        CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
        msg->event().setEvent(EVENT_STORE_COMMIT);
        if (0 != m_pMsgPipeFunc(m_pApp, msg, false, szErr2K)){
            CwxMsgBlockAlloc::free(msg);
        }
    }
    return 0;
}

//0:成功；-1：失败
int UnistorStoreBdbc::_updateSysInfo(DB_TXN* tid, CWX_UINT64 ullSid, char* szErr2K){
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    //sid
    char szSid[64];
    char szKey[128];
    CwxCommon::toString(ullSid, szSid, 10);
    CWX_INFO(("Set bdb sid valus is %s", szSid));
    strcpy(szKey, UNISTOR_KEY_SID);
    if (0 != _setBdbKey(m_sysDb, tid, szKey, strlen(szKey), 128, szSid, strlen(szSid), 0, szErr2K)){
        return -1;
    }
    return 0;
}

//获取系统key。1：成功；0：不存在；-1：失败;
int UnistorStoreBdbc::_getSysKey(UnistorTss* tss, ///<线程tss对象
                                   char const* key, ///<要获取的key
                                   CWX_UINT16 unKeyLen, ///<key的长度
                                   char* szData, ///<若存在，则返回数据。内存有存储引擎分配
                                   CWX_UINT32& uiLen  ///<szData数据的字节数
                                )
{
    UnistorStoreBdbcCounterMap* pCounterMap = (UnistorStoreBdbcCounterMap*)tss->m_engineConf;
    if (!pCounterMap){
        pCounterMap = new UnistorStoreBdbcCounterMap();
        m_pCounterDef->clone(*pCounterMap);
        tss->m_engineConf = pCounterMap;
    }else if (pCounterMap->isDiff(m_pCounterDef->getSeq())){
        m_pCounterDef->clone(*pCounterMap);
    }

    string strValue(key, unKeyLen);

    if (1 == getSysKey(key, unKeyLen, szData, uiLen)) return 1;
    if (strValue == UNISTOR_KEY_SID){
        CwxCommon::toString(m_ullStoreSid, szData, 10);
    }else if (strValue == BDBC_KEY_GROUP_START_TIME_BIT){
        CwxCommon::snprintf(szData, 2047, "%u", m_bdbcConf.m_ucGroupStartBit);
    }else if (strValue == BDBC_KEY_GROUP_END_TIME_BIT){
        CwxCommon::snprintf(szData, 2047, "%u", m_bdbcConf.m_ucGroupEndBit);
    }else if (strValue == BDBC_KEY_COUNTERS){
        CwxCommon::snprintf(szData, 2047, "%s", pCounterMap->getDef().c_str());
    }else if (strValue == BDBC_KEY_KEY_TYPE){
        CwxCommon::snprintf(szData, 2047, "%s", m_bdbcConf.m_strKeyType.c_str());
    }else{
        return 0;
    }
    uiLen = strlen(szData);
    return 1;
}
//0:成功；-1：成功
int UnistorStoreBdbc::_loadSysInfo(DB_TXN* tid, char* szErr2K){
    char szBuf[512];
    char szKey[128];
    CWX_UINT32 uiBufLen=511;

    ///获取UNISTOR_KEY_SID
    int ret = _getBdbKey(m_sysDb,
        tid,
        UNISTOR_KEY_SID,
        strlen(UNISTOR_KEY_SID),
        szBuf,
        uiBufLen,
        szKey,
        128,
        0,
        m_szErrMsg);
    if (-1 == ret){
        CWX_ERROR(("Failure to get [%s] key, err=%s", UNISTOR_KEY_SID, m_szErrMsg));
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }else if (0 == ret){
        m_ullStoreSid = 0;
        CWX_INFO(("Not find bdb's sid[%s]", UNISTOR_KEY_SID));
    }else{
        szBuf[uiBufLen] = 0x00;
        m_ullStoreSid = strtoull(szBuf, NULL, 10);
        CWX_INFO(("Bdb's sid value is:%s", szBuf));
    }

    //获取、设置BDBC_KEY_GROUP_START_TIME_BIT
    uiBufLen=511;
    ret = _getBdbKey(m_sysDb,
        tid,
        BDBC_KEY_GROUP_START_TIME_BIT,
        strlen(BDBC_KEY_GROUP_START_TIME_BIT),
        szBuf,
        uiBufLen,
        szKey,
        128,
        0,
        m_szErrMsg);
    if (-1 == ret){
        CWX_ERROR(("Failure to get [%s] key, err=%s", BDBC_KEY_GROUP_START_TIME_BIT, m_szErrMsg));
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }else if (0 == ret){
        CWX_INFO(("Not find sys key [%s], insert it for value:%u", 
            BDBC_KEY_GROUP_START_TIME_BIT,
            m_bdbcConf.m_ucGroupStartBit));
        sprintf(szBuf, "%d", m_bdbcConf.m_ucGroupStartBit);
        strcpy(szKey, BDBC_KEY_GROUP_START_TIME_BIT);
        if (0 != _setBdbKey(m_sysDb, tid, szKey, strlen(szKey), 128, szBuf, strlen(szBuf), 0, m_szErrMsg)){
            CWX_ERROR(("Failure to set [%s] key, err=%s", BDBC_KEY_GROUP_START_TIME_BIT, m_szErrMsg));
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
            return -1;
        }
    }else{
        ///比较返回值
        szBuf[uiBufLen] = 0x00;
        CWX_INFO(("Sys's [%s] is %u", BDBC_KEY_GROUP_START_TIME_BIT, strtoul(szBuf, NULL, 10)));
        if (strtoul(szBuf, NULL, 10) != m_bdbcConf.m_ucGroupStartBit){
            CwxCommon::snprintf(m_szErrMsg, 2048, "[%s] key in sys-db is [%d], but config is [%d]",
                BDBC_KEY_GROUP_START_TIME_BIT,
                strtoul(szBuf, NULL, 10),
                m_bdbcConf.m_ucGroupStartBit);
            CWX_ERROR((m_szErrMsg));
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
            return -1;
        }
    }

    //获取、设置BDBC_KEY_GROUP_END_TIME_BIT
    uiBufLen=511;
    ret = _getBdbKey(m_sysDb,
        tid,
        BDBC_KEY_GROUP_END_TIME_BIT,
        strlen(BDBC_KEY_GROUP_END_TIME_BIT),
        szBuf,
        uiBufLen,
        szKey,
        128,
        0,
        m_szErrMsg);
    if (-1 == ret){
        CWX_ERROR(("Failure to get [%s] key, err=%s", BDBC_KEY_GROUP_END_TIME_BIT, m_szErrMsg));
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }else if (0 == ret){
        CWX_INFO(("Not find sys key [%s], insert it for value:%u", 
            BDBC_KEY_GROUP_END_TIME_BIT,
            m_bdbcConf.m_ucGroupEndBit));
        sprintf(szBuf, "%d", m_bdbcConf.m_ucGroupEndBit);
        strcpy(szKey, BDBC_KEY_GROUP_END_TIME_BIT);
        if (0 != _setBdbKey(m_sysDb, tid, szKey, strlen(szKey), 128, szBuf, strlen(szBuf), 0, m_szErrMsg)){
            CWX_ERROR(("Failure to set [%s] key, err=%s", BDBC_KEY_GROUP_END_TIME_BIT, m_szErrMsg));
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
            return -1;
        }
    }else{
        ///比较返回值
        szBuf[uiBufLen] = 0x00;
        CWX_INFO(("Sys's [%s] is %u", BDBC_KEY_GROUP_END_TIME_BIT, strtoul(szBuf, NULL, 10)));
        if (strtoul(szBuf, NULL, 10) != m_bdbcConf.m_ucGroupEndBit){
            CwxCommon::snprintf(m_szErrMsg, 2048, "[%s] key in sys-db is [%d], but config is [%d]",
                BDBC_KEY_GROUP_END_TIME_BIT,
                strtoul(szBuf, NULL, 10),
                m_bdbcConf.m_ucGroupEndBit);
            CWX_ERROR((m_szErrMsg));
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
            return -1;
        }
    }
    ///获取key类型BDBC_KEY_KEY_TYPE
    uiBufLen=511;
    ret = _getBdbKey(m_sysDb,
        tid,
        BDBC_KEY_KEY_TYPE,
        strlen(BDBC_KEY_KEY_TYPE),
        szBuf,
        uiBufLen,
        szKey,
        128,
        0,
        m_szErrMsg);
    if (-1 == ret){
        CWX_ERROR(("Failure to get [%s] key, err=%s", BDBC_KEY_KEY_TYPE, m_szErrMsg));
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }else if (0 == ret){
        CWX_INFO(("Not find sys key [%s], insert it for value:%s", 
            BDBC_KEY_KEY_TYPE,
            m_bdbcConf.m_strKeyType.c_str()));
        strcpy(szBuf, m_bdbcConf.m_strKeyType.c_str());
        strcpy(szKey, BDBC_KEY_KEY_TYPE);
        if (0 != _setBdbKey(m_sysDb, tid, szKey, strlen(szKey), 128, szBuf, strlen(szBuf), 0, m_szErrMsg)){
            CWX_ERROR(("Failure to set [%s] key, err=%s", BDBC_KEY_KEY_TYPE, m_szErrMsg));
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
            return -1;
        }
    }else{
        ///比较返回值
        szBuf[uiBufLen] = 0x00;
        CWX_INFO(("Sys's [%s] is %s", BDBC_KEY_KEY_TYPE, szBuf));
        if (m_bdbcConf.m_strKeyType != szBuf){
            CwxCommon::snprintf(m_szErrMsg, 2048, "[%s] key in sys-db is [%s], but config is [%s]",
                BDBC_KEY_KEY_TYPE,
                szBuf,
                m_bdbcConf.m_strKeyType.c_str());
            CWX_ERROR((m_szErrMsg));
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
            return -1;
        }
    }

    return 0;
}

//0:成功；-1：失败
int UnistorStoreBdbc::_setKey(UnistorBdbcKey const* key,
                                char const* szData,
                                CWX_UINT32 uiLen,
                                bool& bWriteCache,
                                bool bCache,
                                char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = getCache()->updateKey(key->m_szKey, key->m_unKeyLen, szData, uiLen, 0, bCache, bWriteCache);
    if (0 == ret){
        if (-1 == _commit(szErr2K)) return -1;
        ret = getCache()->updateKey(key->m_szKey, key->m_unKeyLen, szData, uiLen, 0, bCache, bWriteCache);
    };
    if (-1 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, getCache()->getErrMsg());
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }else if (0 == ret){
        m_bValid = true;
        strcpy(m_szErrMsg, "UnistorCache::updateKey can't return 0 after commit");
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }else if (-2 == ret){
        CWX_ASSERT(0);
    }
    m_uiUncommitBinlogNum++;
    if (isNeedCommit()){
        if (0 != _commit(szErr2K)) return -1;
    }
    return 0;
}


//1：获取；-1：失败
int UnistorStoreBdbc::_getKey(UnistorBdbcKey const* key,
                                char* szData,
                                CWX_UINT32& uiLen,
                                char* szStoreKeyBuf,
                                CWX_UINT16 unKeyBufLen,
                                bool& isCached,
                                bool bCache,
                                bool bAddEmpty,
                                char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = 0;
    bool bDel = false;
    CWX_UINT32 uiIndex =_getBdbIndex(*key);
    ret = getCache()->getKey(key->m_szKey, key->m_unKeyLen, szData, uiLen, bDel, isCached);
    if (-1 == ret){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Data buf size[%u] is too small.", uiLen);
        return -1;
    }else if (1 == ret){
        isCached = true;
        if (bDel){
            memcpy(szData, m_szEmptyData, m_uiEmptyDataLen);
            uiLen = m_uiEmptyDataLen;
        }
        return 1;
    }
    isCached = false;
    ret =  _getBdbKey(m_bdbs[uiIndex], NULL, key->m_szKey, key->m_unKeyLen, szData, uiLen, szStoreKeyBuf, unKeyBufLen, DB_READ_UNCOMMITTED, szErr2K);

    if (-1 == ret) return -1;

    if (0 == ret){
        memcpy(szData, m_szEmptyData, m_uiEmptyDataLen);
        uiLen = m_uiEmptyDataLen;
    }
    //cache数据
    if ((bCache && 1==ret) || bAddEmpty){
        getCache()->cacheKey(key->m_szKey, key->m_unKeyLen, szData, uiLen, true);
    }
    return 1;
}


//0:成功；-1：失败
int UnistorStoreBdbc::_delKey(UnistorBdbcKey const* key,
                                bool& bWriteCache,
                                char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = getCache()->delKey(key->m_szKey, key->m_unKeyLen, 0, bWriteCache);
    if (0 == ret){
        if (-1 == _commit(szErr2K)) return -1;
        ret = getCache()->delKey(key->m_szKey, key->m_unKeyLen, 0, bWriteCache);
    };
    if (-1 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, getCache()->getErrMsg());
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }else if (0 == ret){
        m_bValid = false;
        strcpy(m_szErrMsg, "UnistorCache::updateKey can't return 0 after commit");
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        CWX_ERROR((m_szErrMsg));
        return -1;
    }
    m_uiUncommitBinlogNum++;
    if (isNeedCommit()){
        if (0 != _commit(szErr2K)) return -1;
    }
    return 0;
}


//0:成功；-1：失败
int UnistorStoreBdbc::_setBdbKey(DB* db,
                                   DB_TXN* tid,
                                   char const* szKey,
                                   CWX_UINT16 unKeyLen,
                                   CWX_UINT16 unKeyBufLen,
                                   char const* szData,
                                   CWX_UINT32 uiDataLen,
                                   CWX_UINT32 flags,
                                   char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = 0;
    DBT bdb_key;
    memset(&bdb_key, 0x00, sizeof(bdb_key));
    bdb_key.size = unKeyLen;
    bdb_key.data = (void*)szKey;
    bdb_key.flags=DB_DBT_USERMEM;
    bdb_key.ulen = unKeyBufLen;
    DBT bdb_data;
    memset(&bdb_data, 0, sizeof(DBT));
    bdb_data.data = (void*)szData;
    bdb_data.size = uiDataLen;
    ret = db->put(db, tid, &bdb_key, &bdb_data, flags);
    if (0 != ret){
        CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to put key to bdb, key=%s, err-code:%d, err-msg=%s",
            szKey,
            ret,
            db_strerror(ret));
        CWX_ERROR((szErr2K));
        if (szErr2K) {
            if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        }
        if ((DB_LOCK_DEADLOCK!=ret)){
            m_bValid = false;
        }
        return -1;
    }
    return 0;
}


//0:不存在；1：获取；-1：失败
int UnistorStoreBdbc::_getBdbKey(DB* db,
                                   DB_TXN* tid,
                                   char const* szKey,
                                   CWX_UINT16 unKeyLen,
                                   char* szData,
                                   CWX_UINT32& uiLen,
                                   char* szStoreKeyBuf,
                                   CWX_UINT16 unKeyBufLen,
                                   CWX_UINT32 flags,
                                   char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret = 0;
    DBT bdb_key;
    memset(&bdb_key, 0x00, sizeof(bdb_key));
    bdb_key.size = unKeyLen;
    memcpy(szStoreKeyBuf, szKey, unKeyLen);
    bdb_key.data = (void*)szStoreKeyBuf;
    bdb_key.flags=DB_DBT_USERMEM;
    bdb_key.ulen = unKeyBufLen;
    DBT bdb_data;
    memset(&bdb_data, 0, sizeof(bdb_data));
    bdb_data.data = szData;
    bdb_data.ulen = uiLen;
    bdb_data.flags = DB_DBT_USERMEM;
    {
        ret = db->get(db, tid, &bdb_key, &bdb_data, flags);
        if (0 == ret){
            uiLen = bdb_data.size;
            return 1;
        }else if (DB_NOTFOUND == ret){
            return 0;
        }
    }
    if (szErr2K){
        CwxCommon::snprintf(szErr2K, 2047, "Failure to fetch bdb key:%s, err-code:%d, err-msg=%s",
            szKey,
            ret,
            db_strerror(ret));
        CWX_ERROR((szErr2K));
    }else{
        CWX_ERROR(("Failure to fetch bdb key:%s, err-code:%d, err-msg=%s",
            szKey,
            ret,
            db_strerror(ret)));
    }
    return -1;
}
//0:成功；-1：失败
int UnistorStoreBdbc::_delBdbKey(DB* db,
                                   DB_TXN* tid,
                                   char const* szKey,
                                   CWX_UINT16 unKeyLen,
                                   CWX_UINT16 unKeyBufLen,
                                   CWX_UINT32 flags,
                                   char* szErr2K)
{
    if (!m_bValid){
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
        return -1;
    }
    int ret  = 0;
    DBT bdb_key;
    memset(&bdb_key, 0x00, sizeof(bdb_key));
    bdb_key.size = unKeyLen;
    bdb_key.data = (void*)szKey;
    bdb_key.flags=DB_DBT_USERMEM;
    bdb_key.ulen = unKeyBufLen;
    //delete from bdb
    ret = db->del(db, tid, &bdb_key, flags);
    if ((0 == ret)||(DB_NOTFOUND==ret)){
        return 0;
    }
    CwxCommon::snprintf(m_szErrMsg, 2047, "Failure to del key from bdb, key=%s, err-code:%d, err-msg=%s",
        szKey,
        ret,
        db_strerror(ret));
    CWX_ERROR((szErr2K));
    if (szErr2K) {
        if (szErr2K) strcpy(szErr2K, m_szErrMsg);
    }
    if ((DB_LOCK_DEADLOCK!=ret)){
        m_bValid = false;
    }
    return -1;
}


//导出mod模式的订阅。-1：失败；0：结束；1：获取一个；2：skip数量为0
int UnistorStoreBdbc::_exportNext(UnistorTss* tss,
                                    UnistorStoreCursor& cursor,
                                    char const*& szKey,
                                    CWX_UINT16& unKeyLen,
                                    char const*& szData,
                                    CWX_UINT32& uiDataLen,
                                    bool& bKeyValue,
                                    CWX_UINT32& uiVersion,
                                    CWX_UINT32& uiExpire,
                                    CWX_UINT16& unSkipNum,
                                    char const*& szExtra,
                                    CWX_UINT32& uiExtraLen
                                    )
{
    UnistorStoreBdcCursor* pCursor = (UnistorStoreBdcCursor*)cursor.m_cursorHandle;
    char const* pPos = NULL;
    DBT bdb_key;
    DBT bdb_data;
    int ret = 0;
    CWX_UINT32 uiGroupId = 0;
    if (!pCursor->m_cursor){
        ret = _createCursor(pCursor->m_uiDbIndex, pCursor->m_cursor, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (0 == ret) return 0;
        pCursor->m_bFirst = true;
    }
    do{
        if (!unSkipNum) return 2;
        memset(&bdb_key, 0x00, sizeof(bdb_key));
        memset(&bdb_data, 0x00, sizeof(bdb_data));        
        bdb_key.data = (void*)pCursor->m_szStoreKey;
        bdb_key.flags=DB_DBT_USERMEM;
        bdb_key.ulen = UNISTOR_MAX_KEY_SIZE;
        szKey = pCursor->m_szStoreKey;
        bdb_data.data = (void*)pCursor->m_szStoreData;
        bdb_data.ulen = UNISTOR_MAX_KV_SIZE;
        bdb_data.flags = DB_DBT_USERMEM;
        szData = pCursor->m_szStoreData;
        if (pCursor->m_bFirst){///第一次
            pCursor->m_bFirst = false;
            if (pCursor->m_unStoreExportBeginKeyLen){
                bdb_key.size = sizeof(UnistorBdbcKey);
                memcpy(pCursor->m_szStoreKey, pCursor->m_szStoreExportBeginKey, pCursor->m_unStoreExportBeginKeyLen);
                if ((ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_SET_RANGE)) == 0){
                    if (UnistorBdbcKey::m_storeKeyLessFn((char const*)bdb_key.data,
                        bdb_key.size,
                        pCursor->m_szStoreExportBeginKey,
                        pCursor->m_unStoreExportBeginKeyLen) == 0)
                    {///下一个
                        ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_NEXT);
                    }
                }

            }else{
                ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_FIRST);
            }
        }else{
            ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_NEXT);
        }

        if (0 == ret){
            uiDataLen = bdb_data.size;
            if (!cursor.m_scribe.m_bAll){
                uiGroupId = UnistorBdbcKey::m_storeKeyGroupFn(pCursor->m_szStoreKey, bdb_key.size);
                if (UnistorSubscribe::SUBSCRIBE_MODE_MOD == cursor.m_scribe.m_uiMode){
                    if (!cursor.m_scribe.m_mod.isSubscribe(uiGroupId)){
                        unSkipNum--;
                        continue;
                    }
                }else{
                    if (!cursor.m_scribe.m_range.isSubscribe(uiGroupId)){
                        unSkipNum--;
                        continue;
                    }
                }
            }
            if (UnistorBdbcKey::isNeedDecode()){
                unKeyLen = UNISTOR_MAX_KEY_SIZE;
                UnistorBdbcKey::decode((char const*)bdb_key.data, bdb_key.size, pCursor->m_szStoreKey, unKeyLen, NULL);
            }else{
                memcpy(pCursor->m_szStoreKey, bdb_key.data, bdb_key.size);
                unKeyLen = bdb_key.size;
            }
            bKeyValue = false;
            if (!pCursor->m_bdbcData.decode((unsigned char*)bdb_data.data, bdb_data.size, tss->m_szBuf2K)) return -1;
            uiDataLen = UNISTOR_MAX_KV_SIZE;
            if (!_outputSyncCounter(pCursor->m_bdbcData,
                pCursor->m_szStoreData,
                uiDataLen,
                tss->m_szBuf2K))
            {
                return -1;
            }
            uiDataLen = pPos - pCursor->m_szStoreData;
            uiVersion = pCursor->m_bdbcData.m_uiVersion;
            uiExpire = 0;
            bKeyValue = false;
            szExtra = cursor.m_cacheCursor->m_szCacheData;
            uiExtraLen = sprintf(cursor.m_cacheCursor->m_szCacheData, "%u", pCursor->m_uiDbIndex);
            unSkipNum--;
            return 1;
        }else if (DB_NOTFOUND == ret){
            pCursor->m_cursor->close(pCursor->m_cursor);
            pCursor->m_cursor = NULL;
            pCursor->m_uiDbIndex++;
            ret = _createCursor(pCursor->m_uiDbIndex, pCursor->m_cursor, tss->m_szBuf2K);
            if (-1 == ret) return -1;
            if (0 == ret) return 0;
            pCursor->m_bFirst = true;
            continue;
        }else{
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Failure to move cursor. err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            return -1;
        }
    }while(1);
    return 0;
}


//导出key模式的订阅。-1：失败；0：结束；1：获取一个；2：skip数量为0
int UnistorStoreBdbc::_exportKeyNext(UnistorTss* tss,
                                       UnistorStoreCursor& cursor,
                                       char const*& szKey,
                                       CWX_UINT16& unKeyLen,
                                       char const*& szData,
                                       CWX_UINT32& uiDataLen,
                                       bool& bKeyValue,
                                       CWX_UINT32& uiVersion,
                                       CWX_UINT32& uiExpire,
                                       CWX_UINT16& unSkipNum,
                                       char const*& szExtra,
                                       CWX_UINT32& uiExtraLen)
{
    UnistorStoreBdcCursor* pCursor = (UnistorStoreBdcCursor*)cursor.m_cursorHandle;
    char const* pPos = NULL;
    DBT bdb_key;
    DBT bdb_data;
    int ret = 0;
    bool  bSeek = false; ///是否重新定位
    if (!pCursor->m_cursor){
        ret = _createCursor(pCursor->m_uiDbIndex, pCursor->m_cursor, tss->m_szBuf2K);
        if (-1 == ret) return -1;
        if (0 == ret) return 0;
        pCursor->m_bFirst = true;
    }
    bool  bNext = pCursor->m_bFirst;

    do{
        if (!unSkipNum) return 2;
        memset(&bdb_key, 0x00, sizeof(bdb_key));
        memset(&bdb_data, 0x00, sizeof(bdb_data));        
        bdb_key.data = (void*)pCursor->m_szStoreKey;
        bdb_key.flags=DB_DBT_USERMEM;
        bdb_key.ulen = UNISTOR_MAX_KEY_SIZE;
        szKey = pCursor->m_szStoreKey;
        bdb_data.data = (void*)pCursor->m_szStoreData;
        bdb_data.ulen = UNISTOR_MAX_KV_SIZE;
        bdb_data.flags = DB_DBT_USERMEM;
        szData = pCursor->m_szStoreData;
        if (pCursor->m_bFirst || bSeek){///第一次
            ///对于key模式，修订begin的值
            string strKey;
            string strBegin;
            string strEnd;
            if (pCursor->m_bFirst){
                strKey.assign(pCursor->m_szStoreExportBeginKey, pCursor->m_unStoreExportBeginKeyLen);
            }else{
                strKey.assign(cursor.m_cacheCursor->m_szCacheKey, cursor.m_cacheCursor->m_unCacheKeyLen);
            }
            pCursor->m_bFirst = false;
            if (!_exportKeyInit(strKey, strBegin, strEnd, cursor.m_scribe.m_key)){
                pCursor->m_cursor->close(pCursor->m_cursor);
                pCursor->m_cursor = NULL;
                pCursor->m_uiDbIndex++;
                ret = _createCursor(pCursor->m_uiDbIndex, pCursor->m_cursor, tss->m_szBuf2K);
                if (-1 == ret) return -1;
                if (0 == ret) return 0;
                pCursor->m_bFirst = true;
                continue;
            }
            cursor.m_cacheCursor->m_unCacheKeyLen = strBegin.size();
            if(cursor.m_cacheCursor->m_unCacheKeyLen){
                memcpy(cursor.m_cacheCursor->m_szCacheKey, strBegin.c_str(), cursor.m_cacheCursor->m_unCacheKeyLen);
            }

            cursor.m_cacheCursor->m_uiCacheDataLen = strEnd.size();
            if (cursor.m_cacheCursor->m_uiCacheDataLen){
                memcpy(cursor.m_cacheCursor->m_szCacheData, strEnd.c_str(), cursor.m_cacheCursor->m_uiCacheDataLen);
            }

            if (cursor.m_cacheCursor->m_unCacheKeyLen){
                bdb_key.size = cursor.m_cacheCursor->m_unCacheKeyLen;
                memcpy(pCursor->m_szStoreKey, cursor.m_cacheCursor->m_szCacheKey, cursor.m_cacheCursor->m_unCacheKeyLen);
                if ((ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_SET_RANGE)) == 0){
                    if (bNext &&
                        (UnistorBdbcKey::m_storeKeyLessFn((char const*)bdb_key.data, bdb_key.size, cursor.m_cacheCursor->m_szCacheKey, cursor.m_cacheCursor->m_unCacheKeyLen) == 0))
                    {///下一个
                        bNext = false;
                        ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_NEXT);
                    }
                }
            }else{
                ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_FIRST);
            }
        }else{
            ret = pCursor->m_cursor->get(pCursor->m_cursor, &bdb_key, &bdb_data, DB_NEXT);
        }
        if (0 == ret){
            if (cursor.m_cacheCursor->m_uiCacheDataLen && ///不能做unSkipNum--
                (UnistorBdbcKey::m_storeKeyLessFn((char*)bdb_key.data, bdb_key.size, cursor.m_cacheCursor->m_szCacheData, cursor.m_cacheCursor->m_uiCacheDataLen) >= 0)){///已经结束
                memcpy(cursor.m_cacheCursor->m_szCacheKey, szKey, unKeyLen);
                cursor.m_cacheCursor->m_unCacheKeyLen = unKeyLen;
                bSeek = true;
                continue;
            }
            if (UnistorBdbcKey::isNeedDecode()){
                unKeyLen = UNISTOR_MAX_KEY_SIZE;
                UnistorBdbcKey::decode((char const*)bdb_key.data, bdb_key.size, pCursor->m_szStoreKey, unKeyLen, NULL);
            }else{
                memcpy(pCursor->m_szStoreKey, bdb_key.data, bdb_key.size);
                unKeyLen = bdb_key.size;
            }
            bKeyValue = false;
            if (!pCursor->m_bdbcData.decode((unsigned char*)bdb_data.data, bdb_data.size, tss->m_szBuf2K)) return -1;
            uiDataLen = UNISTOR_MAX_KV_SIZE;
            if (!_outputSyncCounter(pCursor->m_bdbcData,
                pCursor->m_szStoreData,
                uiDataLen,
                tss->m_szBuf2K))
            {
                return -1;
            }
            uiDataLen = pPos - pCursor->m_szStoreData;
            uiVersion = pCursor->m_bdbcData.m_uiVersion;
            uiExpire = 0;
            bKeyValue = false;
            szExtra = cursor.m_cacheCursor->m_szCacheData;
            uiExtraLen = sprintf(cursor.m_cacheCursor->m_szCacheData, "%u", pCursor->m_uiDbIndex);
            unSkipNum--;
            return 1;
        }else if(DB_NOTFOUND == ret){
            pCursor->m_cursor->close(pCursor->m_cursor);
            pCursor->m_cursor = NULL;
            pCursor->m_uiDbIndex++;
            ret = _createCursor(pCursor->m_uiDbIndex, pCursor->m_cursor, tss->m_szBuf2K);
            if (-1 == ret) return -1;
            if (0 == ret) return 0;
            pCursor->m_bFirst = true;
            continue;
        }else{
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Failure to move cursor. err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            return -1;
        }
    }while(1);
    return 0;
}

bool UnistorStoreBdbc::_exportKeyInit(string const& strKeyBegin,
                                        string& strBegin,
                                        string& strEnd,
                                        UnistorSubscribeKey const& keys)
{
    char szBegin[UNISTOR_MAX_KEY_SIZE];
    CWX_UINT16 unKeyBeginLen = 0;
    char szEnd[UNISTOR_MAX_KEY_SIZE];
    CWX_UINT16 unKeyEndLen = 0;
    map<string,string>::const_iterator iter =  keys.m_keys.begin();
    while(iter != keys.m_keys.end()){
        if (iter->first.size()){
            if (UnistorBdbcKey::isNeedEncode()){
                unKeyBeginLen = UNISTOR_MAX_KEY_SIZE;
                UnistorBdbcKey::encode(iter->first.c_str(), iter->first.length(), szBegin, unKeyBeginLen, NULL);
            }else{
                strcpy(szBegin, iter->first.c_str());
                unKeyBeginLen = iter->first.length();
            }
        }
        if (iter->second.size()){
            if (UnistorBdbcKey::isNeedEncode()){
                unKeyEndLen = UNISTOR_MAX_KEY_SIZE;
                UnistorBdbcKey::encode(iter->second.c_str(), iter->second.length(), szEnd, unKeyEndLen, NULL);
            }else{
                strcpy(szBegin, iter->second.c_str());
                unKeyBeginLen = iter->second.length();
            }
        }

        if (!iter->second.size() || 
            (UnistorBdbcKey::m_storeKeyLessFn(strKeyBegin.c_str(), strKeyBegin.size(), szEnd, unKeyEndLen)<0))
        {
            ///设置begin
            if (iter->first.size() && strKeyBegin.size()){
                if (UnistorBdbcKey::m_storeKeyLessFn(strKeyBegin.c_str(), strKeyBegin.size(), szBegin, unKeyBeginLen) <= 0){
                    strBegin.assign(szBegin, unKeyBeginLen);
                }else{
                    strBegin.assign(strKeyBegin.c_str(), strKeyBegin.size());
                }
            }else if (iter->first.size()){
                strBegin.assign(szBegin, unKeyBeginLen);
            }else{
                strBegin.assign(strKeyBegin.c_str(), strKeyBegin.size());
            }
            ///设置end
            if (iter->second.size()){
                strEnd.assign(szEnd, unKeyEndLen);
            }else{
                strEnd.erase();
            }
            return true;

        }
        iter++;
    }
    return false;

}
