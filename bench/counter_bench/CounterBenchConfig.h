#ifndef __COUNTER_BENCH_CONFIG_H__
#define __COUNTER_BENCH_CONFIG_H__

#include "CwxHostInfo.h"
#include "CwxCommon.h"
#include "CwxIniParse.h"

CWINUX_USING_NAMESPACE

///kvѹ�����Ե������ļ����ض���
class CounterBenchConfig
{
public:
    CounterBenchConfig(){
        m_unConnNum = 0;
		m_uiDataBase=10000000; ///<��ѯ�ķ�Χ�����µ���������
        m_bLasting = true;
		m_uiKeyGroup = 1;
		m_uiKeyIndex = 0;
        m_uiDataMod = 0;
        m_bGetMaster = false;
        m_bCache = false;
        m_uiValue = 0;

    }
    
    ~CounterBenchConfig(){}
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
    CWX_UINT32          m_uiDataMod;  ///<���ݵķ�Χ
    bool                m_bGetMaster; ///<�Ƿ��master��ȡ����
    bool                m_bCache; ///<�Ƿ�cache
	CWX_UINT32			m_uiDataBase; ///<�������ݵķ�Χ��
	CWX_UINT32			m_uiKeyGroup; ///<��ǰ��key�ķ���
	CWX_UINT32			m_uiKeyIndex; ///<key�����
    string              m_strKeyType; ///<key������
    list<string>        m_counters; ///<������
    CWX_UINT32          m_uiValue; ///<������ֵ
    string              m_strUser; ///<�û���
    string              m_strPasswd; ///<�û�����
    char                m_szError[2048];///<������Ϣbuf
};

#endif
