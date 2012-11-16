#include "UnistorApp.h"
#include "CwxDate.h"
///���캯��
UnistorApp::UnistorApp(){
    m_store = NULL;
    m_recvChannel = NULL;
    m_recvThreadPool = NULL;
    m_recvArgs = NULL;
    m_writeThreadPool = NULL;
    m_masterHandler = NULL;
    m_recvWriteHandler = NULL;
    m_innerSyncThreadPool = NULL;
    m_innerSyncChannel = NULL;
    m_outerSyncThreadPool = NULL;
    m_outerSyncChannel = NULL;
    m_checkpointHandler = NULL;
    m_checkpointThreadPool = NULL;
    m_transThreadPool = NULL;
    m_transChannel = NULL;
    m_zkThreadPool = NULL;
    m_zkHandler = NULL;
}

///��������
UnistorApp::~UnistorApp(){
}

///��ʼ��
int UnistorApp::init(int argc, char** argv){
    string strErrMsg;
    ///���ȵ��üܹ���init api
    if (CwxAppFramework::init(argc, argv) == -1) return -1;
    ///����Ƿ�ͨ��-fָ���������ļ�����û�У������Ĭ�ϵ������ļ�
    if ((NULL == this->getConfFile()) || (strlen(this->getConfFile()) == 0)){
        this->setConfFile("unistor.cnf");
    }
    ///���������ļ�����ʧ�����˳�
    if (0 != m_config.init(getConfFile())){
        CWX_ERROR((m_config.getErrMsg()));
        return -1;
    }
    ///����������־�����level
    setLogLevel(CwxLogger::LEVEL_ERROR|CwxLogger::LEVEL_INFO|CwxLogger::LEVEL_WARNING|CwxLogger::LEVEL_DEBUG);
    return 0;
}

///�������л�����Ϣ
int UnistorApp::initRunEnv(){
    ///����ϵͳ��ʱ�Ӽ������С�̶�Ϊ1ms����Ϊ1s��
    this->setClick(100);//0.1s
    ///���ù���Ŀ¼
    this->setWorkDir(m_config.getCommon().m_strWorkDir.c_str());
    ///����ѭ��������־������
    this->setLogFileNum(LOG_FILE_NUM);
    ///����ÿ����־�ļ��Ĵ�С
    this->setLogFileSize(LOG_FILE_SIZE*1024*1024);
    ///���üܹ���initRunEnv��ʹ�������õĲ�����Ч
    if (CwxAppFramework::initRunEnv() == -1 ) return -1;
    ///�����ص������ļ���Ϣ�������־�ļ��У��Թ��鿴���
    m_config.outputConfig();
    ///block����signal
    this->blockSignal(SIGTERM);
    this->blockSignal(SIGUSR1);
    this->blockSignal(SIGUSR2);
    this->blockSignal(SIGCHLD);
    this->blockSignal(SIGCLD);
    this->blockSignal(SIGHUP);
    this->blockSignal(SIGPIPE);
    this->blockSignal(SIGALRM);
    this->blockSignal(SIGCONT);
    this->blockSignal(SIGSTOP);
    this->blockSignal(SIGTSTP);
    this->blockSignal(SIGTTOU);

    //set version
    this->setAppVersion(UNISTOR_VERSION);
    //set last modify date
    this->setLastModifyDatetime(UNISTOR_MODIFY_DATE);
    //set compile date
    this->setLastCompileDatetime(CWX_COMPILE_DATE(_BUILD_DATE));
    ///���÷���״̬
    this->setAppRunValid(false);
    ///���÷�����Ϣ
    this->setAppRunFailReason("Starting..............");
	CwxDate::getDateY4MDHMS2(time(NULL), m_strStartTime);

    //����store driver
    char szErr2K[2048];
    string strEnginePath = m_config.getCommon().m_strWorkDir + "engine/";
	if (m_store) delete m_store;
    int ret = -1;
    do{
        m_store = new UnistorStore();
        if (0 != m_store->init(UnistorApp::storeMsgPipe, UnistorApp::getSysKey, this, &m_config, strEnginePath, szErr2K)){
            CWX_ERROR(("Failure to init store, err=%s", szErr2K));
            break;
        }

        ///�����������������
        if (0 != startNetwork()) break;

        //����checkpoint�߳�
        {
            m_checkpointHandler = new UnistorHandler4Checkpoint(this);
            getCommander().regHandle(SVR_TYPE_CHECKPOINT, m_checkpointHandler);
            ///�����߳�
            m_checkpointThreadPool = new CwxThreadPool(THREAD_GROUP_CHECKPOINT,
                1,
                getThreadPoolMgr(),
                &getCommander());
            ///�����߳�
            if ( 0 != m_checkpointThreadPool->start(NULL)){
                CWX_ERROR(("Failure to start checkpoint thread pool"));
                break;
            }
        }

        //ע��kvдhandler
        m_recvWriteHandler = new UnistorHandler4RecvWrite(this);
        getCommander().regHandle(SVR_TYPE_RECV_WRITE, m_recvWriteHandler);
        //ע��master����ͬ������handler
        m_masterHandler = new UnistorHandler4Master(this);
        getCommander().regHandle(SVR_TYPE_MASTER, m_masterHandler);
        //�����ڲ��ַ��߳�
        {
            m_innerSyncChannel = new CwxAppChannel();
            m_innerSyncThreadPool = new CwxThreadPool(THREAD_GROUP_INNER_SYNC,
                1,
                getThreadPoolMgr(),
                &getCommander(),
                UnistorApp::innerSyncThreadMain,
                this);
            ///�����߳�
            CwxTss** pTss = new CwxTss*[1];
            pTss[0] = new UnistorTss();
            ((UnistorTss*)pTss[0])->init(new UnistorDispatchThreadUserObj());
            if ( 0 != m_innerSyncThreadPool->start(pTss)){
                CWX_ERROR(("Failure to start inner sync thread pool"));
                break;
            }
        }

        //�����ⲿ�ַ����߳�
        {
            m_outerSyncChannel = new CwxAppChannel();
            m_outerSyncThreadPool = new CwxThreadPool(THREAD_GROUP_OUTER_SYNC,
                1,
                getThreadPoolMgr(),
                &getCommander(),
                UnistorApp::outerSyncThreadMain,
                this);
            ///�����߳�
            CwxTss** pTss = new CwxTss*[1];
            pTss[0] = new UnistorTss();
            ((UnistorTss*)pTss[0])->init(new UnistorDispatchThreadUserObj());
            if ( 0 != m_outerSyncThreadPool->start(pTss)){
                CWX_ERROR(("Failure to start outer sync thread pool"));
                break;
            }
        }

        //��������д���̳߳�
        {
            m_writeThreadPool = new CwxThreadPool(THREAD_GROUP_WRITE,
                1,
                getThreadPoolMgr(),
                &getCommander());
            ///�����̵߳�tss����
            CwxTss** pTss = new CwxTss*[1];
            pTss[0] = new UnistorTss();
            ((UnistorTss*)pTss[0])->init(NULL);
            ///�����߳�
            if ( 0 != m_writeThreadPool->start(pTss)){
                CWX_ERROR(("Failure to start write thread pool"));
                break;
            }
        }

        //�������ݲ�ѯ�̳߳�
        ///����recv�ĳ�ʼ��
        CWX_UINT32 i=0;
        m_recvChannel = new CwxAppChannel*[m_config.getCommon().m_uiThreadNum];
        m_recvThreadPool = new CwxThreadPool*[m_config.getCommon().m_uiThreadNum];
        m_recvArgs = new pair<UnistorApp*, CWX_UINT32>[m_config.getCommon().m_uiThreadNum];
        for (i=0; i<m_config.getCommon().m_uiThreadNum; i++){
            m_recvChannel[i] = new CwxAppChannel();
            m_recvArgs[i].first = this;
            m_recvArgs[i].second = i;
            m_recvThreadPool[i] = new CwxThreadPool(THREAD_GROUP_RECV_BASE + i,
                1,
                getThreadPoolMgr(),
                &getCommander(),
                UnistorApp::recvThreadMain,
                &m_recvArgs[i]);
            ///�����߳�
            CwxTss**pTss = new CwxTss*[1];
            pTss[0] = new UnistorTss();
            ((UnistorTss*)pTss[0])->init(new UnistorRecvThreadUserObj());
            if ( 0 != m_recvThreadPool[i]->start(pTss)){
                CWX_ERROR(("Failure to start kv query thread pool"));
                break;
            }
        }
        if (i != m_config.getCommon().m_uiThreadNum) break;

        //����master��Ϣת���̳߳�
        ///ת��handler��Դ�ĳ�ʼ��
        UnistorHandler4Trans::init(getConfig().getCommon().m_uiTranConnNum);
        {
            m_transChannel = new CwxAppChannel();
            m_transThreadPool = new CwxThreadPool(THREAD_GROUP_TRANSFER,
                1,
                getThreadPoolMgr(),
                &getCommander(),
                UnistorApp::transThreadMain,
                this);
            ///�����߳�
            CwxTss** pTss = new CwxTss*[1];
            pTss[0] = new UnistorTss();
            ((UnistorTss*)pTss[0])->init(NULL);
            if ( 0 != m_transThreadPool->start(pTss)){
                CWX_ERROR(("Failure to start trans thread pool"));
                break;
            }
        }

        //����zk�̳߳�
        {
            m_zkThreadPool = new CwxThreadPool(THREAD_GROUP_ZK,
                1,
                getThreadPoolMgr(),
                &getCommander(),
                UnistorApp::zkThreadMain,
                this);
            ///�����߳�
            if ( 0 != m_zkThreadPool->start(NULL)){
                CWX_ERROR(("Failure to start zookeeper thread pool"));
                break;
            }
        }
        //����zk��handler
        m_zkHandler = new UnistorHandler4Zk(this);
        if (0 != m_zkHandler->init()){
            CWX_ERROR(("Failure to init zk handler"));
        }
        ret = 0;
    }while(0);
    if (0 != ret){
        this->blockSignal(SIGQUIT);
    }
    return ret;
}


