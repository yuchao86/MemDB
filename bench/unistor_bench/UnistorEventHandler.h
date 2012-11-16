#ifndef __UNISTOR_EVENT_HANDLER_H__
#define __UNISTOR_EVENT_HANDLER_H__

#include "CwxCommander.h"
#include "UnistorTss.h"

CWINUX_USING_NAMESPACE

class UnistorBenchApp;
///echo请求的处理handle，为command的handle
class UnistorEventHandler : public CwxCmdOp 
{
public:
    ///构造函数
    UnistorEventHandler(UnistorBenchApp* pApp);
    ///析构函数
    virtual ~UnistorEventHandler(){
		if (m_szBuf) delete [] m_szBuf;
    }
public:
	///连接建立
	virtual int onConnCreated(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    ///收到echo请求的处理函数
    virtual int onRecvMsg(CwxMsgBlock*& msg,///<echo数据包及相关的请求连接信息
                            CwxTss* pThrEnv///<处理线程的thread-specific-store
                            );
private:
	//发送echo请求
	void sendNextMsg(UnistorTss* tss,
        CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_UINT32 uiConnId);
	//获取key
	void getKey(CWX_UINT32 uiKey, char* szKey, bool bMd5);
private:
    UnistorBenchApp*  m_pApp;  ///<app对象
	char*           m_szBuf; ///<发送的数据buf及内容
	CWX_UINT32      m_uiSendNum;///<发送请求的数量
	CWX_UINT32      m_uiRecvNum;///<接收到回复的数量
	CWX_UINT32		m_uiSuccessNum; ///<成功的数量
};

#endif 
