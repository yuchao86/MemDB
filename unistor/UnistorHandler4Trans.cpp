#include "UnistorHandler4Trans.h"
#include "UnistorApp.h"
#include "UnistorTask4Trans.h"
#include "CwxZlib.h"
#include "UnistorConnector.h"


volatile bool UnistorHandler4Trans::m_bCanTrans=false;
string  UnistorHandler4Trans::m_strMasterHost="";
CWX_UINT32 UnistorHandler4Trans::m_uiMaxConnNum=0;
CWX_UINT32 UnistorHandler4Trans::m_uiAuthConnNum=0;
UnistorHandler4Trans**  UnistorHandler4Trans::m_authConn=0;
map<CWX_UINT32, UnistorHandler4Trans*>* UnistorHandler4Trans::m_handlers = NULL;
bool UnistorHandler4Trans::m_bRebuildConn=false;
CWX_UINT32  UnistorHandler4Trans::m_ttLastRebuildConn=0; ///<上次重建连接的时间


int UnistorHandler4Trans::open (void * arg){
    m_bAuth = false;
    int ret = CwxAppHandler4Channel::open(arg);
    if (0 == ret){
        UnistorTss* tss = (UnistorTss*)CwxTss::instance();
        ///发送认证消息
        CwxMsgBlock* block = NULL;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvAuth(tss->m_pWriter,
            block,
            0,
            m_pApp->getConfig().getRecv().getUser().length()?m_pApp->getConfig().getRecv().getUser().c_str():"",
            m_pApp->getConfig().getRecv().getPasswd().length()?m_pApp->getConfig().getRecv().getPasswd().c_str():"",
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack recv auth package, err=%s", tss->m_szBuf2K));
            ret = channel()->removeHandler(this);
            if (0 != ret){
                CWX_ERROR(("Failure to remove handler from channel"));
            }
            return -1;
        }
        (*m_handlers)[m_uiConnId] = this;
        block->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
        ///发送消息
        putMsg(block);
    }
    return 0;
}

int UnistorHandler4Trans::onConnClosed()
{
    list<CwxTaskBoardTask*> tasks;
    CwxTss* pThrEnv = CwxTss::instance();
    CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
    msg->event().setHostId(0);
    msg->event().setConnId(m_uiConnId);
    m_pApp->getTaskBoard().noticeConnClosed(msg, pThrEnv, tasks);
    if (!tasks.empty())
    {
        list<CwxTaskBoardTask*>::iterator iter = tasks.begin();
        while(iter != tasks.end()){
            (*iter)->execute(pThrEnv);
            iter++;
        }
        tasks.clear();
    }
    m_handlers->erase(m_uiConnId);
    m_bRebuildConn = true;
    m_bCanTrans = false;
    return -1;
}

int UnistorHandler4Trans::onInput()
{
    ///接受消息
    int ret = CwxAppHandler4Channel::recvPackage(getHandle(),
        m_uiRecvHeadLen,
        m_uiRecvDataLen,
        m_szHeadBuf,
        m_header,
        m_recvMsgData);
    ///如果没有接受完毕（0）或失败（-1），则返回
    if (1 != ret) return ret;
    ///接收到一个完整的数据包
    if (!m_bAuth){///第一个消息必须是认证回复消息
        int ret=UNISTOR_ERR_SUCCESS;
        char const* szErrMsg=NULL;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseRecvAuthReply(m_tss->m_pReader,
            m_recvMsgData, ret, szErrMsg, m_tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to auth, user=%s, passwd=%s, ret=%d, err=%s",
                m_pApp->getConfig().getRecv().getUser().c_str(),
                m_pApp->getConfig().getRecv().getPasswd().c_str(),
                ret,
                szErrMsg));
            m_bRebuildConn = true;
            return -1;
        }
        m_authConn[m_uiAuthConnNum] = this;
        m_uiAuthConnNum++;
        m_bAuth = true;
        m_bCanTrans = true; 
    }else{
        ///收到了回复消息
        CwxTaskBoardTask* pTask = NULL;
        m_pApp->getTaskBoard().noticeRecvMsg(m_header.getTaskId(),
            m_recvMsgData,
            m_tss,
            pTask);
        if (pTask) pTask->execute(m_tss);
    }
    if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
    this->m_recvMsgData = NULL;
    this->m_uiRecvHeadLen = 0;
    this->m_uiRecvDataLen = 0;

    return 0;
}

CWX_UINT32 UnistorHandler4Trans::onEndSendMsg(CwxMsgBlock*& msg)
{
    CwxTss* pThrEnv = CwxTss::instance();
    CwxTaskBoardTask* pTask = NULL;
    msg->event().setConnId(m_uiConnId);
    m_pApp->getTaskBoard().noticeEndSendMsg(msg->event().getTaskId(), msg, pThrEnv, pTask);
    if (pTask) pTask->execute(pThrEnv);
    return 0;
}

