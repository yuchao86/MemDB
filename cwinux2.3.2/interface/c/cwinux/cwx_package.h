#ifndef __CWX_PACKAGE_H__
#define __CWX_PACKAGE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file cwx_package.h
@brief ����key/value���ݰ��Ĳ���
@author cwinux@gmail.com
@version 1.0
@date 2010-10-04
@warning
@bug
*/
#include "cwx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

///Key/value��־λ
#define   CWX_PACKAGE_KV_BIT  31
///����kv����
#define   CWX_PACKAGE_MAX_KV_LEN  0X7FFFFFFF

///����key/value���ݶ�
typedef struct CWX_KEY_VALUE_ITEM_S
{
    char*           m_szKey; ///<key������
    char*           m_szData; ///<key������
    CWX_UINT32      m_uiDataLen; ///<���ݵĳ���
    CWX_UINT16      m_unKeyLen; ///<key�ĳ���
    CWX_INT16       m_bKeyValue; ///<1��value�ı���Ҳ��key/value��ʽ;0��value����key/value��ʽ
}CWX_KEY_VALUE_ITEM_S;

/**
*@brief ��ȡpackage�е���һ��Key��
*@param [in] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
*@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
*/
int cwx_pg_get_next_key(char const* szMsg,
                             CWX_UINT32 uiMsgLen,
                             CWX_KEY_VALUE_ITEM_S* item);

/**
*@brief ��ȡpackage�еĵ�uiIndex Key/Value,���unIndexΪ0�����൱��GetNextKey()��
*@param [in] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [in] unIndex Ҫ��ȡ��key��������
*@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
*@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
*/
int cwx_pg_get_key_by_index(char const *szMsg,
                                 CWX_UINT32 uiMsgLen,
                                 CWX_UINT32 uiIndex,
                                 CWX_KEY_VALUE_ITEM_S* item);

/**
*@brief ��ȡpackage�еĵ�һ��key������ΪszKey��Key/Value��
*@param [in] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [in] szKey Ҫ��ȡ��key�����֣���key�������ظ�����ֻ��ȡ��һ����
*@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
*@param [in] bCaseSensive key�������Ƿ��Сд���С�1����Сд���У�0Ϊ�����С�
*@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
*/
int cwx_pg_get_key_by_name(char const *szMsg,
                                CWX_UINT32 uiMsgLen,
                                char const* szKey,
                                CWX_KEY_VALUE_ITEM_S* item,
                                int bCaseSensive
                                );

/**
*@brief ��package�����һ����key/value��
*@param [in,out] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [in] szKey Ҫ��ӵ�key�����֡�
*@param [in] szValue key��data��
*@param [in] uiDatalen data�ĳ���
*@param [in] bKeyValue data�Ƿ�Ϊkey/value,1:��;0:����
*@return -1�����Ŀռ�̫С��>0 ����İ��ĳ��ȡ�
*/
int cwx_pg_append_key(char *szMsg,
                           CWX_UINT32 uiMsgLen,
                           char const* szKey,
                           char const* szValue,
                           CWX_UINT32 uiDatalen,
                           int bKeyValue);
/**
*@brief ��package��ɾ��key����ΪszKey��Key/value��
*@param [in, out] szMsg package��
*@param [in, out] uiMsgLen ����package�ĳ��ȣ������µĳ��ȡ�
*@param [in] szKey Ҫɾ����key���֡�
*@param [in] bAll �Ƿ�Ҫɾ������key������ΪszKey��key/value��1���ǣ�0������
*@param [in] bCaseSensive key�������Ƿ��Сд���С�1����Сд���У�0Ϊ�����С�
*@return -1����Ч��package��0��û�з��֣�>0��ɾ����������
*/
int  cwx_pg_remove_key(char *szMsg,
                            CWX_UINT32* uiMsgLen,
                            char const* szKey,
                            int bAll,
                            int bCaseSensive);
/**
*@brief ��package��ɾ����unIndex��Key��
*@param [in,out] szMsg package��
*@param [in,out] uiMsgLen ����package�ĳ��ȣ������µĳ��ȡ�
*@param [in] unIndex Ҫɾ��key��Index��
*@return -1����Ч��package��0��û�з��֣�1��ɾ����һ��KEY��
*/
int  cwx_pg_remove_key_by_index(char *szMsg,
                                     CWX_UINT32* uiMsgLen,
                                     CWX_UINT16 unIndex);
