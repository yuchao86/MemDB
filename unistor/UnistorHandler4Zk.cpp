#include "UnistorHandler4Zk.h"
#include "UnistorApp.h"

UnistorHandler4Zk::UnistorHandler4Zk(UnistorApp* pApp):m_pApp(pApp){
    m_zk = NULL;
    m_zkLocker = NULL;
    m_bInit = false;
    m_bConnected = false;
    m_bAuth = false;
    m_bValid = false;
    m_clientId = NULL;
    m_strZkLockNode = pApp->getConfig().getZk().m_strPath + 
        pApp->getConfig().getCommon().m_strIdc  + "/"+
        pApp->getConfig().getCommon().m_strGroup + "/" + "lock";
    m_strZkConfNode = pApp->getConfig().getZk().m_strPath + 
        pApp->getConfig().getCommon().m_strIdc  + "/"+
        pApp->getConfig().getCommon().m_strGroup + "/" + "conf";
    m_strMasterNode = pApp->getConfig().getZk().m_strPath + 
        pApp->getConfig().getCommon().m_strIdc  + "/" +
        pApp->getConfig().getCommon().m_strGroup + "/" + "master";
    m_strHostNode = pApp->getConfig().getZk().m_strPath + 
        pApp->getConfig().getCommon().m_strIdc  + "/" +
        pApp->getConfig().getCommon().m_strGroup + "/" + "host";
    strcpy(m_szErr2K, "Not init.");
    m_ullVersion = 1;
}

///析构函数
UnistorHandler4Zk::~UnistorHandler4Zk(){
    stop();
    if (m_clientId){
        delete m_clientId;
        m_clientId = NULL;
    }
}

void UnistorHandler4Zk::stop(){
    CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
    if (m_zk){
        m_zk->disconnect();
    }
    if (m_zkLocker){
        m_zkLocker->unlock();
        delete m_zkLocker;
        m_zkLocker = NULL;
    }
    if (m_zk){
        delete m_zk;
        m_zk = NULL;
    }
}



int UnistorHandler4Zk::init(){
    CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
    return _init();
}

int UnistorHandler4Zk::_init(){
    _reset();
    m_zk = new ZkAdaptor(m_pApp->getConfig().getZk().m_strZkServer);
    if (0 != m_zk->init()){
        CwxCommon::snprintf(m_szErr2K, 2048, "Failure to invoke ZkAdaptor::init(), err:%s", m_zk->getErrMsg());
        CWX_ERROR((m_szErr2K));
        return -1;
    }
    if (0 != _connect()) return -1;
    ///初始化锁
    m_zkLocker = new ZkLocker();
    if (0 != m_zkLocker->init(m_zk,
        m_strZkLockNode,
        m_pApp->getConfig().getCommon().m_strHost,
        lock_complete,
        this))
    {
        CwxCommon::snprintf(m_szErr2K, 2048, "Failure to init zk lock");
        CWX_ERROR((m_szErr2K));
        return -1;
    }
    m_bInit = true;
    return 0;
}

///初始化
void UnistorHandler4Zk::_reset(){
    m_bInit = false;
    m_bConnected = false;
    m_bAuth = false;
    m_bValid = false;
    strcpy(m_szErr2K, "Not init.");
    if (m_zkLocker){
        m_zkLocker->unlock();
    }
    if (m_zk){
        m_zk->disconnect();
    }
    if (m_zkLocker){
        delete m_zkLocker;
        m_zkLocker = NULL;
    }
    if (m_zk){
        delete m_zk;
        m_zk = NULL;
    }
}

int UnistorHandler4Zk::_dealTimeoutEvent(CwxTss* , CwxMsgBlock*& , CWX_UINT32 ){
    if (!m_bInit){
        if (m_lock.m_strMaster.length()){
            m_lock.m_strMaster="";
            m_lock.m_strPrev="";
            m_lock.m_bMaster = false;
            _noticeLockChange();
        }
        _init();
    }
    if (m_bValid){
        _setHostInfo();
        if (m_zkLocker && m_zkLocker->isLocked()){
            _setMasterInfo();
        }
    }
    return 0;
}

