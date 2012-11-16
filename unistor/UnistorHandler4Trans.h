#ifndef __UNISTOR_HANDLER_4_TRANS_H__
#define __UNISTOR_HANDLER_4_TRANS_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "UnistorTss.h"
#include "CwxAppHandler4Channel.h"
#include "UnistorStoreBase.h"

///前置声明
class UnistorApp;

///消息转发的handle
class UnistorHandler4Trans : public CwxAppHandler4Channel{
public:
    ///构造函数
    UnistorHandler4Trans(UnistorApp* pApp, CWX_UINT32 uiConnid, CwxAppChannel *channel):
      CwxAppHandler4Channel(channel),m_uiConnId(uiConnid), m_pApp(pApp)
      {
          m_uiRecvHeadLen = 0;
          m_uiRecvDataLen = 0;
          m_recvMsgData = 0;
          m_bAuth = false;
          m_tss = NULL;
      }
      ///析构函数
      virtual ~UnistorHandler4Trans(){
          if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
      }
public:
    /**
    @brief 初始化建立的连接，并往Reactor注册连接
    @param [in] arg 建立连接的acceptor或为NULL
    @return -1：放弃建立的连接； 0：连接建立成功
    */
    virtual int open (void * arg= 0);
    /**
    @brief 通知连接关闭。
    @return 1：不从engine中移除注册；0：从engine中移除注册但不删除handler；-1：从engine中将handle移除并删除。
    */
    virtual int onConnClosed();
    /**
    @brief 连接可读事件，返回-1，close()会被调用
    @return -1：处理失败，会调用close()； 0：处理成功
    */
    virtual int onInput();
    /**
    @brief 通知连接完成一个消息的发送。<br>
    只有在Msg指定FINISH_NOTICE的时候才调用.
    @param [in,out] msg 传入发送完毕的消息，若返回NULL，则msg有上层释放，否则底层释放。
    @return 
    CwxMsgSendCtrl::UNDO_CONN：不修改连接的接收状态
    CwxMsgSendCtrl::RESUME_CONN：让连接从suspend状态变为数据接收状态。
    CwxMsgSendCtrl::SUSPEND_CONN：让连接从数据接收状态变为suspend状态
    */
    virtual CWX_UINT32 onEndSendMsg(CwxMsgBlock*& msg);

    /**
    @brief 通知连接上，一个消息发送失败。<br>
    只有在Msg指定FAIL_NOTICE的时候才调用.
    @param [in,out] msg 发送失败的消息，若返回NULL，则msg有上层释放，否则底层释放。
    @return void。
    */
    virtual void onFailSendMsg(CwxMsgBlock*& msg);

public:

    ///消息分发处理函数
    static void doEvent(UnistorApp* pApp, ///<app对象
        UnistorTss* tss, ///<线程tss
        CwxMsgBlock*& msg ///<分发的消息
        );

    ///往master转发消息。返回值：true：成功；false：失败。
    static bool transMsg(UnistorTss* tss, ///<线程tss
        CWX_UINT32 uiTaskId, ///<消息的taskid
        CwxMsgBlock* msg ///<转发的消息
        );

    ///初始化Handler环境信息。返回值：0：成功；-1：失败
    static int init(CWX_UINT32 uiConnNum/*连接的数量*/){
        m_bCanTrans = false;
        m_strMasterHost = "";
        m_uiMaxConnNum = uiConnNum;
        m_uiAuthConnNum = 0; ///<已经认证的host的数量
        m_authConn = new UnistorHandler4Trans*[uiConnNum]; ///<认证的连接
        m_handlers = new map<CWX_UINT32, UnistorHandler4Trans*>;
        m_bRebuildConn = false;
        m_ttLastRebuildConn = 0;
        return 0;
    }

    ///释放转发handle的环境信息。返回值：0：成功；-1：失败
    static int destroy(){
        m_bCanTrans = false;
        m_strMasterHost = "";
        m_uiMaxConnNum = 0;
        m_uiAuthConnNum = 0; ///<已经认证的host的数量
        if (m_authConn) delete [] m_authConn;
        m_authConn = NULL;
        if (m_handlers) delete m_handlers;
        m_handlers = NULL;
        m_bRebuildConn = false;
        m_ttLastRebuildConn = 0;
        return 0;
    }

    ///重建转发的连接。返回值：0：成功；-1：失败
    static int rebuildConn(UnistorApp* app);
    
    ///检测并建立转发连接
    static void checkTrans(UnistorApp* app, ///<app对象
        UnistorTss* tss ///<线程tss
        );
private:
    CWX_UINT32				m_uiConnId; ///<连接id
    UnistorApp*             m_pApp;  ///<app对象
    CwxMsgHead              m_header; ///<消息的消息头
    char                    m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN + 1];
    CWX_UINT32              m_uiRecvHeadLen; ///<recieved msg header's byte number.
    CWX_UINT32              m_uiRecvDataLen; ///<recieved data's byte number.
    CwxMsgBlock*            m_recvMsgData; ///<the recieved msg data
    bool                    m_bAuth;       ///<当前连接是否认证完成
    UnistorTss*              m_tss;        ///<对象对应的tss对象
public:
    static volatile bool                m_bCanTrans; ///<是否能够转发
private:
    static string                       m_strMasterHost; ///<当前的主机
    static CWX_UINT32                   m_uiMaxConnNum; ///<最大的连接数量
    static CWX_UINT32                   m_uiAuthConnNum; ///<已经认证的host的数量
    static UnistorHandler4Trans**       m_authConn; ///<认证的连接
    static map<CWX_UINT32, UnistorHandler4Trans*>* m_handlers; ///<所有连接的map
    static bool                         m_bRebuildConn; ///<是否需要重新建立连接
    static CWX_UINT32                   m_ttLastRebuildConn; ///<上次重建连接的时间

};
#endif 
