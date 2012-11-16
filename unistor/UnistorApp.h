#ifndef __UNISTOR_APP_H__
#define __UNISTOR_APP_H__

#include "UnistorMacro.h"
#include "CwxAppFramework.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "UnistorHandler4Recv.h"
#include "UnistorStore.h"
#include "CwxThreadPool.h"
#include "CwxAppChannel.h"
#include "UnistorHandler4Dispatch.h"
#include "UnistorHandler4Master.h"
#include "UnistorHandler4RecvWrite.h"
#include "UnistorHandler4Checkpoint.h"
#include "UnistorDef.h"
#include "CwxBinLogMgr.h"
#include "UnistorHandler4Zk.h"
#include "UnistorHandler4Trans.h"

///应用信息定义
#define UNISTOR_VERSION "1.0.8"
#define UNISTOR_MODIFY_DATE "20120604080000"

///unistor服务的app对象
class UnistorApp : public CwxAppFramework{
public:
    enum{
		MAX_MONITOR_REPLY_SIZE = 1024 * 1024, ///<监控的buf的大小
        LOG_FILE_SIZE = 30, ///<每个可循环使用日志文件的MByte
        LOG_FILE_NUM = 7, ///<可循环使用日志文件的数量
        SVR_TYPE_RECV = CwxAppFramework::SVR_TYPE_USER_START, ///<数据接受svr-id类型
		SVR_TYPE_RECV_WRITE = CwxAppFramework::SVR_TYPE_USER_START + 1, ///<数据写的svr-id
		SVR_TYPE_MONITOR = CwxAppFramework::SVR_TYPE_USER_START + 2, ///<网络监听
		SVR_TYPE_INNER_SYNC = CwxAppFramework::SVR_TYPE_USER_START + 3, ///<内部异步分发监听
        SVR_TYPE_OUTER_SYNC = CwxAppFramework::SVR_TYPE_USER_START + 4, ///<外部异步分发监听
		SVR_TYPE_MASTER = CwxAppFramework::SVR_TYPE_USER_START + 5, ///<Master数据分发
		SVR_TYPE_CHECKPOINT = CwxAppFramework::SVR_TYPE_USER_START + 6, ///<checkpoint线程
        SVR_TYPE_ZK = CwxAppFramework::SVR_TYPE_USER_START + 7, ///<zookeeper的消息类型
        SVR_TYPE_TRANSFER=CwxAppFramework::SVR_TYPE_USER_START + 8, ///<slave的写及查询转发

		THREAD_GROUP_INNER_SYNC = CwxAppFramework::THREAD_GROUP_USER_START + 1, ///<内部数据同步线程池的id
        THREAD_GROUP_OUTER_SYNC = CwxAppFramework::THREAD_GROUP_USER_START + 2, ///<外部数据同步线程池的id
		THREAD_GROUP_WRITE = CwxAppFramework::THREAD_GROUP_USER_START + 3, ///数据更新的线程池的id
		THREAD_GROUP_CHECKPOINT = CwxAppFramework::THREAD_GROUP_USER_START + 4, ///<checkpoint的线程group 
        THREAD_GROUP_ZK = CwxAppFramework::THREAD_GROUP_USER_START + 5, ///<zookeeper的线程池
        THREAD_GROUP_TRANSFER = CwxAppFramework::THREAD_GROUP_USER_START + 6, ///<消息转发的线程池
		THREAD_GROUP_RECV_BASE = CwxAppFramework::THREAD_GROUP_USER_START + 7 ///<数据接收线程组的起始svr id
    };
    ///构造函数
	UnistorApp();
    ///析构函数
	virtual ~UnistorApp();
    ///重载初始化函数。返回值：0：成功；-1:失败。
    virtual int init(int argc, ///<main的argc
        char** argv ///<main的argv
        );
public:
    /**
    @brief 时钟通知，只要设置了时钟间隔，则会定时调用此API。
    @param [in] current 当前的时间。
    @return void
    */
    virtual void onTime(CwxTimeValue const& current);
    /**
    @brief 信号通知，若收到了一个没有屏蔽的信号，则会定时调用此API。
    @param [in] signum 收到的信号。
    @return void
    */
    virtual void onSignal(int signum);
    /**
    @brief 通知建立建立一个CWX_APP_EVENT_MODE连接。建立的连接是非阻塞模式。<br>
    @param [in] uiSvrId 连接的svr id。
    @param [in] uiHostId 连接的host id。
    @param [in] handle 连接的handle。
    @param [out] bSuspendListen 对于被动连接，若为true,则停止继续监听，否则继续监听
    @return <0：关闭连接； >=0连接有效。
    */
    virtual int onConnCreated(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_HANDLE handle,
        bool& bSuspendListen
        );

