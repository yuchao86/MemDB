#include "CwxEchoClientApp.h"

///���캯������ʼ�����͵�echo��������
CwxEchoClientApp::CwxEchoClientApp():m_uiSendNum(0){
    CWX_UINT32 i=0;
    for (i=0; i<sizeof(m_szBuf100K) - 1; i++){
        m_szBuf100K[i] = 'a' + i % 26;
    }
    m_szBuf100K[i] = 0x00;
    m_uiRecvNum = 0;

}
///��������
CwxEchoClientApp::~CwxEchoClientApp(){

}

///��ʼ��APP�����������ļ�
int CwxEchoClientApp::init(int argc, char** argv){
    string strErrMsg;
    ///���ȵ��üܹ���init
    if (CwxAppFramework::init(argc, argv) == -1) return -1;
    ///��û��ͨ��-fָ�������ļ��������Ĭ�ϵ������ļ�
    if ((NULL == this->getConfFile()) || (strlen(this->getConfFile()) == 0)){
        this->setConfFile("svr_conf.cnf");
    }
    ///���������ļ�
    if (0 != m_config.loadConfig(getConfFile())){
        CWX_ERROR((m_config.getError()));
        return -1;
    }
    if (m_config.m_uiDataSize > sizeof(m_szBuf100K) -1) m_config.m_uiDataSize = sizeof(m_szBuf100K) -1;
    ///�������������־��level
    setLogLevel(CwxLogger::LEVEL_ERROR|CwxLogger::LEVEL_INFO|CwxLogger::LEVEL_WARNING);
    return 0;
}

//init the Enviroment before run.0:success, -1:failure.
int CwxEchoClientApp::initRunEnv(){
    ///����ʱ�ӵĿ̶ȣ���СΪ1ms����Ϊ1s��
    this->setClick(1000);//1s
    //set work dir
    this->setWorkDir(this->m_config.m_strWorkDir.c_str());
    //Set log file
    this->setLogFileNum(LOG_FILE_NUM);
    this->setLogFileSize(LOG_FILE_SIZE*1024*1024);
    ///���üܹ���initRunEnv��ʹ���õĲ�����Ч
    if (CwxAppFramework::initRunEnv() == -1 ) return -1;
    //set version
    this->setAppVersion(ECHO_CLIENT_APP_VERSION);
    //set last modify date
    this->setLastModifyDatetime(ECHO_CLIENT_MODIFY_DATE);
    //set compile date
    this->setLastCompileDatetime(CWX_COMPILE_DATE(_BUILD_DATE));

    //output config
    string strConfOut;
    m_config.outputConfig(strConfOut);
    CWX_INFO((strConfOut.c_str()));


    CWX_UINT16 i=0;
    //���������ļ������õġ���echo���������
    for (i=0; i<m_config.m_unConnNum; i++){
        if (m_config.m_bTcp){
            //create  conn
            if (0 > this->noticeTcpConnect(SVR_TYPE_ECHO,
                0,
                this->m_config.m_listen.getHostName().c_str(),
                this->m_config.m_listen.getPort(),
                false,
                1,
                60,
                CwxEchoClientApp::setSockAttr,
                this))
            {
                CWX_ERROR(("Can't connect the echo service: addr=%s, port=%d",
                    this->m_config.m_listen.getHostName().c_str(),
                    this->m_config.m_listen.getPort()));
                return -1;
            }
        }else{
            //create  conn
            if (0 > this->noticeLsockConnect(SVR_TYPE_ECHO,
                0,
                this->m_config.m_strUnixPathFile.c_str()))
            {
                CWX_ERROR(("Can't connect the echo service: addr=%s, port=%d",
                    this->m_config.m_listen.getHostName().c_str(),
                    this->m_config.m_listen.getPort()));
                return -1;
            }
        }
    }
    return 0;
}

///ʱ����Ӧ������ʲôҲû����
void CwxEchoClientApp::onTime(CwxTimeValue const& current){
    CwxAppFramework::onTime(current);
}

///�źŴ�����
void CwxEchoClientApp::onSignal(int signum){
    switch(signum){
    case SIGQUIT: 
        CWX_INFO(("Recv exit signal, exit right now."));
        this->stop();
        break;
    default:
        ///�����źţ�����
        CWX_INFO(("Recv signal=%d, ignore it.", signum));
        break;
    }
}