int UnistorHandler4Zk::_dealConnectedEvent(CwxTss* tss, CwxMsgBlock*& msg, CWX_UINT32 ){
    int ret = 0;
    CWX_INFO(("Connected to zookeeper:%s", m_pApp->getConfig().getZk().m_strZkServer.c_str()));
    if (!m_bInit) return 0; ///如果没有init，则忽略
    ///连接成功
    m_bConnected = true;
    ///保存zk的client id
    if (msg->length()){
        if (!m_clientId) m_clientId = new clientid_t;
        memcpy(m_clientId, msg->rd_ptr(), sizeof(clientid_t));
    }else{
        if (m_clientId) delete m_clientId;
        m_clientId = NULL;
    }
    ///鉴权，若失败则返回
    ret = _auth();
    if (-1 == ret){///重新初始化
        return -1;
    }
    if (1 == ret){
        _setHostInfo();
        if (0 != _loadZkConf(tss)){
            return -1; ///加载配置失败则返回
        }
        if (0 !=_lock()){
            return -1;
        }
        m_bValid = true;
    }
    return 0;
}

void UnistorHandler4Zk::doEvent(CwxTss* tss, CwxMsgBlock*& msg , CWX_UINT32 uiLeftEvent){
    CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
    bool bSuccess=false;
    do{
        if (CwxEventInfo::TIMEOUT_CHECK == msg->event().getEvent()){
            if (0 != _dealTimeoutEvent(tss, msg, uiLeftEvent)) break;
        }else if (EVENT_ZK_CONNECTED == msg->event().getEvent()){
            if (0 != _dealConnectedEvent(tss, msg, uiLeftEvent)) break;
        }else if (EVENT_ZK_EXPIRED == msg->event().getEvent()){
            if (m_clientId) delete m_clientId;
            m_clientId = NULL;
            m_bInit = false;
        }else if (EVENT_ZK_FAIL_AUTH == msg->event().getEvent()){
            if (m_clientId) delete m_clientId;
            m_clientId = NULL;
            m_bInit = false;
        }else if (EVENT_ZK_SUCCESS_AUTH == msg->event().getEvent()){
            _setHostInfo();
            if (0 != _loadZkConf(tss)) break;
            if (0 != _lock()) break;
            if (m_bInit) m_bValid = true;
        }else if (EVENT_ZK_CONF_CHANGE == msg->event().getEvent()){
            CWX_INFO(("Conf node[%s] is changed", m_strZkConfNode.c_str()));
            if (0 != _loadZkConf(tss)) break;
            if (m_bInit) m_bValid = true;
        }else if (EVENT_ZK_LOCK_CHANGE == msg->event().getEvent()){
            if (0 != _loadLockInfo(tss)) break;
        }else if (EVENT_ZK_LOST_WATCH == msg->event().getEvent()){
            if (0 != _loadZkConf(tss)) break;
            if (0 != _lock()) break;
        }else {
            CWX_ERROR(("Unknown event type:%u", msg->event().getEvent()));
        }
        if (!m_zk) break;
        bSuccess = true;
    }while(0);

    if (!bSuccess){ ///重新初始化
        m_bInit = false;
    }
}

///连接0：成功；-1：失败
int UnistorHandler4Zk::_connect(){
    if (0 != m_zk->connect(m_clientId, 0, UnistorHandler4Zk::watcher, this)){
        CwxCommon::snprintf(m_szErr2K, 2048, "Failure to invoke ZkAdaptor::connect(), err:%s", m_zk->getErrMsg());
        CWX_ERROR((m_szErr2K));
        return -1;
    }
    return 0;
}