///ʱ�Ӻ���
void UnistorApp::onTime(CwxTimeValue const& current){
    ///���û����onTime����
    CwxAppFramework::onTime(current);
	static CWX_UINT32 ttCheckTimeoutTime = 0;
    static CWX_UINT32 ttTimeBase = 0;
	CWX_UINT32 uiNow = time(NULL);
    bool bClockBack = isClockBack(ttTimeBase, uiNow);

    if (bClockBack || (uiNow - ttCheckTimeoutTime >= 1)){
        ttCheckTimeoutTime = uiNow;
        ///�����recv�������������������ʱ���ź�
        CwxMsgBlock* block = NULL;
        CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
        msg->event().setEvent(CwxEventInfo::TIMEOUT_CHECK);
        msg->event().setTimestamp(uiNow);
        ///��write��������
        if (m_writeThreadPool){
            block = CwxMsgBlockAlloc::clone(msg);
            block->event().setSvrId(SVR_TYPE_RECV_WRITE);
            m_writeThreadPool->append(block);
        }
        ///��ת����������
        if (m_transThreadPool){
            block = CwxMsgBlockAlloc::clone(msg);
            block->event().setSvrId(SVR_TYPE_TRANSFER);
            m_transThreadPool->append(block);
        }
        ///��checkpoint�߳�����
        if (m_checkpointThreadPool &&  m_checkpointHandler && m_checkpointHandler->isNeedCheckOut(uiNow)){
            block = CwxMsgBlockAlloc::clone(msg);
            block->event().setSvrId(SVR_TYPE_CHECKPOINT);
            m_checkpointThreadPool->append(block);
        }
        ///��zk�߳�����
        if (m_zkThreadPool){
            msg->event().setSvrId(SVR_TYPE_ZK);
            m_zkThreadPool->append(msg);
        }else{
            CwxMsgBlockAlloc::free(msg);
        }
    }
}

///�źŴ�����
void UnistorApp::onSignal(int signum){
    switch(signum){
    case SIGQUIT: 
        ///����ؽ���֪ͨ�˳������Ƴ�
        CWX_INFO(("Recv exit signal, exit right now."));
        this->stop();
        break;
    default:
        ///�����źţ�ȫ������
        CWX_INFO(("Recv signal=%d, ignore it.", signum));
        break;
    }
}

///�����������ӵ����ӽ���
int UnistorApp::onConnCreated(CWX_UINT32 uiSvrId,
                          CWX_UINT32 uiHostId,
                          CWX_HANDLE handle,
                          bool& )
{
    if ((SVR_TYPE_RECV == uiSvrId)||
        (SVR_TYPE_INNER_SYNC == uiSvrId) ||
        (SVR_TYPE_OUTER_SYNC == uiSvrId))
    {
        CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
        msg->event().setSvrId(uiSvrId);
        msg->event().setHostId(uiHostId);
        msg->event().setConnId(CWX_APP_INVALID_CONN_ID);
        msg->event().setIoHandle(handle);
        msg->event().setEvent(CwxEventInfo::CONN_CREATED);
        if (SVR_TYPE_RECV == uiSvrId){///���ݽ������ӽ������ŵ�recv�߳�
            CWX_UINT32 uiIndex = handle%m_config.getCommon().m_uiThreadNum;
            if (m_recvThreadPool[uiIndex]->append(msg) <= 1) m_recvChannel[uiIndex]->notice();
        }else if (SVR_TYPE_INNER_SYNC == uiSvrId){///�ڲ�ͬ��
            if (m_innerSyncThreadPool->append(msg) <= 1) m_innerSyncChannel->notice();
        }else if (SVR_TYPE_OUTER_SYNC == uiSvrId){///�ⲿͬ��
            if (m_outerSyncThreadPool->append(msg) <= 1) m_outerSyncChannel->notice();
        }
    }else{
        CWX_ERROR(("Unknown svr-type[%u] for connection", uiSvrId));
        CWX_ASSERT(0);
    }
    return 0;

}

///����msg�����ӽ���
int UnistorApp::onConnCreated(CwxAppHandler4Msg& conn,
						  bool& ,
						  bool& )
{
    if (SVR_TYPE_MONITOR == conn.getConnInfo().getSvrId()){///����Ǽ�ص����ӽ���������һ��string��buf�����ڻ��治����������
		string* buf = new string();
		conn.getConnInfo().setUserData(buf);
        return 0;
    }
    CWX_ERROR(("Unknow svr-type[%u] for connect create.", conn.getConnInfo().getSvrId()));
	return 0;
}

///���ӹر�
int UnistorApp::onConnClosed(CwxAppHandler4Msg& conn){
	if (SVR_TYPE_MASTER == conn.getConnInfo().getSvrId()){///�����master��ͬ�����ӹر�
		CwxMsgBlock* pBlock = CwxMsgBlockAlloc::malloc(0);
		pBlock->event().setSvrId(conn.getConnInfo().getSvrId());
		pBlock->event().setHostId(conn.getConnInfo().getHostId());
		pBlock->event().setConnId(conn.getConnInfo().getConnId());
		///�����¼�����
		pBlock->event().setEvent(CwxEventInfo::CONN_CLOSED);
		m_writeThreadPool->append(pBlock);
	}else if (SVR_TYPE_MONITOR == conn.getConnInfo().getSvrId()){///���Ǽ�ص����ӹرգ�������ͷ���ǰ��������string����
		if (conn.getConnInfo().getUserData()){
			delete (string*)conn.getConnInfo().getUserData();
			conn.getConnInfo().setUserData(NULL);
		}
	}else{
        CWX_ERROR(("Unknown svr-type[%u]'s connection is closed.", conn.getConnInfo().getSvrId()));
		CWX_ASSERT(0);
	}
	return 0;
}


