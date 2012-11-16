#include "UnistorHandler4Master.h"
#include "UnistorApp.h"
#include "CwxZlib.h"

///配置变化
void UnistorHandler4Master::configChange(UnistorTss* pTss){
    string strSyncHost = pTss->getSyncHost();
    if (pTss->isMasterIdc()){
        if (pTss->isMaster() || (strSyncHost == m_pApp->getConfig().getCommon().m_strHost) ){///我是master idc的master，则不需要同步
            CWX_INFO(("UnistorHandler4Master: I'am master-idc's master."));
            ///如果存在同步session，则释放
            if (m_syncSession) closeSession();
        }else{///不是master，则需要从prev，若前一个不存在则从master同步
            CWX_INFO(("UnistorHandler4Master: I'am master-idc's slave. sync host:%s", pTss->getSyncHost()));
            if (!m_syncSession || m_syncSession->m_strHost != pTss->getSyncHost()){
                CWX_INFO(("UnistorHandler4Master: Sync host is changed from [%s] to [%s]", m_syncSession?m_syncSession->m_strHost.c_str():"", pTss->getSyncHost()));
                if (m_syncSession) closeSession();
            }
            if (!m_syncSession &&  strSyncHost.length()){
                CWX_INFO(("UnistorHandler4Master: create connection to sync host:%s, conn_num:%u", strSyncHost.c_str(), m_pApp->getConfig().getCommon().m_uiSyncConnNum));
                createSession(pTss,
                    strSyncHost,
                    m_pApp->getConfig().getInnerDispatch().getPort(),
                    m_pApp->getConfig().getCommon().m_uiSyncConnNum,
                    m_pApp->getConfig().getInnerDispatch().getUser().c_str(),
                    m_pApp->getConfig().getInnerDispatch().getPasswd().c_str(),
                    m_pApp->getConfig().getCommon().m_uiChunkSize,
                    "*",
                    "",
                    false);
            }
        }
    }else{
        if (pTss->isMaster() || (strSyncHost == m_pApp->getConfig().getCommon().m_strHost)){///从主idc同步
            CWX_INFO(("UnistorHandler4Master: I'm slave idc's master."));
            if (pTss->m_pZkConf && isNeedReconnect(pTss->m_pZkConf->m_transInfo)){
                CWX_INFO(("UnistorHandler4Master: Reconnect to master-idc host...."));
                list<string>::iterator iter = pTss->m_pZkConf->m_transInfo.m_ips.begin();
                while(iter != pTss->m_pZkConf->m_transInfo.m_ips.end()){
                    CWX_INFO(("UnistorHandler4Master: Reconnect to master-idc host:%s", iter->c_str()));
                    if (0 == createSession(pTss,
                        *iter,
                        pTss->m_pZkConf->m_transInfo.m_unPort,
                        pTss->m_pZkConf->m_transInfo.m_unConnNum,
                        pTss->m_pZkConf->m_transInfo.m_strUser.c_str(),
                        pTss->m_pZkConf->m_transInfo.m_strPasswd.c_str(),
                        pTss->m_pZkConf->m_transInfo.m_uiChunkSize,
                        "*",
                        pTss->m_pZkConf->m_transInfo.m_strSign.c_str(),
                        pTss->m_pZkConf->m_transInfo.m_bZip))
                    {
                        break;
                    }
                    iter++;
                }
            }
        }else{///不是slave idc的master，则需要从master同步
            CWX_INFO(("UnistorHandler4Master: I'm slave idc's slave."));
            if (!m_syncSession || m_syncSession->m_strHost != pTss->getSyncHost()){
                if (m_syncSession) closeSession();
            }
            if (!m_syncSession && strSyncHost.length()){
                CWX_INFO(("UnistorHandler4Master: Connect to self-idc's master:%s.", strSyncHost.c_str()));
                createSession(pTss,
                    strSyncHost,
                    m_pApp->getConfig().getInnerDispatch().getPort(),
                    m_pApp->getConfig().getCommon().m_uiSyncConnNum,
                    m_pApp->getConfig().getInnerDispatch().getUser().c_str(),
                    m_pApp->getConfig().getInnerDispatch().getPasswd().c_str(),
                    m_pApp->getConfig().getCommon().m_uiChunkSize,
                    "*",
                    "",
                    false);
            }
        }
    }
}

