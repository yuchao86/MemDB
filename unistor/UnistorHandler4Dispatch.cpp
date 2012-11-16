#include "UnistorHandler4Dispatch.h"
#include "UnistorApp.h"

///���һ��������
void UnistorDispatchSyncSession::addConn(UnistorHandler4Dispatch* conn){
    CWX_ASSERT(m_conns.find(conn->getConnId()) == m_conns.end());
    m_conns[conn->getConnId()] = conn;
}

///�Ƿ�user object������
void UnistorDispatchThreadUserObj::free(UnistorApp* pApp){
    map<CWX_UINT64, UnistorDispatchSyncSession* >::iterator iter = m_sessionMap.begin();
    while(iter != m_sessionMap.end()){
        if (iter->second->m_pCursor) pApp->getStore()->getBinLogMgr()->destoryCurser(iter->second->m_pCursor);
        delete iter->second;
        iter++;
    }
    m_sessionMap.clear();
}


///���캯��
UnistorHandler4Dispatch::UnistorHandler4Dispatch(UnistorApp* pApp,
                                                 CwxAppChannel* channel,
                                                 CWX_UINT32 uiConnId, 
                                                 bool bInner):CwxAppHandler4Channel(channel)
{
    m_bInner = bInner;
    m_bReport = false;
    m_ucDispatchType = DISPATCH_TYPE_INIT;
    m_uiConnId = uiConnId;
    m_ullSentSeq = 0;
    m_syncSession = NULL;
    m_exportSession = NULL;
    m_pApp = pApp;
    m_uiRecvHeadLen = 0;
    m_uiRecvDataLen = 0;
    m_recvMsgData = 0;
    m_ullSessionId = 0;
    m_tss = NULL;
}

///��������
UnistorHandler4Dispatch::~UnistorHandler4Dispatch(){
    if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
    m_recvMsgData = NULL;
}


void UnistorHandler4Dispatch::doEvent(UnistorApp* app, UnistorTss* tss, CwxMsgBlock*& msg){
    if (CwxEventInfo::CONN_CREATED == msg->event().getEvent()){///���ӽ���

        bool bInner =  msg->event().getSvrId() == UnistorApp::SVR_TYPE_INNER_SYNC?true:false;
        CwxAppChannel* channel =bInner?app->getInnerSyncChannel():app->getOuterSyncChannel();
        if (channel->isRegIoHandle(msg->event().getIoHandle())){
            CWX_ERROR(("Handler[%] is register, it's a big bug. exit....", msg->event().getIoHandle()));
            app->stop();
            return;
        }
        UnistorHandler4Dispatch* pHandler = new UnistorHandler4Dispatch(app, channel, app->reactor()->getNextConnId(), bInner);
        ///��ȡ���ӵ���Դ��Ϣ
        CwxINetAddr  remoteAddr;
        CwxSockStream stream(msg->event().getIoHandle());
        stream.getRemoteAddr(remoteAddr);
        pHandler->m_unPeerPort = remoteAddr.getPort();
        if (remoteAddr.getHostIp(tss->m_szBuf2K, 2047)){
            pHandler->m_strPeerHost = tss->m_szBuf2K;
        }
        ///����handle��io��open handler
        pHandler->setHandle(msg->event().getIoHandle());
        if (0 != pHandler->open()){
            CWX_ERROR(("Failure to register sync handler[%d], from:%s:%u", pHandler->getHandle(), pHandler->m_strPeerHost.c_str(), pHandler->m_unPeerPort));
            delete pHandler;
            return;
        }
        ///���ö����tss����
        pHandler->m_tss = (UnistorTss*)CwxTss::instance();
        CWX_INFO(("Accept %s sync connection from %s:%u", bInner?"inner":"outer", pHandler->m_strPeerHost.c_str(), pHandler->m_unPeerPort));
    }else{
        CWX_ERROR(("Unkwown event type:%d", msg->event().getEvent()));
    }
}

///�ͷŹرյ�session
void UnistorHandler4Dispatch::dealClosedSession(UnistorApp* app, UnistorTss* tss){
    list<UnistorDispatchSyncSession*>::iterator iter;
    UnistorHandler4Dispatch* handler;
    ///��ȡ�û�object����
    UnistorDispatchThreadUserObj* userObj = (UnistorDispatchThreadUserObj*) tss->getUserObj();
    if (userObj->m_freeSession.begin() != userObj->m_freeSession.end()){
        iter = userObj->m_freeSession.begin();
        while(iter != userObj->m_freeSession.end()){
            ///session������closed״̬
            CWX_ASSERT((*iter)->m_bClosed);
            CWX_INFO(("Close sync session from host:%s", (*iter)->m_strHost.c_str()));
            ///��session��session��map��ɾ��
            userObj->m_sessionMap.erase((*iter)->m_ullSessionId);
            ///��ʼ�ر�����
            map<CWX_UINT32, UnistorHandler4Dispatch*>::iterator conn_iter = (*iter)->m_conns.begin();
            while(conn_iter != (*iter)->m_conns.end()){
                handler = conn_iter->second;
                (*iter)->m_conns.erase(handler->getConnId());
                handler->close();///��Ϊͬ������
                conn_iter = (*iter)->m_conns.begin();
            }
            ///�ͷŶ�Ӧ��cursor
            if ((*iter)->m_pCursor){
                app->getStore()->getBinLogMgr()->destoryCurser((*iter)->m_pCursor);
            }
            delete *iter;
            iter++;
        }
        userObj->m_freeSession.clear();
    }
}


/**
@brief ���ӿɶ��¼�������-1��close()�ᱻ����
@return -1������ʧ�ܣ������close()�� 0������ɹ�
*/
int UnistorHandler4Dispatch::onInput(){
    ///������Ϣ
    int ret = CwxAppHandler4Channel::recvPackage(getHandle(),
        m_uiRecvHeadLen,
        m_uiRecvDataLen,
        m_szHeadBuf,
        m_header,
        m_recvMsgData);
    ///���û�н�����ϣ�0����ʧ�ܣ�-1�����򷵻�
    if (1 != ret) return ret;
    ///���յ�һ�����������ݰ�����Ϣ����
    ret = recvMessage();
    ///���û���ͷŽ��յ����ݰ����ͷ�
    if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
    this->m_recvMsgData = NULL;
    this->m_uiRecvHeadLen = 0;
    this->m_uiRecvDataLen = 0;
    return ret;
}