///�յ���Ϣ
int UnistorApp::onRecvMsg(CwxMsgBlock* msg,
						CwxAppHandler4Msg& conn,
						CwxMsgHead const& header,
						bool& )
{
    if (SVR_TYPE_MASTER == conn.getConnInfo().getSvrId()){///ֻ��ת��������Ϣ
        msg->event().setSvrId(conn.getConnInfo().getSvrId());
        msg->event().setHostId(conn.getConnInfo().getHostId());
        msg->event().setConnId(conn.getConnInfo().getConnId());
        msg->event().setMsgHeader(header);
        msg->event().setEvent(CwxEventInfo::RECV_MSG);
        m_writeThreadPool->append(msg);
    }else{
        CWX_ERROR(("Recv msg from Unknown svr-type[%u]'s connection.", conn.getConnInfo().getSvrId()));
        CwxMsgBlockAlloc::free(msg);
    }
    return 0;
}

///�յ���Ϣ����Ӧ����
int UnistorApp::onRecvMsg(CwxAppHandler4Msg& conn,
						bool& ){
	if (SVR_TYPE_MONITOR == conn.getConnInfo().getSvrId()){
		char  szBuf[1024];
		ssize_t recv_size = CwxSocket::recv(conn.getHandle(),
			szBuf,
			1024);
		if (recv_size <=0 ){ //error or signal
			if ((0==recv_size) || ((errno != EWOULDBLOCK) && (errno != EINTR))){
				return -1; //error
			}else{//signal or no data
				return 0;
			}
		}
		///�����Ϣ
		return monitorStats(szBuf, (CWX_UINT32)recv_size, conn);
	}else{
        CWX_ERROR(("Recv msg from Unknown svr-type[%u]'s connection.", conn.getConnInfo().getSvrId()));
		CWX_ASSERT(0);
	}
	return -1;
}


void UnistorApp::destroy(){
	CWX_UINT32 i=0;
    //ֹͣzk�̣߳���������zkHandler->stop�������������
    if (m_zkThreadPool) m_zkThreadPool->stop();
    //ֹͣzookeeper�ײ��߳�
    if (m_zkHandler){
        m_zkHandler->stop();
    }
    //ֹͣrecv�߳�
    if (m_recvThreadPool){
        for (i=0; i<m_config.getCommon().m_uiThreadNum; i++){
            if (m_recvThreadPool[i]) m_recvThreadPool[i]->stop();
        }
    }
    ///ֹͣд�߳�
    if (m_writeThreadPool) m_writeThreadPool->stop();
    //ֹͣinnersync�߳�
    if (m_innerSyncThreadPool) m_innerSyncThreadPool->stop();
    //ֹͣoutersync�߳�
    if (m_outerSyncThreadPool) m_outerSyncThreadPool->stop();
    //ֹͣcheckpoint�߳�
    if (m_checkpointThreadPool) m_checkpointThreadPool->stop();
    //ֹͣת���߳�
    if (m_transThreadPool) m_transThreadPool->stop();

    //�ͷ�handler
    UnistorHandler4Trans::destroy();
    if (m_masterHandler) delete m_masterHandler;
    m_masterHandler = NULL;

    if (m_recvWriteHandler) delete m_recvWriteHandler;
    m_recvWriteHandler = NULL;

    if (m_checkpointHandler) delete m_checkpointHandler;
    m_checkpointHandler = NULL;

    if (m_zkHandler) delete m_zkHandler;
    m_zkHandler = NULL;

    //�ͷ��̳߳ؼ�channel
    if (m_recvThreadPool){
		for (i=0; i<m_config.getCommon().m_uiThreadNum; i++){
			if (m_recvThreadPool[i]) delete m_recvThreadPool[i];
		}
        delete [] m_recvThreadPool;
        m_recvThreadPool = NULL;
    }
    if (m_recvChannel){
		for (i=0; i<m_config.getCommon().m_uiThreadNum; i++){
			if (m_recvChannel[i]) delete m_recvChannel[i];
		}
		delete [] m_recvChannel;
		m_recvChannel = NULL;
    }

	if (m_innerSyncThreadPool) delete m_innerSyncThreadPool;
    m_innerSyncThreadPool = NULL;
    if (m_innerSyncChannel) delete m_innerSyncChannel;
    m_innerSyncChannel = NULL;

    //ɾ���ⲿͬ�����̼߳�channel
    if (m_outerSyncThreadPool) delete m_outerSyncThreadPool;
    m_outerSyncThreadPool = NULL;
    if (m_outerSyncChannel) delete m_outerSyncChannel;
    m_outerSyncChannel = NULL;

    //ɾ��write pool��handler
	if (m_writeThreadPool) delete m_writeThreadPool;
    m_writeThreadPool = NULL;

    //ɾ��checkpoint�߳�
	if (m_checkpointThreadPool) delete m_checkpointThreadPool;
    m_checkpointThreadPool = NULL;

    //ɾ��ת���߳�
    if (m_transThreadPool) delete m_transThreadPool;
    m_transThreadPool = NULL;
    if (m_transChannel) delete m_transChannel;
    m_transChannel = NULL;

    //ɾ��zk�߳�
    if (m_zkThreadPool) delete m_zkThreadPool;
    m_zkThreadPool = NULL;

    if (m_store) delete m_store;
    m_store = NULL;

    if (m_recvArgs) delete [] m_recvArgs;
    m_recvArgs = NULL;

    CwxAppFramework::destroy();
}


int UnistorApp::monitorStats(char const* buf,
                             CWX_UINT32 uiDataLen,
                             CwxAppHandler4Msg& conn)
{
	string* strCmd = (string*)conn.getConnInfo().getUserData();
	strCmd->append(buf, uiDataLen);
	CwxMsgBlock* msg = NULL;
	string::size_type end = 0;
	do{
		CwxCommon::trim(*strCmd);
		end = strCmd->find('\n');
		if (string::npos == end){
			if (strCmd->length() > 10){//��Ч������
				strCmd->erase(); ///��ս��ܵ�������
				///�ظ���Ϣ
				msg = CwxMsgBlockAlloc::malloc(1024);
				strcpy(msg->wr_ptr(), "ERROR\r\n");
				msg->wr_ptr(strlen(msg->wr_ptr()));
			}else{
				return 0;
			}
		}else{
			if (memcmp(strCmd->c_str(), "stats", 5) == 0){
				strCmd->erase(); ///��ս��ܵ�������
				CWX_UINT32 uiLen = packMonitorInfo();
				msg = CwxMsgBlockAlloc::malloc(uiLen);
				memcpy(msg->wr_ptr(), m_szBuf, uiLen);
				msg->wr_ptr(uiLen);
			}else if(memcmp(strCmd->c_str(), "quit", 4) == 0){
				return -1;
			}else{//��Ч������
				strCmd->erase(); ///��ս��ܵ�������
				///�ظ���Ϣ
				msg = CwxMsgBlockAlloc::malloc(1024);
				strcpy(msg->wr_ptr(), "ERROR\r\n");
				msg->wr_ptr(strlen(msg->wr_ptr()));
			}
		}
	}while(0);

	msg->send_ctrl().setConnId(conn.getConnInfo().getConnId());
	msg->send_ctrl().setSvrId(UnistorApp::SVR_TYPE_MONITOR);
	msg->send_ctrl().setHostId(0);
	msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
	if (-1 == sendMsgByConn(msg)){
		CWX_ERROR(("Failure to send monitor reply"));
		CwxMsgBlockAlloc::free(msg);
		return -1;
	}
	return 0;
}

#define UNISTOR_MONITOR_APPEND()\
	uiLen = strlen(szLine);\
	if (uiPos + uiLen > MAX_MONITOR_REPLY_SIZE - 20) break;\
	memcpy(m_szBuf + uiPos, szLine, uiLen);\
	uiPos += uiLen; 

#define UNISTOR_STATS_READ_THREAD(name,attr)\
    ullNum = 0;\
    for (i=0; i<m_config.getCommon().m_uiThreadNum; i++){\
        tss = (UnistorTss*)getThreadPoolMgr()->getTss(THREAD_GROUP_RECV_BASE + i, 0);\
        ullNum += tss->attr;\
    };\
    CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", name,\
        CwxCommon::toString(ullNum, szTmp, 10));\
    UNISTOR_MONITOR_APPEND();