/**
*@brief ��package�е�һ��Key������ΪszKey�����ݣ��޸�ΪszDataָ�������ݡ�
*@param [in,out] szMsg package��
*@param [in,out] uiMsgLen ����package�ĳ��ȣ������µĳ��ȡ�
*@param [in] uiMaxMsgLen Package�����������
*@param [in] szKey Ҫ�޸ĵ�key��
*@param [in] szData Ҫ�ı�ɵ���data��
*@param [in] uiDataLen Ҫ�ı�ɵ���data�ĳ��ȡ�
*@param [in] bKeyValue �������Ƿ�ΪKey/value��ʽ��1���ǣ�0������
*@param [in] bCaseSensive key�������Ƿ��Сд���С�1����Сд���У�0�������С�
*@return -2�ռ䲻����-1����Ч��package��0��û�з��֣�1���޸���һ��KEY��
*/
int  cwx_pg_modify_key(char *szMsg,
                      CWX_UINT32* uiMsgLen,
                      CWX_UINT32 uiMaxMsgLen,
                      char const* szKey,
                      char const* szData,
                      CWX_UINT32 uiDataLen,
                      int bKeyValue,
                      int bCaseSensive);
/**
*@brief ��package�е�unIndex��Key�����ݣ��޸�ΪszDataָ�������ݡ�
*@param [in,out] szMsg package��
*@param [in,out] uiMsgLen ����package�ĳ��ȣ������µĳ��ȡ�
*@param [in] uiMaxMsgLen Package�����������
*@param [in] unIndex Ҫ�޸ĵ�key��������
*@param [in] szData Ҫ�ı�ɵ���data��
*@param [in] uiDataLen Ҫ�ı�ɵ���data�ĳ��ȡ�
*@param [in] bKeyValue �������Ƿ�ΪKey/value��ʽ��1���ǣ�0������
*@return -2�ռ䲻����-1����Ч��package��0��û�з��֣�1���޸���һ��KEY��
*/
int  cwx_pg_modify_key_by_index(char *szMsg,
                                     CWX_UINT32* uiMsgLen,
                                     CWX_UINT32 uiMaxMsgLen,
                                     CWX_UINT16 unIndex,
                                     char const* szData,
                                     CWX_UINT32 uiDataLen,
                                     int bKeyValue);

/**
*@brief ���szMsg�Ƿ���һ����Ч��Package��uiMsgLenΪ0��ʱ�򣬱�ʾΪ�հ����հ���һ����Ч�İ���
*@param [in] szMsg Ҫ���İ�
*@param [in] uiMsgLen ���ĳ���
*@return 1:��Ч�İ���0����Ч�İ�.
*/
int cwx_pg_is_valid(char const *szMsg, CWX_UINT32 uiMsgLen);
/**
*@brief ��ȡpackage��key��������
*@param [in] szMsg package��buf
*@param [in] uiMsgLen ���ĳ���
*@return -1����Ч�İ�������Ϊkey��������
*/
int cwx_pg_get_key_num(char const* szMsg, CWX_UINT32 uiMsgLen);
/**
*@brief ͨ��Key�ĳ��ȼ�data�ĳ��ȣ���ȡ������Key/value���ȡ�
*@param [in] unKeyLen key�ĳ���
*@param [in] uiDataLen valude�ĳ���
*@return �γɵ�key-value���ȡ�
*/
CWX_UINT32 cwx_pg_get_kv_len(CWX_UINT16 unKeyLen, CWX_UINT32 uiDataLen);
/**
*@brief ����key-value������key�ĳ��ȣ������value���ֵĳ��ȡ�
*@param [in] uiKeyValueLen key-value�ĳ���
*@param [in] unKeyLen key�ĳ���
*@return ����value�ĳ��ȡ�
*/
CWX_UINT32 cwx_pg_get_value_len(CWX_UINT32 uiKeyValueLen, CWX_UINT16 unKeyLen);

/**
*@brief ��ȡkey��key/value�е��ڴ�����ƫ��ֵ��
*@return ����key��key/value�е�ƫ�ơ�
*/
CWX_UINT16 cwx_pg_get_key_offset();
#ifdef __cplusplus
}
#endif

#endif