bool UnistorHandler4Master::isNeedReconnect(UnistorTransInfo const& trans){
    if (!m_syncSession) return true;
    list<string>::const_iterator iter = trans.m_ips.begin();
    while(iter != trans.m_ips.end()){
        if (*iter == m_syncSession->m_strHost) break;
        iter++;
    }
    if (iter == trans.m_ips.end()) return true; ///主机改变
    if (trans.m_bZip != m_syncSession->m_bZip) return true;
    if (trans.m_strSign != m_syncSession->m_strSign) return true;
    if (trans.m_strUser != m_syncSession->m_strUser) return true;
    if (trans.m_strPasswd != m_syncSession->m_strPasswd) return true;
    if (trans.m_unPort != m_syncSession->m_unPort) return true;
    if (trans.m_uiChunkSize != m_syncSession->m_uiChunkSize) return true;
    if (trans.m_unConnNum != m_syncSession->m_unConnNum) return true;
    return false;
}

void UnistorHandler4Master::timecheck(UnistorTss* pTss){
    if (!m_syncSession && (!pTss->isMasterIdc() ||  !pTss->isMaster())){
        static CWX_UINT32 ttCheckTimeoutTime = 0;
        static CWX_UINT32 ttTimeBase = 0;
        CWX_UINT32 uiNow = time(NULL);
        bool bClockBack = UnistorApp::isClockBack(ttTimeBase, uiNow);
        if (bClockBack || (ttCheckTimeoutTime + UNISTOR_CONN_TIMEOUT_SECOND < uiNow)){
            ttCheckTimeoutTime = uiNow;
            CWX_INFO(("UnistorHandler4Master: Rebuild sync session."));
            configChange(pTss);
        }
    }else{
        ///如果回复超时，则关闭连接
        CWX_UINT32 now =time(NULL);
        if (m_syncSession){
            if (!m_syncSession->m_ullSessionId){
                if (m_syncSession->m_uiReportDatetime + UNISTOR_REPORT_TIMEOUT < now){
                    CWX_INFO(("UnistorHandler4Master: report timeout, report_time=%u, now=%u, close session for %s", m_syncSession->m_uiReportDatetime, now, m_syncSession->m_strHost.c_str()));
                    closeSession();
                }
            }
            if (m_syncSession->isTimeout(now)){
                CWX_INFO(("UnistorHandler4Master: Sync timeout, close session for %s", m_syncSession->m_strHost.c_str()));
                closeSession();
            }
        }
    }
}