#define UNISTOR_STATS_WRITE_THREAD(name,attr)\
    tss = (UnistorTss*)getThreadPoolMgr()->getTss(THREAD_GROUP_WRITE, 0);\
    ullNum = tss->attr;\
    CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", name,\
    CwxCommon::toString(ullNum, szTmp, 10));\
    UNISTOR_MONITOR_APPEND();

CWX_UINT32 UnistorApp::packMonitorInfo(){
	string strValue;
	char szTmp[64];
	char szLine[4096];
	CWX_UINT32 uiLen = 0;
	CWX_UINT32 uiPos = 0;
    CWX_UINT32 i=0;
	do{
		//�������pid
		CwxCommon::snprintf(szLine, 4096, "STAT %s %d\r\n", UNISTOR_SYS_KEY_PID, getpid());
		UNISTOR_MONITOR_APPEND();
		//���������pid
		CwxCommon::snprintf(szLine, 4096, "STAT %s %d\r\n", UNISTOR_SYS_KEY_PPID, getppid());
		UNISTOR_MONITOR_APPEND();
		//�汾��
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_VERSION, this->getAppVersion().c_str());
		UNISTOR_MONITOR_APPEND();
		//�޸�ʱ��
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_MODIFY, this->getLastModifyDatetime().c_str());
		UNISTOR_MONITOR_APPEND();
		//����ʱ��
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_COMPILE, this->getLastCompileDatetime().c_str());
		UNISTOR_MONITOR_APPEND();
		//����ʱ��
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_START, m_strStartTime.c_str());
		UNISTOR_MONITOR_APPEND();
        //��������
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_ENGINE, getConfig().getCommon().m_strStoreType.c_str());
		UNISTOR_MONITOR_APPEND();
        //����汾
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_ENGINE_VERSION, getStore()->getVersion());
		UNISTOR_MONITOR_APPEND();
        //����״̬
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_ENGINE_STATE, m_store->isValid()?"valid":"invalid");
		UNISTOR_MONITOR_APPEND();
        if (!m_store->isValid()){
            //������Ϣ
            CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_ENGINE_ERROR, m_store->getErrMsg());
            UNISTOR_MONITOR_APPEND();
        }
        //binlog��״̬
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_STATE, m_store->getBinLogMgr()->isInvalid()?"invalid":"valid");
		UNISTOR_MONITOR_APPEND();
        //binlog������Ϣ
        if (!m_store->getBinLogMgr()->isInvalid()){
            CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_ERROR, m_store->getBinLogMgr()->isInvalid()? m_store->getBinLogMgr()->getInvalidMsg():"");
            UNISTOR_MONITOR_APPEND();
        }
        //��ǰ��С��sid
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_MIN_SID,
			CwxCommon::toString(m_store->getBinLogMgr()->getMinSid(), szTmp));
		UNISTOR_MONITOR_APPEND();
        //��ǰ��Сsid��ʱ���
		CwxDate::getDateY4MDHMS2(m_store->getBinLogMgr()->getMinTimestamp(), strValue);
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_MIN_TIMESTAMP,
			strValue.c_str());
		UNISTOR_MONITOR_APPEND();
        //��С��binlog�ļ�
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_MIN_FILE,
			m_store->getBinLogMgr()->getMinFile(strValue).c_str());
		UNISTOR_MONITOR_APPEND();
        //����binlog��sid
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_MAX_SID,
			CwxCommon::toString(m_store->getBinLogMgr()->getMaxSid(), szTmp));
		UNISTOR_MONITOR_APPEND();
        //���binlog sid��ʱ���
		CwxDate::getDateY4MDHMS2(m_store->getBinLogMgr()->getMaxTimestamp(), strValue);
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n",UNISTOR_SYS_KEY_BINLOG_MAX_TIMESTAMP,
			strValue.c_str());
		UNISTOR_MONITOR_APPEND();
        //����binlog�ļ�
		CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_BINLOG_MAX_FILE,
			m_store->getBinLogMgr()->getMaxFile(strValue).c_str());
		UNISTOR_MONITOR_APPEND();
        //���̵߳�����
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_READ_THREAD_NUM,
            m_config.getCommon().m_uiThreadNum);
        UNISTOR_MONITOR_APPEND();
        UnistorTss * tss=NULL;
        CWX_UINT32 uiMsgNum = 0;
        CWX_UINT32 uiConnNum = 0;
        for (i=0; i<m_config.getCommon().m_uiThreadNum; i++){
            tss = (UnistorTss*)getThreadPoolMgr()->getTss(THREAD_GROUP_RECV_BASE + i, 0);
            //���̵߳���Ϣ������������Ϣ
            CwxCommon::snprintf(szLine, 4096, "STAT %s%d %u\r\n", UNISTOR_SYS_KEY_READ_THREAD_QUEUE_PREX,
                i, m_recvThreadPool[i]->getQueuedMsgNum());
            UNISTOR_MONITOR_APPEND();
            uiMsgNum += m_recvThreadPool[i]->getQueuedMsgNum();
            //���̵߳�������
            CwxCommon::snprintf(szLine, 4096, "STAT %s%d %u\r\n", UNISTOR_SYS_KEY_READ_THREAD_CONNECT_PREX,
                i, ((UnistorRecvThreadUserObj*)tss->getUserObj())->getConnNum());
            UNISTOR_MONITOR_APPEND();
            uiConnNum += ((UnistorRecvThreadUserObj*)tss->getUserObj())->getConnNum();
        }
        //���̵߳���Ϣ������������Ϣ
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_READ_THREAD_QUEUE,
            uiMsgNum);
        UNISTOR_MONITOR_APPEND();
        //���̵߳�������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_READ_THREAD_CONNECT,
            uiConnNum);
        UNISTOR_MONITOR_APPEND();

        ///д�̵߳�������Ϣ����
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_WRITE_THREAD_QUEUE,
            m_writeThreadPool->getQueuedMsgNum());
        UNISTOR_MONITOR_APPEND();
        //ת���߳���������Ϣ
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_TRANS_THREAD_QUEUE,
            m_transThreadPool->getQueuedMsgNum());
        UNISTOR_MONITOR_APPEND();
        //checkpoint�߳���������Ϣ
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_CHECKPOINT_THREAD_QUEUE,
            m_checkpointThreadPool->getQueuedMsgNum());
        UNISTOR_MONITOR_APPEND();
        //zk�߳���������Ϣ
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_ZK_THREAD_QUEUE,
            m_zkThreadPool->getQueuedMsgNum());
        UNISTOR_MONITOR_APPEND();
        //�ڲ�ת���߳���������Ϣ
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_INNER_SYNC_THREAD_QUEUE,
            m_innerSyncThreadPool->getQueuedMsgNum());
        UNISTOR_MONITOR_APPEND();
        //�ⲿת���߳���������Ϣ
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_OUTER_SYNC_THREAD_QUEUE,
            m_outerSyncThreadPool->getQueuedMsgNum());
        UNISTOR_MONITOR_APPEND();
        //masterת����Ϣ������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_MASTER_TRANS_MSG_NUM,
            getTaskBoard().getTaskNum());
        UNISTOR_MONITOR_APPEND();
        //zookeeper������״̬
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_ZK_STATE,
            m_zkHandler->isValid()?"valid":"invalid");
        UNISTOR_MONITOR_APPEND();
        //zookeeper�Ĵ���
        if (!m_zkHandler->isValid()){
            m_zkHandler->getErrMsg(strValue);
            CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_ZK_ERROR, strValue.c_str());
            UNISTOR_MONITOR_APPEND();
        }
        //cache��״̬
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_CACHE_STATE,
            m_store->getStoreEngine()->isCacheValid()?"valid":"invalid");
        UNISTOR_MONITOR_APPEND();
        //cache�Ĵ�����Ϣ
        if (!m_store->getStoreEngine()->isCacheValid()){
            CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_CACHE_ERR, m_store->getStoreEngine()->getCacheErrMsg());
            UNISTOR_MONITOR_APPEND();
        }
        //cache��write key������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_WRITE_CACHE_KEY,
            m_store->getStoreEngine()->getWriteCacheKeyNum());
        UNISTOR_MONITOR_APPEND();
        //cache��write key�Ŀռ�
        CwxCommon::snprintf(szLine, 4096, "STAT %s %u\r\n", UNISTOR_SYS_KEY_WRITE_CACHE_SPACE,
            m_store->getStoreEngine()->getWriteCacheUsedSize());
        UNISTOR_MONITOR_APPEND();
        //read cache�Ŀռ�
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_MAX_SIZE,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheMaxSize(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache���������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_MAX_KEY,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheMaxKeyNum(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache��ʹ�ÿռ�
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_USED_SIZE,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheUsedSize(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache��ʹ������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_USED_CAPACITY,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheUsedCapacity(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache����������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_USED_DATA_SIZE,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheUsedDataSize(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache��free�ռ�
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_FREE_SIZE,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheFreeSize(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache��free����
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_FREE_CAPACITY,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheFreeCapacity(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        //read cache��cache��key������
        CwxCommon::snprintf(szLine, 4096, "STAT %s %s\r\n", UNISTOR_SYS_KEY_READ_CACHE_KEY,
            CwxCommon::toString((CWX_UINT64)m_store->getStoreEngine()->getReadCacheKeyCount(),szTmp, 10));
        UNISTOR_MONITOR_APPEND();
        CWX_UINT64 ullNum = 0;
        //get�ķ�������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GET_NUM,m_ullStatsGetNum);
        ///read cache������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GET_READ_CACHE_NUM,m_ullStatsGetReadCacheNum);
        ///get��ѯ���ڽ��������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GET_EXIST_NUM,m_ullStatsGetExistNum);
        ///gets�ķ�������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GETS_NUM,m_ullStatsGetsNum);
        ///gets��key������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GETS_KEY_NUM,m_ullStatsGetsKeyNum);
        ///gets��key��cache����
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GETS_KEY_READ_CACHE_NUM,m_ullStatsGetsKeyReadCacheNum);
        ///gets��key�Ĵ��ڵ�����
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_GETS_KEY_EXIST_NUM,m_ullStatsGetsKeyExistNum);
        ///list������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_LIST_NUM,m_ullStatsListNum);
        ///exist������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_EXIST_NUM,m_ullStatsExistNum);
        ///exist��read cache������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_EXIST_READ_CACHE_NUM,m_ullStatsExistReadCacheNum);
        ///exist�Ĵ�������
        UNISTOR_STATS_READ_THREAD(UNISTOR_SYS_KEY_EXIST_EXIST_NUM,m_ullStatsExistExistNum);
        ///add������
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_ADD_NUM,m_ullStatsAddNum);
        ///add��read cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_ADD_READ_CACHE_NUM,m_ullStatsAddReadCacheNum);
        ///add��write cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_ADD_WRITE_CACHE_NUM,m_ullStatsAddWriteCacheNum);
        ///set������
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_SET_NUM,m_ullStatsSetNum);
        ///set��read cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_SET_READ_CACHE_NUM,m_ullStatsSetReadCacheNum);
        ///set��write cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_SET_WRITE_CACHE_NUM,m_ullStatsSetWriteCacheNum);
        ///update������
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_UPDATE_NUM,m_ullStatsUpdateNum);
        ///update��read cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_UPDATE_READ_CACHE_NUM,m_ullStatsUpdateReadCacheNum);
        ///update��write cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_UPDATE_WRITE_CACHE_NUM,m_ullStatsUpdateWriteCacheNum);
        ///inc������
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_INC_NUM,m_ullStatsIncNum);
        ///inc��read cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_INC_READ_CACHE_NUM,m_ullStatsIncReadCacheNum);
        ///inc��write cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_INC_WRITE_CACHE_NUM,m_ullStatsIncWriteCacheNum);
        ///del������
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_DEL_NUM,m_ullStatsDelNum);
        ///del��read cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_DEL_READ_CACHE_NUM,m_ullStatsDelReadCacheNum);
        ///del��write cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_DEL_WRITE_CACHE_NUM,m_ullStatsDelWriteCacheNum);
        ///import������
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_IMPORT_NUM,m_ullStatsImportNum);
        ///import��read cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_IMPORT_READ_CACHE_NUM,m_ullStatsImportReadCacheNum);
        ///import��write cache����
        UNISTOR_STATS_WRITE_THREAD(UNISTOR_SYS_KEY_IMPORT_WRITE_CACHE_NUM,m_ullStatsImportWriteCacheNum);

	}while(0);
	strcpy(m_szBuf + uiPos, "END\r\n");
	return strlen(m_szBuf);

}