///认证；1：认证成功；0：等待认证结果；-1：认证失败
int UnistorHandler4Zk::_auth(){
    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (m_pApp->getConfig().getZk().m_strAuth.length()){
        if (!m_zk->addAuth("digest",
            m_pApp->getConfig().getZk().m_strAuth.c_str(),
            m_pApp->getConfig().getZk().m_strAuth.length(),
            0,
            UnistorHandler4Zk::zk_auth_callback,
            this))
        {
            CwxCommon::snprintf(m_szErr2K, 2048, "Failure to invoke ZkAdaptor::addAuth() ,err:%s", m_zk->getErrMsg());
            CWX_ERROR((m_szErr2K));
            return - 1;
        }
        ///等待鉴权
        return 0;
    }
    ///无需鉴权
    m_bAuth = true;
    return 1;
}

int UnistorHandler4Zk::_loadZkConf(CwxTss* ){
    string strPath;
    struct Stat stats;
    CWX_UINT32 uiLen = MAX_ZK_DATA_SIZE;
    int ret = 0;
    if (!m_bInit) return 0; ///如果没有init，则忽略
    //get master
    if (1 != (ret = m_zk->wgetNodeData(m_strZkConfNode,
        m_szZkDataBuf,
        uiLen,
        stats,
        UnistorHandler4Zk::nodeDataWatcher,
        this)))
    {
        if (0 != ret){
            CwxCommon::snprintf(m_szErr2K, 2048, "Failure to fetch conf node:%s, err:%s", m_strZkConfNode.c_str(), m_zk->getErrMsg());
        }else{
            CwxCommon::snprintf(m_szErr2K, 2048, "Conf node[%s] doesn't exist", m_strZkConfNode.c_str());
        }
        CWX_ERROR((m_szErr2K));
        return -1;
    }
    UnistorZkConf conf;
    if (0 != _parseConf(conf)){
        return -1;
    }
    if (!(m_conf == conf)){///配置发生改变
        m_conf = conf;
        _noticeConfChange();
    }
    return 0;
}

