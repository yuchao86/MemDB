#include "UnistorTask4Trans.h"
#include "UnistorApp.h"
#include "UnistorPoco.h"
#include "UnistorHandler4Trans.h"

void UnistorTask4Trans::noticeTimeout(CwxTss* ){
    m_strErrMsg = "Timeout";
    m_uiErrCode = UNISTOR_ERR_TIMEOUT;
    setTaskState(CwxTaskBoardTask::TASK_STATE_FINISH);
}

void UnistorTask4Trans::noticeRecvMsg(CwxMsgBlock*& msg, CwxTss* , bool& ){
    m_msg = msg;
    setTaskState(CwxTaskBoardTask::TASK_STATE_FINISH);
    msg = NULL;
}

void UnistorTask4Trans::noticeFailSendMsg(CwxMsgBlock*& , CwxTss* ){
    setTaskState(CwxTaskBoardTask::TASK_STATE_FINISH);
    m_strErrMsg = "No master.";
    m_uiErrCode = UNISTOR_ERR_NO_MASTER;
}

void UnistorTask4Trans::noticeEndSendMsg(CwxMsgBlock*& , CwxTss* , bool& ){
}


void UnistorTask4Trans::noticeConnClosed(CWX_UINT32 , CWX_UINT32 , CWX_UINT32 , CwxTss*){
    setTaskState(CwxTaskBoardTask::TASK_STATE_FINISH);
    m_strErrMsg = "No master.";
    m_uiErrCode = UNISTOR_ERR_NO_MASTER;
}

int UnistorTask4Trans::noticeActive(CwxTss* ThrEnv){
    ///����Ȼ��trans�̣߳���ˣ����԰�ȫ�Ĳ���UnistorHandler4Trans�е�static����
    UnistorTss* tss= (UnistorTss*)ThrEnv;
    setTaskState(TASK_STATE_WAITING);

    if (!UnistorHandler4Trans::transMsg(tss, getTaskId(), m_tranMsg)){
        m_strErrMsg = "No master.";
        m_uiErrCode = UNISTOR_ERR_NO_MASTER;
        setTaskState(TASK_STATE_FINISH);
        return -1;
    }
    return 0;
}

void UnistorTask4Trans::execute(CwxTss* pThrEnv){
    UnistorTss* tss= (UnistorTss*)pThrEnv;
    if (CwxTaskBoardTask::TASK_STATE_INIT == getTaskState()){
        m_msg = NULL;
        m_strErrMsg = "";
        m_uiErrCode = UNISTOR_ERR_SUCCESS;
        ///������Ҫ����Ϣ
        m_uiRecvThreadIndex = m_tranMsg->event().getHostId();
        m_uiRecvConnId = m_tranMsg->event().getConnId();
        m_recvMsgHead = m_tranMsg->event().getMsgHeader();
        ///���ó�ʱ
        CWX_UINT64 timeStamp = UNISTOR_TRANS_TIMEOUT_SECOND;
        timeStamp *= 1000000;
        timeStamp += CwxDate::getTimestamp();
        this->setTimeoutValue(timeStamp);
        ///��������
        getTaskBoard()->noticeActiveTask(this, pThrEnv);
    }
    ///�ͷ��յ�����Ϣ
    if (m_tranMsg) CwxMsgBlockAlloc::free(m_tranMsg);
    m_tranMsg = NULL;
    ///�����ɣ���ظ�    
    if (CwxTaskBoardTask::TASK_STATE_FINISH == getTaskState()){
        reply(m_pApp, tss, m_uiRecvConnId, m_uiRecvThreadIndex, m_recvMsgHead, m_msg, m_uiErrCode, m_strErrMsg.c_str());
        if (m_msg){
            CwxMsgBlockAlloc::free(m_msg);
            m_msg = NULL;
        }
        delete this;
    }
}

void UnistorTask4Trans::reply(UnistorApp* pApp,
                              UnistorTss* tss,
                              CWX_UINT32 uiRecvConnId, ///<�յ�������id
                              CWX_UINT32 uiRecvThreadIndex, ///<�յ����߳�����
                              CwxMsgHead const& recvHead,  ///<�յ�����Ϣͷ
                              CwxMsgBlock* replyMsg,
                              CWX_UINT32 uiErrCode,
                              char const* szErrMsg)
{
    CwxMsgBlock* block = NULL;
    if (!replyMsg){
        CWX_ASSERT(uiErrCode != UNISTOR_ERR_SUCCESS);
        UnistorPoco::packRecvReply(tss->m_pWriter,
            block,
            recvHead.getTaskId(),
            recvHead.getMsgType() + 1,
            uiErrCode,
            0,
            0,
            szErrMsg,
            tss->m_szBuf2K);
    }else{
        block = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + replyMsg->length());
        CwxMsgHead head(recvHead);
        head.setMsgType(head.getMsgType() + 1);
        head.setDataLen(replyMsg->length());
        memcpy(block->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
        memcpy(block->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, replyMsg->rd_ptr(), replyMsg->length());
        block->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + replyMsg->length());
    }
    block->event().setSvrId(UnistorApp::SVR_TYPE_RECV);
    block->event().setEvent(EVENT_SEND_MSG);
    block->event().setConnId(uiRecvConnId);
    if (pApp->getRecvThreadPools()[uiRecvThreadIndex]->append(block)<=1){
        pApp->getRecvChannels()[uiRecvThreadIndex]->notice();
    }
}