int UnistorHandler4Master::createSession(UnistorTss* pTss,
                                         string const& strHost,
                                         CWX_UINT16 unPort,
                                         CWX_UINT16 unConnNum,
                                         char const* user,
                                         char const* passwd,
                                         CWX_UINT32 uiChunk,
                                         char const* subscribe,
                                         char const* sign,
                                         bool  bZip)
{
    if (m_syncSession) closeSession();
    if (strHost.length()){
        ///重建所有连接
        CwxINetAddr addr;
        if (0 != addr.set(unPort, strHost.c_str())){
            CWX_ERROR(("Failure to init addr, addr:%s, port:%u, err=%d", strHost.c_str(), unPort, errno));
            return -1;
        }
        CWX_UINT32 i=0;
        if (unConnNum < 1) unConnNum = 1;
        int* fds = new int[unConnNum];
        for (i=0; i<unConnNum; i++){
            fds[i] = -1;
        }
        CwxTimeValue timeout(UNISTOR_CONN_TIMEOUT_SECOND);
        CwxTimeouter timeouter(&timeout);
        if (0 != UnistorConnector::connect(addr,
            unConnNum,
            fds,
            &timeouter,
            true,
            UnistorApp::setConnSockAttr,
            m_pApp->getSyncCliSockAttr()))
        {
            CWX_ERROR(("Failure to connect to addr:%s, port:%u, err=%d", strHost.c_str(), unPort, errno)); 
            return -1;
        }
        m_uiCurHostId++;
        if (0 == m_uiCurHostId) m_uiCurHostId = 1;
        m_syncSession = new UnistorSyncSession(m_uiCurHostId);
        ///注册连接id
        int ret = 0;
        for (i=0; i<unConnNum; i++){
            if ((ret = m_pApp->noticeHandle4Msg(UnistorApp::SVR_TYPE_MASTER,
                m_uiCurHostId,
                fds[i]))<0)
            {
                CWX_ERROR(("Failure to register sync handler to framework."));
                break;
            }
            m_syncSession->m_conns[(CWX_UINT32)ret] = false;
        }
        if (i != unConnNum){///失败
            for (; i<unConnNum; i++){
                ::close(fds[i]);
            }
            closeSession();
            return -1;
        }
        ///发送report的消息
        ///创建往master报告sid的通信数据包
        CwxMsgBlock* pBlock = NULL;
        ret = UnistorPoco::packReportData(pTss->m_pWriter,
            pBlock,
            0,
            m_pApp->getStore()->getBinLogMgr()->getMaxSid(),
            false,
            uiChunk,
            subscribe,
            user,
            passwd,
            sign,
            bZip,
            pTss->m_szBuf2K);
        if (ret != UNISTOR_ERR_SUCCESS){///数据包创建失败
            CWX_ERROR(("Failure to create report package, err:%s", pTss->m_szBuf2K));
            closeSession();
            return -1;
        } else {
            CWX_INFO(("Report sid to master:%s", strHost.c_str()));
            ///发送消息
            pBlock->send_ctrl().setConnId(m_syncSession->m_conns.begin()->first);
            pBlock->send_ctrl().setSvrId(UnistorApp::SVR_TYPE_MASTER);
            pBlock->send_ctrl().setHostId(m_syncSession->m_uiHostId);
            pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
            if (0 != m_pApp->sendMsgByConn(pBlock)){//connect should be closed
                CWX_ERROR(("Failure to report sid to master:%s", strHost.c_str()));
                CwxMsgBlockAlloc::free(pBlock);
                closeSession();
                return -1;
            }
        }
        m_syncSession->m_strHost = strHost;
        m_syncSession->m_unPort = unPort;
        m_syncSession->m_strUser = user;
        m_syncSession->m_strPasswd = passwd;
        m_syncSession->m_strSign = sign;
        m_syncSession->m_bZip = bZip;
        m_syncSession->m_uiChunkSize = uiChunk;
        m_syncSession->m_unConnNum = unConnNum;
        m_syncSession->m_uiReportDatetime = time(NULL);
        return 0;
    }
    return -1;
}


//关闭已有连接
void UnistorHandler4Master::closeSession(){
    CWX_INFO(("UnistorHandler4Master: Close sync session"));
    if (m_syncSession){
        map<CWX_UINT32,  bool>::iterator iter = m_syncSession->m_conns.begin();
        while(iter != m_syncSession->m_conns.end()){
            m_pApp->noticeCloseConn(iter->first);
            iter++;
        }
        delete m_syncSession;
        m_syncSession = NULL;
    }
}

///master的连接关闭后，需要清理环境
int UnistorHandler4Master::onConnClosed(CwxMsgBlock*& msg, CwxTss* ){
    CWX_ERROR(("Master is closed. master:%s", m_syncSession?m_syncSession->m_strHost.c_str():""));
    if (m_pApp->getRecvWriteHandler()->isCanWrite()) return 1;
    CWX_INFO(("session is null:%s, conn_host id:%u, session host_id:%u", m_syncSession?"no":"yes",msg->event().getHostId() ,  m_syncSession?m_syncSession->m_uiHostId:0));
    if (m_syncSession && (msg->event().getHostId() == m_syncSession->m_uiHostId)){
        CWX_INFO(("Close session, host_id=%u", m_syncSession->m_uiHostId));
        closeSession();
        CWX_INFO(("Flush binlog for sync-connection closed."));
        m_pApp->getStore()->flushBinlog();
    }
    return 1;
}

