#ifndef __UNISTOR_BENCH_CONFIG_H__
#define __UNISTOR_BENCH_CONFIG_H__

#include "CwxHostInfo.h"
#include "CwxCommon.h"
#include "CwxIniParse.h"

CWINUX_USING_NAMESPACE

///kvѹ�����Ե������ļ����ض���
class UnistorBenchConfig
{
public:
    UnistorBenchConfig(){
        m_unConnNum = 0;
        m_uiDataSize = 0;
		m_bKeyOrder = true; ///<key�Ƿ�����
		m_uiDataBase=10000000; ///<��ѯ�ķ�Χ�����µ���������
        m_bLasting = true;
		m_uiKeyGroup = 1;
		m_uiKeyIndex = 0;
        m_uiDataMod = 0;
        m_uiExpire = 0;
        m_bGetMaster = false;
        m_bCache = false;

    }
    
    ~UnistorBenchConfig(){}
public:
    //���������ļ�.-1:failure, 0:success
    int loadConfig(string const & strConfFile);
    //��������ļ�
    void outputConfig();
    //��ȡ���������ļ���ʧ�ܴ�����Ϣ
    char const* getError() { return m_szError; };
    
public:
	string              m_strWorkDir;///<����Ŀ¼
	CwxHostInfo         m_listen;///<tcp���ӵĶԷ�listen��ַ
	string              m_strUnixPathFile;///<������unix domain���ӣ���Ϊ���ӵ�path-file
	CWX_UINT16          m_unConnNum;///<���ӵ�����
	bool                m_bLasting;///<�Ƿ�Ϊ�־����ӣ�����HTTP��keep-alive
	string				m_strOpr; ///<����
	CWX_UINT32           m_uiDataSize;///<���ݵĴ�С
    CWX_UINT32          m_uiDataMod;  ///<���ݵķ�Χ
    bool                m_bGetMaster; ///<�Ƿ��master��ȡ����
	bool				 m_bKeyOrder; ///<key�Ƿ�����
	bool				m_bGetOrder; ///<�Ƿ�get��ʱ����order
    bool                m_bCache; ///<�Ƿ�cache
	CWX_UINT32			m_uiDataBase; ///<�������ݵķ�Χ��
	CWX_UINT32			m_uiKeyGroup; ///<��ǰ��key�ķ���
	CWX_UINT32			m_uiKeyIndex; ///<key�����
    CWX_UINT32          m_uiExpire; ///<��ʱʱ��
    string              m_strUser; ///<�û���
    string              m_strPasswd; ///<�û�����
    char                m_szError[2048];///<������Ϣbuf
};

#endif