    /**
    @brief 通知建立CWX_APP_MSG_MODE一个连接。建立的连接全部是非阻塞模式。<br>
    @param [in] conn 连接兑现，只能在此API中时候，上层不能缓存。
    @param [out] bSuspendConn 若为true,则暂停消息接收；false，接收连接上的消息
    @param [out] bSuspendListen 对于被动连接，若为true,则停止继续监听，否则继续监听
    @return <0：关闭连接； >=0连接有效。
    */
	virtual int onConnCreated(CwxAppHandler4Msg& conn,
		bool& bSuspendConn,
		bool& bSuspendListen);
    /**
    @brief 通知CWX_APP_MSG_MODE模式的连接关闭。
    @param [in] conn 关闭的连接。
    @return 对于主动连接，-1：表示不连接，0：默认方式， >0：下次重连的时间间隔，单位为ms；被动连接忽略。
    */
    virtual int onConnClosed(CwxAppHandler4Msg& conn);
    /**
    @brief 通知从CWX_APP_MSG_MODE模式的、非raw类型连接收到一个消息
    @param [in] msg 收到的消息，空表示没有消息体。
    @param [in] conn 收到消息的连接。
    @param [in] header 收到消息的消息头。
    @param [out] bSuspendConn 若为true,则暂停消息接收；false，接收连接上的消息。
    @return -1：消息无效，关闭连接。 0：不连续接受消息； >0：连续从此连接上接受消息。
    */
    virtual int onRecvMsg(CwxMsgBlock* msg,
		CwxAppHandler4Msg& conn,
		CwxMsgHead const& header,
		bool& bSuspendConn);
    /**
    @brief 通知CWX_APP_MSG_MODE模式的、raw类型连接有数据到达，数据需要用户自己读取
    @param [in] conn 有消息的连接。
    @param [out] bSuspendConn 若为true,则暂停消息接收；false，接收连接上的消息。
    @return -1：消息无效，关闭连接。 0：成功。
    */
    virtual int onRecvMsg(CwxAppHandler4Msg& conn,
		bool& bSuspendConn);
public:
    ///获取配置信息对象
    inline UnistorConfig const& getConfig() const{
        return m_config;
    }

    ///获取Store driver对象指针
    inline UnistorStore* getStore(){
        return m_store;
    }

    ///获取master handler
    inline UnistorHandler4Master* getMasterHandler(){
        return m_masterHandler;
    }

    ///获取write handler
    inline UnistorHandler4RecvWrite* getRecvWriteHandler(){
        return m_recvWriteHandler;
    }

    ///获取内部同步的线程池
    inline CwxThreadPool* getInnerSyncThreadPool(){
        return m_innerSyncThreadPool;
    }

    ///获取内部同步的channel
    inline CwxAppChannel* getInnerSyncChannel(){
        return m_innerSyncChannel;
    }

    ///获取外部同步的线程池
    inline CwxThreadPool* getOuterSyncThreadPool(){
        return m_outerSyncThreadPool;
    }

    ///获取外部同步的channel
    inline CwxAppChannel* getOuterSyncChannel(){
        return m_outerSyncChannel;
    }
    
    ///获取write的线程池
    inline 	CwxThreadPool* getWriteTheadPool(){
        return m_writeThreadPool;
    }

    ///获取recv 通信channel数组
	inline  CwxAppChannel** getRecvChannels(){
        return m_recvChannel;
    }

    ///获取recv线程池的数组
	inline  CwxThreadPool** getRecvThreadPools(){
        return m_recvThreadPool;
    }

    ///获取转发通信channel
    inline CwxAppChannel* getTransChannel(){
        return m_transChannel;
    }

    ///获取转发线程池
    inline CwxThreadPool* getTransThreadPool() {
        return m_transThreadPool;
    }

    ///获取zookeeper的线程池
    inline CwxThreadPool*  getZkThreadPool(){
        return m_zkThreadPool;
    }

    ///获取zookeeper的消息处理handler
    inline UnistorHandler4Zk* getZkHandler() const{
        return m_zkHandler;
    }

    ///获取recv的连接属性
    inline UnistorConnAttr* getRecvSockAttr(){
        return &m_recvCliSockAttr;
    }

    ///获取分发接收的连接属性
    inline UnistorConnAttr* getSyncCliSockAttr(){
        return &m_syncCliSockAttr;
    }

    ///设置kv变更recv连接的属性
    static int setConnSockAttr(CWX_HANDLE handle, void* arg);

    ///计算机的时钟是否回调
    static bool isClockBack(CWX_UINT32& uiLastTime, CWX_UINT32 uiNow){
        if (uiLastTime > uiNow + 1){
            uiLastTime = uiNow;
            return true;
        }
        uiLastTime = uiNow;
        return false;
    }

protected:
    ///重载运行环境设置API
    virtual int initRunEnv();
    ///释放资源
    virtual void destroy();
private:
	///stats命令，-1：因为错误关闭连接；0：不关闭连接
	int monitorStats(char const* buf, ///<收到的数据
        CWX_UINT32 uiDataLen, ///<数据长度
        CwxAppHandler4Msg& conn ///<收到数据的连接
        );

	///形成监控内容，返回监控内容的长度
	CWX_UINT32 packMonitorInfo();
    
    ///分发channel的队列消息函数。返回值：0：正常；-1：队列停止
    static int dealRecvThreadQueue(UnistorTss* tss, ///<线程tss
        CwxMsgQueue* queue, ///<recv线程池的queue
        CWX_UINT32 uiQueueIndex, ///<recv线程的序号
        UnistorApp* app, ///<app对象
        CwxAppChannel* channel ///<线程对应的通信channel
        );

    ///receive channel的线程函数，arg为pair<app,int>对象
    static void* recvThreadMain(CwxTss* tss, ///<线程的的tss
        CwxMsgQueue* queue, ///<receive线程的队列
        void* arg ///<此为pair，第一个参数为app，第二个参数为receive线程的序号
        );
    
    ///内部同步channel的队列消息函数。返回值：0：正常；-1：队列停止
    static int dealInnerSyncThreadQueue(UnistorTss* tss, ///<线程tss
        CwxMsgQueue* queue, ///<inner binlog线程池的queue
        UnistorApp* app, ///<app对象
        CwxAppChannel* channel ///<线程的通信channel
        );

    ///部分发channel的线程函数，arg为app对象
	static void* innerSyncThreadMain(CwxTss* tss, ///<线程tss
        CwxMsgQueue* queue, ///<线程池的queue
        void* arg ///<app参数
        );
    
    ///外部同步channel的队列消息函数。返回值：0：正常；-1：队列停止
    static int dealOuterSyncThreadQueue(UnistorTss* tss,  ///<线程tss
        CwxMsgQueue* queue, ///<outer binlog线程池的queue
        UnistorApp* app, ///<app对象
        CwxAppChannel* channel ///<线程的通信channel
        );

