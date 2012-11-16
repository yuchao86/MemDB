#ifndef __COUNTER_BENCH_CONFIG_H__
#define __COUNTER_BENCH_CONFIG_H__

#include "CwxHostInfo.h"
#include "CwxCommon.h"
#include "CwxIniParse.h"

CWINUX_USING_NAMESPACE

///kv压力测试的配置文件加载对象
class CounterBenchConfig
{
public:
    CounterBenchConfig(){
        m_unConnNum = 0;
		m_uiDataBase=10000000; ///<查询的范围，最新的数据量。
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
    //加载配置文件.-1:failure, 0:success
    int loadConfig(string const & strConfFile);
    //输出配置文件
    void outputConfig();
    //获取加载配置文件的失败错误信息
    char const* getError() { return m_szError; };
    
public:
	string              m_strWorkDir;///<工作目录
	CwxHostInfo         m_listen;///<tcp连接的对方listen地址
	string              m_strUnixPathFile;///<若采用unix domain连接，则为连接的path-file
	CWX_UINT16          m_unConnNum;///<连接的数量
	bool                m_bLasting;///<是否为持久连接，类似HTTP的keep-alive
	string				m_strOpr; ///<操作
    CWX_UINT32          m_uiDataMod;  ///<数据的范围
    bool                m_bGetMaster; ///<是否从master获取数据
    bool                m_bCache; ///<是否cache
	CWX_UINT32			m_uiDataBase; ///<已有数据的范围。
	CWX_UINT32			m_uiKeyGroup; ///<当前的key的分组
	CWX_UINT32			m_uiKeyIndex; ///<key分组号
    string              m_strKeyType; ///<key的类型
    list<string>        m_counters; ///<计数器
    CWX_UINT32          m_uiValue; ///<计数的值
    string              m_strUser; ///<用户名
    string              m_strPasswd; ///<用户口令
    char                m_szError[2048];///<错误消息buf
};

#endif
