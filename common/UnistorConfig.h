#ifndef __UNISTOR_CONFIG_H__
#define __UNISTOR_CONFIG_H__


#include "UnistorMacro.h"
#include "CwxGlobalMacro.h"
#include "CwxHostInfo.h"
#include "CwxCommon.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxBinLogMgr.h"
#include "CwxIniParse.h"



CWINUX_USING_NAMESPACE

///配置文件的common参数对象
class UnistorConfigCmn{
public:
    UnistorConfigCmn(){
		m_uiThreadNum = 4;
		m_uiSockBufSize = UNISTOR_DEF_SOCK_BUF_KB;
		m_uiChunkSize = UNISTOR_DEF_CHUNK_SIZE_KB;
		m_uiWriteCacheFlushNum = UNISTOR_DEF_WRITE_CACHE_FLUSH_NUM;
        m_uiWriteCacheFlushSecond = UNISTOR_DEF_WRITE_CACHE_FLUSH_SECOND;
        m_uiTranConnNum = UNISTOR_DEF_CONN_NUM;
        m_uiSyncConnNum = UNISTOR_DEF_CONN_NUM;
        m_uiWriteCacheMByte = 0;
        m_uiReadCacheMByte = 0;
        m_uiReadCacheMaxKeyNum = 0;
        m_uiMasterLostBinlog = 0;
        m_bEnableExpire = false;
        m_uiDefExpire = 0;
        m_uiExpireConcurrent = UNISTOR_DEF_EXPIRE_CONNCURRENT;
        m_uiMaxWriteQueueNum = UNISTOR_DEF_MAX_WRITE_QUEUE_MSG_NUM;
        m_uiMaxMasterTranMsgNum = UNISTOR_DEF_MAX_MASTER_TRAN_MSG_NUM;
    };
public:
    string              m_strWorkDir;///<工作目录
	CWX_UINT32			m_uiThreadNum; ///<数据获取线程数量
	string              m_strStoreType; ///<存储类型。
	CWX_UINT32          m_uiSockBufSize; ///<分发的socket连接的buf大小
	CWX_UINT32          m_uiChunkSize; ///<Trunk的大小
	CWX_UINT32          m_uiWriteCacheFlushNum; ///<更新多少flush write cache
    CWX_UINT32          m_uiWriteCacheFlushSecond; ///<多少时间flush write cache
    CWX_UINT32          m_uiMasterLostBinlog; ///<变为master允许丢失的最多binlog
    string              m_strHost; ///<主机标示
    string              m_strIdc; ///<主机所属IDC
    string              m_strGroup; ///<主机所属分组
    CwxHostInfo			m_monitor; ///<监控端口
    CWX_UINT32          m_uiWriteCacheMByte; ///<写cache的M大小
    CWX_UINT32          m_uiReadCacheMByte;   ///<read cache的M大小
    CWX_UINT32          m_uiReadCacheMaxKeyNum; ///<read cache的hash key数量
    CWX_UINT32          m_uiTranConnNum; ///<转发的连接数量
    CWX_UINT32          m_uiSyncConnNum; ///<同步的连接数量
    bool                m_bEnableExpire; ///<是否支持expire
    CWX_UINT32          m_uiDefExpire; ///<若支持超时，则缺省的超时时间
    CWX_UINT32          m_uiExpireConcurrent; ///<超时处理的并发
    CWX_UINT32          m_uiMaxWriteQueueNum; ///<写队列最大等待消息的梳理
    CWX_UINT32          m_uiMaxMasterTranMsgNum; ///<转发给master的最大消息数量
};

///zk的配置
class UnistorConfigZk{
public:
    UnistorConfigZk(){}
public:
    string      m_strZkServer; ///<zk的server地址，多个server以[;]分割
    string      m_strAuth; ///<认证字符串，多个以[;]分割
    string      m_strPath; ///<根路径
};


///配置文件的binlog参数对象
class UnistorConfigBinLog{
public:
	enum
	{
		DEF_BINLOG_MSIZE = 1024, ///<缺省的binlog大小
		MIN_BINLOG_MSIZE = 64, ///<最小的binlog大小
		MAX_BINLOG_MSIZE = 2048 ///<最大的binlog大小
	};
public:
	UnistorConfigBinLog()	{
		m_uiBinLogMSize = DEF_BINLOG_MSIZE;
		m_uiMgrFileNum = CwxBinLogMgr::DEF_MANAGE_FILE_NUM;
		m_bDelOutdayLogFile = false;
		m_uiFlushNum = 100;
		m_uiFlushSecond = 30;
		m_bCache = true;
	}
public:
	string              m_strBinlogPath; ///<binlog的目录
	string              m_strBinlogPrex; ///<binlog的文件的前缀
	CWX_UINT32          m_uiBinLogMSize; ///<binlog文件的最大大小，单位为M
	CWX_UINT32          m_uiMgrFileNum; ///<管理的binglog的最大文件数
	bool                m_bDelOutdayLogFile; ///<是否删除不管理的消息文件
	CWX_UINT32          m_uiFlushNum; ///<接收多少条记录后，flush binlog文件
	CWX_UINT32          m_uiFlushSecond; ///<间隔多少秒，必须flush binlog文件
	bool				m_bCache;        ///<是否对写入的数据进行cache
};


///配置文件加载对象
class UnistorConfig{
public:
    ///构造函数
    UnistorConfig(){
        m_szErrMsg[0] = 0x00;
    }
    ///析构函数
    ~UnistorConfig(){
    }
public:
    //加载配置文件.-1:failure, 0:success
    int init(string const& strCnfFile);
    //输出加载的配置文件信息
    void outputConfig() const;
public:
    ///获取common配置信息
    inline UnistorConfigCmn const& getCommon() const {
        return  m_common;
    }
    ///获取zookeeper的配置
	inline UnistorConfigZk const& getZk() const{
        return  m_zk;
    }
    ///获取内部dispatch的配置
    inline CwxHostInfo const& getInnerDispatch() const{
        return m_dispatchInner;  ///<内部分发配置
    }
    ///获取外部dispatch的配置
    inline CwxHostInfo const& getOuterDispatch() const{
        return m_dispatchOuter;   ///<外部分发配置
    }
    ///获取recv的配置
    inline CwxHostInfo const& getRecv() const{
        return m_recv;           ///<数据接收配置
    }
    ///获取binlog
    inline UnistorConfigBinLog const& getBinlog() const{
        return m_binlog; ///<binlog的配置
    }
	///获取配置文件定义信息
	inline CwxIniParse const& getConfFileCnf() const{
		return m_cnf;
	}
    ///获取配置文件加载的失败原因
    inline char const* getErrMsg() const{
        return m_szErrMsg;
    };
	///获取存储的驱动
	inline string const& getDriverFile(string const& strEnginePath, string& strEngineFile) const{
		strEngineFile = strEnginePath + "libuni_" + m_common.m_strStoreType + ".so";
		return strEngineFile;
	}
private:
    bool getListenInfo(string const& session, CwxHostInfo& host);
private:
    UnistorConfigCmn      m_common; ///<common的配置信息
	UnistorConfigBinLog   m_binlog; ///<binlog的配置信息
	UnistorConfigZk       m_zk; ///<zookeeper的配置信息
	CwxHostInfo           m_dispatchInner;  ///<内部分发配置
    CwxHostInfo           m_dispatchOuter;   ///<外部分发配置
    CwxHostInfo           m_recv;           ///<数据接收配置
    CwxIniParse           m_cnf;    ///<配置
    char                  m_szErrMsg[2048];///<错误消息的buf
};

#endif
