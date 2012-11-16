#ifndef __UNISTOR_TASK_4_TRANS_H__
#define __UNISTOR_TASK_4_TRANS_H__

#include "UnistorMacro.h"
#include "CwxTaskBoard.h"
#include "UnistorTss.h"
///前置声明
class UnistorApp;


//UI控制台的任务执行结果获取的task对象
class UnistorTask4Trans : public CwxTaskBoardTask
{
public:
    enum{
        TASK_STATE_WAITING = CwxTaskBoardTask::TASK_STATE_USER
    };
    ///构造函数
    UnistorTask4Trans(UnistorApp* pApp, CwxTaskBoard* pTaskBoard):CwxTaskBoardTask(pTaskBoard)
    {
        m_tranMsg = NULL;
        m_uiRecvThreadIndex = 0;
        m_uiRecvConnId = 0;
        m_pApp = pApp;
        m_msg = NULL;
        m_uiErrCode = UNISTOR_ERR_SUCCESS;
    }
    ///析构函数
    ~UnistorTask4Trans()
    {
        if (m_tranMsg) CwxMsgBlockAlloc::free(m_tranMsg);
        if (m_msg) CwxMsgBlockAlloc::free(m_msg);
    }
public:
    /**
    @brief 通知Task已经超时
    @param [in] pThrEnv 调用线程的Thread-env
    @return void
    */
    virtual void noticeTimeout(CwxTss* pThrEnv);
    /**
    @brief 通知Task的收到一个数据包。
    @param [in] msg 收到的消息
    @param [in] pThrEnv 调用线程的Thread-env
    @param [out] bConnAppendMsg 收到消息的连接上，是否还有待接收的其他消息。true：是；false：没有
    @return void
    */
    virtual void noticeRecvMsg(CwxMsgBlock*& msg,
        CwxTss* pThrEnv,
        bool& bConnAppendMsg);
    /**
    @brief 通知Task往外发送的一个数据包发送失败。
    @param [in] msg 收到的消息
    @param [in] pThrEnv 调用线程的Thread-env
    @return void
    */
    virtual void noticeFailSendMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief 通知Task通过某条连接，发送了一个数据包。
    @param [in] msg 发送的数据包的信息
    @param [in] pThrEnv 调用线程的Thread-env
    @param [out] bConnAppendMsg 发送消息的连接上，是否有等待回复的消息。true：是；false：没有
    @return void
    */
    virtual void noticeEndSendMsg(CwxMsgBlock*& msg,
        CwxTss* pThrEnv,
        bool& bConnAppendMsg);
    /**
    @brief 通知Task等待回复消息的一条连接关闭。
    @param [in] uiSvrId 关闭连接的SVR-ID
    @param [in] uiHostId 关闭连接的HOST-ID
    @param [in] uiConnId 关闭连接的CONN-ID
    @param [in] pThrEnv 调用线程的Thread-env
    @return void
    */
    virtual void noticeConnClosed(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_UINT32 uiConnId,
        CwxTss* pThrEnv);
    /**
    @brief 激活Task。在Task启动前，Task有Task的创建线程所拥有。
    在启动前，Task可以接受自己的异步消息，但不能处理。
    此时有Taskboard的noticeActiveTask()接口调用的。
    @param [in] pThrEnv 调用线程的Thread-env
    @return 0：成功；-1：失败
    */
    virtual int noticeActive(CwxTss* pThrEnv);
    /**
    @brief 执行Task。在调用此API前，Task在Taskboard中不存在，也就是说对别的线程不可见。
    Task要么是刚创建状态，要么是完成了前一个阶段的处理，处于完成状态。
    通过此接口，由Task自己控制自己的step的跳转而外界无需关系Task的类型及处理过程。
    @param [in] pTaskBoard 管理Task的Taskboard
    @param [in] pThrEnv 调用线程的Thread-env
    @return void
    */
    virtual void execute(CwxTss* pThrEnv);
public:
    ///<回复转发消息
    static void reply(UnistorApp* pApp, ///<app对象
        UnistorTss* tss, ///<线程tss
        CWX_UINT32 uiRecvConnId, ///<收到的连接id
        CWX_UINT32 uiRecvThreadIndex, ///<收到的线程索引
        CwxMsgHead const& recvHead,  ///<收到的消息头
        CwxMsgBlock* replyMsg, ///<转发消息的回复，若为null，表示错误，uiErrCode及szErrMsg有效
        CWX_UINT32 uiErrCode=UNISTOR_ERR_SUCCESS, ///<转发失败的错误代码
        char const* szErrMsg = NULL ///<转发失败的错误消息
        );
public:
    CwxMsgBlock*        m_tranMsg; ///转发的消息msg
private:
    CWX_UINT16          m_uiRecvThreadIndex; ///<接收消息的线程池
    CWX_UINT32          m_uiRecvConnId; ///<接收消息的连接id
    CwxMsgHead          m_recvMsgHead; ///<接收到消息的消息头
    UnistorApp*   m_pApp; ///<app对象
    CwxMsgBlock*        m_msg; ///<收到的消息message
    string              m_strErrMsg; ///<错误消息
    CWX_UINT32          m_uiErrCode; ///<错误代码
};



#endif