void UnistorHandler4Trans::onFailSendMsg(CwxMsgBlock*& msg)
{
    CwxTss* pThrEnv = CwxTss::instance();
    CwxTaskBoardTask* pTask = NULL;
    m_pApp->getTaskBoard().noticeFailSendMsg(msg->event().getTaskId(), msg, pThrEnv, pTask);
    if (pTask) pTask->execute(pThrEnv);
    m_bRebuildConn = true;
    m_bCanTrans = false;
}

void UnistorHandler4Trans::doEvent(UnistorApp* pApp, UnistorTss* tss, CwxMsgBlock*& msg)
{
    if (CwxEventInfo::RECV_MSG == msg->event().getEvent()){///转发消息
        if (!m_bCanTrans){
            UnistorTask4Trans::reply(pApp,
                tss, 
                msg->event().getConnId(),
                msg->event().getHostId(),
                msg->event().getMsgHeader(),
                NULL,
                UNISTOR_ERR_NO_MASTER,
                "No master.");
        }
        ///创建task
        UnistorTask4Trans* pTask = new UnistorTask4Trans(pApp, &pApp->getTaskBoard());
        pTask->m_tranMsg = msg;
        pTask->setTaskId(pApp->getTaskBoard().getNextTaskId());
        pTask->execute(tss);
        msg = NULL;
    }else if (CwxEventInfo::TIMEOUT_CHECK == msg->event().getEvent()){
        ///检测超时的转发任务
        list<CwxTaskBoardTask*> tasks;
        pApp->getTaskBoard().noticeCheckTimeout(tss, tasks);
        if (!tasks.empty())
        {
            list<CwxTaskBoardTask*>::iterator iter=tasks.begin();
            while(iter != tasks.end()){
                (*iter)->execute(tss);
                iter++;
            }
        }
        if (m_bRebuildConn)  UnistorHandler4Trans::rebuildConn(pApp);
    }else if (EVENT_ZK_CONF_CHANGE == msg->event().getEvent()){
        UnistorZkConf* pConf = NULL;
        memcpy(&pConf, msg->rd_ptr(), sizeof(pConf));
        if (tss->m_pZkConf){
            if (tss->m_pZkConf->m_ullVersion > pConf->m_ullVersion){///<采用旧版本
                delete pConf;
            }else{///采用新版本
                delete tss->m_pZkConf;
                tss->m_pZkConf = pConf;
            }
        }else{///<采用新版本
            tss->m_pZkConf = pConf;
        }
        checkTrans(pApp, tss);
    }else if (EVENT_ZK_LOCK_CHANGE == msg->event().getEvent()){
        UnistorZkLock* pLock = NULL;
        memcpy(&pLock, msg->rd_ptr(), sizeof(pLock));
        if (tss->m_pZkLock){
            if (tss->m_pZkLock->m_ullVersion > pLock->m_ullVersion){///<采用旧版本
                delete pLock;
            }else{///采用新版本
                delete tss->m_pZkLock;
                tss->m_pZkLock = pLock;
            }
        }else{///<采用新版本
            tss->m_pZkLock = pLock;
        }
        checkTrans(pApp, tss);
    }else{
        CWX_ERROR(("UnKnown event type:%u", msg->event().getEvent()));
    }
}

///往master转发消息
bool UnistorHandler4Trans::transMsg(UnistorTss* , CWX_UINT32 uiTaskId, CwxMsgBlock* msg){
    CwxMsgBlock* block = NULL;
    ///如果没有完成认证的连接，则失败
    if (!m_uiAuthConnNum) return false;
    ///往master转发消息
    block = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + msg->length());
    CwxMsgHead header(msg->event().getMsgHeader());
    header.setDataLen(msg->length());
    header.setTaskId(uiTaskId);
    CWX_UINT8 ucAttr=header.getAttr();
    header.setAttr(UnistorPoco::clearFromMaster(ucAttr));
    memcpy(block->wr_ptr(), header.toNet(), CwxMsgHead::MSG_HEAD_LEN);
    memcpy(block->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, msg->rd_ptr(), msg->length());
    block->wr_ptr( CwxMsgHead::MSG_HEAD_LEN + msg->length());
    block->event().setTaskId(uiTaskId);
    block->send_ctrl().setMsgAttr(CwxMsgSendCtrl::FAIL_FINISH_NOTICE);
    if (!m_authConn[msg->event().getConnId()%m_uiAuthConnNum]->putMsg(block)){
        CwxMsgBlockAlloc::free(block);
        return false;
    }
    return true;
}