///解析配置文件；0：成功；-1：失败
int  UnistorHandler4Zk::_parseConf(UnistorZkConf& conf){
    CwxIniParse parse;
    if (!parse.parse(m_szZkDataBuf)){
        CwxCommon::snprintf(m_szErr2K, 2048, "Failure to parse conf, err=%s, node=%s", parse.getErrMsg(), m_strZkConfNode.c_str());
        CWX_ERROR((m_szErr2K));
        return 0;
    }
    string strValue;
    //get common:master_idc
    if (!parse.getAttr("common", "master_idc", strValue) || !strValue.length()){
        CwxCommon::snprintf(m_szErr2K, 2048, "Must set common:master_idc in conf, node=%s",  m_strZkConfNode.c_str());
        CWX_ERROR((m_szErr2K));
        return 0;
    }
    conf.m_strMasterIdc = strValue;
    if (conf.m_strMasterIdc != m_pApp->getConfig().getCommon().m_strIdc){///必须配置sync节点
        conf.m_bMasterIdc = false;
        //get sync:ip
        if (!parse.getAttr("sync", "ip", strValue) || !strValue.length()){
            CwxCommon::snprintf(m_szErr2K, 2048, "Must set sync:ip in conf, node=%s",  m_strZkConfNode.c_str());
            CWX_ERROR((m_szErr2K));
            return 0;
        }
        list<string> ips;
        list<string>::iterator iter;
        CwxCommon::split(strValue, ips, ',');
        iter = ips.begin();
        while(iter != ips.end()){
            strValue = *iter;
            CwxCommon::trim(strValue);
            if (strValue.length()) conf.m_transInfo.m_ips.push_back(strValue);
            iter++;
        }
        if (conf.m_transInfo.m_ips.begin() == conf.m_transInfo.m_ips.end()){
            CwxCommon::snprintf(m_szErr2K, 2048, "Must set sync:ip in conf, node=%s",  m_strZkConfNode.c_str());
            CWX_ERROR((m_szErr2K));
            return 0;
        }
        //get sync:port
        if (!parse.getAttr("sync", "port", strValue) || !strValue.length()){
            CwxCommon::snprintf(m_szErr2K, 2048, "Must set sync:port in conf, node=%s",  m_strZkConfNode.c_str());
            CWX_ERROR((m_szErr2K));
            return 0;
        }
        conf.m_transInfo.m_unPort = strtoul(strValue.c_str(), NULL, 10);
        //get sync:user
        conf.m_transInfo.m_strUser = "";
        if (parse.getAttr("sync", "user", strValue) && strValue.length()){
            conf.m_transInfo.m_strUser = strValue;
        }
        //get sync:passwd
        conf.m_transInfo.m_strPasswd = "";
        if (parse.getAttr("sync", "passwd", strValue) && strValue.length()){
            conf.m_transInfo.m_strPasswd = strValue;
        }
        //get sync:zip
        conf.m_transInfo.m_bZip = false;
        if (parse.getAttr("sync", "zip", strValue) && strValue.length()){
            conf.m_transInfo.m_bZip = strtoul(strValue.c_str(), NULL, 10)?true:false;
        }
        //get sync:sign
        conf.m_transInfo.m_strSign = "";
        if (parse.getAttr("sync", "sign", strValue) && strValue.length()){
            if ((strValue==UNISTOR_KEY_CRC32) || (strValue == UNISTOR_KEY_MD5))
                conf.m_transInfo.m_strSign = strValue;
        }
        //get sync:sync_conn_num
        if (parse.getAttr("sync", "sync_conn_num", strValue) && strValue.length()){
            conf.m_transInfo.m_unConnNum = strtoul(strValue.c_str(), NULL, 10);
            if (conf.m_transInfo.m_unConnNum < UNISTOR_MIN_CONN_NUM) conf.m_transInfo.m_unConnNum = UNISTOR_MIN_CONN_NUM;
            if (conf.m_transInfo.m_unConnNum > UNISTOR_MAX_CONN_NUM) conf.m_transInfo.m_unConnNum = UNISTOR_MAX_CONN_NUM;
        }else{
            conf.m_transInfo.m_unConnNum = UNISTOR_DEF_CONN_NUM;
        }
        //get sync:sync_conn_num
        if (parse.getAttr("sync", "max_chunk_kbyte", strValue) && strValue.length()){
            conf.m_transInfo.m_uiChunkSize = strtoul(strValue.c_str(), NULL, 10);
            if (conf.m_transInfo.m_uiChunkSize){
                if (conf.m_transInfo.m_uiChunkSize < UNISTOR_MIN_CHUNK_SIZE_KB) conf.m_transInfo.m_uiChunkSize = UNISTOR_MIN_CHUNK_SIZE_KB;
            }
            if (conf.m_transInfo.m_uiChunkSize > UNISTOR_MAX_CHUNK_SIZE_KB) conf.m_transInfo.m_uiChunkSize = UNISTOR_MAX_CHUNK_SIZE_KB;
        }else{
            conf.m_transInfo.m_uiChunkSize = 0;
        }

    }else{
        conf.m_bMasterIdc = true;
    }
    return 0;
}

