#ifndef __CWX_MSG_HEADER_H__
#define __CWX_MSG_HEADER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file cwx_msg_header.h
@brief ����cwinux ��Ϣͷ
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


///����ͨ�Ű�ͷ�ĳ���
#define  CWX_MSG_HEAD_LEN   14

///����ϵͳ��Ϣ������
#define  CWX_MSG_ATTR_SYS_MSG (1<<0)
///��������ѹ��������
#define  CWX_MSG_ATTR_COMPRESS (1<<1)

///keep alive����Ϣ����
#define CWX_MSG_TYPE_KEEPALIVE  1
///keep alive�ظ�����Ϣ����
#define CWX_MSG_TYPE_KEEPALIVE_REPLY 2

///����ͨ�Ű�ͷ
typedef struct CWX_MSG_HEADER_S
{
    CWX_UINT8      m_ucVersion;  ///<��Ϣ�汾��
    CWX_UINT8      m_ucAttr;     ///<��Ϣ����
    CWX_UINT16     m_unMsgType;  ///<��Ϣ����
    CWX_UINT32     m_uiTaskId;   ///<�����ID
    CWX_UINT32     m_uiDataLen;  ///<���͵����ݳ���
}CWX_MSG_HEADER_S;


/**
*@brief ����Ϣͷ����������ֽ�������ݰ�����
*@param [in] header ����pack����Ϣ��ͷ��
*@param [out] szHead packed����Ϣ��ͷbuf
*@return ����szHead
*/
char const* cwx_msg_pack_head(CWX_MSG_HEADER_S const*  header, char* szHead);
/**
*@brief ���յ�����Ϣͷ�������Ϣͷ����
*@param [in] szHead ��Ϣ��ͷ��buf
*@param [out] header ��Ϣ��ͷ
*@return -1:У�������0:�����ȷ
*/
int cwx_msg_unpack_head(char const* szHead, CWX_MSG_HEADER_S*  header);

/**
*@brief �Ƿ�Ϊkeep-alive����Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return 1:�ǣ�0������
*/
int cwx_msg_is_keepalive(CWX_MSG_HEADER_S const*  header);

/**
*@brief �Ƿ�Ϊkeep-alive�Ļظ���Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return 1:�ǣ�0������
*/
int cwx_msg_is_keepalive_reply(CWX_MSG_HEADER_S const*  header);

/**
*@brief �Ƿ�ϵͳ��Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return 1:�ǣ�0������
*/
int cwx_msg_is_sys_msg(CWX_MSG_HEADER_S const*  header);

/**
*@brief ����ϵͳ��Ϣ���
*@param [in] header ��Ϣ��ͷ��
*@return void
*/
void cwx_msg_set_sys_msg(CWX_MSG_HEADER_S*  header);

/**
*@brief ������ϢͷΪkeep-alive��Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return void
*/
void cwx_msg_set_keepalive(CWX_MSG_HEADER_S *  header);

/**
*@brief ������ϢͷΪkeep-alive��reply��Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return void
*/
void cwx_msg_set_keepalive_reply(CWX_MSG_HEADER_S *  header);


#ifdef __cplusplus
}
#endif

#endif

