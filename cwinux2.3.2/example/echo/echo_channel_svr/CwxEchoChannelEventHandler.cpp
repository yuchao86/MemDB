#include "CwxEchoChannelEventHandler.h"

/**
@brief ���ӿɶ��¼�������-1��close()�ᱻ����
@return -1������ʧ�ܣ������close()�� 0������ɹ�
*/
int CwxEchoChannelEventHandler::onInput()
{
    int ret = CwxAppHandler4Channel::recvPackage(getHandle(),
        m_uiRecvHeadLen,
        m_uiRecvDataLen,
        m_szHeadBuf,
        m_header,
        m_recvMsgData);
    if (1 != ret) return ret;
    replyMessage();
    if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
    this->m_recvMsgData = NULL;
    this->m_uiRecvHeadLen = 0;
    this->m_uiRecvDataLen = 0;
    return 0;
}
/**
@brief ֪ͨ���ӹرա�
@return �����������ӣ�1������engine���Ƴ�ע�᣻0������engine���Ƴ�ע�ᵫ��ɾ��handler��-1����engine�н�handle�Ƴ���ɾ����
*/
int CwxEchoChannelEventHandler::onConnClosed()
{
    return -1;
}

void CwxEchoChannelEventHandler::replyMessage()
{
    ///����echo�ظ�����Ϣ���ͣ�Ϊ�������Ϣ����+1
    m_recvMsgData->event().getMsgHeader().setMsgType(m_recvMsgData->event().getMsgHeader().getMsgType() + 1);
    ///����echo�ظ������ݰ�����
    m_recvMsgData->event().getMsgHeader().setDataLen(m_recvMsgData->length());
    ///�����ظ������ݰ�
    CwxMsgBlock* pBlock = CwxMsgBlockAlloc::malloc(m_recvMsgData->length() + CwxMsgHead::MSG_HEAD_LEN);
    ///�������ݰ��İ�ͷ
    memcpy(pBlock->wr_ptr(), m_recvMsgData->event().getMsgHeader().toNet(), CwxMsgHead::MSG_HEAD_LEN);
    ///����block��дָ��
    pBlock->wr_ptr(CwxMsgHead::MSG_HEAD_LEN);
    ///�������ݰ�������
    memcpy(pBlock->wr_ptr(), m_recvMsgData->rd_ptr(), m_recvMsgData->length());
    ///����block��дָ��
    pBlock->wr_ptr(m_recvMsgData->length());
    if (!putMsg(pBlock))
    {
        CWX_ERROR(("Failure to put message"));
        CwxMsgBlockAlloc::free(pBlock);
    }
    m_ullMsgNum ++;
    if (m_ullMsgNum && !(m_ullMsgNum%10000))
    {
        char szBuf[64];
        CwxCommon::toString(m_ullMsgNum, szBuf, 10);
        CWX_INFO(("Recv echo message num:%s", szBuf));
    }


}