int UnistorHandler4Master::recvMsg(CwxMsgBlock*& msg, list<CwxMsgBlock*>& msgs){
    CWX_UINT64 ullSeq = 0;
    if (!m_syncSession->m_ullSessionId){
        CWX_ERROR(("No Session"));
        return -1;
    }
    if (msg->length() < sizeof(ullSeq)){
        CWX_ERROR(("sync data's size[%u] is invalid, min-size=%u", msg->length(), sizeof(ullSeq)));
        return -1;
    }
    map<CWX_UINT32,  bool/*是否已经report*/>::iterator conn_iter = m_syncSession->m_conns.find(msg->event().getConnId());
    if (conn_iter == m_syncSession->m_conns.end()){
        CWX_ERROR(("Conn-id[%u] doesn't exist.", msg->event().getConnId()));
        return -1;
    }
    ullSeq =  UnistorPoco::getSeq(msg->rd_ptr());
    if (!m_syncSession->recv(ullSeq, msg, msgs)){
        CWX_ERROR(("Conn-id[%u] doesn't exist.", msg->event().getConnId()));
        return -1;
    }
    return 0;
}

///接收来自master的消息
int UnistorHandler4Master::onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv){
    UnistorTss* pTss = (UnistorTss*)pThrEnv;
    if (!m_syncSession){///关闭连接
        m_pApp->noticeCloseConn(msg->event().getConnId());
        return 1;
    }
    if (msg->event().getHostId() != m_syncSession->m_uiHostId){///关闭连接
        m_pApp->noticeCloseConn(msg->event().getConnId());
        return 1;
    }
    int ret = -1;
    do{
        if (!msg || !msg->length()){
            CWX_ERROR(("Recv empty msg from master:%s", m_syncSession->m_strHost.c_str()));
            break;
        }
        //SID报告的回复，此时，一定是报告失败
        if (UnistorPoco::MSG_TYPE_SYNC_DATA == msg->event().getMsgHeader().getMsgType() ||
            UnistorPoco::MSG_TYPE_SYNC_DATA_CHUNK == msg->event().getMsgHeader().getMsgType())
        {
            list<CwxMsgBlock*> msgs;
            if (0 != recvMsg(msg, msgs)) break;
            msg = NULL;
            CwxMsgBlock* block = NULL;
            list<CwxMsgBlock*>::iterator msg_iter = msgs.begin();
            while(msg_iter != msgs.end()){
                block = *msg_iter;
                if (UnistorPoco::MSG_TYPE_SYNC_DATA == block->event().getMsgHeader().getMsgType()){
                    ret = dealSyncData(block, pTss);
                }else{
                    ret = dealSyncChunkData(block, pTss);
                }
                if (block) CwxMsgBlockAlloc::free(block);
                if (0 != ret) break;
                msg_iter++;
            }
            if (msg_iter != msgs.end()){
                while(msg_iter != msgs.end()){
                    block = *msg_iter;
                    CwxMsgBlockAlloc::free(block);
                    msg_iter++;
                }
                break;
            }
        }else if (UnistorPoco::MSG_TYPE_SYNC_REPORT_REPLY == msg->event().getMsgHeader().getMsgType()){
            if (0 != dealSyncReportReply(msg, pTss)) break;
        }else if (UnistorPoco::MSG_TYPE_SYNC_ERR == msg->event().getMsgHeader().getMsgType()){
            dealErrMsg(msg, pTss);
            break;
        }else{
            CWX_ERROR(("Recv invalid msg type from master, msg_type=%u, server=%s", msg->event().getMsgHeader().getMsgType(), m_syncSession->m_strHost.c_str()));
            break;
        }
        ret = 0;
    }while(0);
    if (-1 == ret){
        closeSession();
    }
    return 1;
}

