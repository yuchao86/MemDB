#ifndef __UNISTOR_BENCH_APP_H__
#define __UNISTOR_BENCH_APP_H__

#include "CwxAppFramework.h"
#include "CwxAppHandler4Msg.h"
#include "UnistorBenchConfig.h"
#include "UnistorTss.h"
#include "UnistorPoco.h"
#include "UnistorEventHandler.h"


CWINUX_USING_NAMESPACE;

///unistor��ѹ������app
class UnistorBenchApp : public CwxAppFramework{
public:
    enum{
        LOG_FILE_SIZE = 30, ///<ÿ��ѭ��������־�ļ���MBTYE
        LOG_FILE_NUM = 7,///<ѭ����־�ļ�������
        SVR_TYPE_RECV = CwxAppFramework::SVR_TYPE_USER_START ///<��ѯ��svr-id����
    };
    ///���캯��
	UnistorBenchApp();
    ///��������
	virtual ~UnistorBenchApp();
    //��ʼ��app, -1:failure, 0 success;
    virtual int init(int argc, char** argv);
public:
    //�ź���Ӧ����
    virtual void onSignal(int signum);
    //echo���ӽ�������
    virtual int onConnCreated(CwxAppHandler4Msg& conn,
        bool& bSuspendConn,
        bool& bSuspendListen);
    //echo���ص���Ӧ����
    virtual int onRecvMsg(CwxMsgBlock* msg,
        CwxAppHandler4Msg& conn,
        CwxMsgHead const& header,
        bool& bSuspendConn);
public:
	UnistorBenchConfig const& getConfig() const{
        return  m_config;
    }
protected:
    //init the Enviroment before run.0:success, -1:failure.
	virtual int initRunEnv();
	virtual void destroy();
private:
    ///�������ӵ�����
    static int setSockAttr(CWX_HANDLE handle, void* arg);
private:
    UnistorBenchConfig               m_config; ///<�����ļ�����
	CwxThreadPool*                 m_threadPool;///<�̳߳ض���
	UnistorEventHandler*			   m_eventHandler; ///<��Ϣ�����handler 
};

#endif

