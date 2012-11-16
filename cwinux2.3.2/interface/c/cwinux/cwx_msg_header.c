#include "cwx_msg_header.h"
#ifdef __cplusplus
extern "C" {
#endif
char const* cwx_msg_pack_head(CWX_MSG_HEADER_S const*  header, char* szHead)
{
    CWX_UINT32 byte4;
    CWX_UINT16 byte2;
    CWX_UINT8  pos = 0;
    CWX_UINT16 unCheckSum=0;
    unCheckSum += (CWX_UINT16)(header->m_ucVersion + header->m_ucAttr + header->m_unMsgType + header->m_uiTaskId + header->m_uiDataLen);
    //version
    szHead[pos] = header->m_ucVersion; pos++;
    //attr
    szHead[pos] = header->m_ucAttr; pos++;
    //Msg Type
    byte2 = htons(header->m_unMsgType); memcpy(szHead+pos, &byte2, 2); pos+=2;
    //task ID
    byte4 = htonl(header->m_uiTaskId); memcpy(szHead+pos, &byte4, 4); pos+=4;
    //data length
    byte4 = htonl(header->m_uiDataLen); memcpy(szHead+pos, &byte4, 4); pos+=4;
    //checksum
    byte2 = htons(unCheckSum); memcpy(szHead+pos, &byte2, 2); pos+=2;
    return szHead;
}

int cwx_msg_unpack_head(char const* szHead, CWX_MSG_HEADER_S *  header)
{
    CWX_UINT32 byte4;
    CWX_UINT16 byte2;
    CWX_UINT8  pos = 0;
    CWX_UINT16  unCheckSum=0;
    //version
    header->m_ucVersion = szHead[pos];pos++;
    //attr
    header->m_ucAttr = szHead[pos]; pos++;
    //Msg Type
    memcpy(&byte2, szHead+pos, 2); header->m_unMsgType = ntohs(byte2); pos+=2;
    //task ID
    memcpy(&byte4, szHead+pos, 4); header->m_uiTaskId = ntohl(byte4); pos+=4;
    //data length
    memcpy(&byte4, szHead+pos, 4); header->m_uiDataLen = ntohl(byte4); pos+=4;
    //checksum
    memcpy(&byte2, szHead+pos, 2); unCheckSum = ntohs(byte2); pos+=2;
    return (unCheckSum == (CWX_UINT16)(header->m_ucVersion + header->m_ucAttr + header->m_unMsgType + header->m_uiTaskId + header->m_uiDataLen))?0:-1;
}

/**
*@brief �Ƿ�Ϊkeep-alive����Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return 1:�ǣ�0������
*/
int cwx_msg_is_keepalive(CWX_MSG_HEADER_S const*  header)
{
    return CWX_CHECK_ATTR(header->m_ucAttr, CWX_MSG_ATTR_SYS_MSG)&&(CWX_MSG_TYPE_KEEPALIVE==header->m_unMsgType)?1:0;
}

/**
*@brief �Ƿ�Ϊkeep-alive�Ļظ���Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return 1:�ǣ�0������
*/
int cwx_msg_is_keepalive_reply(CWX_MSG_HEADER_S const*  header)
{
    return CWX_CHECK_ATTR(header->m_ucAttr, CWX_MSG_ATTR_SYS_MSG)&&(CWX_MSG_TYPE_KEEPALIVE_REPLY==header->m_unMsgType)?1:0;
}

/**
*@brief �Ƿ�ϵͳ��Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return 1:�ǣ�0������
*/
int cwx_msg_is_sys_msg(CWX_MSG_HEADER_S const*  header)
{
    return CWX_CHECK_ATTR(header->m_ucAttr, CWX_MSG_ATTR_SYS_MSG)?1:0;
}

/**
*@brief ����ϵͳ��Ϣ���
*@param [in] header ��Ϣ��ͷ��
*@return void
*/
void cwx_msg_set_sys_msg(CWX_MSG_HEADER_S*  header)
{
    CWX_SET_ATTR(header->m_ucAttr, CWX_MSG_ATTR_SYS_MSG);
}

/**
*@brief ������ϢͷΪkeep-alive��Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return void
*/
void cwx_msg_set_keepalive(CWX_MSG_HEADER_S *  header)
{
    CWX_SET_ATTR(header->m_ucAttr, CWX_MSG_ATTR_SYS_MSG);
    header->m_unMsgType = CWX_MSG_TYPE_KEEPALIVE;
}

/**
*@brief ������ϢͷΪkeep-alive��reply��Ϣ
*@param [in] header ��Ϣ��ͷ��
*@return void
*/
void cwx_msg_set_keepalive_reply(CWX_MSG_HEADER_S *  header)
{
    CWX_SET_ATTR(header->m_ucAttr, CWX_MSG_ATTR_SYS_MSG);
    header->m_unMsgType = CWX_MSG_TYPE_KEEPALIVE_REPLY;
}

#ifdef __cplusplus
}
#endif
