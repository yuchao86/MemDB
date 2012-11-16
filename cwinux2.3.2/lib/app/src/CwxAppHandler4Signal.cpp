#include "CwxAppHandler4Signal.h"
#include "CwxAppFramework.h"

CWINUX_BEGIN_NAMESPACE
///���캯��
CwxAppHandler4Signal::CwxAppHandler4Signal(CwxAppFramework* pApp,
                                           CwxAppReactor* reactor,
                                           int sig)
:CwxAppHandler4Base(reactor)
{
    m_pApp = pApp;
    setHandle(sig);
}
CwxAppHandler4Signal::~CwxAppHandler4Signal()
{
}

/**
@brief ���ӽ����Ĵ��������������ӽ����ĳ�ʼ��
@param [in] parent �������ӵ�acceptor��connector
@return -1���������������ӣ� 0�����ӽ����ɹ�
*/
int CwxAppHandler4Signal::open (void *)
{
    if (0 != reactor()->registerSignal(getHandle(), this))
    {
        CWX_ERROR(("Failure to register signal handle, sig[%d]", (int)getHandle()));
        return -1;
    }
    return 0;

}
/**
@brief ���������ϵ��¼�
@param [in] handle ���ӵ�handle
@return -1������ʧ�ܣ� 0������ɹ�
*/
int CwxAppHandler4Signal::handle_event(int, CWX_HANDLE )
{
    if (getApp()->isStopped()) return 0;
    this->getApp()->onSignal(getHandle());
    return 0;
}

int CwxAppHandler4Signal::close(CWX_HANDLE)
{
    reactor()->removeSignal(this);
    delete this;
    return 0;
}



CWINUX_END_NAMESPACE

