#ifndef __COUNTER_EVENT_HANDLER_H__
#define __COUNTER_EVENT_HANDLER_H__

#include "CwxCommander.h"
#include "UnistorTss.h"

CWINUX_USING_NAMESPACE

class CounterBenchApp;
///counter����Ĵ���handle��Ϊcommand��handle
class CounterEventHandler : public CwxCmdOp 
{
public:
    enum{
        COUNTER_BUF_SIZE=1024 * 1024
    };

    ///���캯��
    CounterEventHandler(CounterBenchApp* pApp);
    ///��������
    virtual ~CounterEventHandler(){
    }
public:
	///���ӽ���
	virtual int onConnCreated(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    ///�յ�echo����Ĵ�����
    virtual int onRecvMsg(CwxMsgBlock*& msg,///<echo���ݰ�����ص�����������Ϣ
                            CwxTss* pThrEnv///<�����̵߳�thread-specific-store
                            );
private:
	//����echo����
	void sendNextMsg(UnistorTss* tss,
        CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_UINT32 uiConnId);
	//��ȡkey
	void getKey(CWX_UINT32 uiKey, char* szKey);
private:
    CounterBenchApp*  m_pApp;  ///<app����
    char             m_szBuf[COUNTER_BUF_SIZE]; ///<���͵�����buf������
    CWX_UINT32       m_uiBufLen; ///<buf�����ݵĳ���
	CWX_UINT32      m_uiSendNum;///<�������������
	CWX_UINT32      m_uiRecvNum;///<���յ��ظ�������
	CWX_UINT32		m_uiSuccessNum; ///<�ɹ�������
};

#endif 
