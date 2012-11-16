#include "CwxEchoEventHandler.h"
#include "CwxEchoApp.h"

///echo����Ĵ�����
int CwxEchoEventHandler::onRecvMsg(CwxMsgBlock*& msg, CwxTss* )
{
    ///����echo�ظ�����Ϣ���ͣ�Ϊ�������Ϣ����+1
    msg->event().getMsgHeader().setMsgType(msg->event().getMsgHeader().getMsgType() + 1);
    ///����echo�ظ������ݰ�����
    msg->event().getMsgHeader().setDataLen(msg->length());
    ///�����ظ������ݰ�
    CwxMsgBlock* pBlock = CwxMsgBlockAlloc::malloc(msg->length() + CwxMsgHead::MSG_HEAD_LEN);
    ///�������ݰ��İ�ͷ
    memcpy(pBlock->wr_ptr(), msg->event().getMsgHeader().toNet(), CwxMsgHead::MSG_HEAD_LEN);
    ///����block��дָ��
    pBlock->wr_ptr(CwxMsgHead::MSG_HEAD_LEN);
    ///�������ݰ�������
    memcpy(pBlock->wr_ptr(), msg->rd_ptr(), msg->length());
    ///����block��дָ��
    pBlock->wr_ptr(msg->length());
    ///���ûظ���Ϣ�ķ��Ϳ�����Ϣ
    pBlock->send_ctrl().reset();
    ///���ûظ���Ϣ��Ӧ���ӵ�svr-id
    pBlock->send_ctrl().setSvrId(msg->event().getSvrId());
    ///���ûظ���Ϣ��Ӧ���ӵ�host-id
    pBlock->send_ctrl().setHostId(msg->event().getHostId());
    ///���ûظ���Ϣ������id
    pBlock->send_ctrl().setConnId(msg->event().getConnId());
    ///�ظ���Ϣ
    if (0 != this->m_pApp->sendMsgByConn(pBlock))
    {
        CWX_ERROR(("Failure to send msg"));
        return -1;
    }
    m_ullMsgNum ++;
    if (m_ullMsgNum && !(m_ullMsgNum%10000))
    {
        char szBuf[64];
        CwxCommon::toString(m_ullMsgNum, szBuf, 10);
        CWX_INFO(("Recv echo message num:%s", szBuf));
    }
    return 1;
}