#define UNISTOR_STATS_READ_OUTPUT(attr)\
    ullNum = 0;\
    for (i=0; i<app->m_config.getCommon().m_uiThreadNum; i++){\
    tss = (UnistorTss*)app->getThreadPoolMgr()->getTss(THREAD_GROUP_RECV_BASE + i, 0);\
    ullNum += tss->attr;\
    };\
    CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString(ullNum, szTmp, 10));\


#define UNISTOR_STATS_WRITE_OUTPUT(attr)\
    tss = (UnistorTss*)app->getThreadPoolMgr()->getTss(THREAD_GROUP_WRITE, 0);\
    ullNum = tss->attr;\
    CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString(ullNum, szTmp, 10));\

//��ȡϵͳkey��1���ɹ���0�������ڣ�-1��ʧ��;
int UnistorApp::getSysKey(void* pApp, ///<app����
                          char const* key, ///<Ҫ��ȡ��key
                          CWX_UINT16 unKeyLen, ///<key�ĳ���
                          char* szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
                          CWX_UINT32& uiLen  ///<szData���ݵ��ֽ���
                          )
{
    UnistorApp* app=(UnistorApp*)pApp;
    string strKey(key, unKeyLen);
    string strValue;
    char szTmp[64];
    CWX_UINT32 i=0;
    UnistorTss * tss=NULL;
    CWX_UINT64 ullNum = 0;
    if (strKey == UNISTOR_SYS_KEY_PID){
        CwxCommon::snprintf(szData, uiLen, "%d", getpid());
    }else if (strKey == UNISTOR_SYS_KEY_PPID){
        CwxCommon::snprintf(szData, uiLen, "%d", getppid());
    }else if (strKey == UNISTOR_SYS_KEY_VERSION){
        CwxCommon::snprintf(szData, uiLen, "%s", app->getAppVersion().c_str());
    }else if (strKey == UNISTOR_SYS_KEY_MODIFY){
        CwxCommon::snprintf(szData, uiLen, "%s", app->getLastModifyDatetime().c_str());
    }else if (strKey == UNISTOR_SYS_KEY_COMPILE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->getLastCompileDatetime().c_str());
    }else if (strKey == UNISTOR_SYS_KEY_START){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_strStartTime.c_str());
    }else if (strKey == UNISTOR_SYS_KEY_ENGINE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->getConfig().getCommon().m_strStoreType.c_str());
    }else if (strKey == UNISTOR_SYS_KEY_ENGINE_VERSION){
        CwxCommon::snprintf(szData, uiLen, "%s", app->getStore()->getVersion());
    }else if (strKey == UNISTOR_SYS_KEY_ENGINE_STATE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->isValid()?"valid":"invalid");
    }else if (strKey == UNISTOR_SYS_KEY_ENGINE_ERROR){
        if (!app->m_store->isValid())
            CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getErrMsg());
        else
            szData[0]=0x00;
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_STATE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getBinLogMgr()->isInvalid()?"invalid":"valid");
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_ERROR){
        if (app->m_store->getBinLogMgr()->isInvalid()){
            CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getBinLogMgr()->getInvalidMsg());
        }else{
            szData[0] = 0x00;
        }
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_MIN_SID){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString(app->m_store->getBinLogMgr()->getMinSid(), szTmp));
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_MIN_SID){
        CwxDate::getDate(app->m_store->getBinLogMgr()->getMinTimestamp(), strValue);
        CwxCommon::snprintf(szData, uiLen, "%s", strValue.c_str());
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_MIN_FILE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getBinLogMgr()->getMinFile(strValue).c_str());
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_MAX_SID){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString(app->m_store->getBinLogMgr()->getMaxSid(), szTmp));
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_MAX_TIMESTAMP){
        CwxDate::getDate(app->m_store->getBinLogMgr()->getMaxTimestamp(), strValue);
        CwxCommon::snprintf(szData, uiLen, "%s", strValue.c_str());
    }else if (strKey == UNISTOR_SYS_KEY_BINLOG_MAX_FILE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getBinLogMgr()->getMaxFile(strValue).c_str());
    }else if (strKey == UNISTOR_SYS_KEY_READ_THREAD_NUM){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_config.getCommon().m_uiThreadNum);
    }else if (strKey == UNISTOR_SYS_KEY_READ_THREAD_QUEUE){
        CWX_UINT32 uiNum = 0;
        for (i =0; i<app->m_config.getCommon().m_uiThreadNum; i++)
            uiNum += app->m_recvThreadPool[i]->getQueuedMsgNum();
        CwxCommon::snprintf(szData, uiLen, "%u", uiNum);
    }else if (strKey == UNISTOR_SYS_KEY_READ_THREAD_CONNECT){
        CWX_UINT32 uiNum = 0;
        for (i =0; i<app->m_config.getCommon().m_uiThreadNum; i++){
            tss = (UnistorTss*)(app->getThreadPoolMgr()->getTss(THREAD_GROUP_RECV_BASE + i, 0));
            uiNum += ((UnistorRecvThreadUserObj*)tss->getUserObj())->getConnNum();
        }
        CwxCommon::snprintf(szData, uiLen, "%u", uiNum);
    }else if (strKey.substr(0, strlen(UNISTOR_SYS_KEY_READ_THREAD_QUEUE_PREX))==UNISTOR_SYS_KEY_READ_THREAD_QUEUE_PREX){
        i = strtoul(strKey.substr(0, strlen(UNISTOR_SYS_KEY_READ_THREAD_QUEUE_PREX)).c_str(), NULL, 0);
        if (i >= app->m_config.getCommon().m_uiThreadNum) return 0;
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_recvThreadPool[i]->getQueuedMsgNum());
    }else if (strKey.substr(0, strlen(UNISTOR_SYS_KEY_READ_THREAD_CONNECT_PREX))==UNISTOR_SYS_KEY_READ_THREAD_CONNECT_PREX){
        i = strtoul(strKey.substr(0, strlen(UNISTOR_SYS_KEY_READ_THREAD_CONNECT_PREX)).c_str(), NULL, 0);
        if (i >= app->m_config.getCommon().m_uiThreadNum) return 0;
        tss = (UnistorTss*)(app->getThreadPoolMgr()->getTss(THREAD_GROUP_RECV_BASE + i, 0));
        CwxCommon::snprintf(szData, uiLen, "%u", ((UnistorRecvThreadUserObj*)tss->getUserObj())->getConnNum());
    }else if (strKey == UNISTOR_SYS_KEY_WRITE_THREAD_QUEUE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_writeThreadPool->getQueuedMsgNum());
    }else if (strKey == UNISTOR_SYS_KEY_TRANS_THREAD_QUEUE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_transThreadPool->getQueuedMsgNum());
    }else if (strKey == UNISTOR_SYS_KEY_CHECKPOINT_THREAD_QUEUE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_checkpointThreadPool->getQueuedMsgNum());
    }else if (strKey == UNISTOR_SYS_KEY_ZK_THREAD_QUEUE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_zkThreadPool->getQueuedMsgNum());
    }else if (strKey == UNISTOR_SYS_KEY_INNER_SYNC_THREAD_QUEUE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_innerSyncThreadPool->getQueuedMsgNum());
    }else if (strKey == UNISTOR_SYS_KEY_OUTER_SYNC_THREAD_QUEUE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_outerSyncThreadPool->getQueuedMsgNum());
    }else if (strKey == UNISTOR_SYS_KEY_MASTER_TRANS_MSG_NUM){
        CwxCommon::snprintf(szData, uiLen, "%u", app->getTaskBoard().getTaskNum());
    }else if (strKey == UNISTOR_SYS_KEY_ZK_STATE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_zkHandler->isValid()?"valid":"invalid");
    }else if (strKey == UNISTOR_SYS_KEY_ZK_ERROR){
        if (app->m_zkHandler->isValid()){
            szData[0] = 0X00;
        }else{
            app->m_zkHandler->getErrMsg(strValue);
            CwxCommon::snprintf(szData, uiLen, "%s", strValue.c_str());
        }
    }else if (strKey == UNISTOR_SYS_KEY_CACHE_STATE){
        CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getStoreEngine()->isCacheValid()?"valid":"invalid");
    }else if (strKey == UNISTOR_SYS_KEY_CACHE_STATE){
        if (app->m_store->getStoreEngine()->isCacheValid()){
            szData[0] = 0X00;
        }else{
            CwxCommon::snprintf(szData, uiLen, "%s", app->m_store->getStoreEngine()->getCacheErrMsg());
        }
    }else if (strKey == UNISTOR_SYS_KEY_WRITE_CACHE_KEY){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_store->getStoreEngine()->getWriteCacheKeyNum());
    }else if (strKey == UNISTOR_SYS_KEY_WRITE_CACHE_SPACE){
        CwxCommon::snprintf(szData, uiLen, "%u", app->m_store->getStoreEngine()->getWriteCacheUsedSize());
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_MAX_SIZE){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheMaxSize(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_MAX_KEY){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheMaxKeyNum(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_USED_SIZE){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheUsedSize(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_USED_CAPACITY){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheUsedCapacity(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_USED_DATA_SIZE){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheUsedDataSize(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_FREE_SIZE){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheFreeSize(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_FREE_CAPACITY){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheFreeCapacity(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_READ_CACHE_KEY){
        CwxCommon::snprintf(szData, uiLen, "%s", CwxCommon::toString((CWX_UINT64)app->m_store->getStoreEngine()->getReadCacheKeyCount(),szTmp, 10));
    }else if (strKey == UNISTOR_SYS_KEY_GET_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetNum);
    }else if (strKey == UNISTOR_SYS_KEY_GET_READ_CACHE_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_GET_EXIST_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetExistNum);
    }else if (strKey == UNISTOR_SYS_KEY_GETS_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetsNum);
    }else if (strKey == UNISTOR_SYS_KEY_GETS_KEY_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetsKeyNum);
    }else if (strKey == UNISTOR_SYS_KEY_GETS_KEY_READ_CACHE_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetsKeyReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_GETS_KEY_EXIST_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsGetsKeyExistNum);
    }else if (strKey == UNISTOR_SYS_KEY_LIST_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsListNum);
    }else if (strKey == UNISTOR_SYS_KEY_EXIST_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsExistNum);
    }else if (strKey == UNISTOR_SYS_KEY_EXIST_READ_CACHE_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsExistReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_EXIST_EXIST_NUM){
        UNISTOR_STATS_READ_OUTPUT(m_ullStatsExistExistNum);
    }else if (strKey == UNISTOR_SYS_KEY_ADD_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsAddNum);
    }else if (strKey == UNISTOR_SYS_KEY_ADD_READ_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsAddReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_ADD_WRITE_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsAddWriteCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_SET_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsSetNum);
    }else if (strKey == UNISTOR_SYS_KEY_SET_READ_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsSetReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_SET_WRITE_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsSetWriteCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_UPDATE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsUpdateNum);
    }else if (strKey == UNISTOR_SYS_KEY_UPDATE_READ_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsUpdateReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_UPDATE_WRITE_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsUpdateWriteCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_INC_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsIncNum);
    }else if (strKey == UNISTOR_SYS_KEY_INC_READ_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsIncReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_INC_WRITE_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsIncWriteCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_DEL_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsDelNum);
    }else if (strKey == UNISTOR_SYS_KEY_DEL_READ_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsDelReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_DEL_WRITE_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsDelWriteCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_IMPORT_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsImportNum);
    }else if (strKey == UNISTOR_SYS_KEY_IMPORT_READ_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsImportReadCacheNum);
    }else if (strKey == UNISTOR_SYS_KEY_IMPORT_WRITE_CACHE_NUM){
        UNISTOR_STATS_WRITE_OUTPUT(m_ullStatsImportWriteCacheNum);
    }else{
        return 0;
    }

    uiLen = strlen(szData);
    return 1;

}


