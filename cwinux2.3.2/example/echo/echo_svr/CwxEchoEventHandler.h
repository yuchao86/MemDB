#ifndef __CWX_ECHO_EVENT_HANDLER_H__
#define __CWX_ECHO_EVENT_HANDLER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
#include "CwxCommander.h"

CWINUX_USING_NAMESPACE

class CwxEchoApp;
///echo����Ĵ���handle��Ϊcommand��handle
class CwxEchoEventHandler : public CwxCmdOp 
{
public:
    ///���캯��
    CwxEchoEventHandler(CwxEchoApp* pApp):m_pApp(pApp)
    {
        m_ullMsgNum = 0;
    }
    ///��������
    virtual ~CwxEchoEventHandler()
    {

    }
public:
    ///�յ�echo����Ĵ�����
    virtual int onRecvMsg(CwxMsgBlock*& msg,///<echo���ݰ�����ص�����������Ϣ
                            CwxTss* pThrEnv///<�����̵߳�thread-specific-store
                            );
private:
    CwxEchoApp*     m_pApp;  ///<app����
    CWX_UINT64      m_ullMsgNum;
};

#endif 