//1������engine���Ƴ�ע�᣻0����engine���Ƴ�ע�ᵫ��ɾ��handler��-1����engine�н�handle�Ƴ���ɾ����
int UnistorHandler4Dispatch::onConnClosed(){
    CWX_INFO(("UnistorHandler4Dispatch: conn closed, conn_id=%u", m_uiConnId));
    if (DISPATCH_TYPE_SYNC == m_ucDispatchType){
        ///һ�����ӹرգ�������sessionʧЧ
        if (m_syncSession){
            UnistorDispatchThreadUserObj* userObj=(UnistorDispatchThreadUserObj*)m_tss->getUserObj();
            ///������Ӷ�Ӧ��session����
            if (userObj->m_sessionMap.find(m_ullSessionId) != userObj->m_sessionMap.end()){
                CWX_INFO(("UnistorHandler4Dispatch: conn closed, conn_id=%u", m_uiConnId));
                if (!m_syncSession->m_bClosed){
                    ///��session���Ϊclose
                    m_syncSession->m_bClosed = true;
                    ///��session�ŵ���Ҫ�Ƿ��session�б�
                    userObj->m_freeSession.push_back(m_syncSession);
                }
                ///�����Ӵ�session��������ɾ������˴����ӽ���delete
                m_syncSession->m_conns.erase(m_uiConnId);
            }
        }
    }else if (DISPATCH_TYPE_EXPORT == m_ucDispatchType){
        if (m_exportSession){
            if (m_exportSession->m_pCursor){
                m_pApp->getStore()->exportEnd(*m_exportSession->m_pCursor);
                delete m_exportSession->m_pCursor;
                m_exportSession->m_pCursor = NULL;
            }
            delete m_exportSession;
            m_exportSession = NULL;
        }

    }
    ///��epoll engine���Ƴ�handler��ɾ��
    return -1;
}

///���ӹرպ���Ҫ������
int UnistorHandler4Dispatch::recvMessage(){
    if (UnistorPoco::MSG_TYPE_SYNC_REPORT == m_header.getMsgType()){
        return recvSyncReport(m_tss);
    }else if (UnistorPoco::MSG_TYPE_SYNC_CONN == m_header.getMsgType()){
        return recvSyncNewConnection(m_tss);
    }else if (UnistorPoco::MSG_TYPE_SYNC_DATA_REPLY == m_header.getMsgType()){
        return recvSyncReply(m_tss);
    }else if (UnistorPoco::MSG_TYPE_SYNC_DATA_CHUNK_REPLY == m_header.getMsgType()){
        return recvSyncChunkReply(m_tss);
    }else if (UnistorPoco::MSG_TYPE_EXPORT_REPORT == m_header.getMsgType()){
        return recvExportReport(m_tss);
    }else if (UnistorPoco::MSG_TYPE_EXPORT_DATA_REPLY == m_header.getMsgType()){
        return recvExportReply(m_tss);
    }
    ///ֱ�ӹر�����
    CWX_ERROR(("Recv invalid msg type:%u from host:%s:%u, close connection.", m_header.getMsgType(), m_strPeerHost.c_str(), m_unPeerPort));
    return -1;
}

