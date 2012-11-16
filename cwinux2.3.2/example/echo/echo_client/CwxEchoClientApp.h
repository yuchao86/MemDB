#ifndef __CWX_ECHO_CLIENT_APP_H__
#define __CWX_ECHO_CLIENT_APP_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
#include "CwxAppFramework.h"
#include "CwxEchoClientConfig.h"

#define ECHO_CLIENT_APP_VERSION "1.0"
#define ECHO_CLIENT_MODIFY_DATE "2010-08-29"

CWINUX_USING_NAMESPACE;

///echo��ѹ������app
class CwxEchoClientApp : public CwxAppFramework{
public:
    enum{
        LOG_FILE_SIZE = 30, ///<ÿ��ѭ��������־�ļ���MBTYE
        LOG_FILE_NUM = 7,///<ѭ����־�ļ�������
        SVR_TYPE_ECHO = CwxAppFramework::SVR_TYPE_USER_START ///<echo��ѯ��svr-id����
    };
    enum{
        SEND_MSG_TYPE = 1, ///<echo���͵���Ϣ����
        RECV_MSG_TYPE =2 ///<echo�ظ�����Ϣ����
    };

    ///���캯��
	CwxEchoClientApp();
    ///��������
	virtual ~CwxEchoClientApp();
    //��ʼ��app, -1:failure, 0 success;
    virtual int init(int argc, char** argv);
public:
    //ʱ����Ӧ����
    virtual void onTime(CwxTimeValue const& current);
    //�ź���Ӧ����
    virtual void onSignal(int signum);
    //echo���ӽ�������
    virtual int onConnCreated(CwxAppHandler4Msg& conn, bool& bSuspendConn, bool& bSuspendListen);
    //echo���ص���Ӧ����
    virtual int onRecvMsg(CwxMsgBlock* msg, CwxAppHandler4Msg& conn, CwxMsgHead const& header, bool& bSuspendConn);
    
protected:
    //init the Enviroment before run.0:success, -1:failure.
	virtual int initRunEnv();
private:
    ///����socket������
    static int setSockAttr(CWX_HANDLE handle, void* arg);
    //����echo����
    void sendNextMsg(CWX_UINT32 uiSvrId, CWX_UINT32 uiHostId, CWX_UINT32 uiConnId);
private:
    CwxEchoClientConfig               m_config; ///<�����ļ�����
    char                           m_szBuf100K[64 * 1024*1024+1]; ///<���͵�echo����buf������
    CWX_UINT32                     m_uiSendNum;///<����echo���������
    CWX_UINT32                     m_uiRecvNum;///<���յ�echo�ظ�������
};

#endif