///�ַ�channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
int UnistorApp::dealRecvThreadQueue(UnistorTss* tss,
                                    CwxMsgQueue* queue,
                                    CWX_UINT32 uiQueueIndex,
                                    UnistorApp* app,
                                    CwxAppChannel*)
{
    int iRet = 0;
    CwxMsgBlock* block = NULL;
    while (!queue->isEmpty()){
        do{
            iRet = queue->dequeue(block);
            if (-1 == iRet) return -1;
            CWX_ASSERT(block->event().getSvrId() == SVR_TYPE_RECV);
            UnistorHandler4Recv::doEvent(app, tss, block, uiQueueIndex);
        } while(0);
        if (block) CwxMsgBlockAlloc::free(block);
        block = NULL;
    }
    if (queue->isDeactived()) return -1;
    return 0;
}


///�ַ�channel���̺߳�����argΪapp����
void* UnistorApp::recvThreadMain(CwxTss* tss, CwxMsgQueue* queue, void* arg)
{
	pair<UnistorApp*, CWX_UINT32>* item = (pair<UnistorApp*, CWX_UINT32>*)arg;
    if (0 != item->first->m_recvChannel[item->second]->open()){
        CWX_ERROR(("Failure to open unistor query channel, index=%d. exit......",item->second));
        ///ֹͣ����
        item->first->stop();
        return NULL;
    }
    ((UnistorTss*)tss)->m_uiThreadIndex = item->second;
    while(1) {
        //��ȡ�����е���Ϣ������
        if (0 != dealRecvThreadQueue((UnistorTss*)tss, queue, item->second,  item->first, item->first->m_recvChannel[item->second])) break;
        if (-1 == item->first->m_recvChannel[item->second]->dispatch(1)){
            CWX_ERROR(("Failure to invoke kv query channel CwxAppChannel::dispatch(), index=%d",item->second));
            ::sleep(1);
        }
    }
    item->first->m_recvChannel[item->second]->stop();
    item->first->m_recvChannel[item->second]->close();
    if (!item->first->isStopped()) {
        CWX_INFO(("Stop app for unistor query channel thread stopped."));
        item->first->stop();
    }
    return NULL;
}