int UnistorHandler4Dispatch::recvSyncReport(UnistorTss* pTss){
    int iRet = 0;
    CWX_UINT64 ullSid = 0;
    bool bNewly = false;
    CWX_UINT32 uiChunk = 0;
    char const* subscribe = NULL;
    char const* user=NULL;
    char const* passwd=NULL;
    char const* sign=NULL;
    bool bzip = false;
    CwxMsgBlock* msg = NULL;
    CWX_INFO(("Recv report from host:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
    do{
        if (!m_recvMsgData){
            strcpy(pTss->m_szBuf2K, "No data.");
            CWX_ERROR(("Report package is empty, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (DISPATCH_TYPE_INIT != m_ucDispatchType){
            iRet = UNISTOR_ERR_ERROR;
            if (DISPATCH_TYPE_EXPORT == m_ucDispatchType){
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report sync from export connection.");
                CWX_ERROR(("Report sync from export connnection, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            }else if (DISPATCH_TYPE_SYNC == m_ucDispatchType){
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report sync sid duplicate.");
                CWX_ERROR(("Report is duplicate, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            }else{
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Incorrect connection type.");
                CWX_ERROR(("Incorrect connection type:%u, from:%s:%u", m_ucDispatchType, m_strPeerHost.c_str(), m_unPeerPort));
            }
            break;
        }
        ///��ֹ�ظ�report sid����cursor���ڣ���ʾ�Ѿ������һ��
        if (m_syncSession){
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report sync sid duplicate.");
            CWX_ERROR(("Report is duplicate, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        ///������������
        m_ucDispatchType = DISPATCH_TYPE_SYNC;
        ///����ͬ��sid�ı�����Ϣ,���ȡ�����sid
        iRet = UnistorPoco::parseReportData(pTss->m_pReader,
            m_recvMsgData,
            ullSid,
            bNewly,
            uiChunk,
            subscribe,
            user,
            passwd,
            sign,
            bzip,
            pTss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != iRet){
            CWX_ERROR(("Failure to parse report msg, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }

        CWX_INFO(("Recv report from:%s:%u, sid=%s, from_new=%s, chunk=%u, subscribe=%s, user=%s, passwd=%s, sign=%s, zip=%s",
            m_strPeerHost.c_str(),
            m_unPeerPort,
            CwxCommon::toString(ullSid, pTss->m_szBuf2K, 10),
            bNewly?"yes":"no",
            uiChunk,
            subscribe?subscribe:"",
            user?user:"",
            passwd?passwd:"",
            sign?sign:"",
            bzip?"yes":"no"));

        if (m_bInner){
            if (m_pApp->getConfig().getInnerDispatch().getUser().length()){
                if ( (m_pApp->getConfig().getInnerDispatch().getUser() != user) ||
                    (m_pApp->getConfig().getInnerDispatch().getPasswd() != passwd))
                {
                    iRet = UNISTOR_ERR_FAIL_AUTH;
                    CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Failure to auth user[%s] passwd[%s]", user, passwd);
                    CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                    break;
                }
            }
        }else{
            if (m_pApp->getConfig().getOuterDispatch().getUser().length()){
                if ( (m_pApp->getConfig().getOuterDispatch().getUser() != user) ||
                    (m_pApp->getConfig().getOuterDispatch().getPasswd() != passwd))
                {
                    iRet = UNISTOR_ERR_FAIL_AUTH;
                    CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Failure to auth user[%s] passwd[%s]", user, passwd);
                    CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                    break;
                }
            }
        }
        string strSubcribe=subscribe?subscribe:"";
        string strErrMsg;
        m_syncSession = new UnistorDispatchSyncSession();
        if (!m_syncSession->m_subscribe.parseSubsribe(subscribe)){
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Invalid subscribe[%s], err=%s",subscribe,  m_syncSession->m_subscribe.m_strErrMsg.c_str());
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            delete m_syncSession;
            m_syncSession = NULL;
            break;
        }
        if (!m_pApp->getStore()->isValidSubscribe(m_syncSession->m_subscribe, pTss->m_szBuf2K)){
            iRet = UNISTOR_ERR_ERROR;
            CWX_ERROR(("Invalid subscribe[%s], err:%s, from:%s:%u", strSubcribe.c_str(), pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            delete m_syncSession;
            m_syncSession = NULL;
            break;
        }
        m_syncSession->m_strHost = m_strPeerHost;
        m_syncSession->m_uiChunk = uiChunk;
        m_syncSession->m_bZip = bzip;
        m_syncSession->m_strSign = sign;
        if ((m_syncSession->m_strSign != UNISTOR_KEY_CRC32) &&
            (m_syncSession->m_strSign != UNISTOR_KEY_MD5))
        {//���ǩ������CRC32��MD5�������
            m_syncSession->m_strSign.erase();
        }
        if (m_syncSession->m_uiChunk){
            if (m_syncSession->m_uiChunk > UNISTOR_MAX_CHUNK_SIZE_KB) m_syncSession->m_uiChunk = UNISTOR_MAX_CHUNK_SIZE_KB;
            if (m_syncSession->m_uiChunk < UNISTOR_MIN_CHUNK_SIZE_KB) m_syncSession->m_uiChunk = UNISTOR_MIN_CHUNK_SIZE_KB;
            m_syncSession->m_uiChunk *= 1024;
        }
        if (bNewly){///��sidΪ�գ���ȡ��ǰ���sid-1
            ullSid = m_pApp->getStore()->getBinLogMgr()->getMaxSid();
            if (ullSid) ullSid--;
        }
        m_syncSession->reformSessionId();
        ///��session���뵽session��map
        UnistorDispatchThreadUserObj* tssUserObj= (UnistorDispatchThreadUserObj*)pTss->getUserObj();
        while(tssUserObj->m_sessionMap.find(m_syncSession->m_ullSessionId) != tssUserObj->m_sessionMap.end()){
            m_syncSession->reformSessionId();
        }
        tssUserObj->m_sessionMap[m_syncSession->m_ullSessionId]=m_syncSession;
        m_ullSessionId = m_syncSession->m_ullSessionId;
        m_syncSession->addConn(this);
        ///�ظ�iRet��ֵ
        iRet = UNISTOR_ERR_SUCCESS;
        ///����binlog��ȡ��cursor
        CwxBinLogCursor* pCursor = m_pApp->getStore()->getBinLogMgr()->createCurser(ullSid);
        if (!pCursor){
            iRet = UNISTOR_ERR_ERROR;
            strcpy(pTss->m_szBuf2K, "Failure to create cursor");
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        if (!bNewly){
            if (ullSid && ullSid < m_pApp->getStore()->getBinLogMgr()->getMinSid()){
                m_pApp->getStore()->getBinLogMgr()->destoryCurser(pCursor);
                iRet = UNISTOR_ERR_LOST_SYNC;
                char szBuf1[64], szBuf2[64];
                sprintf(pTss->m_szBuf2K, "Lost sync state, report sid:%s, min sid:%s",
                    CwxCommon::toString(ullSid, szBuf1),
                    CwxCommon::toString(m_pApp->getStore()->getBinLogMgr()->getMinSid(), szBuf2));
                CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                break;
            }
        }
        ///����cursor
        m_syncSession->m_pCursor = pCursor;
        m_syncSession->m_ullStartSid = ullSid;

        ///����session id����Ϣ
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packReportDataReply(pTss->m_pWriter,
            msg,
            m_header.getTaskId(),
            m_syncSession->m_ullSessionId,
            pTss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack sync data reply, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            return -1;
        }
        msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
        if (!putMsg(msg)){
            CwxMsgBlockAlloc::free(msg);
            CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            return -1;
        }
        ///������һ��binlog
        int iState = syncSendBinLog(pTss);
        if (-1 == iState){
            iRet = UNISTOR_ERR_ERROR;
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }else if (0 == iState){///����continue����Ϣ
            channel()->regRedoHander(this);
        }
        return 0;
    }while(0);
    ///����һ������
    CWX_ASSERT(UNISTOR_ERR_SUCCESS != iRet);
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncErr(pTss->m_pWriter,
        msg,
        m_header.getTaskId(),
        iRet,
        pTss->m_szBuf2K,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack err msg, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
    if (!putMsg(msg)){
        CwxMsgBlockAlloc::free(msg);
        CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    return 0;
}

int UnistorHandler4Dispatch::recvSyncNewConnection(UnistorTss* pTss){
    int iRet = 0;
    CWX_UINT64 ullSession = 0;
    CwxMsgBlock* msg = NULL;
    do{
        if (!m_recvMsgData){
            strcpy(pTss->m_szBuf2K, "No data.");
            CWX_ERROR(("Session connect-report package is empyt, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (DISPATCH_TYPE_INIT != m_ucDispatchType){
            iRet = UNISTOR_ERR_ERROR;
            if (DISPATCH_TYPE_EXPORT == m_ucDispatchType){
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report sync from export connection.");
                CWX_ERROR(("Report sync from export connnection, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            }else if (DISPATCH_TYPE_SYNC == m_ucDispatchType){
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report sync sid duplicate.");
                CWX_ERROR(("Report is duplicate, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            }else{
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Incorrect connection type.");
                CWX_ERROR(("Incorrect connection type:%u, from:%s:%u", m_ucDispatchType, m_strPeerHost.c_str(), m_unPeerPort));
            }
            break;
        }
        ///��ֹ�ظ�report sid����cursor���ڣ���ʾ�Ѿ������һ��
        if (m_syncSession){
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report sync sid duplicatly.");
            CWX_ERROR(("Session connect-report is duplicate, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        ///������������
        m_ucDispatchType = DISPATCH_TYPE_SYNC;
        ///��ȡ�����session id
        iRet = UnistorPoco::parseReportNewConn(pTss->m_pReader,
            m_recvMsgData,
            ullSession,
            pTss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != iRet){
            CWX_ERROR(("Failure to parse report new conn msg, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        UnistorDispatchThreadUserObj* tssUserObj= (UnistorDispatchThreadUserObj*)pTss->getUserObj();
        if (tssUserObj->m_sessionMap.find(ullSession) == tssUserObj->m_sessionMap.end()){
            iRet = UNISTOR_ERR_ERROR;
            char szTmp[64];
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Session[%s] doesn't exist", CwxCommon::toString(ullSession, szTmp, 10));
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        m_syncSession = tssUserObj->m_sessionMap.find(ullSession)->second;
        m_ullSessionId = m_syncSession->m_ullSessionId;
        m_syncSession->addConn(this);
        ///������һ��binlog
        int iState = syncSendBinLog(pTss);
        if (-1 == iState){
            iRet = UNISTOR_ERR_ERROR;
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }else if (0 == iState){///����continue����Ϣ
            channel()->regRedoHander(this);
        }
        return 0;
    }while(0);
    ///����һ������
    CWX_ASSERT(UNISTOR_ERR_SUCCESS != iRet);
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncErr(pTss->m_pWriter,
        msg,
        m_header.getTaskId(),
        iRet,
        pTss->m_szBuf2K,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack sync data reply, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
    if (!putMsg(msg)){
        CwxMsgBlockAlloc::free(msg);
        CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    return 0;
}

int UnistorHandler4Dispatch::recvSyncReply(UnistorTss* pTss){
    int iRet = UNISTOR_ERR_SUCCESS;
    CWX_UINT64 ullSeq = 0;
    CwxMsgBlock* msg = NULL;
    do {
        if (!m_syncSession){ ///������Ӳ���ͬ��״̬�����Ǵ���
            strcpy(pTss->m_szBuf2K, "Client no in sync state");
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (DISPATCH_TYPE_SYNC != m_ucDispatchType){
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Connection is sync type.");
            CWX_ERROR(("Connection is sync type[%u],from:%s:%u", m_ucDispatchType, m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (!m_recvMsgData){
            strcpy(pTss->m_szBuf2K, "No data.");
            CWX_ERROR(("Sync reply package is empty, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        ///����ͬ��sid�ı�����Ϣ,���ȡ�����sid
        iRet = UnistorPoco::parseSyncDataReply(pTss->m_pReader,
            m_recvMsgData,
            ullSeq,
            pTss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != iRet){
            CWX_ERROR(("Failure to parse sync_data reply package, err:%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        if (ullSeq != m_ullSentSeq){
            char szTmp1[64];
            char szTmp2[64];
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Seq[%s] is not same with the connection's[%s].",
                CwxCommon::toString(ullSeq, szTmp1, 10),
                CwxCommon::toString(m_ullSentSeq, szTmp2, 10));
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        ///������һ��binlog
        int iState = syncSendBinLog(pTss);
        if (-1 == iState){
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            return -1; ///�ر�����
        }else if (0 == iState){///����continue����Ϣ
            channel()->regRedoHander(this);
        }
        return 0;
    } while(0);
    ///����һ������
    CWX_ASSERT(UNISTOR_ERR_SUCCESS != iRet);
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncErr(pTss->m_pWriter,
        msg,
        m_header.getTaskId(),
        iRet,
        pTss->m_szBuf2K,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack sync data reply, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
    if (!putMsg(msg)){
        CwxMsgBlockAlloc::free(msg);
        CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    return 0;
}

int UnistorHandler4Dispatch::recvSyncChunkReply(UnistorTss* pTss){
    return recvSyncReply(pTss);
}


/**
@brief Handler��redo�¼�����ÿ��dispatchʱִ�С�
@return -1������ʧ�ܣ������close()�� 0������ɹ�
*/
int UnistorHandler4Dispatch::onRedo(){
    if (DISPATCH_TYPE_SYNC == m_ucDispatchType){
        ///�ж��Ƿ��пɷ��͵���Ϣ
        if (m_syncSession->m_ullSid < m_pApp->getStore()->getBinLogMgr()->getMaxSid()){
            ///������һ��binlog
            int iState = syncSendBinLog(m_tss);
            if (-1 == iState){
                CWX_ERROR(("%s, from:%s:%u", m_tss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                CwxMsgBlock* msg=NULL;
                if (UNISTOR_ERR_ERROR != UnistorPoco::packSyncErr(m_tss->m_pWriter,
                    msg,
                    m_header.getTaskId(),
                    UNISTOR_ERR_ERROR,
                    m_tss->m_szBuf2K,
                    m_tss->m_szBuf2K))
                {
                    CWX_ERROR(("Failure to pack sync data reply, err=%s, from:%s:%u", m_tss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                    return -1;
                }
                msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
                if (!putMsg(msg)){
                    CwxMsgBlockAlloc::free(msg);
                    CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
                    return -1;
                }
            }else if (0 == iState){///����continue����Ϣ
                channel()->regRedoHander(this);
            }
        }else{
            ///����ע��
            channel()->regRedoHander(this);
        }
    }else if(DISPATCH_TYPE_EXPORT == m_ucDispatchType){
        ///������һ��binlog
        int iState = exportSendData(m_tss);
        if (-1 == iState){
            CWX_ERROR(("%s, from:%s:%u", m_tss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            CwxMsgBlock* msg=NULL;
            if (UNISTOR_ERR_ERROR != UnistorPoco::packSyncErr(m_tss->m_pWriter,
                msg,
                m_header.getTaskId(),
                UNISTOR_ERR_ERROR,
                m_tss->m_szBuf2K,
                m_tss->m_szBuf2K))
            {
                CWX_ERROR(("Failure to pack sync data reply, err=%s, from:%s:%u", m_tss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                return -1;
            }
            msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
            if (!putMsg(msg)){
                CwxMsgBlockAlloc::free(msg);
                CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
                return -1;
            }
        }else if (0 == iState){///����continue����Ϣ
            channel()->regRedoHander(this);
        }
    }
    ///����
    return 0;
}


///0��δ����һ��binlog��
///1��������һ��binlog��
///-1��ʧ�ܣ�
int UnistorHandler4Dispatch::syncSendBinLog(UnistorTss* pTss){
    int iRet = 0;
    CwxMsgBlock* pBlock = NULL;
    CWX_UINT32 uiSkipNum = 0;
    CWX_UINT32 uiKeyLen = 0;
    CWX_UINT32 uiTotalLen = 0;
    CWX_UINT64 ullSeq = m_syncSession->m_ullSeq;
    if (m_syncSession->m_pCursor->isUnseek()){//��binlog�Ķ�ȡcursor���գ���λ
        if (1 != (iRet = syncSeekToReportSid(pTss))) return iRet;
    }

    if (m_syncSession->m_uiChunk)  pTss->m_pWriter->beginPack();
    while(1){
        if ( 1 != (iRet = syncSeekToBinlog(pTss, uiSkipNum))) break;
        //�����Ƶ���һ����¼λ��
        m_syncSession->m_bNext = true;
        if (!m_syncSession->m_uiChunk){
            iRet = syncPackOneBinLog(pTss->m_pWriter,
                pBlock,
                ullSeq,
                pTss->m_uiBinLogVersion,
                pTss->m_uiBinlogType,
                pTss->m_pBinlogData,
                pTss->m_szBuf2K);
            break;
        }else{
            iRet = syncPackMultiBinLog(pTss->m_pWriter,
                pTss->m_pItemWriter,
                pTss->m_uiBinLogVersion,
                pTss->m_uiBinlogType,
                pTss->m_pBinlogData,
                uiKeyLen,
                pTss->m_szBuf2K);
            if (1 == iRet){
                uiTotalLen += uiKeyLen;
                if (uiTotalLen >= m_syncSession->m_uiChunk) break;
            }
            if (-1 == iRet) break;
            continue;
        }
    }

    if (-1 == iRet) return -1;

    if (!m_syncSession->m_uiChunk){ ///������chunk
        if (0 == iRet) return 0; ///û������
    }else{
        if (0 == uiTotalLen) return 0;
        //add sign
        if (m_syncSession->m_strSign.length()){
            if (m_syncSession->m_strSign == UNISTOR_KEY_CRC32){//CRC32ǩ��
                CWX_UINT32 uiCrc32 = CwxCrc32::value(pTss->m_pWriter->getMsg(), pTss->m_pWriter->getMsgSize());
                if (!pTss->m_pWriter->addKeyValue(UNISTOR_KEY_CRC32, (char*)&uiCrc32, sizeof(uiCrc32))){
                    CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Failure to add key value, err:%s", pTss->m_pWriter->getErrMsg());
                    CWX_ERROR((pTss->m_szBuf2K));
                    return -1;
                }
            } else if (m_syncSession->m_strSign == UNISTOR_KEY_MD5){//md5ǩ��
                CwxMd5 md5;
                unsigned char szMd5[16];
                md5.update((unsigned char*)pTss->m_pWriter->getMsg(), pTss->m_pWriter->getMsgSize());
                md5.final(szMd5);
                if (!pTss->m_pWriter->addKeyValue(UNISTOR_KEY_MD5, (char*)szMd5, 16)){
                    CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Failure to add key value, err:%s", pTss->m_pWriter->getErrMsg());
                    CWX_ERROR((pTss->m_szBuf2K));
                    return -1;
                }
            }
        }
        pTss->m_pWriter->pack();
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packMultiSyncData(0,
            pTss->m_pWriter->getMsg(),
            pTss->m_pWriter->getMsgSize(),
            pBlock,
            ullSeq,
            m_syncSession->m_bZip,
            pTss->m_szBuf2K))
        {
            return -1;
        }
    }
    ///����svr���ͣ��������ݰ�
    pBlock->send_ctrl().setConnId(CWX_APP_INVALID_CONN_ID);
    pBlock->send_ctrl().setSvrId(m_bInner?UnistorApp::SVR_TYPE_INNER_SYNC:UnistorApp::SVR_TYPE_OUTER_SYNC);
    pBlock->send_ctrl().setHostId(0);
    pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
    if (!putMsg(pBlock)){
        CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Failure to send binlog");
        CWX_ERROR((pTss->m_szBuf2K));
        CwxMsgBlockAlloc::free(pBlock);
        return -1;
    }
    m_ullSentSeq = ullSeq;
    m_syncSession->m_ullSeq++;
    return 1; ///������һ����Ϣ
}

//1���ɹ���0��̫��-1������
int UnistorHandler4Dispatch::syncSeekToReportSid(UnistorTss* tss){
    int iRet = 0;
    if (m_syncSession->m_pCursor->isUnseek()){//��binlog�Ķ�ȡcursor���գ���λ
        if (m_syncSession->m_ullStartSid < m_pApp->getStore()->getBinLogMgr()->getMaxSid()){
            iRet = m_pApp->getStore()->getBinLogMgr()->seek(m_syncSession->m_pCursor, m_syncSession->m_ullStartSid);
            if (-1 == iRet){
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Failure to seek,  err:%s", m_syncSession->m_pCursor->getErrMsg());
                CWX_ERROR((tss->m_szBuf2K));
                return -1;
            }else if (0 == iRet){
                char szBuf1[64];
                char szBuf2[64];
                CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Should seek to sid[%s] with max_sid[[%s], but not.",
                    CwxCommon::toString(m_syncSession->m_ullStartSid, szBuf1),
                    CwxCommon::toString(m_pApp->getStore()->getBinLogMgr()->getMaxSid(), szBuf2));
                CWX_ERROR((tss->m_szBuf2K));
                return 0;
            }
            ///���ɹ���λ�����ȡ��ǰ��¼
            m_syncSession->m_bNext = m_syncSession->m_ullStartSid == m_syncSession->m_pCursor->getHeader().getSid()?true:false;
        }else{///����Ҫͬ�����͵�sid��С�ڵ�ǰ��С��sid��������Ϊ����״̬
            return 0;///���״̬
        }
    }
    return 1;
}


///-1��ʧ�ܣ�1���ɹ�
int UnistorHandler4Dispatch::syncPackOneBinLog(CwxPackageWriterEx* writer,
                                               CwxMsgBlock*& block,
                                               CWX_UINT64 ullSeq,
                                               CWX_UINT32 uiVersion,
                                               CWX_UINT32 uiType,
                                               CwxKeyValueItemEx const* pData,
                                               char* szErr2K)
{
    ///�γ�binlog���͵����ݰ�
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncData(writer,
        block,
        0,
        m_syncSession->m_pCursor->getHeader().getSid(),
        m_syncSession->m_pCursor->getHeader().getDatetime(),
        *pData,
        m_syncSession->m_pCursor->getHeader().getGroup(),
        uiType,
        uiVersion,
        ullSeq,
        m_syncSession->m_strSign.c_str(),
        m_syncSession->m_bZip,
        szErr2K))
    {
        ///�γ����ݰ�ʧ��
        CWX_ERROR(("Failure to pack binlog package, err:%s", szErr2K));
        return -1;
    }
    return 1;
}

///-1��ʧ�ܣ����򷵻�������ݵĳߴ�
int UnistorHandler4Dispatch::syncPackMultiBinLog(CwxPackageWriterEx* writer,
                                                 CwxPackageWriterEx* writer_item,
                                                 CWX_UINT32 uiVersion,
                                                 CWX_UINT32 uiType,
                                                 CwxKeyValueItemEx const* pData,
                                                 CWX_UINT32&  uiLen,
                                                 char* szErr2K)
{
    ///�γ�binlog���͵����ݰ�
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncDataItem(writer_item,
        m_syncSession->m_pCursor->getHeader().getSid(),
        m_syncSession->m_pCursor->getHeader().getDatetime(),
        *pData,
        m_syncSession->m_pCursor->getHeader().getGroup(),
        uiType,
        uiVersion,
        NULL,
        szErr2K))
    {
        ///�γ����ݰ�ʧ��
        CWX_ERROR(("Failure to pack binlog package, err:%s", szErr2K));
        return -1;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_M, writer_item->getMsg(), writer_item->getMsgSize(),true)){
        ///�γ����ݰ�ʧ��
        CwxCommon::snprintf(szErr2K, 2047, "Failure to pack binlog package, err:%s", writer->getErrMsg());
        CWX_ERROR((szErr2K));
        return -1;
    }
    uiLen = CwxPackageEx::getKvLen(strlen(UNISTOR_KEY_M),  writer_item->getMsgSize());
    return 1;
}

//1�����ּ�¼��0��û�з��֣�-1������
int UnistorHandler4Dispatch::syncSeekToBinlog(UnistorTss* tss, CWX_UINT32& uiSkipNum){
    int iRet = 0;
    if (m_syncSession->m_bNext){
        iRet = m_pApp->getStore()->getBinLogMgr()->next(m_syncSession->m_pCursor);
        if (0 == iRet) return 0; ///���״̬
        if (-1 == iRet){///<ʧ��
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Failure to seek cursor, err:%s", m_syncSession->m_pCursor->getErrMsg());
            CWX_ERROR((tss->m_szBuf2K));
            return -1;
        }
    }
    bool bFind = false;
    char const* szKey=NULL;
    CwxKeyValueItemEx const* pKeyItem=NULL;
    CWX_UINT32 uiDataLen = m_syncSession->m_pCursor->getHeader().getLogLen();
    ///׼��data��ȡ��buf
    char* szData = tss->getBuf(uiDataLen);        
    ///��ȡdata
    iRet = m_pApp->getStore()->getBinLogMgr()->fetch(m_syncSession->m_pCursor, szData, uiDataLen);
    if (-1 == iRet){//��ȡʧ��
        CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Failure to fetch data, err:%s", m_syncSession->m_pCursor->getErrMsg());
        CWX_ERROR((tss->m_szBuf2K));
        return -1;
    }
    uiSkipNum++;
    m_syncSession->m_bNext = false;
    while(1){
        bFind = false;
        do{
            if (!tss->m_pReader->unpack(szData, uiDataLen, false,true)){
                CWX_ERROR(("Can't unpack binlog, sid=%s", CwxCommon::toString(m_syncSession->m_pCursor->getHeader().getSid(), tss->m_szBuf2K)));
                break;
            }
            ///��ȡ�汾
            if (!tss->m_pReader->getKey(UNISTOR_KEY_V, tss->m_uiBinLogVersion)){
                CWX_ERROR(("Can't find key[%s] in binlog, sid=%s", UNISTOR_KEY_V,
                    CwxCommon::toString(m_syncSession->m_pCursor->getHeader().getSid(), tss->m_szBuf2K)));
                break;
            }
            ///��ȡ����
            if (!tss->m_pReader->getKey(UNISTOR_KEY_TYPE, tss->m_uiBinlogType)){
                CWX_ERROR(("Can't find key[%s] in binlog, sid=%s", UNISTOR_KEY_TYPE,
                    CwxCommon::toString(m_syncSession->m_pCursor->getHeader().getSid(), tss->m_szBuf2K)));
                break;
            }
            ///��ȡUNISTOR_KEY_D��key����Ϊ����data����
            tss->m_pBinlogData = tss->m_pReader->getKey(UNISTOR_KEY_D);
            if (!tss->m_pBinlogData){
                CWX_ERROR(("Can't find key[%s] in binlog, sid=%s", UNISTOR_KEY_D,
                    CwxCommon::toString(m_syncSession->m_pCursor->getHeader().getSid(), tss->m_szBuf2K)));
                break;
            }
            if ((UnistorPoco::MSG_TYPE_TIMESTAMP != tss->m_uiBinlogType) &&
                (UnistorSubscribe::SUBSCRIBE_MODE_KEY == m_syncSession->m_subscribe.m_uiMode))
            {///��Ҫ��ȡkey
                if (!tss->m_pItemReader->unpack(tss->m_pBinlogData->m_szData, tss->m_pBinlogData->m_uiDataLen, false, true)){
                    CWX_ERROR(("Can't unpack binlog's data, sid=%s", CwxCommon::toString(m_syncSession->m_pCursor->getHeader().getSid(), tss->m_szBuf2K)));
                    break;
                }
                //��ȡkey
                pKeyItem = tss->m_pItemReader->getKey(UNISTOR_KEY_K);
                if (!pKeyItem){
                    CWX_ERROR(("Can't find key[%s] in binlog, sid=%s", UNISTOR_KEY_K,
                        CwxCommon::toString(m_syncSession->m_pCursor->getHeader().getSid(), tss->m_szBuf2K)));
                    break;
                }
                szKey = pKeyItem->m_szKey;
            }else{
                szKey="";
            }
            bFind = true;
        }while(0);
        if (bFind){
            if ((UnistorPoco::MSG_TYPE_TIMESTAMP == tss->m_uiBinlogType) ||
                (m_pApp->getStore()->isSubscribe(m_syncSession->m_subscribe,m_syncSession->m_pCursor->getHeader().getGroup(), szKey)))
            {
                break;
            }
        }
        iRet = m_pApp->getStore()->getBinLogMgr()->next(m_syncSession->m_pCursor);
        if (0 == iRet){
            m_syncSession->m_bNext = true;
            return 0; ///���״̬
        }
        if (-1 == iRet){///<ʧ��
            CwxCommon::snprintf(tss->m_szBuf2K, 2047, "Failure to seek cursor, err:%s", m_syncSession->m_pCursor->getErrMsg());
            CWX_ERROR((tss->m_szBuf2K));
            return -1;
        }
        uiSkipNum ++;
        if (!UnistorPoco::isContinueSeek(uiSkipNum)){
            return 0;///δ���״̬
        }
    };
    return 1;
}

///0���ɹ���-1��ʧ��
int UnistorHandler4Dispatch::recvExportReport(UnistorTss* pTss){
    int iRet = 0;
    CWX_UINT64 ullSid = 0;
    CWX_UINT32 uiChunk = 0;
    char const* subscribe = NULL;
    char const* key=NULL;
    char const* extra=NULL;
    char const* user=NULL;
    char const* passwd=NULL;
    CwxMsgBlock* msg = NULL;
    CWX_INFO(("Recv export report from host:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
    do{
        if (!m_recvMsgData){
            strcpy(pTss->m_szBuf2K, "No data.");
            CWX_ERROR(("Report package is empty, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (DISPATCH_TYPE_INIT != m_ucDispatchType){
            iRet = UNISTOR_ERR_ERROR;
            if (DISPATCH_TYPE_EXPORT == m_ucDispatchType){
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report export duplicate.");
                CWX_ERROR(("Report is duplicate, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            }else if (DISPATCH_TYPE_SYNC == m_ucDispatchType){
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report export from sync connection.");
                CWX_ERROR(("Report export from sync connection, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            }else{
                CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Incorrect connection type.");
                CWX_ERROR(("Incorrect connection type:%u, from:%s:%u", m_ucDispatchType, m_strPeerHost.c_str(), m_unPeerPort));
            }
            break;
        }
        ///��ֹ�ظ�report export����session���ڣ���ʾ�Ѿ������һ��
        if (m_exportSession){
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Can't report export duplicate.");
            CWX_ERROR(("Report is duplicate, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        ///������������
        m_ucDispatchType = DISPATCH_TYPE_EXPORT;
        ///��ȡexport����Ϣ
        iRet = UnistorPoco::parseExportReport(pTss->m_pReader,
            m_recvMsgData,
            uiChunk,
            subscribe,
            key,
            extra,
            user,
            passwd,
            pTss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != iRet){
            CWX_ERROR(("Failure to parse export report msg, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        CWX_INFO(("Recv report from:%s:%u, chunk=%u, subscribe=%s, user=%s, passwd=%s", 
            m_strPeerHost.c_str(),
            m_unPeerPort,
            uiChunk,
            subscribe?subscribe:"",
            user?user:"",
            passwd?passwd:""));

        if (m_bInner){
            if (m_pApp->getConfig().getInnerDispatch().getUser().length()){
                if ( (m_pApp->getConfig().getInnerDispatch().getUser() != user) ||
                    (m_pApp->getConfig().getInnerDispatch().getPasswd() != passwd))
                {
                    iRet = UNISTOR_ERR_FAIL_AUTH;
                    CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Failure to auth user[%s] passwd[%s]", user, passwd);
                    CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                    break;
                }
            }
        }else{
            if (m_pApp->getConfig().getOuterDispatch().getUser().length()){
                if ( (m_pApp->getConfig().getOuterDispatch().getUser() != user) ||
                    (m_pApp->getConfig().getOuterDispatch().getPasswd() != passwd))
                {
                    iRet = UNISTOR_ERR_FAIL_AUTH;
                    CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Failure to auth user[%s] passwd[%s]", user, passwd);
                    CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
                    break;
                }
            }
        }
        string strSubcribe=subscribe?subscribe:"";
        string strErrMsg;
        m_exportSession = new UnistorDispatchExportSession();
        if (!m_exportSession->m_subscribe.parseSubsribe(subscribe)){
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Invalid subscribe[%s], err=%s",subscribe,  m_exportSession->m_subscribe.m_strErrMsg.c_str());
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            delete m_exportSession;
            m_exportSession = NULL;
            break;
        }
        if (!m_pApp->getStore()->isValidSubscribe(m_exportSession->m_subscribe, pTss->m_szBuf2K)){
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Invalid subscribe[%s], err=%s", subscribe,  m_exportSession->m_subscribe.m_strErrMsg.c_str());
            CWX_ERROR(("Invalid subscribe[%s], err=%s, from:%s:%u", strSubcribe.c_str(), pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            delete m_exportSession;
            m_exportSession = NULL;
            break;
        }
        m_exportSession->m_strHost = m_strPeerHost;
        m_exportSession->m_uiChunk = uiChunk;
        if (m_exportSession->m_uiChunk){
            if (m_exportSession->m_uiChunk > UNISTOR_MAX_CHUNK_SIZE_KB) m_exportSession->m_uiChunk = UNISTOR_MAX_CHUNK_SIZE_KB;
            if (m_exportSession->m_uiChunk < UNISTOR_MIN_CHUNK_SIZE_KB) m_exportSession->m_uiChunk = UNISTOR_MIN_CHUNK_SIZE_KB;
        }else{
            m_exportSession->m_uiChunk = UNISTOR_DEF_CHUNK_SIZE_KB;
        }
        m_exportSession->m_uiChunk *= 1024;
        ///�ظ�iRet��ֵ
        iRet = UNISTOR_ERR_SUCCESS;
        ///����binlog��ȡ��cursor
        m_exportSession->m_pCursor = new UnistorStoreCursor();
        if (0 != m_pApp->getStore()->exportBegin(*m_exportSession->m_pCursor, key, extra, m_exportSession->m_subscribe, ullSid, pTss->m_szBuf2K)){
            iRet = UNISTOR_ERR_ERROR;
            CWX_ERROR(("Failure to create export cursor, err:%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        ///����session id����Ϣ
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packExportReportReply(pTss->m_pWriter,
            msg,
            m_header.getTaskId(),
            0,
            ullSid,
            pTss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack export report data reply, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            return -1;
        }
        msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
        if (!putMsg(msg)){
            CwxMsgBlockAlloc::free(msg);
            CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            return -1;
        }


        ///������һ��binlog
        int iState = exportSendData(pTss);
        if (-1 == iState){
            iRet = UNISTOR_ERR_ERROR;
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }else if (0 == iState){///����continue����Ϣ
            channel()->regRedoHander(this);
        }
        return 0;
    }while(0);
    ///����һ������
    CWX_ASSERT(UNISTOR_ERR_SUCCESS != iRet);
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncErr(pTss->m_pWriter,
        msg,
        m_header.getTaskId(),
        iRet,
        pTss->m_szBuf2K,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack err msg, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
    if (!putMsg(msg)){
        CwxMsgBlockAlloc::free(msg);
        CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    return 0;

}
///0���ɹ���-1��ʧ��
int UnistorHandler4Dispatch::recvExportReply(UnistorTss* pTss){
    int iRet = UNISTOR_ERR_SUCCESS;
    CWX_UINT64 ullSeq = 0;
    CwxMsgBlock* msg = NULL;
    do {
        if (!m_exportSession){ ///������Ӳ���ͬ��״̬�����Ǵ���
            strcpy(pTss->m_szBuf2K, "Client no in export state");
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (DISPATCH_TYPE_EXPORT != m_ucDispatchType){
            CwxCommon::snprintf(pTss->m_szBuf2K, 2048, "Connection is export type.");
            CWX_ERROR(("Connection is export type[%u],from:%s:%u", m_ucDispatchType, m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        if (!m_recvMsgData){
            strcpy(pTss->m_szBuf2K, "No data.");
            CWX_ERROR(("export reply package is empty, from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
            iRet = UNISTOR_ERR_ERROR;
            break;
        }
        ///��ȡ���к�
        iRet = UnistorPoco::parseExportDataReply(pTss->m_pReader,
            m_recvMsgData,
            ullSeq,
            pTss->m_szBuf2K);
        if (UNISTOR_ERR_SUCCESS != iRet){
            CWX_ERROR(("Failure to parse export_data reply package, err:%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        if (ullSeq != m_ullSentSeq){
            char szTmp1[64];
            char szTmp2[64];
            iRet = UNISTOR_ERR_ERROR;
            CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Seq[%s] is not same with the connection's[%s].",
                CwxCommon::toString(ullSeq, szTmp1, 10),
                CwxCommon::toString(m_ullSentSeq, szTmp2, 10));
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            break;
        }
        ///������һ��binlog
        int iState = exportSendData(pTss);
        if (-1 == iState){
            CWX_ERROR(("%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
            return -1; ///�ر�����
        }else if (0 == iState){///����continue����Ϣ
            channel()->regRedoHander(this);
        }
        return 0;
    } while(0);
    ///����һ������
    CWX_ASSERT(UNISTOR_ERR_SUCCESS != iRet);
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncErr(pTss->m_pWriter,
        msg,
        m_header.getTaskId(),
        iRet,
        pTss->m_szBuf2K,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack sync data reply, err=%s, from:%s:%u", pTss->m_szBuf2K, m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    msg->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
    if (!putMsg(msg)){
        CwxMsgBlockAlloc::free(msg);
        CWX_ERROR(("Failure push msg to send queue. from:%s:%u", m_strPeerHost.c_str(), m_unPeerPort));
        return -1;
    }
    return 0;
}


///0��δ����һ�����ݣ�
///1��������һ�����ݣ�
///-1��ʧ�ܣ�
int UnistorHandler4Dispatch::exportSendData(UnistorTss* pTss){
    int iRet = 0;
    CwxMsgBlock* pBlock = NULL;
    CWX_UINT16 unSkipNum = UNISTOR_EXPORT_CONTINUE_SEEK_NUM;
    CWX_UINT16 unKeyLen = 0;
    CWX_UINT32 uiVersion = 0;
    CWX_UINT32 uiExpire=0;
    CwxKeyValueItemEx key;
    CwxKeyValueItemEx value;
    CwxKeyValueItemEx extra;

    CWX_UINT32 uiTotalLen = 0;
    CWX_UINT64 ullSeq = m_exportSession->m_ullSeq;
    key.m_bKeyValue = false;
    extra.m_bKeyValue = false;
    pTss->m_pWriter->beginPack();
    while(1){
        iRet =  m_pApp->getStore()->exportNext(pTss,
            *m_exportSession->m_pCursor,
            key.m_szData,
            unKeyLen,
            value.m_szData,
            value.m_uiDataLen,
            value.m_bKeyValue,
            uiVersion,
            uiExpire,
            unSkipNum,
            extra.m_szData,
            extra.m_uiDataLen);
        if (1 != iRet) break;
        key.m_uiDataLen = unKeyLen;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packExportDataItem(pTss->m_pItemWriter,
            key,
            value,
            extra.m_szData?&extra:NULL,
            uiVersion,
            uiExpire,
            pTss->m_szBuf2K))
        {
            ///�γ����ݰ�ʧ��
            CWX_ERROR(("Failure to pack key package, err:%s", pTss->m_szBuf2K));
            iRet = -1;
            break;
        }
        if (!pTss->m_pWriter->addKeyValue(UNISTOR_KEY_K, pTss->m_pItemWriter->getMsg(), pTss->m_pItemWriter->getMsgSize(),true)){
            CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Failure to pack  package, err:%s", pTss->m_pItemWriter->getErrMsg());
            CWX_ERROR((pTss->m_szBuf2K));
            iRet = -1;
            break;
        }
        uiTotalLen += CwxPackageEx::getKvLen(strlen(UNISTOR_KEY_M),  pTss->m_pItemWriter->getMsgSize());
        if (uiTotalLen >= m_exportSession->m_uiChunk) break;
    }
    if (-1 == iRet) return -1;
    if ((0 != iRet) && (0 == uiTotalLen)) return 0;
    if (uiTotalLen){
        pTss->m_pWriter->pack();
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packMultiExportData(0,
            pTss->m_pWriter->getMsg(),
            pTss->m_pWriter->getMsgSize(),
            pBlock,
            ullSeq,
            pTss->m_szBuf2K))
        {
            return -1;
        }
        ///����svr���ͣ��������ݰ�
        pBlock->send_ctrl().setConnId(CWX_APP_INVALID_CONN_ID);
        pBlock->send_ctrl().setSvrId(m_bInner?UnistorApp::SVR_TYPE_INNER_SYNC:UnistorApp::SVR_TYPE_OUTER_SYNC);
        pBlock->send_ctrl().setHostId(0);
        pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
        if (!putMsg(pBlock)){
            CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Failure to send export data");
            CWX_ERROR((pTss->m_szBuf2K));
            CwxMsgBlockAlloc::free(pBlock);
            return -1;
        }
        m_ullSentSeq = ullSeq;
        m_exportSession->m_ullSeq++;
    }
    if (0 == iRet){///���������Ϣ���ر�����
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packExportEnd(pTss->m_pWriter,
            pBlock,
            0,
            m_pApp->getStore()->getBinLogMgr()->getMaxSid(),
            pTss->m_szBuf2K))
        {
            return -1;
        }
        ///����svr���ͣ��������ݰ�
        pBlock->send_ctrl().setConnId(CWX_APP_INVALID_CONN_ID);
        pBlock->send_ctrl().setSvrId(m_bInner?UnistorApp::SVR_TYPE_INNER_SYNC:UnistorApp::SVR_TYPE_OUTER_SYNC);
        pBlock->send_ctrl().setHostId(0);
        pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::CLOSE_NOTICE);
        if (!putMsg(pBlock)){
            CwxCommon::snprintf(pTss->m_szBuf2K, 2047, "Failure to send export end msg");
            CWX_ERROR((pTss->m_szBuf2K));
            CwxMsgBlockAlloc::free(pBlock);
            return -1;
        }
    }
    return 1; ///������һ����Ϣ
}
