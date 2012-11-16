#ifndef __CWX_MSG_HEADER_H__
#define __CWX_MSG_HEADER_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file cwx_msg_header.h
@brief ����cwinux module���̨����ͨ�����ݰ��İ�ͷ������
@author cwinux@gmail.com
@version 0.1
@date 2009-10-12
@warning
@bug
*/
#include "cwx_config.h"

///����ͨ�Ű�ͷ�ĳ���
#define  CWINUX_MSG_HEAD_LEN   14
//����attr
///����ϵͳ��Ϣ������
#define  CWINUX_MSG_ATTR_SYS_MSG (1<<0)
///��������ѹ��������
#define  CWINUX_MSG_ATTR_COMPRESS (1<<1)
///����ͨ�Ű�ͷ
typedef struct CWX_MSG_HEADER_S{
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
inline char const* cwx_pack_head(CWX_MSG_HEADER_S const*  header, char* szHead);
/**
*@brief ���յ�����Ϣͷ�������Ϣͷ����
*@param [in] szHead ��Ϣ��ͷ��buf
*@param [out] header ��Ϣ��ͷ
*@return false:У�������true:�����ȷ
*/
inline bool cwx_unpack_head(char const* szHead, CWX_MSG_HEADER_S*  header);

#include "cwx_msg_header.inl"

#endif

