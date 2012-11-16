#ifndef __UNISTOR_HANDLER_4_RECV_WRITE_H__
#define __UNISTOR_HANDLER_4_RECV_WRITE_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"
#include "UnistorPoco.h"

///ǰ����������
class UnistorApp;

///KV��Ϣ����handle
class UnistorHandler4RecvWrite : public CwxCmdOp{
public:
	///���캯��
	UnistorHandler4RecvWrite(UnistorApp* pApp):m_pApp(pApp){
        m_bCanWrite = false;
	}
	///��������
    virtual ~UnistorHandler4RecvWrite(){}
public:
    /**
    @brief �յ�ͨ�����ݰ��¼��Ĵ�������
    @param [in] msg �յ�ͨ�����ݰ����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv);

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
public:
    ///�Ƿ�master��д��Ҳ���Ƿ���master
    inline bool isCanWrite() const{
        return m_bCanWrite;
    }

private:
	///���һ��key������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
	int addKey(UnistorTss* pTss, ///<�߳�tss
        UnistorWriteMsgArg* pWriteArg, ///<add�Ĳ���
        CWX_UINT32& uiVersion, ///<����key���°汾
        CWX_UINT32& uiFieldNum ///<����key��field����
        );

	///setһ��key������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
    int setKey(UnistorTss* pTss, ///<�߳�tss
        UnistorWriteMsgArg* pWriteArg, ///<add�Ĳ���
        CWX_UINT32& uiVersion, ///<����key���°汾
        CWX_UINT32& uiFieldNum ///<����key��field����
        );

	///updateһ��key������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
    int updateKey(UnistorTss* pTss, ///<�߳�tss
        UnistorWriteMsgArg* pWriteArg, ///<add�Ĳ���
        CWX_UINT32& uiVersion, ///<����key���°汾
        CWX_UINT32& uiFieldNum ///<����key��field����
        );

	///incһ��key�ļ�����������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
    int incKey(UnistorTss* pTss, ///<�߳�tss
        UnistorWriteMsgArg* pWriteArg, ///<add�Ĳ���
        CWX_INT64& llValue, ///<���ؼ���������ֵ
        CWX_UINT32& uiVersion ///<����key��field����
        );

	///deleteһ��key������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
    int delKey(UnistorTss* pTss, ///<�߳�tss
        UnistorWriteMsgArg* pWriteArg, ///<add�Ĳ���
        CWX_UINT32& uiVersion, ///<����key���°汾
        CWX_UINT32& uiFieldNum ///<����key��field����
        );

    ///importһ��key������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
    int importKey(UnistorTss* pTss, ///<�߳�tss
        UnistorWriteMsgArg* pWriteArg, ///<add�Ĳ���
        CWX_UINT32& uiVersion, ///<����key���°汾
        CWX_UINT32& uiFieldNum ///<����key��field����
        );

    ///master�仯������
    void configChange(UnistorTss* pTss);
private:
	UnistorApp*               m_pApp;  ///<app����
    volatile bool             m_bCanWrite; ///<�Ƿ�master idc������master
};
#endif 
