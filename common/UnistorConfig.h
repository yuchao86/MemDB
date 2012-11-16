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

///�����ļ���common��������
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
    string              m_strWorkDir;///<����Ŀ¼
	CWX_UINT32			m_uiThreadNum; ///<���ݻ�ȡ�߳�����
	string              m_strStoreType; ///<�洢���͡�
	CWX_UINT32          m_uiSockBufSize; ///<�ַ���socket���ӵ�buf��С
	CWX_UINT32          m_uiChunkSize; ///<Trunk�Ĵ�С
	CWX_UINT32          m_uiWriteCacheFlushNum; ///<���¶���flush write cache
    CWX_UINT32          m_uiWriteCacheFlushSecond; ///<����ʱ��flush write cache
    CWX_UINT32          m_uiMasterLostBinlog; ///<��Ϊmaster����ʧ�����binlog
    string              m_strHost; ///<������ʾ
    string              m_strIdc; ///<��������IDC
    string              m_strGroup; ///<������������
    CwxHostInfo			m_monitor; ///<��ض˿�
    CWX_UINT32          m_uiWriteCacheMByte; ///<дcache��M��С
    CWX_UINT32          m_uiReadCacheMByte;   ///<read cache��M��С
    CWX_UINT32          m_uiReadCacheMaxKeyNum; ///<read cache��hash key����
    CWX_UINT32          m_uiTranConnNum; ///<ת������������
    CWX_UINT32          m_uiSyncConnNum; ///<ͬ������������
    bool                m_bEnableExpire; ///<�Ƿ�֧��expire
    CWX_UINT32          m_uiDefExpire; ///<��֧�ֳ�ʱ����ȱʡ�ĳ�ʱʱ��
    CWX_UINT32          m_uiExpireConcurrent; ///<��ʱ����Ĳ���
    CWX_UINT32          m_uiMaxWriteQueueNum; ///<д�������ȴ���Ϣ������
    CWX_UINT32          m_uiMaxMasterTranMsgNum; ///<ת����master�������Ϣ����
};

///zk������
class UnistorConfigZk{
public:
    UnistorConfigZk(){}
public:
    string      m_strZkServer; ///<zk��server��ַ�����server��[;]�ָ�
    string      m_strAuth; ///<��֤�ַ����������[;]�ָ�
    string      m_strPath; ///<��·��
};


///�����ļ���binlog��������
class UnistorConfigBinLog{
public:
	enum
	{
		DEF_BINLOG_MSIZE = 1024, ///<ȱʡ��binlog��С
		MIN_BINLOG_MSIZE = 64, ///<��С��binlog��С
		MAX_BINLOG_MSIZE = 2048 ///<����binlog��С
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
	string              m_strBinlogPath; ///<binlog��Ŀ¼
	string              m_strBinlogPrex; ///<binlog���ļ���ǰ׺
	CWX_UINT32          m_uiBinLogMSize; ///<binlog�ļ�������С����λΪM
	CWX_UINT32          m_uiMgrFileNum; ///<�����binglog������ļ���
	bool                m_bDelOutdayLogFile; ///<�Ƿ�ɾ�����������Ϣ�ļ�
	CWX_UINT32          m_uiFlushNum; ///<���ն�������¼��flush binlog�ļ�
	CWX_UINT32          m_uiFlushSecond; ///<��������룬����flush binlog�ļ�
	bool				m_bCache;        ///<�Ƿ��д������ݽ���cache
};


///�����ļ����ض���
class UnistorConfig{
public:
    ///���캯��
    UnistorConfig(){
        m_szErrMsg[0] = 0x00;
    }
    ///��������
    ~UnistorConfig(){
    }
public:
    //���������ļ�.-1:failure, 0:success
    int init(string const& strCnfFile);
    //������ص������ļ���Ϣ
    void outputConfig() const;
public:
    ///��ȡcommon������Ϣ
    inline UnistorConfigCmn const& getCommon() const {
        return  m_common;
    }
    ///��ȡzookeeper������
	inline UnistorConfigZk const& getZk() const{
        return  m_zk;
    }
    ///��ȡ�ڲ�dispatch������
    inline CwxHostInfo const& getInnerDispatch() const{
        return m_dispatchInner;  ///<�ڲ��ַ�����
    }
    ///��ȡ�ⲿdispatch������
    inline CwxHostInfo const& getOuterDispatch() const{
        return m_dispatchOuter;   ///<�ⲿ�ַ�����
    }
    ///��ȡrecv������
    inline CwxHostInfo const& getRecv() const{
        return m_recv;           ///<���ݽ�������
    }
    ///��ȡbinlog
    inline UnistorConfigBinLog const& getBinlog() const{
        return m_binlog; ///<binlog������
    }
	///��ȡ�����ļ�������Ϣ
	inline CwxIniParse const& getConfFileCnf() const{
		return m_cnf;
	}
    ///��ȡ�����ļ����ص�ʧ��ԭ��
    inline char const* getErrMsg() const{
        return m_szErrMsg;
    };
	///��ȡ�洢������
	inline string const& getDriverFile(string const& strEnginePath, string& strEngineFile) const{
		strEngineFile = strEnginePath + "libuni_" + m_common.m_strStoreType + ".so";
		return strEngineFile;
	}
private:
    bool getListenInfo(string const& session, CwxHostInfo& host);
private:
    UnistorConfigCmn      m_common; ///<common��������Ϣ
	UnistorConfigBinLog   m_binlog; ///<binlog��������Ϣ
	UnistorConfigZk       m_zk; ///<zookeeper��������Ϣ
	CwxHostInfo           m_dispatchInner;  ///<�ڲ��ַ�����
    CwxHostInfo           m_dispatchOuter;   ///<�ⲿ�ַ�����
    CwxHostInfo           m_recv;           ///<���ݽ�������
    CwxIniParse           m_cnf;    ///<����
    char                  m_szErrMsg[2048];///<������Ϣ��buf
};

#endif