///获取锁的信息；0：成功；-1：失败
int UnistorHandler4Zk::_loadLockInfo(CwxTss* ){
    string strValue;
    string strSession;
    uint64_t seq;
    UnistorZkLock lock;
    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (!m_zk) return 0;
    lock.m_bMaster = m_zkLocker->isLocked();
    m_zkLocker->getOwnerNode(strValue);

    CWX_INFO(("Lock changed, master=%s, master_host=%s",
        lock.m_bMaster?"yes":"no",
        strValue.length()?strValue.c_str():""));

    if (strValue.length()){
        if (!ZkLocker::splitSeqNode(strValue, seq, lock.m_strMaster, strSession)){
            return -1;
        }
    }else{
        return -1;
    }
    m_zkLocker->getPrevNode(strValue);
    if (strValue.length()){
        if (!ZkLocker::splitSeqNode(strValue, seq, lock.m_strPrev, strSession)){
            return -1;
        }
    }else{
        lock.m_strPrev = "";
    }
    if (lock.m_bMaster && !m_lock.m_bMaster){///变为master
        string strMaster;
        CWX_UINT64 ullMasterSid=0;
        CWX_UINT64 ullSid=0;
        string strTimeStamp;
        char szTmp1[64];
        char szTmp2[64];
        if (0 !=_getMasterInfo(strMaster, ullMasterSid, ullSid, strTimeStamp)){
            return -1;
        }
        ///设置前一个的最大sid
        lock.m_ullPreMasterMaxSid = ullSid;
        CWX_INFO(("I become master, last master info: master(%s), master_sid(%s), max_sid(%s), max_time(%s)",
            strMaster.length()?strMaster.c_str():"",
            CwxCommon::toString(ullMasterSid, szTmp1, 10),
            CwxCommon::toString(ullSid, szTmp2, 10),
            strTimeStamp.length()?strTimeStamp.c_str():""));

        if (strMaster != m_pApp->getConfig().getCommon().m_strHost){///不是我自己
            ullSid = m_pApp->getStore()->getBinLogMgr()->getMaxSid();
            if (ullSid < ullMasterSid){
                CWX_INFO(("I can't become master, master_sid(%s), mine_max_sid(%s)",
                    CwxCommon::toString(ullMasterSid, szTmp1, 10),
                    CwxCommon::toString(ullSid, szTmp2, 10)));
                return -1;
            }
        }
    }else{
        lock.m_ullPreMasterMaxSid = m_lock.m_ullPreMasterMaxSid;
    }
    if (!(m_lock == lock)){
        m_lock = lock;
        _noticeLockChange();
    }
    return 0;
}
///通知配置变化
void UnistorHandler4Zk::_noticeConfChange(){
    CwxMsgBlock* msg = NULL;
    UnistorZkConf* pConf = NULL;
    m_ullVersion++;
    m_conf.m_ullVersion = m_ullVersion;
    ///通知写线程
    pConf = new UnistorZkConf(m_conf);
    msg = CwxMsgBlockAlloc::malloc(sizeof(pConf));
    memcpy(msg->wr_ptr(), &pConf, sizeof(pConf));
    msg->wr_ptr(sizeof(pConf));
    msg->event().setSvrId(UnistorApp::SVR_TYPE_RECV_WRITE);
    msg->event().setEvent(EVENT_ZK_CONF_CHANGE);
    m_pApp->getWriteTheadPool()->appendHead(msg);
    ///通知转发线程
    pConf = new UnistorZkConf(m_conf);
    msg = CwxMsgBlockAlloc::malloc(sizeof(pConf));
    memcpy(msg->wr_ptr(), &pConf, sizeof(pConf));
    msg->wr_ptr(sizeof(pConf));
    msg->event().setSvrId(UnistorApp::SVR_TYPE_TRANSFER);
    msg->event().setEvent(EVENT_ZK_CONF_CHANGE);
    m_pApp->getTransThreadPool()->appendHead(msg);
    ///通知recieve线程
    CwxThreadPool** pools = m_pApp->getRecvThreadPools();
    for (CWX_UINT32 i=0; i<m_pApp->getConfig().getCommon().m_uiThreadNum; i++){
        pConf = new UnistorZkConf(m_conf);
        CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(sizeof(pConf));
        memcpy(msg->wr_ptr(), &pConf, sizeof(pConf));
        msg->wr_ptr(sizeof(pConf));
        msg->event().setSvrId(UnistorApp::SVR_TYPE_RECV);
        msg->event().setEvent(EVENT_ZK_CONF_CHANGE);
        pools[i]->appendHead(msg);
    }

}
///通知锁变化
void UnistorHandler4Zk::_noticeLockChange(){
    CwxMsgBlock* msg = NULL;
    UnistorZkLock* pLock = NULL;
    m_ullVersion++;
    m_lock.m_ullVersion = m_ullVersion;
    ///通知写线程
    pLock = new UnistorZkLock(m_lock);
    msg = CwxMsgBlockAlloc::malloc(sizeof(pLock));
    memcpy(msg->wr_ptr(), &pLock, sizeof(pLock));
    msg->wr_ptr(sizeof(pLock));
    msg->event().setSvrId(UnistorApp::SVR_TYPE_RECV_WRITE);
    msg->event().setEvent(EVENT_ZK_LOCK_CHANGE);
    m_pApp->getWriteTheadPool()->appendHead(msg);
    ///通知转发线程
    pLock = new UnistorZkLock(m_lock);
    msg = CwxMsgBlockAlloc::malloc(sizeof(pLock));
    memcpy(msg->wr_ptr(), &pLock, sizeof(pLock));
    msg->wr_ptr(sizeof(pLock));
    msg->event().setSvrId(UnistorApp::SVR_TYPE_TRANSFER);
    msg->event().setEvent(EVENT_ZK_LOCK_CHANGE);
    m_pApp->getTransThreadPool()->appendHead(msg);
    ///通知recieve线程
    CwxThreadPool** pools = m_pApp->getRecvThreadPools();
    for (CWX_UINT32 i=0; i<m_pApp->getConfig().getCommon().m_uiThreadNum; i++){
        pLock = new UnistorZkLock(m_lock);
        CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(sizeof(pLock));
        memcpy(msg->wr_ptr(), &pLock, sizeof(pLock));
        msg->wr_ptr(sizeof(pLock));
        msg->event().setSvrId(UnistorApp::SVR_TYPE_RECV);
        msg->event().setEvent(EVENT_ZK_LOCK_CHANGE);
        pools[i]->appendHead(msg);
    }
}