int UnistorHandler4Trans::rebuildConn(UnistorApp* app){
    if (!m_strMasterHost.length()){
        CWX_INFO(("UnistorHandler4Trans: Master is empty, refuse to connect trans connection."));
    }
    if (!m_bRebuildConn) return 0;
    CWX_UINT32 ttNow = time(NULL);
    if (m_ttLastRebuildConn + 2 > ttNow){
        return 0;
    }
    m_ttLastRebuildConn = ttNow;

    m_bCanTrans = false;
    ///关闭已有的所有连接
    UnistorHandler4Trans* handle = NULL;
    map<CWX_UINT32, UnistorHandler4Trans*>::iterator iter = m_handlers->begin();
    while(iter != m_handlers->end()){
        handle = iter->second;
        m_handlers->erase(iter);
        handle->close();
        iter = m_handlers->begin();
    }
    m_uiAuthConnNum = 0;
    CWX_INFO(("UnistorHandler4Trans:Rebuild trans connection to %s:%u, user:passwd=%s:%s",
        m_strMasterHost.c_str(),
        app->getConfig().getRecv().getPort(),
        app->getConfig().getRecv().getUser().length()?app->getConfig().getRecv().getUser().c_str():"",
        app->getConfig().getRecv().getUser().length()?app->getConfig().getRecv().getUser().c_str():""));
    ///重建所有连接
    CwxINetAddr addr;
    if (0 != addr.set(app->getConfig().getRecv().getPort(), m_strMasterHost.c_str())){
        CWX_ERROR(("Failure to init addr, addr:%s, port:%u, err=%d", m_strMasterHost.c_str(), app->getConfig().getRecv().getPort(), errno));
        return -1;
    }
    CWX_UINT32 i=0;
    int* fds = new int[app->getConfig().getCommon().m_uiTranConnNum];
    for (i=0; i<app->getConfig().getCommon().m_uiTranConnNum; i++){
        fds[i] = -1;
    }
    CwxTimeValue timeout(UNISTOR_CONN_TIMEOUT_SECOND);
    CwxTimeouter timeouter(&timeout);
    if (0 != UnistorConnector::connect(addr,
        app->getConfig().getCommon().m_uiTranConnNum,
        fds,
        &timeouter,
        true,
        UnistorApp::setConnSockAttr,
        app->getRecvSockAttr()))
    {
        CWX_ERROR(("Failure to connect to addr:%s, port:%u, err=%d",m_strMasterHost.c_str(), app->getConfig().getRecv().getPort(), errno)); 
        return -1;
    }
    ///将连接注册到channel
    CwxAppChannel* channel = app->getTransChannel();
    for (i=0; i<app->getConfig().getCommon().m_uiTranConnNum; i++){
        if (channel->isRegIoHandle(fds[i])){
            CWX_ERROR(("Handler[%] is register", fds[i]));
            break;
        }
        UnistorHandler4Trans* pHandler = new UnistorHandler4Trans(app,
            app->reactor()->getNextConnId(),
            channel);
        pHandler->setHandle(fds[i]);
        if (0 != pHandler->open()){
            pHandler->setHandle(-1);
            delete pHandler;
            break;
        }
        pHandler->m_tss = (UnistorTss*)CwxTss::instance(); ///<对象对应的tss对象

    }
    if (i < app->getConfig().getCommon().m_uiTranConnNum){
        for (CWX_UINT32 j=i; j<app->getConfig().getCommon().m_uiTranConnNum; j++){
            ::close(fds[j]);
        }
        return -1;
    }
    m_bRebuildConn = false;
    return 0;
}

void UnistorHandler4Trans::checkTrans(UnistorApp* app, UnistorTss* tss){
    CWX_INFO(("UnistorHandler4Trans: ZK config is changed. master_idc:%s, is_master_idc:%s, master_host:%s, is_master=%s, sync_host:%s",
        tss->getMasterIdc(),
        tss->isMasterIdc()?"yes":"no",
        tss->getMasterHost(),
        tss->isMaster()?"yes":"no",
        tss->getSyncHost()));
    bool bTrans = false;
    string strTransMaster = "";
    if (tss->m_pZkConf && tss->m_pZkLock){
        if (tss->m_pZkConf->m_bMasterIdc){///如果是master idc
            if (!tss->m_pZkLock->m_bMaster){
                bTrans = true;
                strTransMaster = tss->m_pZkLock->m_strMaster;
            }else{
                bTrans = false;
            }
        }else{
            bTrans = false;
        }
    }
    if (!bTrans){
        ///关闭已有的所有连接
        map<CWX_UINT32, UnistorHandler4Trans*>::iterator iter = m_handlers->begin();
        while(iter != m_handlers->end()){
            iter->second->close();
            iter = m_handlers->begin();
        }
        CWX_ASSERT(m_handlers->size()==0);
        m_strMasterHost = "";
        m_uiAuthConnNum = 0;
        m_bRebuildConn = false;
        m_bCanTrans = false;
    }else{
        if (strTransMaster != m_strMasterHost){
            map<CWX_UINT32, UnistorHandler4Trans*>::iterator iter = m_handlers->begin();
            while(iter != m_handlers->end()){
                iter->second->close();
                iter = m_handlers->begin();
            }
            CWX_ASSERT(m_handlers->size()==0);
            m_strMasterHost = strTransMaster;
            m_uiAuthConnNum = 0;
            m_bRebuildConn = true;
            m_bCanTrans = false;
            rebuildConn(app);
        }
    }
}