int UnistorHandler4Master::dealSyncReportReply(CwxMsgBlock*& msg, UnistorTss* pTss){
    char szTmp[64];
    ///此时，对端会关闭连接
    CWX_UINT64 ullSessionId = 0;
    if (m_syncSession->m_ullSessionId){
        CWX_ERROR(("Session id is replied, session id:%s", CwxCommon::toString(m_syncSession->m_ullSessionId, szTmp, 10)));
        return -1;
    }
    if (m_syncSession->m_conns.find(msg->event().getConnId()) == m_syncSession->m_conns.end()){
        CWX_ERROR(("Conn-id[%u] doesn't exist.", msg->event().getConnId()));
        return -1;
    }
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseReportDataReply(pTss->m_pReader,
        msg,
        ullSessionId,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to parse report-reply msg from master:%s, err=%s", 
            m_syncSession->m_strHost.c_str(),
            pTss->m_szBuf2K));
        return -1;
    }
    m_syncSession->m_ullSessionId = ullSessionId;
    m_syncSession->m_conns[msg->event().getConnId()] = true;
    ///其他连接报告
    map<CWX_UINT32,  bool/*是否已经report*/>::iterator iter = m_syncSession->m_conns.begin();
    while(iter != m_syncSession->m_conns.end()){
        if (iter->first != msg->event().getConnId()){
            CwxMsgBlock* pBlock = NULL;
            int ret = UnistorPoco::packReportNewConn(pTss->m_pWriter,
                pBlock,
                0,
                ullSessionId,
                pTss->m_szBuf2K);
            if (ret != UNISTOR_ERR_SUCCESS)    {///数据包创建失败
                CWX_ERROR(("Failure to create report package, err:%s", pTss->m_szBuf2K));
                return -1;
            } else {
                ///发送消息
                pBlock->send_ctrl().setConnId(iter->first);
                pBlock->send_ctrl().setSvrId(UnistorApp::SVR_TYPE_MASTER);
                pBlock->send_ctrl().setHostId(m_syncSession->m_uiHostId);
                pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
                if (0 != m_pApp->sendMsgByConn(pBlock)){//connect should be closed
                    CWX_ERROR(("Failure to report new session connection to master"));
                    CwxMsgBlockAlloc::free(pBlock);
                    return -1;
                }
            }
            m_syncSession->m_conns[iter->first]=true;
        }
        iter++;
    }
    return 0;
}

int UnistorHandler4Master::dealSyncData(CwxMsgBlock*& msg, UnistorTss* pTss){
    ///此时，对端会关闭连接
    unsigned long ulUnzipLen = 0;
    bool bZip = false;
    int iRet = 0;
    CWX_UINT32 ullSeq =  UnistorPoco::getSeq(msg->rd_ptr());
    bZip = msg->event().getMsgHeader().isAttr(CwxMsgHead::ATTR_COMPRESS);
    //判断是否压缩数据
    if (bZip){//压缩数据，需要解压
        //首先准备解压的buf
        if (!prepareUnzipBuf()){
            CWX_ERROR(("Failure to prepare unzip buf, size:%u", m_uiBufLen));
            return -1;
        }
        ulUnzipLen = m_uiBufLen;
        //解压
        if (!CwxZlib::unzip(m_unzipBuf,
            ulUnzipLen,
            (const unsigned char*)(msg->rd_ptr() + sizeof(ullSeq)),
            msg->length() - sizeof(ullSeq)))
        {
            CWX_ERROR(("Failure to unzip msg"));
            return -1;
        }
    }
    if (bZip){
        iRet = saveBinlog(pTss, (char*)m_unzipBuf, ulUnzipLen);
    }else{
        iRet = saveBinlog(pTss, msg->rd_ptr() + sizeof(ullSeq), msg->length() - sizeof(ullSeq));
    }
    if (0 != iRet) return -1;
    //回复发送者
    CwxMsgBlock* reply_block = NULL;
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncDataReply(pTss->m_pWriter,
        reply_block,
        msg->event().getMsgHeader().getTaskId(),
        ullSeq,
        UnistorPoco::MSG_TYPE_SYNC_DATA_REPLY,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack sync data reply, errno=%s", pTss->m_szBuf2K));
        return -1;
    }
    reply_block->send_ctrl().setConnId(msg->event().getConnId());
    reply_block->send_ctrl().setSvrId(UnistorApp::SVR_TYPE_MASTER);
    reply_block->send_ctrl().setHostId(m_syncSession->m_uiHostId);
    reply_block->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
    if (0 != m_pApp->sendMsgByConn(reply_block)){//connect should be closed
        CWX_ERROR(("Failure to send sync data reply to master"));
        CwxMsgBlockAlloc::free(reply_block);
        return -1;
    }
    return 0;
}

