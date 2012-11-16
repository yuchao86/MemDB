#include "UnistorBenchConfig.h"
#include "CwxLogger.h"
#include "UnistorDef.h"

int UnistorBenchConfig::loadConfig(string const & strConfFile){
    CwxIniParse  parser;
    string value;
    //½âÎöÅäÖÃÎÄ¼þ
    if (false == parser.load(strConfFile)){
        snprintf(m_szError, 2047, "Failure to Load conf file. err=%s", parser.getErrMsg());
        return -1;
    }

    //load unistor_bench:home
    if (!parser.getAttr("unistor_bench", "home", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:home] for running path.");
        return -1;
    }
	if ('/' != value[value.length()-1]) value +="/";
    m_strWorkDir = value;

	//load unistor_bench:listen
    if (!parser.getAttr("unistor_bench", "listen", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:listen].");
        return -1;
    }
    if (!parseHostPort(value, m_listen)){
        snprintf(m_szError, 2047, "[unistor_bench:listen] is invalid, it should be [ip:port] format.");
        return -1;
    }
    // load echo unistor_bench:conn_num
    if (!parser.getAttr("unistor_bench", "conn_num", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:conn_num].");
        return -1;
    }
    m_unConnNum = strtoul(value.c_str(), NULL, 0);
    // load echo unistor_bench:conn_lasting
    if (!parser.getAttr("unistor_bench", "conn_lasting", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:conn_lasting].");
        return -1;
    }
    m_bLasting = value=="1"?true:false;

    // load echo unistor_bench:data_opr
    if (!parser.getAttr("unistor_bench", "data_opr", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_opr].");
        return -1;
    }
	m_strOpr =value;
	if ((m_strOpr != "import")&&(m_strOpr != "add")&&(m_strOpr != "set")&&(m_strOpr != "update")&&(m_strOpr != "delete")&&(m_strOpr != "get")){
		snprintf(m_szError, 2047, "invalid data opr[%s] for [unistor_bench:data_opr], must be [add/set/update/delete/import/get", m_strOpr.c_str());
		return -1;
	}
    // load echo unistor_bench:data_size
    if (!parser.getAttr("unistor_bench", "data_size", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_size].");
        return -1;
    }
    m_uiDataSize =strtoul(value.c_str(), NULL, 0);
	if (m_uiDataSize > 1024*1024) m_uiDataSize = 1024*1024;

    // load echo unistor_bench:data_key_in_order
    if (!parser.getAttr("unistor_bench", "data_key_in_order", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_key_in_order].");
        return -1;
    }
	m_bKeyOrder = value=="yes"?true:false;

    // load echo unistor_bench:data_get_order
    if (!parser.getAttr("unistor_bench", "data_get_order", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_get_order].");
        return -1;
    }
	m_bGetOrder = value == "yes"?true:false;

    // load echo unistor_bench:data_base
    if (!parser.getAttr("unistor_bench", "data_base", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_base].");
        return -1;
    }
    m_uiDataBase =strtoul(value.c_str(), NULL, 0);
	if (m_uiDataBase < 10000) m_uiDataBase = 10000;
    // load echo unistor_bench:data_mod
    if (!parser.getAttr("unistor_bench", "data_mod", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_mod].");
        return -1;
    }
    m_uiDataMod =strtoul(value.c_str(), NULL, 0);
    // load echo unistor_bench:data_master
    if (!parser.getAttr("unistor_bench", "data_master", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_master].");
        return -1;
    }
    m_bGetMaster = (value == "yes"?true:false);
    // load echo unistor_bench:cache
    if (!parser.getAttr("unistor_bench", "cache", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:cache].");
        return -1;
    }
    m_bCache = (value == "yes"?true:false);

    // load echo unistor_bench:data_group
    if (!parser.getAttr("unistor_bench", "data_group", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_group].");
        return -1;
    }
	m_uiKeyGroup = strtoul(value.c_str(), NULL, 0);
	if (0 == m_uiKeyGroup) m_uiKeyGroup = 1;

    // load echo unistor_bench:data_group_index
    if (!parser.getAttr("unistor_bench", "data_group_index", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:data_group_index].");
        return -1;
    }
	m_uiKeyIndex = strtoul(value.c_str(), NULL, 0);
	if (m_uiKeyIndex >= m_uiKeyGroup) m_uiKeyIndex = m_uiKeyGroup - 1;
    // load echo unistor_bench:expire
    if (!parser.getAttr("unistor_bench", "expire", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:expire].");
        return -1;
    }
    m_uiExpire =strtoul(value.c_str(), NULL, 0);

    // load echo unistor_bench:user
    if (!parser.getAttr("unistor_bench", "user", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:user].");
        return -1;
    }
    m_strUser = value;

    // load echo unistor_bench:passwd
    if (!parser.getAttr("unistor_bench", "passwd", value) || !value.length()){
        snprintf(m_szError, 2047, "Must set [unistor_bench:passwd].");
        return -1;
    }
    m_strPasswd = value;
	return 0;
}

void UnistorBenchConfig::outputConfig(){
	CWX_INFO(("*****************BEGIN CONFIG *******************"));
	CWX_INFO(("home=%s", m_strWorkDir.c_str()));
    CWX_INFO(("listen=%s:%u", m_listen.getHostName().c_str(), m_listen.getPort()));
	CWX_INFO(("conn_conn_num=%u", m_unConnNum));
    CWX_INFO(("conn_lasting=%d", m_bLasting?1:0));
	CWX_INFO(("data_opr=%s", m_strOpr.c_str()));
    CWX_INFO(("data_size=%u", m_uiDataSize));
    CWX_INFO(("data_key_in_order=%s", m_bKeyOrder?"yes":"no"));
    CWX_INFO(("data_get_order=%s", m_bGetOrder?"yes":"no"));
    CWX_INFO(("data_base=%u", m_uiDataBase));
    CWX_INFO(("data_mod=%u", m_uiDataMod));
    CWX_INFO(("data_master=%s", m_bGetMaster?"yes":"no"));
	CWX_INFO(("key_group=%d", m_uiKeyGroup));
    CWX_INFO(("key_group_index=%d", m_uiKeyIndex));
    CWX_INFO(("expire=%d", m_uiExpire));
    CWX_INFO(("cache=%d", m_bCache?"yes":"no"));
    CWX_INFO(("user=%s", m_strUser.c_str()));
    CWX_INFO(("passwd=%s", m_strPasswd.c_str()));
    CWX_INFO(("*****************END   CONFIG *******************"));
}