///echo��������ӽ�����Ӧ����
int CwxEchoClientApp::onConnCreated(CwxAppHandler4Msg& conn, bool& , bool& ){
    ///����һ��echo���ݰ�
    sendNextMsg(conn.getConnInfo().getSvrId(),
        conn.getConnInfo().getHostId(),
        conn.getConnInfo().getConnId());
    return 0;
}

///echo�ظ�����Ϣ��Ӧ����
int CwxEchoClientApp::onRecvMsg(CwxMsgBlock* msg, CwxAppHandler4Msg& conn, CwxMsgHead const& header, bool& bSuspendConn){

    msg->event().setSvrId(conn.getConnInfo().getSvrId());
    msg->event().setHostId(conn.getConnInfo().getHostId());
    msg->event().setConnId(conn.getConnInfo().getConnId());
    msg->event().setIoHandle(conn.getHandle());
    msg->event().setConnUserData(NULL);
    msg->event().setEvent(CwxEventInfo::RECV_MSG);
    msg->event().setMsgHeader(header);
    msg->event().setTimestamp(CwxDate::getTimestamp());
    bSuspendConn = false;
    ///�ͷ��յ������ݰ�
    if (msg) CwxMsgBlockAlloc::free(msg);
    if (m_config.m_bLasting){///����ǳ־����ӣ�������һ��echo�������ݰ�
        sendNextMsg(conn.getConnInfo().getSvrId(),
            conn.getConnInfo().getHostId(),
            conn.getConnInfo().getConnId());
    }else{
        ///�����ǳ־����ӣ����������ӡ�
        this->noticeReconnect(conn.getConnInfo().getConnId());
    }
    ///�յ���echo���ݼ�1
    m_uiRecvNum++;
    ///���յ�10000�����ݰ��������һ����־
    if (!(m_uiRecvNum%10000)){
        CWX_INFO(("Finish num=%u\n", m_uiRecvNum));
    }
    return 0;
}
///����echo��ѯ
void CwxEchoClientApp::sendNextMsg(CWX_UINT32 uiSvrId, CWX_UINT32 uiHostId, CWX_UINT32 uiConnId){
    CwxMsgHead header;
    ///����echo����Ϣ����
    header.setMsgType(SEND_MSG_TYPE);
    ///����echo�����ݰ�����
    header.setDataLen(m_config.m_uiDataSize);
    ///����echo���ݰ���taskid����ʹ�÷��͵��������кţ���ǰû��
    header.setTaskId(m_uiSendNum);
    ///���䷢����Ϣ����block
    CwxMsgBlock* pBlock = CwxMsgBlockAlloc::malloc(m_config.m_uiDataSize + CwxMsgHead::MSG_HEAD_LEN);
    ///������Ϣͷ
    memcpy(pBlock->wr_ptr(), header.toNet(), CwxMsgHead::MSG_HEAD_LEN);
    pBlock->wr_ptr(CwxMsgHead::MSG_HEAD_LEN);
    ///����echo������
    memcpy(pBlock->wr_ptr(), m_szBuf100K, m_config.m_uiDataSize);
    pBlock->wr_ptr(m_config.m_uiDataSize);
    ///������Ϣ�ķ��ͷ�ʽ
    ///������Ϣ��svr-id
    pBlock->send_ctrl().setSvrId(uiSvrId);
    ///������Ϣ��host-id
    pBlock->send_ctrl().setHostId(uiHostId);
    ///������Ϣ���͵�����id
    pBlock->send_ctrl().setConnId(uiConnId);
    ///������Ϣ���͵�user-data
    pBlock->send_ctrl().setUserData(NULL);
    ///������Ϣ���ͽ׶ε���Ϊ��������ʼ�����Ƿ�֪ͨ����������Ƿ�֪ͨ������ʧ���Ƿ�֪ͨ
    pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
    ///����echo����
    if (0 != this->sendMsgByConn(pBlock)){
        CWX_ERROR(("Failure to send msg"));
        return ;
    }
    ///������������+1
    m_uiSendNum++;
}

int CwxEchoClientApp::setSockAttr(CWX_HANDLE handle, void* arg)
{
    CwxEchoClientApp* app = (CwxEchoClientApp*) arg;
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

