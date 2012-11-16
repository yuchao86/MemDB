#ifndef __UNISTOR_HANDLER_4_RECV_H__
#define __UNISTOR_HANDLER_4_RECV_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "UnistorTss.h"
#include "CwxAppHandler4Channel.h"
#include "UnistorStoreBase.h"

///前置声明对象
class UnistorApp;
class UnistorHandler4Recv;

///线程tss的用户对象
class UnistorRecvThreadUserObj:public UnistorTssUserObj{
public:
    UnistorRecvThreadUserObj(){
        m_uiConnNum = 0;
    }
    ~UnistorRecvThreadUserObj(){}
public:
    ///获取连接对象
    UnistorHandler4Recv* getConn(CWX_UINT32 uiConnId){
        hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*连接对象*/>::iterator iter = m_connMap.find(uiConnId);
        if (iter != m_connMap.end()) return iter->second;
        return NULL;
    }
    ///删除连接
    void removeConn(CWX_UINT32 uiConnId){
        hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*连接对象*/>::iterator iter = m_connMap.find(uiConnId);
        if (iter != m_connMap.end()){
            m_uiConnNum --;
            m_connMap.erase(iter);
        }
    }
    ///添加连接
    void addConn(CWX_UINT32 uiConnId, UnistorHandler4Recv* conn){
        hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*连接对象*/>::iterator iter = m_connMap.find(uiConnId);
        CWX_ASSERT(iter == m_connMap.end());
        m_connMap[uiConnId] = conn;
        m_uiConnNum++;
    }
    ///获取连接的数量
    CWX_UINT32 getConnNum() const {
        return m_uiConnNum;
    }
private:
    CWX_UINT32          m_uiConnNum;  ///<此为了防止锁
    hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*连接对象*/> m_connMap; ///线程所管理的连接map
};

///unistor消息处理handle
class UnistorHandler4Recv : public CwxAppHandler4Channel{
public:
    ///构造函数
    UnistorHandler4Recv(UnistorApp* pApp,
        CWX_UINT32 uiConnid,
        CWX_UINT32 uiPoolIndex,
        CwxAppChannel* channel):CwxAppHandler4Channel(channel),m_uiConnId(uiConnid), m_uiThreadPosIndex(uiPoolIndex), m_pApp(pApp)
    {
        m_uiRecvHeadLen = 0;
        m_uiRecvDataLen = 0;
        m_recvMsgData = 0;
        m_bAuth = false;
        m_tss = NULL;
    }
    ///析构函数
    virtual ~UnistorHandler4Recv(){
        if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
        m_recvMsgData = NULL;
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
public:
    ///消息的处理函数
    static void doEvent(UnistorApp* pApp, ///<app
        UnistorTss* tss, ///<线程tss
        CwxMsgBlock*& msg, ///<消息msg
        CWX_UINT32 uiPoolIndex ///<线程的序列号
        );

    ///形成add、set、update、delete操作的回复数据包。返回值：非空：成功；NULL：失败。
    static CwxMsgBlock* packReplyMsg(UnistorTss* tss, ///<线程tss
        CWX_UINT32 uiTaskId, ///<消息的task id
        CWX_UINT16 unMsgType, ///<消息类型
        int ret,  ///<返回值的ret代码
        CWX_UINT32 uiVersion, ///<数据的版本号
        CWX_UINT32 uiFieldNum, ///<key的field数量
        char const* szErrMsg  ///<数据操作的错误信息
        );

private:
	///收到一个数据增删改查消息。返回值：0：成功；-1：失败
	int recvMessage();

    ///认证。 返回值：true：成功；false：错误
    bool checkAuth(UnistorTss* pTss);

    ///Key是否存在的消息处理函数。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    int existKv(UnistorTss* pTss);

	///get 一个key的消息处理函数。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
	int getKv(UnistorTss* pTss);

    ///get 多个key的消息处理函数。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
	int getKvs(UnistorTss* pTss);

	///获取key列表的消息处理函数。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
	int getList(UnistorTss* pTss);

    ///回复收到的消息。返回值：0：成功；-1：失败
	int reply(CwxMsgBlock* msg, ///<回复的消息
        bool bCloseConn=false ///<是否消息发送完毕后关闭连接
        );

    ///将消息转发给write线程。返回值UNISTOR_ERR_SUCCESS表示成功，否则失败
    int relayWriteThread();

    ///将消息转发给transfer线程
    void relayTransThread(CwxMsgBlock* msg);
private:
	CWX_UINT32			   m_uiConnId; ///<连接id
	CWX_UINT32			   m_uiThreadPosIndex; ///<所属的thread pool索引
	UnistorApp*            m_pApp;  ///<app对象
	CwxMsgHead             m_header; ///<收到的消息包的包头
	char                   m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN + 1];
	CWX_UINT32             m_uiRecvHeadLen; ///<recieved msg header's byte number.
	CWX_UINT32             m_uiRecvDataLen; ///<recieved data's byte number.
	CwxMsgBlock*           m_recvMsgData; ///<the recieved msg data
    bool                   m_bAuth; ///<连接是否已经认证
    string                   m_strPeerHost; ///<对端host
    CWX_UINT16               m_unPeerPort; ///<对端port
    UnistorTss*              m_tss;        ///<对象对应的tss对象
};
#endif 
