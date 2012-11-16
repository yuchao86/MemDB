#include "UnistorConfig.h"
#include "UnistorPoco.h"
#include "CwxFile.h"
#include "CwxLogger.h"
#include "UnistorDef.h"
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
///获取主机的所有网卡的IP列表。0：成功；-1：失败
static int getHostIpAddr(list<string>& ips){
    int s;
    struct ifconf conf;
    struct ifreq *ifr;
    char buff[BUFSIZ];
    int num;
    int i;
    string ip;
    s = socket(PF_INET, SOCK_DGRAM, 0);
    conf.ifc_len = BUFSIZ;
    conf.ifc_buf = buff;
    ioctl(s, SIOCGIFCONF, &conf);
    num = conf.ifc_len / sizeof(struct ifreq);
    ifr = conf.ifc_req;
    for(i=0;i < num;i++)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);
        ioctl(s, SIOCGIFFLAGS, ifr);
        ip = inet_ntoa(sin->sin_addr);
        ips.push_back(ip);
        ifr++;
    }
    return 0;
}

int UnistorConfig::init(string const& strCnfFile){
    string value;
    if (!m_cnf.load(strCnfFile)){
		strcpy(m_szErrMsg, m_cnf.getErrMsg());
		return -1;
	}
	/*****************load common config**************************/
    //load common:home
	if (!m_cnf.getAttr("common", "home", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [common:home] for running path.");
        return -1;
    }
	m_common.m_strWorkDir = value;
	if ('/' != value[value.length()-1]) m_common.m_strWorkDir +="/";
    //load common:thread_num
	if (m_cnf.getAttr("common", "thread_num", value)){
		m_common.m_uiThreadNum = strtoul(value.c_str(), NULL, 10);
		if (m_common.m_uiThreadNum < 1) m_common.m_uiThreadNum =1;
	}else{
		m_common.m_uiThreadNum = 4;
	}
	//load common:store_type
	if (!m_cnf.getAttr("common", "store_type", value) || !value.length()){
		snprintf(m_szErrMsg, 2047, "Must set [common:store_type].");
		return -1;
	}
	m_common.m_strStoreType = value;
	//load common:sock_buf_kbyte
	if (m_cnf.getAttr("common", "sock_buf_kbyte", value)){
		m_common.m_uiSockBufSize = strtoul(value.c_str(), NULL, 10);
	}
    if (m_common.m_uiSockBufSize){
        if (m_common.m_uiSockBufSize < UNISTOR_MIN_SOCK_BUF_KB){
            m_common.m_uiSockBufSize = UNISTOR_MIN_SOCK_BUF_KB;
        }
    }
	if (m_common.m_uiSockBufSize > UNISTOR_MAX_SOCK_BUF_KB){
		m_common.m_uiSockBufSize = UNISTOR_MAX_SOCK_BUF_KB;
	}
	//load common:max_chunk_kbyte
	if (m_cnf.getAttr("common", "max_chunk_kbyte", value)){
		m_common.m_uiChunkSize = strtoul(value.c_str(), NULL, 10);
        if (m_common.m_uiChunkSize){
            if (m_common.m_uiChunkSize < UNISTOR_MIN_CHUNK_SIZE_KB){
                m_common.m_uiChunkSize = UNISTOR_MIN_CHUNK_SIZE_KB;
            }
        }
        if (m_common.m_uiChunkSize > UNISTOR_MAX_CHUNK_SIZE_KB){
            m_common.m_uiChunkSize = UNISTOR_MAX_CHUNK_SIZE_KB;
        }
	}
    
	//load common:write_cache_flush_num
	if (m_cnf.getAttr("common", "write_cache_flush_num", value)){
		m_common.m_uiWriteCacheFlushNum = strtoul(value.c_str(), NULL, 10);
	}
	if (m_common.m_uiWriteCacheFlushNum < UNISTOR_MIN_WRITE_CACHE_FLUSH_NUM){
		m_common.m_uiWriteCacheFlushNum = UNISTOR_MIN_WRITE_CACHE_FLUSH_NUM;
	}
	if (m_common.m_uiWriteCacheFlushNum > UNISTOR_MAX_WRITE_CACHE_FLUSH_NUM){
		m_common.m_uiWriteCacheFlushNum = UNISTOR_MAX_WRITE_CACHE_FLUSH_NUM;
	}
    //load common:write_cache_flush_second
    if (m_cnf.getAttr("common", "write_cache_flush_second", value)){
        m_common.m_uiWriteCacheFlushSecond = strtoul(value.c_str(), NULL, 10);
    }
    if (m_common.m_uiWriteCacheFlushSecond < UNISTOR_MIN_WRITE_CACHE_FLUSH_SECOND){
        m_common.m_uiWriteCacheFlushSecond = UNISTOR_MIN_WRITE_CACHE_FLUSH_SECOND;
    }
    if (m_common.m_uiWriteCacheFlushSecond > UNISTOR_MAX_WRITE_CACHE_FLUSH_SECOND){
        m_common.m_uiWriteCacheFlushSecond = UNISTOR_MAX_WRITE_CACHE_FLUSH_SECOND;
    }

    
    //get master_lost_binlog
    if (!m_cnf.getAttr("common", "master_lost_binlog", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [common:master_lost_binlog].");
        return -1;
    }
    m_common.m_uiMasterLostBinlog = strtoul(value.c_str(), NULL, 10);
    //get host
    if (!m_cnf.getAttr("common", "host", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [common:host].");
        return -1;
    }
    m_common.m_strHost = value;
    {//检测host
        list<string> ips;
        getHostIpAddr(ips);
        list<string>::iterator iter = ips.begin();
        while(iter != ips.end()){
            if (value == *iter) break;
            iter++;
        }
        if (iter == ips.end()){
            snprintf(m_szErrMsg, 2047, "[common:host]:%s is the host ip.", value.c_str());
            return -1;
        }
    }
    //get idc
    if (!m_cnf.getAttr("common", "idc", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [common:idc].");
        return -1;
    }
    m_common.m_strIdc = value;
    //get group
    if (!m_cnf.getAttr("common", "group", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [common:group].");
        return -1;
    }
    m_common.m_strGroup = value;

    //load common:monitor
    if (m_cnf.getAttr("common", "monitor", value)){
        if (!parseHostPort(value, m_common.m_monitor)){
            snprintf(m_szErrMsg, 2047, "[common:monitor] is invalid, it should be [ip:port] format.");
            return -1;
        }
    }else{
        m_common.m_monitor.reset();
    }
    //load common:trans_conn_num
    if (m_cnf.getAttr("common", "trans_conn_num", value)){
        m_common.m_uiTranConnNum = strtoul(value.c_str(), NULL, 10);
    }
    if (m_common.m_uiTranConnNum < UNISTOR_MIN_CONN_NUM){
        m_common.m_uiTranConnNum = UNISTOR_MIN_CONN_NUM;
    }
    if (m_common.m_uiTranConnNum > UNISTOR_MAX_CONN_NUM){
        m_common.m_uiTranConnNum = UNISTOR_MAX_CONN_NUM;
    }
    //load common:sync_conn_num
    if (m_cnf.getAttr("common", "sync_conn_num", value)){
        m_common.m_uiSyncConnNum = strtoul(value.c_str(), NULL, 10);
    }
    if (m_common.m_uiSyncConnNum < UNISTOR_MIN_CONN_NUM){
        m_common.m_uiSyncConnNum = UNISTOR_MIN_CONN_NUM;
    }
    if (m_common.m_uiSyncConnNum > UNISTOR_MAX_CONN_NUM){
        m_common.m_uiSyncConnNum = UNISTOR_MAX_CONN_NUM;
    }
    //load common:write_cache_mbyte
    if (m_cnf.getAttr("common", "write_cache_mbyte", value)){
        m_common.m_uiWriteCacheMByte = strtoul(value.c_str(), NULL, 10);
    }else{
        m_common.m_uiReadCacheMByte = 0;
    }
    //load common:read_cache_mbyte
    if (m_cnf.getAttr("common", "read_cache_mbyte", value)){
        m_common.m_uiReadCacheMByte = strtoul(value.c_str(), NULL, 10);
    }else{
        m_common.m_uiReadCacheMByte = 0;
    }
    //load common:read_cache_max_key_num
    if (m_cnf.getAttr("common", "read_cache_max_key_num", value)){
        m_common.m_uiReadCacheMaxKeyNum = strtoul(value.c_str(), NULL, 10);
    }else{
        m_common.m_uiReadCacheMaxKeyNum = 0;
    }
    //load common:max_write_queue_messge
    if (m_cnf.getAttr("common", "max_write_queue_messge", value)){
        m_common.m_uiMaxWriteQueueNum = strtoul(value.c_str(), NULL, 10);
    }
    if (m_common.m_uiMaxWriteQueueNum > UNISTOR_MAX_MAX_WRITE_QUEUE_MSG_NUM){
        m_common.m_uiMaxWriteQueueNum = UNISTOR_MAX_MAX_WRITE_QUEUE_MSG_NUM;
    }else if (m_common.m_uiMaxWriteQueueNum < UNISTOR_MIN_MAX_WRITE_QUEUE_MSG_NUM){
        m_common.m_uiMaxWriteQueueNum = UNISTOR_MIN_MAX_WRITE_QUEUE_MSG_NUM;
    }
    //load common:max_trans_message
    if (m_cnf.getAttr("common", "max_write_queue_messge", value)){
        m_common.m_uiMaxMasterTranMsgNum = strtoul(value.c_str(), NULL, 10);
    }
    if (m_common.m_uiMaxMasterTranMsgNum > UNISTOR_MAX_MAX_MASTER_TRAN_MSG_NUM){
        m_common.m_uiMaxMasterTranMsgNum = UNISTOR_MAX_MAX_MASTER_TRAN_MSG_NUM;
    }else if (m_common.m_uiMaxMasterTranMsgNum < UNISTOR_MIN_MAX_MASTER_TRAN_MSG_NUM){
        m_common.m_uiMaxMasterTranMsgNum = UNISTOR_MIN_MAX_MASTER_TRAN_MSG_NUM;
    }

    //load common:enable_expire
    if (m_cnf.getAttr("common", "enable_expire", value) && value.length()){
        CwxCommon::trim(value);
        if (value == "yes"){
            m_common.m_bEnableExpire = true;
        }else if (value == "no"){
            m_common.m_bEnableExpire = false;
        }else{
            snprintf(m_szErrMsg, 2047, "[common:enable_expire] must be yes/no.");
            return -1;
        }
    }else{
        snprintf(m_szErrMsg, 2047, "Must set [common:enable_expire].");
        return -1;
    }
    if (m_common.m_bEnableExpire){
        //load common:expire_default
        if (m_cnf.getAttr("common", "expire_default", value) && value.length()){
            m_common.m_uiDefExpire = strtoul(value.c_str(), NULL, 0);
            if (!m_common.m_uiDefExpire){
                m_common.m_uiDefExpire = 1;
            }
        }else{
            snprintf(m_szErrMsg, 2047, "Must set [common:expire_default].");
            return -1;
        }
        //load common:expire_default
        if (m_cnf.getAttr("common", "expire_concurrent", value) && value.length()){
            m_common.m_uiExpireConcurrent = strtoul(value.c_str(), NULL, 0);
            if (!m_common.m_uiExpireConcurrent){
                m_common.m_uiExpireConcurrent = UNISTOR_DEF_EXPIRE_CONNCURRENT;
            }else{
                if (m_common.m_uiExpireConcurrent > UNISTOR_MAX_EXPIRE_CONNCURRENT){
                    m_common.m_uiExpireConcurrent = UNISTOR_MAX_EXPIRE_CONNCURRENT;
                }else if (m_common.m_uiExpireConcurrent < UNISTOR_MIN_EXPIRE_CONNCURRENT){
                    m_common.m_uiExpireConcurrent = UNISTOR_MIN_EXPIRE_CONNCURRENT;
                }
            }
        }else{
            snprintf(m_szErrMsg, 2047, "Must set [common:expire_concurrent].");
            return -1;
        }
    }else{
        m_common.m_uiExpireConcurrent = 0;
    }

    //load zk
    //get zk:server
    if (!m_cnf.getAttr("zk", "server", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [zk:server].");
        return -1;
    }
    m_zk.m_strZkServer = value;
    //get zk:auth
    if (m_cnf.getAttr("zk", "auth", value)){
        m_zk.m_strAuth = value;
    }
    //get zk:root
    if (!m_cnf.getAttr("zk", "root", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [zk:root].");
        return -1;
    }
    m_zk.m_strPath = value;
    if (value[value.length()-1] != '/') m_zk.m_strPath += "/";
    //get inner_dispatch
    if (!getListenInfo("inner_dispatch", m_dispatchInner)) return -1;
    //get outer_dispatch
    if (!getListenInfo("outer_dispatch", m_dispatchOuter)) return -1;
    //get recv
    if (!getListenInfo("recv", m_recv)) return -1;


	//load binlog:path
	if (!m_cnf.getAttr("binlog", "path", value) || !value.length()){
		snprintf(m_szErrMsg, 2047, "Must set [binlog:path].");
		return -1;
	}
	m_binlog.m_strBinlogPath = value;
	if ('/' != value[value.length()-1]) m_binlog.m_strBinlogPath+="/"; 
	//load binlog:file_prefix
	if (!m_cnf.getAttr("binlog", "file_prefix", value) || !value.length()){
		snprintf(m_szErrMsg, 2047, "Must set [binlog:file_prefix].");
		return -1;
	}
	m_binlog.m_strBinlogPrex = value;
	//load binlog:file_max_mbyte
	if (m_cnf.getAttr("binlog", "file_max_mbyte", value)){
		m_binlog.m_uiBinLogMSize = strtoul(value.c_str(), NULL, 10);
	}
	if (m_binlog.m_uiBinLogMSize < UnistorConfigBinLog::MIN_BINLOG_MSIZE){
		m_binlog.m_uiBinLogMSize = UnistorConfigBinLog::MIN_BINLOG_MSIZE;
	}
	if (m_binlog.m_uiBinLogMSize > UnistorConfigBinLog::MAX_BINLOG_MSIZE){
		m_binlog.m_uiBinLogMSize = UnistorConfigBinLog::MAX_BINLOG_MSIZE;
	}
	//load binlog:max_file_num
	if (m_cnf.getAttr("binlog", "max_file_num", value)){
		m_binlog.m_uiMgrFileNum = strtoul(value.c_str(), NULL, 10);
	}
	if (m_binlog.m_uiMgrFileNum < CwxBinLogMgr::MIN_MANAGE_FILE_NUM){
		m_binlog.m_uiMgrFileNum = CwxBinLogMgr::MIN_MANAGE_FILE_NUM;
	}
	if (m_binlog.m_uiMgrFileNum > CwxBinLogMgr::MAX_MANAGE_FILE_NUM){
		m_binlog.m_uiMgrFileNum = CwxBinLogMgr::MAX_MANAGE_FILE_NUM;
	}
	//load binlog:del_out_file
	m_binlog.m_bDelOutdayLogFile = false;
	if (m_cnf.getAttr("binlog", "del_out_file", value)){
		if (value == "yes") m_binlog.m_bDelOutdayLogFile = true;
	}
	//load binlog:cache
	m_binlog.m_bCache  = false;
	if (m_cnf.getAttr("binlog", "cache", value)){
		if (value == "yes") m_binlog.m_bCache = true;
	}
	//load binlog:flush_log_num
	if (m_cnf.getAttr("binlog", "flush_log_num", value)){
		m_binlog.m_uiFlushNum = strtoul(value.c_str(), NULL, 10);
	}
	if (m_binlog.m_uiFlushNum < 1){
		m_binlog.m_uiFlushNum = 1;
	}
	if (m_binlog.m_uiFlushNum > UNISTOR_MAX_BINLOG_FLUSH_COUNT){
		m_binlog.m_uiFlushNum = UNISTOR_MAX_BINLOG_FLUSH_COUNT;
	}
	//load binlog:flush_log_second
	if (m_cnf.getAttr("binlog", "flush_log_second", value)){
		m_binlog.m_uiFlushSecond = strtoul(value.c_str(), NULL, 10);
	}
	if (m_binlog.m_uiFlushSecond < 1){
		m_binlog.m_uiFlushSecond = 1;
	}
    return 0;
}
bool UnistorConfig::getListenInfo(string const& session, CwxHostInfo& host)
{
    string value;
    host.reset();
    //load user
    if (!m_cnf.getAttr(session, "user", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [%s:user].", session.c_str());
        return false;
    }
    host.setUser(value);
    //load passwd
    if (!m_cnf.getAttr(session, "passwd", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [%s:passwd].", session.c_str());
        return false;
    }
    host.setPassword(value);
    //load listen
    if (!m_cnf.getAttr(session, "listen", value) || !value.length()){
        snprintf(m_szErrMsg, 2047, "Must set [%s:listen].", session.c_str());
        return false;
    }
    if (!parseHostPort(value, host)){
        snprintf(m_szErrMsg, 2047, "[%s:listen] is invalid, it should be [ip:port] format.", session.c_str());
        return false;
    }
    return true;
}



void UnistorConfig::outputConfig() const
{
    CWX_INFO(("*****************begin common conf*******************"));
    CWX_INFO(("home=%s", m_common.m_strWorkDir.c_str()));
    CWX_INFO(("thread_num=%u", m_common.m_uiThreadNum));
    CWX_INFO(("store_type=%s", m_common.m_strStoreType.c_str()));
    CWX_INFO(("sock_buf_kbyte=%u", m_common.m_uiSockBufSize));
    CWX_INFO(("max_chunk_kbyte=%u", m_common.m_uiChunkSize));
    CWX_INFO(("write_cache_flush_num=%u", m_common.m_uiWriteCacheFlushNum));
    CWX_INFO(("write_cache_flush_second=%u", m_common.m_uiWriteCacheFlushSecond));
    CWX_INFO(("host=%s", m_common.m_strHost.c_str()));
    CWX_INFO(("idc=%s", m_common.m_strIdc.c_str()));
    CWX_INFO(("group=%s", m_common.m_strGroup.c_str()));
    CWX_INFO(("monitor=%s:%u", m_common.m_monitor.getHostName().c_str(), m_common.m_monitor.getPort()));
    CWX_INFO(("trans_conn_num=%u", m_common.m_uiTranConnNum));
    CWX_INFO(("sync_conn_num=%u", m_common.m_uiSyncConnNum));
    CWX_INFO(("write_cache_mbyte=%u", m_common.m_uiWriteCacheMByte));
    CWX_INFO(("read_cache_mbyte=%u", m_common.m_uiReadCacheMByte));
    CWX_INFO(("read_cache_max_key_num=%u", m_common.m_uiReadCacheMaxKeyNum));
    CWX_INFO(("master_lost_binlog=%u", m_common.m_uiMasterLostBinlog));
    CWX_INFO(("max_write_queue_messge=%u", m_common.m_uiMaxWriteQueueNum));
    CWX_INFO(("max_trans_message=%u", m_common.m_uiMaxMasterTranMsgNum));

    CWX_INFO(("enable_expire=%s", m_common.m_bEnableExpire?"yes":"no"));
    CWX_INFO(("expire_default=%u", m_common.m_uiDefExpire));
    CWX_INFO(("expire_concurrent=%u", m_common.m_uiExpireConcurrent));
	CWX_INFO(("*****************end common conf*******************"));

    CWX_INFO(("*****************begin zk conf*******************"));
    CWX_INFO(("server=%s", m_zk.m_strZkServer.c_str()));
    CWX_INFO(("auth=%s", m_zk.m_strAuth.c_str()));
    CWX_INFO(("root=%s", m_zk.m_strPath.c_str()));
    CWX_INFO(("*****************end zk conf*******************"));

    CWX_INFO(("*****************begin inner_dispatch conf*******************"));
    CWX_INFO(("user=%s", m_dispatchInner.getUser().c_str()));
    CWX_INFO(("passwd=%s", m_dispatchInner.getPasswd().c_str()));
    CWX_INFO(("listen=%s:%u", m_dispatchInner.getHostName().c_str(), m_dispatchInner.getPort()));
    CWX_INFO(("*****************end inner_dispatch conf*******************"));

    CWX_INFO(("*****************begin outer_dispatch conf*******************"));
    CWX_INFO(("user=%s", m_dispatchOuter.getUser().c_str()));
    CWX_INFO(("passwd=%s", m_dispatchOuter.getPasswd().c_str()));
    CWX_INFO(("listen=%s:%u", m_dispatchOuter.getHostName().c_str(), m_dispatchOuter.getPort()));
    CWX_INFO(("*****************end outer_dispatch conf*******************"));

    CWX_INFO(("*****************begin recv conf*******************"));
    CWX_INFO(("user=%s", m_recv.getUser().c_str()));
    CWX_INFO(("passwd=%s", m_recv.getPasswd().c_str()));
    CWX_INFO(("listen=%s:%u", m_recv.getHostName().c_str(), m_recv.getPort()));
    CWX_INFO(("*****************end recv conf*******************"));

	CWX_INFO(("*****************begin binlog conf****************"));
	CWX_INFO(("path=%s", m_binlog.m_strBinlogPath.c_str()));
	CWX_INFO(("file_prefix=%s", m_binlog.m_strBinlogPrex.c_str()));
	CWX_INFO(("file_max_mbyte=%u", m_binlog.m_uiBinLogMSize));
	CWX_INFO(("max_file_num=%u", m_binlog.m_uiMgrFileNum));
	CWX_INFO(("del_out_file=%s",  m_binlog.m_bDelOutdayLogFile?"yes":"no"));
	CWX_INFO(("cache=%s", m_binlog.m_bCache?"yes":"no"));
	CWX_INFO(("flush_log_num=%u", m_binlog.m_uiFlushNum));
	CWX_INFO(("flush_log_second=%u", m_binlog.m_uiFlushSecond));
	CWX_INFO(("*****************end binlog conf****************"));
}

