#ifndef __UNISTOR_TASK_4_TRANS_H__
#define __UNISTOR_TASK_4_TRANS_H__

#include "UnistorMacro.h"
#include "CwxTaskBoard.h"
#include "UnistorTss.h"
///ǰ������
class UnistorApp;


//UI����̨������ִ�н����ȡ��task����
class UnistorTask4Trans : public CwxTaskBoardTask
{
public:
    enum{
        TASK_STATE_WAITING = CwxTaskBoardTask::TASK_STATE_USER
    };
    ///���캯��
    UnistorTask4Trans(UnistorApp* pApp, CwxTaskBoard* pTaskBoard):CwxTaskBoardTask(pTaskBoard)
    {
        m_tranMsg = NULL;
        m_uiRecvThreadIndex = 0;
        m_uiRecvConnId = 0;
        m_pApp = pApp;
        m_msg = NULL;
        m_uiErrCode = UNISTOR_ERR_SUCCESS;
    }
    ///��������
    ~UnistorTask4Trans()
    {
        if (m_tranMsg) CwxMsgBlockAlloc::free(m_tranMsg);
        if (m_msg) CwxMsgBlockAlloc::free(m_msg);
    }
public:
    /**
    @brief ֪ͨTask�Ѿ���ʱ
    @param [in] pThrEnv �����̵߳�Thread-env
    @return void
    */
    virtual void noticeTimeout(CwxTss* pThrEnv);
    /**
    @brief ֪ͨTask���յ�һ�����ݰ���
    @param [in] msg �յ�����Ϣ
    @param [in] pThrEnv �����̵߳�Thread-env
    @param [out] bConnAppendMsg �յ���Ϣ�������ϣ��Ƿ��д����յ�������Ϣ��true���ǣ�false��û��
    @return void
    */
    virtual void noticeRecvMsg(CwxMsgBlock*& msg,
        CwxTss* pThrEnv,
        bool& bConnAppendMsg);
    /**
    @brief ֪ͨTask���ⷢ�͵�һ�����ݰ�����ʧ�ܡ�
    @param [in] msg �յ�����Ϣ
    @param [in] pThrEnv �����̵߳�Thread-env
    @return void
    */
    virtual void noticeFailSendMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ֪ͨTaskͨ��ĳ�����ӣ�������һ�����ݰ���
    @param [in] msg ���͵����ݰ�����Ϣ
    @param [in] pThrEnv �����̵߳�Thread-env
    @param [out] bConnAppendMsg ������Ϣ�������ϣ��Ƿ��еȴ��ظ�����Ϣ��true���ǣ�false��û��
    @return void
    */
    virtual void noticeEndSendMsg(CwxMsgBlock*& msg,
        CwxTss* pThrEnv,
        bool& bConnAppendMsg);
    /**
    @brief ֪ͨTask�ȴ��ظ���Ϣ��һ�����ӹرա�
    @param [in] uiSvrId �ر����ӵ�SVR-ID
    @param [in] uiHostId �ر����ӵ�HOST-ID
    @param [in] uiConnId �ر����ӵ�CONN-ID
    @param [in] pThrEnv �����̵߳�Thread-env
    @return void
    */
    virtual void noticeConnClosed(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_UINT32 uiConnId,
        CwxTss* pThrEnv);
    /**
    @brief ����Task����Task����ǰ��Task��Task�Ĵ����߳���ӵ�С�
    ������ǰ��Task���Խ����Լ����첽��Ϣ�������ܴ���
    ��ʱ��Taskboard��noticeActiveTask()�ӿڵ��õġ�
    @param [in] pThrEnv �����̵߳�Thread-env
    @return 0���ɹ���-1��ʧ��
    */
    virtual int noticeActive(CwxTss* pThrEnv);
    /**
    @brief ִ��Task���ڵ��ô�APIǰ��Task��Taskboard�в����ڣ�Ҳ����˵�Ա���̲߳��ɼ���
    TaskҪô�Ǹմ���״̬��Ҫô�������ǰһ���׶εĴ����������״̬��
    ͨ���˽ӿڣ���Task�Լ������Լ���step����ת����������ϵTask�����ͼ�������̡�
    @param [in] pTaskBoard ����Task��Taskboard
    @param [in] pThrEnv �����̵߳�Thread-env
    @return void
    */
    virtual void execute(CwxTss* pThrEnv);
public:
    ///<�ظ�ת����Ϣ
    static void reply(UnistorApp* pApp, ///<app����
        UnistorTss* tss, ///<�߳�tss
        CWX_UINT32 uiRecvConnId, ///<�յ�������id
        CWX_UINT32 uiRecvThreadIndex, ///<�յ����߳�����
        CwxMsgHead const& recvHead,  ///<�յ�����Ϣͷ
        CwxMsgBlock* replyMsg, ///<ת����Ϣ�Ļظ�����Ϊnull����ʾ����uiErrCode��szErrMsg��Ч
        CWX_UINT32 uiErrCode=UNISTOR_ERR_SUCCESS, ///<ת��ʧ�ܵĴ������
        char const* szErrMsg = NULL ///<ת��ʧ�ܵĴ�����Ϣ
        );
public:
    CwxMsgBlock*        m_tranMsg; ///ת������Ϣmsg
private:
    CWX_UINT16          m_uiRecvThreadIndex; ///<������Ϣ���̳߳�
    CWX_UINT32          m_uiRecvConnId; ///<������Ϣ������id
    CwxMsgHead          m_recvMsgHead; ///<���յ���Ϣ����Ϣͷ
    UnistorApp*   m_pApp; ///<app����
    CwxMsgBlock*        m_msg; ///<�յ�����Ϣmessage
    string              m_strErrMsg; ///<������Ϣ
    CWX_UINT32          m_uiErrCode; ///<�������
};



#endif

