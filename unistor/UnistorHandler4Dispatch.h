#ifndef __UNISTOR_HANDLER_4_DISPATCH_H__
#define __UNISTOR_HANDLER_4_DISPATCH_H__


#include "CwxCommander.h"
#include "CwxAppAioWindow.h"
#include "UnistorMacro.h"
#include "UnistorTss.h"
#include "CwxAppHandler4Channel.h"
#include "CwxAppChannel.h"
#include "UnistorDef.h"
#include "CwxBinLogMgr.h"
#include "UnistorPoco.h"
#include "UnistorSubscribe.h"
#include "UnistorStore.h"

///前置声明对象
class UnistorApp;
class UnistorHandler4Dispatch;

///分发连接的sync session信息对象
class UnistorDispatchSyncSession{
public:
    ///构造函数
    UnistorDispatchSyncSession(){
        m_ullSeq = 0;
        m_ullSessionId = 0;
        m_bClosed = false;
        m_pCursor = NULL;
        m_uiChunk = 0;
        m_ullStartSid = 0;
        m_ullSid = 0;
        m_bNext = false;
        m_bZip = false;
    }
    ~UnistorDispatchSyncSession(){
    }
public:
    void addConn(UnistorHandler4Dispatch* conn);
    ///重新形成session id，返回session id
    CWX_UINT64 reformSessionId(){
        CwxTimeValue timer;
        timer.now();
        m_ullSessionId = timer.to_usec();
        return m_ullSessionId;
    }

public:
    CWX_UINT64        m_ullSessionId; ///<session id
    CWX_UINT64        m_ullSeq; ///<当前的序列号，从0开始。
    bool                     m_bClosed; ///<是否需要关闭
    map<CWX_UINT32, UnistorHandler4Dispatch*> m_conns; ///<建立的连接
    CwxBinLogCursor*         m_pCursor; ///<binlog的读取cursor
    CWX_UINT32               m_uiChunk; ///<chunk大小
    CWX_UINT64               m_ullStartSid; ///<report的sid
    CWX_UINT64               m_ullSid; ///<当前发送到的sid
    bool                     m_bNext; ///<是否发送下一个消息
    UnistorSubscribe         m_subscribe; ///<消息订阅对象
    string                   m_strSign; ///<签名类型
    bool                     m_bZip; ///<是否压缩
    string                   m_strHost; ///<session的来源主机
};

///export的sesion对象
class UnistorDispatchExportSession{
public:

    UnistorDispatchExportSession(){
        m_ullSeq = 0;
        m_pCursor = NULL;
        m_uiChunk = 0;
    }

    ~UnistorDispatchExportSession(){
    }
public:
    CWX_UINT64                  m_ullSeq; ///<当前的序列号，从0开始。
    UnistorStoreCursor*         m_pCursor; ///<数据同步的cursor对象
    CWX_UINT32                  m_uiChunk; ///<chunk大小
    UnistorSubscribe            m_subscribe; ///<消息订阅对象
    string                      m_strHost; ///<session的来源主机
};

///tss的user object对象
class UnistorDispatchThreadUserObj:public UnistorTssUserObj{
public:
    UnistorDispatchThreadUserObj(){}
    ~UnistorDispatchThreadUserObj(){}
public:
    ///释放资源
    void free(UnistorApp* pApp);
public:
    map<CWX_UINT64, UnistorDispatchSyncSession* > m_sessionMap;  ///<session的map，key为session id
    list<UnistorDispatchSyncSession*>             m_freeSession; ///<需要关闭的session
};


///异步binlog分发的消息处理handler
class UnistorHandler4Dispatch : public CwxAppHandler4Channel{
public:
    enum{
        DISPATCH_TYPE_INIT = 0, ///<初始化状态
        DISPATCH_TYPE_SYNC = 1, ///<数据同步连接
        DISPATCH_TYPE_EXPORT = 2 ///<export的连接
    };
public:
    ///构造函数
    UnistorHandler4Dispatch(UnistorApp* pApp,
        CwxAppChannel* channel,
        CWX_UINT32 uiConnId,
        bool bInner=false);
    ///析构函数
    virtual ~UnistorHandler4Dispatch();
public:
    /**
    @brief 连接可读事件，返回-1，close()会被调用
    @return -1：处理失败，会调用close()； 0：处理成功
    */
    virtual int onInput();
    /**
    @brief 通知连接关闭。
    @return 1：不从engine中移除注册；0：从engine中移除注册但不删除handler；-1：从engine中将handle移除并删除。
    */
    virtual int onConnClosed();
    /**
    @brief Handler的redo事件，在每次dispatch时执行。
    @return -1：处理失败，会调用close()； 0：处理成功
    */
    virtual int onRedo();

public:
    ///发送binlog。返回值：0：未发送一条binlog；1：发送了一条binlog；-1：失败；
    int syncSendBinLog(UnistorTss* pTss);