///�ַ�channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
int UnistorApp::dealInnerSyncThreadQueue(UnistorTss* tss,
                                         CwxMsgQueue* queue,
                                         UnistorApp* app,
                                         CwxAppChannel* )
{
    int iRet = 0;
    CwxMsgBlock* block = NULL;
    while (!queue->isEmpty()){
        do {
            iRet = queue->dequeue(block);
            if (-1 == iRet) return -1;
            CWX_ASSERT(block->event().getSvrId() == SVR_TYPE_INNER_SYNC);
            UnistorHandler4Dispatch::doEvent(app, tss, block);
        } while(0);
        if (block) CwxMsgBlockAlloc::free(block);
        block = NULL;
    }
    if (queue->isDeactived()) return -1;
    return 0;
}

///�ַ�channel���̺߳�����argΪapp����
void* UnistorApp::innerSyncThreadMain(CwxTss* pThr, CwxMsgQueue* queue, void* arg){
	UnistorApp* app = (UnistorApp*) arg;
    UnistorTss* tss = (UnistorTss*)pThr;
	if (0 != app->getInnerSyncChannel()->open()){
		CWX_ERROR(("Failure to open inner sync channel, exit...."));
        app->stop();
		return NULL;
	}
	while(1){
		//��ȡ�����е���Ϣ������
		if (0 != dealInnerSyncThreadQueue(tss, queue, app, app->getInnerSyncChannel())) break;
		if (-1 == app->getInnerSyncChannel()->dispatch(1)){
			CWX_ERROR(("Failure to invoke inner sync channel CwxAppChannel::dispatch()"));
            ::sleep(1);
		}
        UnistorHandler4Dispatch::dealClosedSession(app, tss);
	}
	app->getInnerSyncChannel()->stop();
	app->getInnerSyncChannel()->close();
	if (!app->isStopped()){
		CWX_INFO(("Stop app for inner sync channel thread stopped."));
		app->stop();
	}
    ///�ͷ��߳���Դ
    ((UnistorDispatchThreadUserObj*)tss->getUserObj())->free(app);
	return NULL;
}

///�ַ�channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
int UnistorApp::dealOuterSyncThreadQueue(UnistorTss* tss,
                                         CwxMsgQueue* queue,
                                         UnistorApp* app,
                                         CwxAppChannel* )
{
    int iRet = 0;
    CwxMsgBlock* block = NULL;
    while (!queue->isEmpty()){
        do {
            iRet = queue->dequeue(block);
            if (-1 == iRet) return -1;
            CWX_ASSERT(block->event().getSvrId() == SVR_TYPE_OUTER_SYNC);
            UnistorHandler4Dispatch::doEvent(app, tss, block);
        } while(0);
        CwxMsgBlockAlloc::free(block);
        block = NULL;
    }
    if (queue->isDeactived()) return -1;
    return 0;
}


///�ⲿ�ַ�channel���̺߳�����argΪapp����
void* UnistorApp::outerSyncThreadMain(CwxTss* pThr, CwxMsgQueue* queue, void* arg){
    UnistorApp* app = (UnistorApp*) arg;
    UnistorTss* tss = (UnistorTss*)pThr;
    if (0 != app->getOuterSyncChannel()->open()){
        CWX_ERROR(("Failure to open outer sync channel, exit...."));
        app->stop();
        return NULL;
    }
    while(1){
        //��ȡ�����е���Ϣ������
        if (0 != dealOuterSyncThreadQueue((UnistorTss*)tss, queue, app, app->getOuterSyncChannel())) break;
        if (-1 == app->getOuterSyncChannel()->dispatch(1)){
            CWX_ERROR(("Failure to invoke outer sync channel CwxAppChannel::dispatch()"));
            ::sleep(1);
        }
        UnistorHandler4Dispatch::dealClosedSession(app, tss);
    }
    app->getOuterSyncChannel()->stop();
    app->getOuterSyncChannel()->close();
    if (!app->isStopped()){
        CWX_INFO(("Stop app for outer sync channel thread stopped."));
        app->stop();
    }
    ///�ͷ��߳���Դ
    ((UnistorDispatchThreadUserObj*)tss->getUserObj())->free(app);
    return NULL;
}

///�ַ�channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
int UnistorApp::dealTransThreadQueue(UnistorTss* tss,
                                     CwxMsgQueue* queue,
                                     UnistorApp* app,
                                     CwxAppChannel* )
{
    int iRet = 0;
    CwxMsgBlock* block = NULL;
    while (!queue->isEmpty()){
        do {
            iRet = queue->dequeue(block);
            if (-1 == iRet) return -1;
            CWX_ASSERT(block->event().getSvrId() == SVR_TYPE_TRANSFER);
            UnistorHandler4Trans::doEvent(app, tss, block);
        } while(0);
        if (block) CwxMsgBlockAlloc::free(block);
        block = NULL;
    }
    if (queue->isDeactived()) return -1;
    return 0;
}

