#include "UnistorReadCache.h"

UNISTOR_KEY_CMP_EQUAL_FN UnistorReadCacheItem::m_fnEqual = NULL; ///<key��ȵıȽϺ���
UNISTOR_KEY_CMP_LESS_FN UnistorReadCacheItem::m_fnLess = NULL; ///<keyС�ڵıȽϺ���
UNISTOR_KEY_HASH_FN UnistorReadCacheItem::m_fnHash = NULL; ///<key��hashֵ�ļ��㺯��
