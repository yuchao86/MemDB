#ifndef __COUNTER_BENCH_APP_H__
#define __COUNTER_BENCH_APP_H__

#include "CwxAppFramework.h"
#include "CwxAppHandler4Msg.h"
#include "CounterBenchConfig.h"
#include "UnistorTss.h"
#include "UnistorPoco.h"
#include "CounterEventHandler.h"


CWINUX_USING_NAMESPACE;

///unistor counter的压力测试app
class CounterBenchApp : public CwxAppFramework{
public:
    enum{
        LOG_FILE_SIZE = 30, ///<每个循环运行日志文件的MBTYE
        LOG_FILE_NUM = 7,///<循环日志文件的数量
        SVR_TYPE_RECV = CwxAppFramework::SVR_TYPE_USER_START ///<查询的svr-id类型
    };
    ///构造函数
	CounterBenchApp();
    ///析构函数
	virtual ~CounterBenchApp();
    //初始化app, -1:failure, 0 success;
    virtual int init(int argc, char** argv);
public:
    //信号响应函数
    virtual void onSignal(int signum);
    //echo连接建立函数
    virtual int onConnCreated(CwxAppHandler4Msg& conn,
        bool& bSuspendConn,
        bool& bSuspendListen);
    //echo返回的响应函数
    virtual int onRecvMsg(CwxMsgBlock* msg,
        CwxAppHandler4Msg& conn,
        CwxMsgHead const& header,
        bool& bSuspendConn);
public:
	CounterBenchConfig const& getConfig() const{
        return  m_config;
    }
protected:
    //init the Enviroment before run.0:success, -1:failure.
	virtual int initRunEnv();
	virtual void destroy();
private:
    ///设置连接的属性
    static int setSockAttr(CWX_HANDLE handle, void* arg);
private:
    CounterBenchConfig               m_config; ///<配置文件对象
	CwxThreadPool*                 m_threadPool;///<线程池对象
	CounterEventHandler*			   m_eventHandler; ///<消息处理的handler 
};

#endif