///�ַ�channel���̺߳�����argΪapp����
void* UnistorApp::transThreadMain(CwxTss* tss, CwxMsgQueue* queue, void* arg){
    UnistorApp* app = (UnistorApp*) arg;
    if (0 != app->getTransChannel()->open()){
        CWX_ERROR(("Failure to open trans channel, exit...."));
        app->stop();
        return NULL;
    }
    while(1){
        //��ȡ�����е���Ϣ������
        if (0 != dealTransThreadQueue((UnistorTss*) tss, queue, app, app->getTransChannel())) break;
        if (-1 == app->getTransChannel()->dispatch(1)){
            CWX_ERROR(("Failure to invoke trans channel CwxAppChannel::dispatch()"));
            ::sleep(1);
        }
    }
    app->getTransChannel()->stop();
    app->getTransChannel()->close();
    if (!app->isStopped()){
        CWX_INFO(("Stop app for trans channel thread stopped."));
        app->stop();
    }

    return NULL;
}


///�ⲿ�ַ�channel���̺߳�����argΪapp����
void* UnistorApp::zkThreadMain(CwxTss* tss, CwxMsgQueue* queue, void* arg){
    UnistorApp* app = (UnistorApp*) arg;
    CwxMsgBlock* block = NULL;
    int ret = 0;
    while(-1 != (ret = queue->dequeue(block))){
        app->getZkHandler()->doEvent(tss, block, ret);
        if (block) CwxMsgBlockAlloc::free(block);
        block = NULL;
    }
    if (!app->isStopped()){
        CWX_INFO(("Stop app for zk thread is stopped."));
        app->stop();
    }
    return NULL;

}

///����master recv���ӵ�����
int UnistorApp::setConnSockAttr(CWX_HANDLE handle, void* arg){
    UnistorConnAttr* attr = (UnistorConnAttr*)arg;
    if (attr->m_bNoDelay){
        int flags= 1;
        if (setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags)) != 0){
            CWX_ERROR(("Failure to set io TCP_NODELAY, errno=%d", errno));
        }
    }
    if (attr->m_bLinger){
        struct linger ling= {0, 0};
        if (setsockopt(handle, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling)) != 0){
            CWX_ERROR(("Failure to set io SO_LINGER, errno=%d", errno));
        }
    }
    if (attr->m_bKeepalive){
        if (0 != CwxSocket::setKeepalive(handle,
            true,
            CWX_APP_DEF_KEEPALIVE_IDLE,
            CWX_APP_DEF_KEEPALIVE_INTERNAL,
            CWX_APP_DEF_KEEPALIVE_COUNT))
        {
            CWX_ERROR(("Failure to set io Keepalive, errno=%d", errno));
        }
    }
    if (attr->m_uiSendBuf){
        int iSockBuf = (attr->m_uiSendBuf + 1023)/1024;
        iSockBuf *= 1024;
        while (setsockopt(handle, SOL_SOCKET, SO_SNDBUF, (void*)&iSockBuf, sizeof(iSockBuf)) < 0){
            iSockBuf -= 1024;
            if (iSockBuf <= 1024) break;
        }
    }
    if (attr->m_uiRecvBuf){
        int iSockBuf = (attr->m_uiRecvBuf + 1023)/1024;
        iSockBuf *= 1024;
        while(setsockopt(handle, SOL_SOCKET, SO_RCVBUF, (void *)&iSockBuf, sizeof(iSockBuf)) < 0){
            iSockBuf -= 1024;
            if (iSockBuf <= 1024) break;
        }
    }
    return 0;
}



int UnistorApp::startNetwork(){
    ///�������ӵ�����
    m_recvSvrSockAttr.m_bNoDelay = m_recvCliSockAttr.m_bNoDelay = true;
    m_recvSvrSockAttr.m_bLinger = m_recvCliSockAttr.m_bLinger = true;
    m_recvSvrSockAttr.m_bKeepalive = m_recvCliSockAttr.m_bKeepalive = true;

    m_syncSvrSockAttr.m_bNoDelay = m_syncCliSockAttr.m_bNoDelay = true;
    m_syncSvrSockAttr.m_bLinger = m_syncCliSockAttr.m_bLinger = true;
    m_syncSvrSockAttr.m_bKeepalive = m_syncCliSockAttr.m_bKeepalive = true;
    m_syncSvrSockAttr.m_uiRecvBuf = m_syncCliSockAttr.m_uiSendBuf = 0;
    m_syncSvrSockAttr.m_uiSendBuf = m_syncCliSockAttr.m_uiRecvBuf = m_config.getCommon().m_uiSockBufSize * 1024;

	///����monitor�ļ���
	if (m_config.getCommon().m_monitor.getHostName().length()){
		if (0 > this->noticeTcpListen(SVR_TYPE_MONITOR,
			m_config.getCommon().m_monitor.getHostName().c_str(),
			m_config.getCommon().m_monitor.getPort(),
			true)){
			CWX_ERROR(("Can't register the monitor tcp accept listen: addr=%s, port=%d",
				m_config.getCommon().m_monitor.getHostName().c_str(),
				m_config.getCommon().m_monitor.getPort()));
			return -1;
		}
	}
	///�������ݸ��¡���ѯlisten
    if (0 > this->noticeTcpListen(SVR_TYPE_RECV, 
        m_config.getRecv().getHostName().c_str(),
        m_config.getRecv().getPort(),
        false,
        CWX_APP_EVENT_MODE,
        UnistorApp::setConnSockAttr,
        &m_recvSvrSockAttr))
    {
        CWX_ERROR(("Can't register the tcp accept listen: addr=%s, port=%d",
            m_config.getRecv().getHostName().c_str(),
            m_config.getRecv().getPort()));
        return -1;
    }

    ///�����ڲ��ַ�
    if (0 > this->noticeTcpListen(SVR_TYPE_INNER_SYNC, 
        m_config.getInnerDispatch().getHostName().c_str(),
        m_config.getInnerDispatch().getPort(),
        false,
        CWX_APP_EVENT_MODE,
        UnistorApp::setConnSockAttr,
        &m_syncSvrSockAttr))
    {
        CWX_ERROR(("Can't register the inner-dispatch tcp accept listen: addr=%s, port=%d",
            m_config.getInnerDispatch().getHostName().c_str(),
            m_config.getInnerDispatch().getPort()));
        return -1;
    }

    ///�ⲿ�����ַ�
    if (0 > this->noticeTcpListen(SVR_TYPE_OUTER_SYNC, 
        m_config.getOuterDispatch().getHostName().c_str(),
        m_config.getOuterDispatch().getPort(),
        false,
        CWX_APP_EVENT_MODE,
        UnistorApp::setConnSockAttr,
        &m_syncSvrSockAttr))
    {
        CWX_ERROR(("Can't register the outer-dispatch tcp accept listen: addr=%s, port=%d",
            m_config.getOuterDispatch().getHostName().c_str(),
            m_config.getOuterDispatch().getPort()));
        return -1;
    }
	return 0;
}

///�洢��������Ϣͨ��
int UnistorApp::storeMsgPipe(void* app,
                             CwxMsgBlock* msg,
                             bool bWriteThread,
                             char* szErr2K)
{
    UnistorApp* pApp = (UnistorApp*)app;
    if (bWriteThread){
        if (pApp->m_writeThreadPool){
            msg->event().setSvrId(SVR_TYPE_RECV_WRITE);
            if (pApp->m_writeThreadPool->append(msg) < 0){
                if (szErr2K) strcpy(szErr2K, "Failure to push store msg to write thread pool.");
                return -1;
            }
        }else{
            return -1;
        }
    }else{
        if (pApp->m_checkpointThreadPool){
            msg->event().setSvrId(SVR_TYPE_CHECKPOINT);
            if (pApp->m_checkpointThreadPool->append(msg) < 0){
                if (szErr2K) strcpy(szErr2K, "Failure to push store msg to checkpoint thread pool.");
                return -1;
            }
        }else{
            return -1;
        }
    }
    return 0;
}
