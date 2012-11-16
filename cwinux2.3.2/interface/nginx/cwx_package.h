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
@version 0.1
@date 2009-10-12
@warning
@bug
*/
#include "cwx_config.h"

///Key/value��־λ
#define   CWX_PACKAGE_KV_BIT  31
///����kv����
#define   CWX_MAX_KV_LEN  0X7FFFFFFF

///����key/value���ݶ�
typedef struct CWX_KEY_VALUE_ITEM_S{
    char*          m_szKey; ///<key������
    char*          m_szData; ///<key������
    CWX_UINT32      m_uiDataLen; ///<���ݵĳ���
    CWX_UINT16      m_unKeyLen; ///<key�ĳ���
    bool           m_bKeyValue; ///<true��value�ı���Ҳ��key/value��ʽ;false��value����key/value��ʽ
}CWX_KEY_VALUE_ITEM_S;

/**
*@brief ��ȡpackage�е���һ��Key��
*@param [in] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
*@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
*/
int cwx_get_next_key(char const* szMsg, CWX_UINT32 uiMsgLen, CWX_KEY_VALUE_ITEM_S* item);

/**
*@brief ��ȡpackage�еĵ�uiIndex Key/Value,���unIndexΪ0�����൱��GetNextKey()��
*@param [in] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [in] unIndex Ҫ��ȡ��key��������
*@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
*@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
*/
int cwx_get_key_by_index(char const *szMsg, CWX_UINT32 uiMsgLen, CWX_UINT32 uiIndex, CWX_KEY_VALUE_ITEM_S* item);

/**
*@brief ��ȡpackage�еĵ�һ��key������ΪszKey��Key/Value��
*@param [in] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [in] szKey Ҫ��ȡ��key�����֣���key�������ظ�����ֻ��ȡ��һ����
*@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
*@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
*/
int cwx_get_key_by_name(char const *szMsg, CWX_UINT32 uiMsgLen, char const* szKey, CWX_KEY_VALUE_ITEM_S* item);

/**
*@brief ��package�����һ����key/value��
*@param [in,out] szMsg ��Ҫ�����package��
*@param [in] uiMsgLen package�ĳ��ȡ�
*@param [in] szKey Ҫ��ӵ�key�����֡�
*@param [in] szValue key��data��
*@param [in] uiDatalen data�ĳ���
*@param [in] bKeyValue data�Ƿ�Ϊkey/value
*@return -1�����Ŀռ�̫С��>0 ����İ��ĳ��ȡ�
*/
int cwx_append_key(char *szMsg, CWX_UINT32 uiMsgLen, char const* szKey, char const* szValue, CWX_UINT32 uiDatalen, bool bKeyValue);

/**
*@brief ���szMsg�Ƿ���һ����Ч��Package.uiMsgLenΪ0��ʱ�򣬱�ʾΪ�հ����հ���һ����Ч�İ���
*@param [in] szMsg Ҫ���İ�
*@param [in] uiMsgLen ���ĳ���
*@return true:��Ч�İ���false����Ч�İ�.
*/
bool cwx_is_valid_package(char const *szMsg, CWX_UINT32 uiMsgLen);
///��ȡpackage��key������, -1: invalid package
int cwx_get_key_value_num(char const* szMsg, CWX_UINT32 uiMsgLen);
///ͨ��Key�ĳ��ȼ�data�ĳ��ȣ���ȡ������Key/value���ȡ�
CWX_UINT32 cwx_get_kv_len(CWX_UINT16 unKeyLen, CWX_UINT32 uiDataLen);
///ͨ��key/value�ĳ��ȼ�key�ĳ��ȣ���ȡdata�ĳ���
CWX_UINT32 cwx_get_data_len(CWX_UINT32 uiKeyValueLen, CWX_UINT16 unKeyLen);

#endif