int UnistorHandler4Master::dealSyncChunkData(CwxMsgBlock*& msg, UnistorTss* pTss){
    ///此时，对端会关闭连接
    CWX_UINT64 ullSeq = 0;
    unsigned long ulUnzipLen = 0;
    bool bZip = false;
    CWX_UINT32 i=0;
    ullSeq =  UnistorPoco::getSeq(msg->rd_ptr());
    bZip = msg->event().getMsgHeader().isAttr(CwxMsgHead::ATTR_COMPRESS);
    //判断是否压缩数据
    if (bZip){//压缩数据，需要解压
        //首先准备解压的buf
        if (!prepareUnzipBuf()){
            CWX_ERROR(("Failure to prepare unzip buf, size:%u", m_uiBufLen));
            return -1;
        }
        ulUnzipLen = m_uiBufLen;
        //解压
        if (!CwxZlib::unzip(m_unzipBuf,
            ulUnzipLen,
            (const unsigned char*)(msg->rd_ptr() + sizeof(ullSeq)),
            msg->length() - sizeof(ullSeq)))
        {
            CWX_ERROR(("Failure to unzip package."));
            return -1;
        }
    }
    if (bZip){
        if (!m_reader.unpack((char*)m_unzipBuf, ulUnzipLen, false, true)){
            CWX_ERROR(("Failure to unpack master multi-binlog, err:%s", m_reader.getErrMsg()));
            return -1;
        }
    }else{
        if (!m_reader.unpack(msg->rd_ptr() + sizeof(ullSeq), msg->length() - sizeof(ullSeq), false, true)){
            CWX_ERROR(("Failure to unpack master multi-binlog, err:%s", m_reader.getErrMsg()));
            return -1;
        }
    }
    //检测签名
    bool bSign = false;
    if (m_syncSession->m_strSign.length()) {
        CwxKeyValueItemEx const* pItem = m_reader.getKey(m_syncSession->m_strSign.c_str());
        if (pItem){//存在签名key
            if (!checkSign(m_reader.getMsg() + sizeof(ullSeq),
                pItem->m_szKey - CwxPackageEx::getKeyOffset(pItem->m_unKeyLen, pItem->m_uiDataLen) - m_reader.getMsg() - sizeof(ullSeq),
                pItem->m_szData,
                m_syncSession->m_strSign.c_str()))
            {
                CWX_ERROR(("Failure to check %s sign", m_syncSession->m_strSign.c_str()));
                return -1;
            }
            bSign = true;
        }
    }
    for (i=0; i<m_reader.getKeyNum() - bSign; i++){
        if(0 != strcmp(m_reader.getKey(i)->m_szKey, UNISTOR_KEY_M)) {
            CWX_ERROR(("Master multi-binlog's key must be:%s, but:%s", UNISTOR_KEY_M, m_reader.getKey(i)->m_szKey));
            return -1;
        }
        if (-1 == saveBinlog(pTss, m_reader.getKey(i)->m_szData, m_reader.getKey(i)->m_uiDataLen)){
            return -1;
        }
    }
    if (i != m_reader.getKeyNum() - bSign) return -1;
    if (0 == i){
        CWX_ERROR(("Master multi-binlog's key hasn't key"));
        return -1;
    }
    //回复发送者
    CwxMsgBlock* reply_block = NULL;
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncDataReply(pTss->m_pWriter,
        reply_block,
        msg->event().getMsgHeader().getTaskId(),
        ullSeq,
        UnistorPoco::MSG_TYPE_SYNC_DATA_CHUNK_REPLY,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to pack sync data reply, errno=%s", pTss->m_szBuf2K));
        return -1;
    }
    reply_block->send_ctrl().setConnId(msg->event().getConnId());
    reply_block->send_ctrl().setSvrId(UnistorApp::SVR_TYPE_MASTER);
    reply_block->send_ctrl().setHostId(m_syncSession->m_uiHostId);
    reply_block->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
    if (0 != m_pApp->sendMsgByConn(reply_block)){//connect should be closed
        CWX_ERROR(("Failure to send sync data reply to master"));
        CwxMsgBlockAlloc::free(reply_block);
        return -1;
    }

    return 0;

}