int UnistorHandler4Zk::_lock(){
    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (!m_zk) return 0;
    CWX_INFO(("Lock..................."));
    if (0 != m_zkLocker->lock(ZkLocker::ZK_WATCH_TYPE_ROOT)){
        CwxCommon::snprintf(m_szErr2K, 2048, "Failure to lock zookeeper node.");
        CWX_ERROR((m_szErr2K));
        return -1;
    }
    return 0;
}

///zk的认证的回调函数
void UnistorHandler4Zk::zk_auth_callback(int rc, const void *data){
    UnistorHandler4Zk* zk = (UnistorHandler4Zk*) data;
    CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
    msg->event().setSvrId(UnistorApp::SVR_TYPE_ZK);
    msg->event().setEvent(ZOK==rc?EVENT_ZK_SUCCESS_AUTH:EVENT_ZK_FAIL_AUTH);
    zk->m_pApp->getZkThreadPool()->append(msg);
}

///zk节点变更的watch回调函数
void UnistorHandler4Zk::nodeDataWatcher(zhandle_t *, int , int , const char *, void* context)
{
    UnistorHandler4Zk* zk = (UnistorHandler4Zk*) context;
    CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
    msg->event().setEvent(EVENT_ZK_CONF_CHANGE);
    msg->event().setSvrId(UnistorApp::SVR_TYPE_ZK);
   zk->m_pApp->getZkThreadPool()->append(msg);
}


///锁的回调函数
void UnistorHandler4Zk::lock_complete(bool , void* context){
    UnistorHandler4Zk* zk = (UnistorHandler4Zk*) context;
    CwxMsgBlock* msg = CwxMsgBlockAlloc::malloc(0);
    msg->event().setEvent(EVENT_ZK_LOCK_CHANGE);
    msg->event().setSvrId(UnistorApp::SVR_TYPE_ZK);
    zk->m_pApp->getZkThreadPool()->append(msg);
}


