#include "UnistorBenchApp.h"
#include "CwxSocket.h"

///构造函数
UnistorBenchApp::UnistorBenchApp(){
	m_threadPool = NULL;
	m_eventHandler = NULL;
}

///析构函数
UnistorBenchApp::~UnistorBenchApp(){

}

///初始化APP，加载配置文件
int UnistorBenchApp::init(int argc, char** argv){
    string strErrMsg;
    ///首先调用架构的init
    if (CwxAppFramework::init(argc, argv) == -1) return -1;
    ///若没有通过-f指定配置文件，则采用默认的配置文件
    if ((NULL == this->getConfFile()) || (strlen(this->getConfFile()) == 0)){
        this->setConfFile("unistor_bench.cnf");
    }
    ///加载配置文件
    if (0 != m_config.loadConfig(getConfFile())){
        CWX_ERROR((m_config.getError()));
        return -1;
    }
    ///设置输出运行日志的level
    setLogLevel(CwxLogger::LEVEL_ERROR|CwxLogger::LEVEL_INFO|CwxLogger::LEVEL_WARNING);
    return 0;
}

//init the Enviroment before run.0:success, -1:failure.
int UnistorBenchApp::initRunEnv(){
    ///设置时钟的刻度，最小为1ms，此为1s。
    this->setClick(1000);//1s
    //set work dir
    this->setWorkDir(this->m_config.m_strWorkDir.c_str());
    //Set log file
    this->setLogFileNum(LOG_FILE_NUM);
    this->setLogFileSize(LOG_FILE_SIZE*1024*1024);
    ///调用架构的initRunEnv，使设置的参数生效
    if (CwxAppFramework::initRunEnv() == -1 ) return -1;

    //output config
    m_config.outputConfig();

    CWX_UINT16 i=0;
    //建立配置文件中设置的、与echo服务的连接
    for (i=0; i<m_config.m_unConnNum; i++){
        //create  conn
        if (0 > this->noticeTcpConnect(SVR_TYPE_RECV,
            0,
            this->m_config.m_listen.getHostName().c_str(),
            this->m_config.m_listen.getPort(),
            false,
            1,
            2,
            UnistorBenchApp::setSockAttr,
            this))
        {
            CWX_ERROR(("Can't connect the service: addr=%s, port=%d",
                this->m_config.m_listen.getHostName().c_str(),
                this->m_config.m_listen.getPort()));
            return -1;
        }
    }
	//注册handle
	m_eventHandler = new UnistorEventHandler(this);         
	this->getCommander().regHandle(SVR_TYPE_RECV, m_eventHandler);
	//启动线程
	m_threadPool = new CwxThreadPool(2,
		1,
		getThreadPoolMgr(),
		&getCommander());
	CwxTss** pTss = new CwxTss*[1];
	pTss[0] = new UnistorTss();
	((UnistorTss*)pTss[0])->init();
	///启动线程。
	if ( 0 != m_threadPool->start(pTss)){
		CWX_ERROR(("Failure to start thread pool"));
		return -1;
	}
    return 0;
}

///信号处理函数
void UnistorBenchApp::onSignal(int signum){
    switch(signum){
    case SIGQUIT: 
        CWX_INFO(("Recv exit signal, exit right now."));
        this->stop();
        break;
    default:
        ///其他信号，忽略
        CWX_INFO(("Recv signal=%d, ignore it.", signum));
        break;
    }
}

///echo服务的连接建立响应函数
int UnistorBenchApp::onConnCreated(CwxAppHandler4Msg& conn, bool& , bool& ){
	CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
	msg->event().setSvrId(conn.getConnInfo().getSvrId());
	msg->event().setHostId(conn.getConnInfo().getHostId());
	msg->event().setConnId(conn.getConnInfo().getConnId());
	msg->event().setEvent(CwxEventInfo::CONN_CREATED);
	m_threadPool->append(msg);
    return 0;
}

///echo回复的消息响应函数
int UnistorBenchApp::onRecvMsg(CwxMsgBlock* msg,
                               CwxAppHandler4Msg& conn,
                               CwxMsgHead const& header,
                               bool& )
{
    msg->event().setSvrId(conn.getConnInfo().getSvrId());
    msg->event().setHostId(conn.getConnInfo().getHostId());
    msg->event().setConnId(conn.getConnInfo().getConnId());
    msg->event().setEvent(CwxEventInfo::RECV_MSG);
    msg->event().setMsgHeader(header);
    msg->event().setTimestamp(CwxDate::getTimestamp());
	m_threadPool->append(msg);
    return 0;
}


///设置连接的属性
int UnistorBenchApp::setSockAttr(CWX_HANDLE handle, void* arg){
    UnistorBenchApp* app = (UnistorBenchApp*)arg;

    if (app->m_config.m_listen.isKeepAlive()){
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
    if (setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags)) != 0){
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


void UnistorBenchApp::destroy(){
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
