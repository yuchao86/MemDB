#ifndef __UNISTOR_HANDLER_4_CHECKPOINT_H__
#define __UNISTOR_HANDLER_4_CHECKPOINT_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"

///����app����
class UnistorApp;

///checkpoint�Ĵ���handle
class UnistorHandler4Checkpoint : public CwxCmdOp{
public:
    ///���캯��
    UnistorHandler4Checkpoint(UnistorApp* pApp):m_pApp(pApp){
		m_bCheckOuting = false;
		m_uiLastCheckTime = time(NULL);
    }
    ///��������
    virtual ~UnistorHandler4Checkpoint(){
    }
public:
    /**
    @brief ��ʱ����¼��Ĵ�������
    @param [in] msg ��ʱ�����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onTimeoutCheck(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief �û��Զ����¼��Ĵ�������
    @param [in] msg �û��Զ����¼����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onUserEvent(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    ///����Ƿ���Ҫcheckpoint
	bool isNeedCheckOut(time_t now) const;
private:
	bool				      m_bCheckOuting; ///<�Ƿ�����checkpoint
	CWX_UINT32				  m_uiLastCheckTime; ///<�ϴ�checkpoint��ʱ��
    UnistorApp*               m_pApp;  ///<app����
};

#endif 