///内部的wacher function
void UnistorHandler4Zk::watcher(zhandle_t *,
                                int type,
                                int state,
                                const char *path,
                                void* context)
{
    UnistorHandler4Zk* zkHandler = (UnistorHandler4Zk*)context;
    CwxMsgBlock* msg = NULL;
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE){
            msg = CwxMsgBlockAlloc::malloc(sizeof(clientid_t));
            if (zkHandler->m_zk->getClientId()){
                memcpy(msg->wr_ptr(),  zkHandler->m_zk->getClientId(), sizeof(clientid_t));
                msg->wr_ptr(sizeof(clientid_t));
            }
            msg->event().setEvent(EVENT_ZK_CONNECTED);
            zkHandler->m_pApp->getZkThreadPool()->append(msg);
        } else if (state == ZOO_AUTH_FAILED_STATE) {
            msg = CwxMsgBlockAlloc::malloc(0);
            msg->event().setEvent(EVENT_ZK_FAIL_AUTH);
            zkHandler->m_pApp->getZkThreadPool()->append(msg);
        } else if (state == ZOO_EXPIRED_SESSION_STATE) {
            msg = CwxMsgBlockAlloc::malloc(0);
            msg->event().setEvent(EVENT_ZK_EXPIRED);
            zkHandler->m_pApp->getZkThreadPool()->append(msg);
        }else if (state == ZOO_CONNECTING_STATE){
            CWX_INFO(("Connecting zookeeper:%s", zkHandler->m_pApp->getConfig().getZk().m_strZkServer.c_str()));
        } else if (state == ZOO_ASSOCIATING_STATE){
            CWX_INFO(("Zookeeper associating"));
        }
    }else if (type == ZOO_CREATED_EVENT){
        CWX_INFO(("Zookeeper node is created:%s", path));
    }else if (type == ZOO_DELETED_EVENT){
        CWX_INFO(("Zookeeper node is dropped:%s", path));
    }else if (type == ZOO_CHANGED_EVENT){
        CWX_INFO(("Zookeeper node is changed:%s", path));
    }else if (type == ZOO_CHILD_EVENT){
        CWX_INFO(("Zookeeper node's child is changed:%s", path));
    }else if (type == ZOO_NOTWATCHING_EVENT){
        msg = CwxMsgBlockAlloc::malloc(0);
        msg->event().setEvent(EVENT_ZK_LOST_WATCH);
        zkHandler->m_pApp->getZkThreadPool()->append(msg);
        CWX_INFO(("Zookeeper node's watcher is lost:%s", path));
    }else{
        CWX_INFO(("Unknown event:type=%d, state=%d, path=%s", type, state, path?path:""));
    }
}

