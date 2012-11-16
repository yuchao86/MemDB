#include "UnistorStore.h"

UnistorStore::UnistorStore(){
	m_impl = NULL;
	m_dllHandle = NULL;
}

///Îö¹¹º¯Êý
UnistorStore::~UnistorStore(){
    if (m_impl){
        delete m_impl;
        m_impl = NULL;
    }
    if (m_dllHandle){
        ::dlclose(m_dllHandle);
        m_dllHandle = NULL;
    }
}

int UnistorStore::init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc,
                       UNISTOR_GET_SYS_INFO_FN getSysInfoFunc,
                       void* pApp,
                       UnistorConfig const* config,
                       string const& strEnginePath,
                       char* szErr2K)
{
    if (m_impl) {
        delete m_impl;
        m_impl = NULL;
    }
    if (m_dllHandle){
        ::dlclose(m_dllHandle);
        m_dllHandle = NULL;
    }

	string strDllFile;
	config->getDriverFile(strEnginePath, strDllFile);
	m_dllHandle = ::dlopen (strDllFile.c_str(), RTLD_LAZY);
	if (!m_dllHandle){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to open [%s] engine's dll:%s, err:%s",
			config->getCommon().m_strStoreType.c_str(),
			strDllFile.c_str(),
			::dlerror());
		return -1;
	}
	//clear any error
	dlerror();
	CWX_LOAD_ENGINE  cwx_load_engine;
	cwx_load_engine = (CWX_LOAD_ENGINE)::dlsym(m_dllHandle, UNISTOR_ENGINE_CREATE_SYMBOL_NAME);
	char *error;
	if ((error = ::dlerror()) != NULL){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to load [%s] engine's symbol[%s],  dll:%s, err:%s",
			config->getCommon().m_strStoreType.c_str(),
			UNISTOR_ENGINE_CREATE_SYMBOL_NAME,
			strDllFile.c_str(),
			error);
		return -1;
	}
	m_impl = cwx_load_engine();
	if (!m_impl){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to create engine[%s],  dll:%s.",
			config->getCommon().m_strStoreType.c_str(),
			strDllFile.c_str());
		return -1;
	}
	if (0 != m_impl->init(msgPipeFunc, getSysInfoFunc, pApp, config)){
		strcpy(szErr2K, m_impl->getErrMsg());
		delete m_impl;
		m_impl = NULL;
		return -1;
	}
	if (config->getCommon().m_strStoreType != m_impl->getName()){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Store engine[%s]'s engine name is [%s], not config's %s.",
			strDllFile.c_str(),
			m_impl->getName(),
			config->getCommon().m_strStoreType.c_str());
		return -1;
	}
	return 0;
}