int UnistorHandler4Master::dealErrMsg(CwxMsgBlock*& msg, UnistorTss* pTss){
    int ret = 0;
    char const* szErrMsg;
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncErr(pTss->m_pReader,
        msg,
        ret,
        szErrMsg,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to parse err msg from master:%s, err=%s", 
            m_syncSession->m_strHost.c_str(),
            pTss->m_szBuf2K));
        return -1;
    }
    CWX_ERROR(("Failure to sync from master:%s, ret=%d, err=%s",
        m_syncSession->m_strHost.c_str(),
        ret,
        szErrMsg));
    ///通知存储引擎失去了连接
    if (UNISTOR_ERR_LOST_SYNC == ret){
        m_pApp->getStore()->lostSync();
    }
    return 0;
}


//0：成功；-1：失败
int UnistorHandler4Master::saveBinlog(UnistorTss* pTss,
                                   char const* szBinLog,
                                   CWX_UINT32 uiLen)
{
    CWX_UINT64 ullSid;
    CWX_UINT32 uiVersion;
    CWX_UINT32 ttTimestamp;
    CWX_UINT32 uiGroup;
    CWX_UINT32 uiType;
    CwxKeyValueItemEx const* data;
    ///获取binlog的数据
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncData(pTss->m_pReader, 
        szBinLog,
        uiLen,
        ullSid,
        ttTimestamp,
        data,
        uiGroup,
        uiType,
        uiVersion,
        pTss->m_szBuf2K))
    {
        CWX_ERROR(("Failure to parse binlog from master, err=%s", pTss->m_szBuf2K));
        return -1;
    }
	CwxKeyValueItemEx item = *data;
	if (0 != m_pApp->getStore()->syncMasterBinlog(pTss,
        pTss->m_pItemReader,
        ullSid,
        ttTimestamp,
        uiGroup,
        uiType,
        item,
        uiVersion,
        false))
	{
		CWX_ERROR(("Failure to sync master binlog , err=%s", pTss->m_szBuf2K));
		if (!m_pApp->getStore()->isValid()) return -1;
	}
    return 0;
}

bool UnistorHandler4Master::checkSign(char const* data,
               CWX_UINT32 uiDateLen,
               char const* szSign,
               char const* sign)
{
    if (!sign) return true;
    if (strcmp(sign, UNISTOR_KEY_CRC32) == 0)//CRC32签名
    {
        CWX_UINT32 uiCrc32 = CwxCrc32::value(data, uiDateLen);
        if (memcmp(&uiCrc32, szSign, sizeof(uiCrc32)) == 0) return true;
        return false;
	}else if (strcmp(sign, UNISTOR_KEY_MD5)==0){//md5签名
        CwxMd5 md5;
        unsigned char szMd5[16];
        md5.update((const unsigned char*)data, uiDateLen);
        md5.final(szMd5);
        if (memcmp(szMd5, szSign, 16) == 0) return true;
        return false;
    }
    return true;
}

bool UnistorHandler4Master::prepareUnzipBuf(){
    if (!m_unzipBuf)
    {
        m_uiBufLen = m_pApp->getConfig().getCommon().m_uiChunkSize * 2;
        if (m_uiBufLen < UNISTOR_MAX_KVS_SIZE) m_uiBufLen = UNISTOR_MAX_KVS_SIZE;
        m_unzipBuf = new unsigned char[m_uiBufLen];
    }
    return m_unzipBuf!=NULL;
}
