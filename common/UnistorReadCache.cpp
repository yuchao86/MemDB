#include "UnistorReadCache.h"

UNISTOR_KEY_CMP_EQUAL_FN UnistorReadCacheItem::m_fnEqual = NULL; ///<key相等的比较函数
UNISTOR_KEY_CMP_LESS_FN UnistorReadCacheItem::m_fnLess = NULL; ///<key小于的比较函数
UNISTOR_KEY_HASH_FN UnistorReadCacheItem::m_fnHash = NULL; ///<key的hash值的计算函数
