#include "CwxEchoChannelApp.h"
#include "CwxDate.h"

///���캯��
CwxEchoChannelApp::CwxEchoChannelApp()
{
    m_eventHandler = NULL;
    m_threadPool = NULL;
    m_channel = NULL;
}

///��������
CwxEchoChannelApp::~CwxEchoChannelApp(){
}

///��ʼ��
int CwxEchoChannelApp::init(int argc, char** argv){
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
int CwxEchoChannelApp::initRunEnv(){
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

    ///����TCP���ӣ��佨�������ӵ�svr-id��ΪSVR_TYPE_ECHO�����յ���Ϣ��svr-id��ΪSVR_TYPE_ECHO��
    ///ȫ����m_eventHandler����������
    if (0 > this->noticeTcpListen(SVR_TYPE_ECHO, 
        this->m_config.m_listen.getHostName().c_str(),
        this->m_config.m_listen.getPort(),
        false,
        CWX_APP_EVENT_MODE,
        CwxEchoChannelApp::setSockAttr,
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
        this->m_config.m_strUnixPathFile.c_str(),
        false,
        CWX_APP_EVENT_MODE))
    {
        CWX_ERROR(("Can't register the echo unix acceptor port: path=%s",
            m_config.m_strUnixPathFile.c_str()));
        return -1;
    }
    m_threadPool = new CwxThreadPool*[m_config.m_unThreadNum];
    m_channel = new CwxAppChannel*[m_config.m_unThreadNum];
    CWX_UINT32 i = 0;
    for (i=0; i< m_config.m_unThreadNum; i++)
    {
        m_threadPool[i] = NULL;
        m_channel[i] = NULL;
    }
    for (CWX_UINT32 i=0; i<m_config.m_unThreadNum; i++)
    {
        m_channel[i] = new CwxAppChannel();
        m_threadPool[i] = new CwxThreadPool( 2 + i,
            1,
            getThreadPoolMgr(),
            &getCommander(),
            CwxEchoChannelApp::ThreadMain,
            m_channel[i]);
        ///�����̣߳��̳߳����̵߳��߳�ջ��СΪ1M��
        if ( 0 != m_threadPool[i]->start(NULL)){
            CWX_ERROR(("Failure to start thread pool"));
            return -1;
        }
    }
    return 0;
}

///ʱ�Ӻ�����ʲôҲû����
void CwxEchoChannelApp::onTime(CwxTimeValue const& current){
    CwxAppFramework::onTime(current);
}

///�źŴ�����
void CwxEchoChannelApp::onSignal(int signum){
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

int CwxEchoChannelApp::onConnCreated(CWX_UINT32 uiSvrId,
                          CWX_UINT32 uiHostId,
                          CWX_HANDLE handle,
                          bool& )
{
    CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
    msg->event().setSvrId(uiSvrId);
    msg->event().setHostId(uiHostId);
    msg->event().setConnId(CWX_APP_INVALID_CONN_ID);
    msg->event().setIoHandle(handle);
    msg->event().setEvent(CwxEventInfo::CONN_CREATED);
    ///����Ϣ�ŵ��̳߳ض����У����ڲ����̵߳����䴦��handle������
    if (m_threadPool[handle%m_config.m_unThreadNum]->append(msg) <= 1)
    {
        m_channel[handle%m_config.m_unThreadNum]->notice();
    }
    return 0;
}

void CwxEchoChannelApp::destroy()
{
    if (m_threadPool)
    {
        for (CWX_UINT32 i=0; i<m_config.m_unThreadNum; i++)
        {
            if (m_threadPool[i])
            {
                m_threadPool[i]->stop();
                delete m_threadPool[i];
            }
        }
        delete [] m_threadPool;
        m_threadPool = NULL;
    }
    if (m_channel)
    {
        for (CWX_UINT32 i=0; i<m_config.m_unThreadNum; i++)
        {
            if (m_channel[i])
            {
                delete m_channel[i];
            }
        }
        delete [] m_channel;
        m_channel = NULL;
    }
    CwxAppFramework::destroy();
}

int CwxEchoChannelApp::ThreadDoQueue(CwxMsgQueue* queue, CwxAppChannel* channel)
{
    int iRet = 0;
    CwxMsgBlock* block = NULL;
    CwxEchoChannelEventHandler* handler = NULL;
    while (!queue->isEmpty())
    {
        do 
        {
            iRet = queue->dequeue(block);
            if (-1 == iRet) return -1;
            CWX_ASSERT(block->event().getEvent() == CwxEventInfo::CONN_CREATED);
            if (channel->isRegIoHandle(block->event().getIoHandle()))
            {
                CWX_ERROR(("Handler[%] is register", block->event().getIoHandle()));
                break;
            }
            handler = new CwxEchoChannelEventHandler(channel);
            handler->setHandle(block->event().getIoHandle());
            if (0 != handler->open(NULL))
            {
                CWX_ERROR(("Failure to register handler[%d]", handler->getHandle()));
                delete handler;
                break;
            }
        } while(0);
        CwxMsgBlockAlloc::free(block);
        block = NULL;
    }
    if (queue->isDeactived()) return -1;
    return 0;
}

void* CwxEchoChannelApp::ThreadMain(CwxTss* , CwxMsgQueue* queue, void* arg)
{
    CwxAppChannel* channel = (CwxAppChannel*) arg;
    if (0 != channel->open())
    {
        CWX_ERROR(("Failure to open channel"));
        return NULL;
    }
    while(1)
    {
        //��ȡ�����е���Ϣ������
        if (0 != ThreadDoQueue(queue, channel)) break;
        if (-1 == channel->dispatch(100))
        {
            CWX_ERROR(("Failure to invoke CwxAppChannel::dispatch()"));
            sleep(1);
        }
    }
    channel->stop();
    channel->close();
    return NULL;
}

int CwxEchoChannelApp::setSockAttr(CWX_HANDLE handle, void* arg)
{
    CwxEchoChannelApp* app = (CwxEchoChannelApp*)arg;
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