///设置host的信息
int UnistorHandler4Zk::_setHostInfo(){
    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (!m_zk) return 0;
    string strNode = m_strHostNode + "/" + m_pApp->getConfig().getCommon().m_strHost;
    char szTmp[128];
    char szSid[64];
    CwxCommon::snprintf(szTmp,
        128,
        "%s:%u",
        CwxCommon::toString(m_pApp->getStore()->getBinLogMgr()->getMaxSid(), szSid, 10),
        m_pApp->getStore()->getBinLogMgr()->getMaxTimestamp());
    int ret = m_zk->setNodeData(strNode, szTmp, strlen(szTmp));
    if (0 == ret){///创建节点
        ret = m_zk->createNode(strNode, "", 0);
        if (-1 == ret){
            CWX_ERROR(("Failure to create node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
            return -1;
        }
        ret = m_zk->setNodeData(strNode, szTmp, strlen(szTmp));
    }
    if (1 == ret) return 0;
    if (-1 == ret){
        CWX_ERROR(("Failure to set node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
        return -1;
    }
    //now, it's zero
    CWX_ERROR(("Failure to set node for not existing, node:%s", strNode.c_str()));
    return -1;
}

///设置master的信息
int UnistorHandler4Zk::_setMasterInfo(){
    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (!m_zk) return 0;
    string strNode = m_strMasterNode;
    string strTime;
    char szTmp[512];
    char szSid[64];
    char szSid1[64];
    CWX_UINT64 ullMaxSid = m_pApp->getStore()->getBinLogMgr()->getMaxSid();
    CWX_UINT64 ullMasterSid =0;
    if (!m_pApp->getStore()->getBinLogMgr()->getLastSidByNo(m_pApp->getConfig().getCommon().m_uiMasterLostBinlog, ullMasterSid, m_szErr2K)){
        CWX_ERROR(("Failure to get sid by no, err:%s", m_szErr2K));
        ullMasterSid = ullMaxSid > m_pApp->getConfig().getCommon().m_uiMasterLostBinlog?ullMaxSid-m_pApp->getConfig().getCommon().m_uiMasterLostBinlog:0;
    }
    CwxCommon::snprintf(szTmp,
        511,
        "%s:%s:%s:%s",
        m_pApp->getConfig().getCommon().m_strHost.c_str(),
        CwxCommon::toString(ullMasterSid, szSid, 10),
        CwxCommon::toString(ullMaxSid, szSid1, 10),
        CwxDate::getDateY4MDHMS2(m_pApp->getStore()->getBinLogMgr()->getMaxTimestamp(), strTime).c_str());
    int ret = m_zk->setNodeData(strNode, szTmp, strlen(szTmp));
    if (0 == ret){///创建节点
        ret = m_zk->createNode(strNode, szTmp, strlen(szTmp));
        if (-1 == ret){
            CWX_ERROR(("Failure to create master node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
            return -1;
        }
        if (-1 == ret){
            CWX_ERROR(("Failure to create master node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
            return -1;
        }
        return 0;
    }
    if (1 == ret) return 0;
    //it must be -1
    CWX_ERROR(("Failure to set master node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
    return -1;
}

///获取host的信息，0：成功；-1：失败
int UnistorHandler4Zk::_getHostInfo(char const* szHost,
                                    CWX_UINT64& ullSid,
                                    CWX_UINT32& uiTimeStamp)
{
    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (!m_zk) return 0;
    string strNode = m_strHostNode + "/" + szHost;
    char szTmp[128];
    struct Stat stat;
    CWX_UINT32 uiLen = 127;
    list<string> items;
    list<string>::iterator iter;
    int ret = m_zk->getNodeData(strNode, szTmp, uiLen, stat);
    if (1 == ret){
        CwxCommon::split(string(szTmp), items, ':');
        if (2 != items.size()){
            CWX_ERROR(("Host's data isn't in format:[sid:timestamp], node:%s, data:%s", strNode.c_str(), szTmp));
            return -1;
        }
        iter = items.begin();
        ullSid = strtoull(iter->c_str(), NULL, 10);
        iter++;
        uiTimeStamp = strtoull(iter->c_str(), NULL, 10);
        return 0;
    }
    if (0 == ret){
        CWX_ERROR(("Host node doesn't exist, node:%s", strNode.c_str()));
    }else{
        CWX_ERROR(("Failure to fetch host node's data, node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
    }
    return -1;
}
///获取master的信息
int UnistorHandler4Zk::_getMasterInfo(string& strMaster,
                                      CWX_UINT64& ullMasterSid,
                                      CWX_UINT64& ullSid,
                                      string& strTimeStamp)
{
    strMaster.erase();
    ullMasterSid = 0;
    ullSid = 0;
    strTimeStamp.erase();

    if (!m_bInit) return 0; ///如果没有init，则忽略
    if (!m_zk) return 0;
    string strNode = m_strMasterNode;
    char szTmp[512];
    struct Stat stat;
    CWX_UINT32 uiLen = 511;
    list<string> items;
    list<string>::iterator iter;
    int ret = m_zk->getNodeData(strNode, szTmp, uiLen, stat);
    szTmp[uiLen] = 0x00;
    if (1 == ret){
        CwxCommon::split(string(szTmp), items, ':');
        if (4 != items.size()){
            CWX_ERROR(("master's data isn't in format:[host:master_sid:sid:timestamp], node:%s, data:%s", strNode.c_str(), szTmp));
            return 0;
        }
        iter = items.begin();
        strMaster = *iter;
        iter++;
        ullMasterSid = strtoull(iter->c_str(), NULL, 10);
        iter++;
        ullSid = strtoull(iter->c_str(), NULL, 10);
        iter++;
        strTimeStamp = *iter;
        return 0;
    }
    if (0 == ret){
        CWX_ERROR(("Master node doesn't exist, node:%s", strNode.c_str()));
        return 0;
    }
    CWX_ERROR(("Failure to fetch master node's data, node:%s, err=%s", strNode.c_str(), m_zk->getErrMsg()));
    return -1;
}
