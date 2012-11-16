#include "CwxEchoApp.h"
#include "CwxDate.h"

///���캯��
CwxEchoApp::CwxEchoApp()
{
    m_eventHandler = NULL;
    m_threadPool = NULL;
}

///��������
CwxEchoApp::~CwxEchoApp(){
}

///��ʼ��
int CwxEchoApp::init(int argc, char** argv){
    string strErrMsg;
    ///���ȵ��üܹ���init api
    if (CwxAppFramework::init(argc, argv) == -1) return -1;
    ///����Ƿ�ͨ��-fָ���������ļ�����û�У������Ĭ�ϵ������ļ�
    if ((NULL == this->getConfFile()) || (strlen(this->getConfFile()) == 0)){
        this->setConfFile("svr_conf.cnf");
    }
    ///���������ļ�����ʧ�����˳�
    if (0 != m_config.loadConfig(getConfFile())){
        CWX_ERROR((m_config.getError()));
        return -1;
    }
    ///����������־�����level
    setLogLevel(CwxLogger::LEVEL_ERROR|CwxLogger::LEVEL_INFO|CwxLogger::LEVEL_WARNING);
    return 0;
}

///�������л�����Ϣ
int CwxEchoApp::initRunEnv(){
    ///����ϵͳ��ʱ�Ӽ������С�̶�Ϊ1ms����Ϊ1s��
    this->setClick(1000);//1s
    ///���ù���Ŀ¼
    this->setWorkDir(this->m_config.m_strWorkDir.c_str());
    ///����ѭ��������־������
    this->setLogFileNum(LOG_FILE_NUM);
    ///����ÿ����־�ļ��Ĵ�С
    this->setLogFileSize(LOG_FILE_SIZE*1024*1024);
    ///���üܹ���initRunEnv��ʹ�������õĲ�����Ч
    if (CwxAppFramework::initRunEnv() == -1 ) return -1;
    blockSignal(SIGPIPE);
    //set version
    this->setAppVersion(ECHO_APP_VERSION);
    //set last modify date
    this->setLastModifyDatetime(ECHO_MODIFY_DATE);
    //set compile date
    this->setLastCompileDatetime(CWX_COMPILE_DATE(_BUILD_DATE));

    ///�����ص������ļ���Ϣ�������־�ļ��У��Թ��鿴���
    string strConfOut;
    m_config.outputConfig(strConfOut);
    CWX_INFO((strConfOut.c_str()));

    ///ע��echo����Ĵ���handle��echo�����svr-idΪSVR_TYPE_ECHO
    m_eventHandler = new CwxEchoEventHandler(this);         
    this->getCommander().regHandle(SVR_TYPE_ECHO, m_eventHandler);

    ///����TCP���ӣ��佨�������ӵ�svr-id��ΪSVR_TYPE_ECHO�����յ���Ϣ��svr-id��ΪSVR_TYPE_ECHO��
    ///ȫ����m_eventHandler����������
    if (0 > this->noticeTcpListen(SVR_TYPE_ECHO, 
        this->m_config.m_listen.getHostName().c_str(),
        this->m_config.m_listen.getPort(),
        false,
        CWX_APP_MSG_MODE,
        CwxEchoApp::setSockAttr,
        this))
    {
        CWX_ERROR(("Can't register the echo acceptor port: addr=%s, port=%d",
            this->m_config.m_listen.getHostName().c_str(),
            this->m_config.m_listen.getPort()));
        return -1;
    }
    ///����UNIX DOMAIN���ӣ��佨�������ӵ�svr-id��ΪSVR_TYPE_ECHO�����յ���Ϣ��svr-id��ΪSVR_TYPE_ECHO��
    ///ȫ����m_eventHandler����������
    if (0 > this->noticeLsockListen(SVR_TYPE_ECHO, 
        this->m_config.m_strUnixPathFile.c_str()))
    {
        CWX_ERROR(("Can't register the echo unix acceptor port: path=%s",
            m_config.m_strUnixPathFile.c_str()));
        return -1;
    }
    ///�����̳߳ض��󣬴��̳߳����̵߳�group-idΪ2���̳߳ص��߳�����Ϊm_config.m_unThreadNum��
    m_threadPool = new CwxThreadPool(2,
        m_config.m_unThreadNum,
        getThreadPoolMgr(),
        &getCommander());
    ///�����̣߳��̳߳����̵߳��߳�ջ��СΪ1M��
    if ( 0 != m_threadPool->start(NULL)){
        CWX_ERROR(("Failure to start thread pool"));
        return -1;
    }
    return 0;

}

