#include "CounterBenchConfig.h"
#include "CwxLogger.h"
#include "UnistorDef.h"

int CounterBenchConfig::loadConfig(string const & strConfFile){
    CwxIniParse  parser;
    string value;
    //Ω‚Œˆ≈‰÷√Œƒº˛
    if (false == parser.load(strConfFile)){
        snprintf(m_szError, 2047, "Failure to Load conf file. err=%s", parser.getErrMsg());
        return -1;
    }

    //load unistor_bench:home
    if (!parser.getAttr("counter_bench", "home", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:home] for running path.");
        return -1;
    }
	if ('/' != value[value.length()-1]) value +="/";
    m_strWorkDir = value;

	//load unistor_bench:listen
    if (!parser.getAttr("counter_bench", "listen", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:listen].");
        return -1;
    }
    if (!parseHostPort(value, m_listen)){
        snprintf(m_szError, 2047, "[counter_bench:listen] is invalid, it should be [ip:port] format.");
        return -1;
    }
    // load unistor_bench:conn_num
    if (!parser.getAttr("counter_bench", "conn_num", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:conn_num].");
        return -1;
    }
    m_unConnNum = strtoul(value.c_str(), NULL, 0);
    // load unistor_bench:conn_lasting
    if (!parser.getAttr("counter_bench", "conn_lasting", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:conn_lasting].");
        return -1;
    }
    m_bLasting = value=="1"?true:false;

    // load unistor_bench:data_opr
    if (!parser.getAttr("counter_bench", "data_opr", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:data_opr].");
        return -1;
    }
	m_strOpr =value;
	if ((m_strOpr != "import")&&(m_strOpr != "add")&&(m_strOpr != "set")&&(m_strOpr != "update")&&(m_strOpr != "delete")&&(m_strOpr != "get")&&(m_strOpr != "inc")){
		snprintf(m_szError, 2047, "invalid data opr[%s] for [counter_bench:data_opr], must be [add/set/update/delete/import/get/inc", m_strOpr.c_str());
		return -1;
	}

    // load counter_bench:data_base
    if (!parser.getAttr("counter_bench", "data_base", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:data_base].");
        return -1;
    }
    m_uiDataBase =strtoul(value.c_str(), NULL, 0);
	if (m_uiDataBase < 10000) m_uiDataBase = 10000;
    // load counter_bench:data_mod
    if (!parser.getAttr("counter_bench", "data_mod", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:data_mod].");
        return -1;
    }
    m_uiDataMod =strtoul(value.c_str(), NULL, 0);
    // load counter_bench:data_master
    if (!parser.getAttr("counter_bench", "data_master", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:data_master].");
        return -1;
    }
    m_bGetMaster = (value == "yes"?true:false);
    // load counter_bench:cache
    if (!parser.getAttr("counter_bench", "cache", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:cache].");
        return -1;
    }
    m_bCache = (value == "yes"?true:false);

    // load counter_bench:data_group
    if (!parser.getAttr("counter_bench", "data_group", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:data_group].");
        return -1;
    }
	m_uiKeyGroup = strtoul(value.c_str(), NULL, 0);
	if (0 == m_uiKeyGroup) m_uiKeyGroup = 1;

    // load counter_bench:data_group_index
    if (!parser.getAttr("counter_bench", "data_group_index", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:data_group_index].");
        return -1;
    }
	m_uiKeyIndex = strtoul(value.c_str(), NULL, 0);
	if (m_uiKeyIndex >= m_uiKeyGroup) m_uiKeyIndex = m_uiKeyGroup - 1;

    // load counter_bench:key_type
    if (!parser.getAttr("counter_bench", "key_type", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:key_type].");
        return -1;
    }
    CwxCommon::trim(value);
    if ((value != "int32") && (value != "int64") && (value != "int128") && (value != "int256") && (value != "char")){
        snprintf(m_szError, 2047, "[counter_bench:key_type] must be  [int32] or [int64] or [int128] or [int256] or [char]");
        return -1;
    }
    m_strKeyType = value;
    
    // load counter_bench:counter
    if (!parser.getAttr("counter_bench", "counter", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:counter].");
        return -1;
    }
    CwxCommon::trim(value, NULL);
    if (!value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:counter].");
        return -1;
    }
    CwxCommon::split(value, m_counters, ';');
    // load counter_bench:value
    if (!parser.getAttr("counter_bench", "value", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:value].");
        return -1;
    }
    m_uiValue = strtoul(value.c_str(), NULL, 0);

    // load counter_bench:user
    if (!parser.getAttr("counter_bench", "user", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:user].");
        return -1;
    }
    m_strUser = value;

    // load counter_bench:passwd
    if (!parser.getAttr("counter_bench", "passwd", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [counter_bench:passwd].");
        return -1;
    }
    m_strPasswd = value;
	return 0;
}

void CounterBenchConfig::outputConfig(){
    string strCounters;
	CWX_INFO(("*****************BEGIN CONFIG *******************"));
	CWX_INFO(("home=%s", m_strWorkDir.c_str()));
    CWX_INFO(("listen=%s:%u", m_listen.getHostName().c_str(), m_listen.getPort()));
	CWX_INFO(("conn_conn_num=%u", m_unConnNum));
    CWX_INFO(("conn_lasting=%d", m_bLasting?1:0));
    CWX_INFO(("key_type=%s", m_strKeyType.c_str()));
    CWX_INFO(("value=%u", m_uiValue));
    list<string>::iterator  iter = m_counters.begin();
    while(iter != m_counters.end()){
        if (strCounters.length()){
            strCounters += ";";
        }
        strCounters += *iter;
        iter++;
    }
    CWX_INFO(("counter=%s", strCounters.c_str()));
	CWX_INFO(("data_opr=%s", m_strOpr.c_str()));
    CWX_INFO(("data_base=%u", m_uiDataBase));
    CWX_INFO(("data_mod=%u", m_uiDataMod));
    CWX_INFO(("data_master=%s", m_bGetMaster?"yes":"no"));
	CWX_INFO(("key_group=%d", m_uiKeyGroup));
    CWX_INFO(("key_group_index=%d", m_uiKeyIndex));
    CWX_INFO(("cache=%d", m_bCache?"yes":"no"));
    CWX_INFO(("user=%s", m_strUser.c_str()));
    CWX_INFO(("passwd=%s", m_strPasswd.c_str()));
    CWX_INFO(("*****************END   CONFIG *******************"));
}
