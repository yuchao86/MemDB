#ifndef __UNISTOR_EVENT_HANDLER_H__
#define __UNISTOR_EVENT_HANDLER_H__

#include "CwxCommander.h"
#include "UnistorTss.h"

CWINUX_USING_NAMESPACE

class UnistorBenchApp;
///echo����Ĵ���handle��Ϊcommand��handle
class UnistorEventHandler : public CwxCmdOp 
{
public:
    ///���캯��
    UnistorEventHandler(UnistorBenchApp* pApp);
    ///��������
    virtual ~UnistorEventHandler(){
		if (m_szBuf) delete [] m_szBuf;
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
	void getKey(CWX_UINT32 uiKey, char* szKey, bool bMd5);
private:
    UnistorBenchApp*  m_pApp;  ///<app����
	char*           m_szBuf; ///<���͵�����buf������
	CWX_UINT32      m_uiSendNum;///<�������������
	CWX_UINT32      m_uiRecvNum;///<���յ��ظ�������
	CWX_UINT32		m_uiSuccessNum; ///<�ɹ�������
};

#endif 