///ʱ�Ӻ�����ʲôҲû����
void CwxEchoApp::onTime(CwxTimeValue const& current){
    CwxAppFramework::onTime(current);
}

///�źŴ�����
void CwxEchoApp::onSignal(int signum){
    switch(signum){
    case SIGQUIT: 
        ///����ؽ���֪ͨ�˳������Ƴ�
        CWX_INFO(("Recv exit signal , exit right now."));
        this->stop();
        break;
    default:
        ///�����źţ�ȫ������
        CWX_INFO(("Recv signal=%d, ignore it.", signum));
        break;
    }

}

///echo�����������Ϣ
int CwxEchoApp::onRecvMsg(CwxMsgBlock* msg, CwxAppHandler4Msg& conn, CwxMsgHead const& header, bool& bSuspendConn){

    msg->event().setSvrId(conn.getConnInfo().getSvrId());
    msg->event().setHostId(conn.getConnInfo().getHostId());
    msg->event().setConnId(conn.getConnInfo().getConnId());
    msg->event().setIoHandle(conn.getHandle());
    msg->event().setConnUserData(NULL);
    msg->event().setMsgHeader(header);
    msg->event().setEvent(CwxEventInfo::RECV_MSG);
    msg->event().setTimestamp(CwxDate::getTimestamp());
    ///��ֹͣ��������
    bSuspendConn = false;
    CWX_ASSERT (msg);
    ///����Ϣ�ŵ��̳߳ض����У����ڲ����̵߳����䴦��handle������
    m_threadPool->append(msg);
    return 0;

}

int CwxEchoApp::setSockAttr(CWX_HANDLE handle, void* arg)
{
    CwxEchoApp* app=(CwxEchoApp*)arg;
    int iSockBuf = 1024 * 1024;
    while (setsockopt(handle, SOL_SOCKET, SO_SNDBUF, (void*)&iSockBuf, sizeof(iSockBuf)) < 0)
    {
        iSockBuf -= 1024;
        if (iSockBuf <= 1024) break;
    }
    iSockBuf = 1024 * 1024;
    while(setsockopt(handle, SOL_SOCKET, SO_RCVBUF, (void *)&iSockBuf, sizeof(iSockBuf)) < 0)
    {
        iSockBuf -= 1024;
        if (iSockBuf <= 1024) break;
    }

    if (app->m_config.m_listen.isKeepAlive())
    {
        if (0 != CwxSocket::setKeepalive(handle,
            true,
            CWX_APP_DEF_KEEPALIVE_IDLE,
            CWX_APP_DEF_KEEPALIVE_INTERNAL,
            CWX_APP_DEF_KEEPALIVE_COUNT))
        {
            CWX_ERROR(("Failure to set listen addr:%s, port:%u to keep-alive, errno=%d",
                app->m_config.m_listen.getHostName().c_str(),
                app->m_config.m_listen.getPort(),
                errno));
            return -1;
        }
    }

    int flags= 1;
    if (setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags)) != 0)
    {
        CWX_ERROR(("Failure to set listen addr:%s, port:%u NODELAY, errno=%d",
            app->m_config.m_listen.getHostName().c_str(),
            app->m_config.m_listen.getPort(),
            errno));
        return -1;
    }
    struct linger ling= {0, 0};
    if (setsockopt(handle, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling)) != 0)
    {
        CWX_ERROR(("Failure to set listen addr:%s, port:%u LINGER, errno=%d",
            app->m_config.m_listen.getHostName().c_str(),
            app->m_config.m_listen.getPort(),
            errno));
        return -1;
    }
    return 0;
}

void CwxEchoApp::destroy()
{
    if (m_threadPool){
        m_threadPool->stop();
        delete m_threadPool;
        m_threadPool = NULL;
    }
    if (m_eventHandler)
    {
        delete m_eventHandler;
        m_eventHandler = NULL;
    }
    CwxAppFramework::destroy();
}