    ///外部分发channel的线程函数，arg为app对象
    static void* outerSyncThreadMain(CwxTss* tss, ///<线程tss
        CwxMsgQueue* queue, ///<线程池的queue
        void* arg ///<app参数
        );

    ///外部同步channel的队列消息函数。返回值：0：正常；-1：队列停止
    static int dealTransThreadQueue(UnistorTss* tss, ///<线程tss
        CwxMsgQueue* queue,///<线程池的queue
        UnistorApp* app, ///<app参数
        CwxAppChannel* channel ///<线程的通信channel
        );

    ///外部分发channel的线程函数，arg为app对象
    static void* transThreadMain(CwxTss* tss, ///<线程tss
        CwxMsgQueue* queue, ///<线程池的queue
        void* arg ///<app参数
        );

    ///外部分发channel的线程函数，arg为app对象
    static void* zkThreadMain(CwxTss* tss, ///<线程tss
        CwxMsgQueue* queue, ///<线程池的queue
        void* arg ///<app参数
        );

    ///存储驱动的消息通道。0：成功；-1：失败
    static int storeMsgPipe(void* app, ///<app参数
        CwxMsgBlock* msg, ///<来自存储引擎的消息
        bool bWriteThread, ///<是否送给write线程，否则给checkpoint线程
        char* szErr2K ///<出错时的错误消息
        ); 

    //获取系统key。1：成功；0：不存在；-1：失败;
    static int getSysKey(void* pApp, ///<app对象
        char const* key, ///<要获取的key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<若存在，则返回数据。内存有存储引擎分配
        CWX_UINT32& uiLen  ///<szData数据的字节数
        );


    ///开启网络。0：成功；-1：失败
	int startNetwork();
private:
    UnistorConfig                 m_config;            ///<配置文件
    UnistorStore*                 m_store;             ///<unistor存储driver对象
    CwxAppChannel**               m_recvChannel;       ///<unistor查询的channel数组
    CwxThreadPool**               m_recvThreadPool;    ///<unistor查询的线程池对象
    pair<UnistorApp*, CWX_UINT32>* m_recvArgs;         ///<线程启动参数数组
    CwxThreadPool*                m_writeThreadPool;    ///<对于slave用于同步master数据，对于master用于串行写数据
	UnistorHandler4Master*         m_masterHandler;    ///<从master接收消息的handle
	UnistorHandler4RecvWrite*      m_recvWriteHandler; ///<数据修改的handle
	CwxThreadPool*                m_innerSyncThreadPool; ///<内部消息分发的线程池对象
	CwxAppChannel*                m_innerSyncChannel;     ///<内部消息分发的channel
    CwxThreadPool*                m_outerSyncThreadPool; ///<外部部消息分发的线程池对象
    CwxAppChannel*                m_outerSyncChannel;     ///<外部消息分发的channel
	UnistorHandler4Checkpoint*     m_checkpointHandler; ///<checkpoint的handle
	CwxThreadPool*                m_checkpointThreadPool;///<checkpoint的线程池对象
    CwxThreadPool*                m_transThreadPool;///<消息转发的线程池对象
    CwxAppChannel*                m_transChannel;     ///<消息转发的channel
    CwxThreadPool*                m_zkThreadPool;///<zk的线程池对象
    UnistorHandler4Zk*             m_zkHandler; ///<zk事件处理的handler
    UnistorConnAttr               m_recvSvrSockAttr; ///<数据接收svr端的socket属性
    UnistorConnAttr               m_recvCliSockAttr; ///<数据接收client端的socket属性
    UnistorConnAttr               m_syncSvrSockAttr; ///<同步svr端的socket属性
    UnistorConnAttr               m_syncCliSockAttr; ///<同步cli端的socket属性
	char                        m_szBuf[MAX_MONITOR_REPLY_SIZE];///<监控消息的回复buf
	string						m_strStartTime;       ///<unistor的启动时间戳
};
#endif

