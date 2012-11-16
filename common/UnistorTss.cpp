#include "UnistorTss.h"

///析构函数
UnistorTss::~UnistorTss(){
    if (m_pZkConf) delete m_pZkConf;
    if (m_pZkLock) delete m_pZkLock;
    if (m_pReader) delete m_pReader;
    if (m_pItemReader) delete m_pItemReader;
    if (m_pEngineReader) delete m_pEngineReader;
    if (m_pEngineItemReader) delete m_pEngineItemReader;
    if (m_pWriter) delete m_pWriter;
    if (m_pItemWriter) delete m_pItemWriter;
    if (m_pEngineWriter) delete m_pEngineWriter;
    if (m_pEngineItemWriter) delete m_pEngineItemWriter;
    if (m_szDataBuf) delete [] m_szDataBuf;
    if (m_szDataBuf1) delete [] m_szDataBuf1;
    if (m_szDataBuf2) delete [] m_szDataBuf2;
    if (m_szDataBuf3) delete [] m_szDataBuf3;
//    if (m_engineConf) delete m_engineConf;
    if (m_userObj) delete m_userObj;
    if (m_writeMsgHead){
        UnistorWriteMsgArg* pItem = NULL;
        while(m_writeMsgHead){
            pItem = m_writeMsgHead->m_next;
            delete m_writeMsgHead;
            m_writeMsgHead = pItem;
        }
        m_writeMsgHead = NULL;
    }
}

int UnistorTss::init(UnistorTssUserObj* pUserObj){
    m_writeMsgHead = NULL;
    m_pZkConf = NULL;
    m_pZkLock = NULL;
    m_pReader = new CwxPackageReaderEx(false);
    m_pItemReader = new CwxPackageReaderEx(false);
    m_pEngineReader = new CwxPackageReaderEx(false);
    m_pEngineItemReader = new CwxPackageReaderEx(false);
    m_pWriter = new CwxPackageWriterEx(UNISTOR_DEF_KV_SIZE);
    m_pItemWriter = new CwxPackageWriterEx(UNISTOR_DEF_KV_SIZE);
    m_pEngineWriter = new CwxPackageWriterEx(UNISTOR_DEF_KV_SIZE);
    m_pEngineItemWriter = new CwxPackageWriterEx(UNISTOR_DEF_KV_SIZE);
    m_szDataBuf = new char[UNISTOR_DEF_KV_SIZE];
    m_uiDataBufLen= UNISTOR_DEF_KV_SIZE;
    m_userObj = pUserObj;
    m_engineConf = NULL;
    ///统计初始化
    resetStats();
    return 0;
}