    ///pack一条binlog。返回值：-1：失败，1：成功
    int syncPackOneBinLog(CwxPackageWriterEx* writer, ///<writer对象
        CwxMsgBlock*& block, ///<pack后形成的数据包
        CWX_UINT64 ullSeq, ///<消息序列号
        CWX_UINT32 uiVersion, ///<binlog的版本号
        CWX_UINT32 uiType, ///<binlog的变更消息类型
        CwxKeyValueItemEx const* pData, ///<变更的数据
        char* szErr2K ///<若失败返回错误消息
        );

    ///pack多条binlog。返回值：-1：失败，1：成功
    int syncPackMultiBinLog(CwxPackageWriterEx* writer, ///<writer对象
        CwxPackageWriterEx* writer_item, ///<writer对象
        CWX_UINT32 uiVersion, ///<binlog的版本号
        CWX_UINT32 uiType, ///<binlog的变更消息类型
        CwxKeyValueItemEx const* pData, ///<变更的数据
        CWX_UINT32&  uiLen, ///<返回pack完当前binlog后，整个数据包的大小
        char* szErr2K ///<若失败返回错误消息
        );

    ///定位到需要的binlog处。返回值：1：发现记录；0：没有发现；-1：错误
    int syncSeekToBinlog(UnistorTss* tss, ///<线程tss
        CWX_UINT32& uiSkipNum ///<最多可以遍历的binlog数量，返回剩余值
        );

    ///将binlog定位到report的sid。返回值：1：成功；0：太大；-1：错误
    int syncSeekToReportSid(UnistorTss* tss);

    ///发送export的数据。返回值：0：未发送一条数据；1：发送了一条数据；-1：失败；
    int exportSendData(UnistorTss* pTss);
    
    ///获取连接id
    inline CWX_UINT32 getConnId() const{
        return m_uiConnId;
    }

public:
    ///分发线程的事件调度处理函数
    static void doEvent(UnistorApp* app, ///<app对象
        UnistorTss* tss, ///<线程tss
        CwxMsgBlock*& msg ///<事件消息
        );

    ///处理关闭的session
    static void dealClosedSession(UnistorApp* app, ///<app对象
        UnistorTss* tss  ///<线程tss
        );
private:
    ///收到一个消息并处理。返回值：0：成功；-1：失败
    int recvMessage();

    ///收到sync report的消息。返回值：0：成功；-1：失败
    int recvSyncReport(UnistorTss* pTss);
    
    ///收到sync new conn的report消息。返回值：0：成功；-1：失败
    int recvSyncNewConnection(UnistorTss* pTss);
    
    ///收到binlog sync的reply消息。返回值：0：成功；-1：失败
    int recvSyncReply(UnistorTss* pTss);

    ///收到chunk模式下的binlog sync reply消息。返回值：0：成功；-1：失败
    int recvSyncChunkReply(UnistorTss* pTss);

    ///收到export的report消息。返回值：0：成功；-1：失败
    int recvExportReport(UnistorTss* pTss);

    ///收到export数据的reply消息。返回值：0：成功；-1：失败
    int recvExportReply(UnistorTss* pTss);
private:
    bool                     m_bInner; ///<是否内部分发
    bool                     m_bReport; ///<是否已经报告
    CWX_UINT8                m_ucDispatchType; ///<分发类型
    UnistorDispatchSyncSession*  m_syncSession; ///<连接对应的session
    UnistorDispatchExportSession* m_exportSession; ///<数据export的session
    CWX_UINT64               m_ullSessionId; ///<session的id
    CWX_UINT64               m_ullSentSeq; ///<发送的序列号
    CWX_UINT32               m_uiConnId; ///<连接id
    UnistorApp*              m_pApp;  ///<app对象
    CwxMsgHead               m_header; ///<消息头
    char                     m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN+1]; ///<消息头的buf
    CWX_UINT32               m_uiRecvHeadLen; ///<recieved msg header's byte number.
    CWX_UINT32               m_uiRecvDataLen; ///<recieved data's byte number.
    CwxMsgBlock*             m_recvMsgData; ///<the recieved msg data
    string                   m_strPeerHost; ///<对端host
    CWX_UINT16               m_unPeerPort; ///<对端port
    UnistorTss*              m_tss;        ///<对象对应的tss对象


};

#endif 
