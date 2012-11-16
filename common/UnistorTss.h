#ifndef __UNISTOR_TSS_H__
#define __UNISTOR_TSS_H__


#include "UnistorMacro.h"
#include "CwxLogger.h"
#include "CwxTss.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "UnistorDef.h"
#include "CwxMsgBlock.h"



///����EVENT����
#define EVENT_ZK_CONNECTED      (CwxEventInfo::SYS_EVENT_NUM + 1) ///<��������zk������
#define EVENT_ZK_EXPIRED        (CwxEventInfo::SYS_EVENT_NUM + 2) ///<zk������ʧЧ
#define EVENT_ZK_FAIL_AUTH      (CwxEventInfo::SYS_EVENT_NUM + 3) ///<zk����֤ʧ��
#define EVENT_ZK_SUCCESS_AUTH   (CwxEventInfo::SYS_EVENT_NUM + 4) ///<zk����֤�ɹ� 
#define EVENT_ZK_CONF_CHANGE    (CwxEventInfo::SYS_EVENT_NUM + 5) ///<ZK��conf�ڵ����ñ仯
#define EVENT_ZK_LOCK_CHANGE    (CwxEventInfo::SYS_EVENT_NUM + 6) ///<ZK�����仯
#define EVENT_ZK_LOST_WATCH     (CwxEventInfo::SYS_EVENT_NUM + 7) ///<ʧȥwatch
#define EVENT_ZK_ERROR          (CwxEventInfo::SYS_EVENT_NUM + 8) ///<zk����
#define EVENT_ZK_SET_SID        (CwxEventInfo::SYS_EVENT_NUM + 9) ///<����zk��sid
#define EVENT_SEND_MSG          (CwxEventInfo::SYS_EVENT_NUM + 10) ///<������Ϣ
#define EVENT_STORE_MSG_START   (CwxEventInfo::SYS_EVENT_NUM + 100)  ///<�洢����Ϣ��ʼֵ
#define EVENT_STORE_COMMIT       EVENT_STORE_MSG_START ///<�洢����ִ����һ��commit
#define EVENT_STORE_DEL_EXPIRE   (EVENT_STORE_MSG_START + 2)  ///<ɾ��ָ���ĳ�ʱkey
#define EVENT_STORE_DEL_EXPIRE_REPLY (EVENT_STORE_MSG_START + 3) ///<ɾ��ָ���ĳ�ʱkey�Ļظ�

///tss���û��߳����ݶ���
class UnistorTssUserObj{
public:
    ///���캯��
    UnistorTssUserObj(){}
    ///��������
    virtual ~UnistorTssUserObj(){}
};

///Ϊ�洢engineԤ�������ö�����Ϣ
class UnistorTssEngineObj{
public:
    ///���캯��
    UnistorTssEngineObj(){}
    ///��������
    virtual ~UnistorTssEngineObj(){}

};

///����Read�߳���Write�̴߳���msg�����Ķ���
class UnistorWriteMsgArg{
public:
    UnistorWriteMsgArg(){
        reset();
    }
public:
    void reset(){
        m_recvMsg = NULL;
        m_replyMsg = NULL;
        m_next = NULL;
        m_key.m_uiDataLen = 0;
        m_field.m_uiDataLen = 0;
        m_extra.m_uiDataLen = 0;
        m_data.m_uiDataLen = 0;
        m_llNum = 0;
        m_llMax = 0;
        m_llMin = 0;
        m_uiSign = 0;
        m_uiVersion = 0;
        m_uiExpire = 0;
        m_bCache = true;
    }
public:
    CwxMsgBlock*              m_recvMsg; ///<Recv�߳��յ���д��Ϣ
    CwxMsgBlock*              m_replyMsg; ///<Write�ظ�����Ϣ
    UnistorWriteMsgArg*       m_next;  ///<����������һ��
    CwxKeyValueItemEx         m_key; ///<������Ӧ��key����
    CwxKeyValueItemEx         m_field; ///<������field����
    CwxKeyValueItemEx         m_extra; ///<������extra����
    CwxKeyValueItemEx         m_data; ///<������data����
    CWX_INT64                 m_llNum; ///<������num����
    CWX_INT64                 m_llMax; ///<������max����
    CWX_INT64                 m_llMin; ///<������min����
    CWX_UINT32                m_uiSign; ///<������sign����
    CWX_UINT32                m_uiVersion; ///<������version����
    CWX_UINT32                m_uiExpire; ///<������expire����
    bool                      m_bCache;  ///<������cache����
};

//unistor��recv�̳߳ص�tss
class UnistorTss:public CwxTss{
public:
    ///���캯��
    UnistorTss():CwxTss(){
        m_writeMsgHead = NULL;
        m_pZkConf = NULL;
        m_pZkLock = NULL;
        m_pReader = NULL;
        m_pItemReader = NULL;
        m_pEngineReader = NULL;
        m_pEngineItemReader = NULL;
        m_pWriter = NULL;
        m_pItemWriter = NULL;
        m_pEngineWriter = NULL;
        m_pEngineItemWriter = NULL;
        m_engineConf = NULL;
        m_uiThreadIndex = 0;
        m_szDataBuf = NULL;
        m_uiDataBufLen = 0;
        m_szDataBuf1 = NULL;
        m_uiDataBufLen1 = 0;
        m_szDataBuf2 = NULL;
        m_uiDataBufLen2 = 0;
        m_szDataBuf3 = NULL;
        m_uiDataBufLen3 = 0;
        m_userObj = NULL;
        m_uiBinLogVersion = 0;
        m_uiBinlogType = 0;
        m_pBinlogData = NULL;
        ///ͳ�Ƴ�ʼ��
        resetStats();
    }
    ///��������
    ~UnistorTss();
public:
    ///tss�ĳ�ʼ����0���ɹ���-1��ʧ��
    int init(UnistorTssUserObj* pUserObj=NULL);
    ///��ȡ�û�������
    UnistorTssUserObj* getUserObj() { return m_userObj;}
    ///��ȡһ��write arg����
    inline UnistorWriteMsgArg* popWriteMsgArg(){
        UnistorWriteMsgArg* arg = m_writeMsgHead;
        if (arg){
            m_writeMsgHead = m_writeMsgHead->m_next;
            arg->reset();
        }else{
            arg = new UnistorWriteMsgArg();
        }
        return arg;
    }
    ///�ͷ�һ��write arg����
    inline void pushWriteMsgArg(UnistorWriteMsgArg* arg){
        arg->m_next = m_writeMsgHead;
        m_writeMsgHead = arg;

    }
    ///��ȡpackage��buf������NULL��ʾʧ��
    inline char* getBuf(CWX_UINT32 uiSize){
        if (m_uiDataBufLen < uiSize){
            delete [] m_szDataBuf;
            m_szDataBuf = new char[uiSize];
            m_uiDataBufLen = uiSize;
        }
        return m_szDataBuf;
    }
    ///��ȡpackage��buf������NULL��ʾʧ��
    inline char* getBuf1(CWX_UINT32 uiSize){
        if (m_uiDataBufLen1 < uiSize){
            delete [] m_szDataBuf1;
            m_szDataBuf1 = new char[uiSize];
            m_uiDataBufLen1 = uiSize;
        }
        return m_szDataBuf1;
    }
    ///��ȡpackage��buf������NULL��ʾʧ��
    inline char* getBuf2(CWX_UINT32 uiSize){
        if (m_uiDataBufLen2 < uiSize){
            delete [] m_szDataBuf2;
            m_szDataBuf2 = new char[uiSize];
            m_uiDataBufLen2 = uiSize;
        }
        return m_szDataBuf2;
    }
    ///��ȡpackage��buf������NULL��ʾʧ��
    inline char* getBuf3(CWX_UINT32 uiSize){
        if (m_uiDataBufLen3 < uiSize){
            delete [] m_szDataBuf3;
            m_szDataBuf3 = new char[uiSize];
            m_uiDataBufLen3 = uiSize;
        }
        return m_szDataBuf3;
    }
    ///�Ƿ���master idc
    bool isMasterIdc(){
        if (m_pZkConf && m_pZkConf->m_bMasterIdc) return true;
        return false;
    }
    ///��ȡmaster idc������
    char const* getMasterIdc() const{
        if (m_pZkConf && m_pZkConf->m_strMasterIdc.length()) return m_pZkConf->m_strMasterIdc.c_str();
        return "";
    }
    ///�Լ��Ƿ���master
    bool isMaster(){
        if (m_pZkLock && m_pZkLock->m_bMaster) return true;
        return false;
    }
    ///�Ƿ����master
    bool isExistMaster(){
        if (m_pZkLock && m_pZkLock->m_strMaster.length()) return true;
        return false;
    }
    ///��ȡmaster��������
    char const* getMasterHost() const{
        if (m_pZkLock && m_pZkLock->m_strMaster.length()) return m_pZkLock->m_strMaster.c_str();
        return "";
    }
    ///��ȡidc�ڵ���һ��sync��������
    char const* getSyncHost() const{
        if (m_pZkLock){
            if (m_pZkLock->m_strPrev.length()) return m_pZkLock->m_strPrev.c_str();
            if (m_pZkLock->m_strMaster.length()) return m_pZkLock->m_strMaster.c_str();
        }
        return "";
    }
    inline void resetStats(){
        ///ͳ�Ƴ�ʼ��
        m_ullStatsGetNum = 0; ///<get��ѯ������
        m_ullStatsGetReadCacheNum = 0; ///<get��ѯʹ��Cache����
        m_ullStatsGetExistNum = 0; ///<get��ѯ���ڽ��������
        m_ullStatsGetsNum = 0; ///<gets��ѯ������
        m_ullStatsGetsKeyNum = 0; ///<gets��key������
        m_ullStatsGetsKeyReadCacheNum = 0; ///<gets��key��cache����
        m_ullStatsGetsKeyExistNum = 0; ///<gets��key�Ĵ��ڵ�����
        m_ullStatsListNum = 0; ///<list������
        m_ullStatsExistNum = 0; ///<exist������
        m_ullStatsExistReadCacheNum = 0; ///<exist��cache����
        m_ullStatsExistExistNum = 0; ///<exist�Ĵ��ڵ�����
        m_ullStatsAddNum = 0; ///<add������
        m_ullStatsAddReadCacheNum = 0; ///<add��read cache����
        m_ullStatsAddWriteCacheNum = 0; ///<add��write cache����
        m_ullStatsSetNum = 0; ///<set������
        m_ullStatsSetReadCacheNum = 0; ///<set��read cache����
        m_ullStatsSetWriteCacheNum = 0; ///<set��write cache����
        m_ullStatsUpdateNum = 0; ///<update������
        m_ullStatsUpdateReadCacheNum = 0; ///<update��read cache����
        m_ullStatsUpdateWriteCacheNum = 0; ///<update��write cache����
        m_ullStatsIncNum = 0; ///<inc������
        m_ullStatsIncReadCacheNum = 0; ///<inc��read cache����
        m_ullStatsIncWriteCacheNum = 0; ///<inc��write cache����
        m_ullStatsDelNum = 0; ///<del������
        m_ullStatsDelReadCacheNum = 0; ///<del��read cache����
        m_ullStatsDelWriteCacheNum = 0; ///<del��write cache����
        m_ullStatsImportNum = 0; ///<del������
        m_ullStatsImportReadCacheNum = 0; ///<del��read cache����
        m_ullStatsImportWriteCacheNum = 0; ///<del��write cache����
    }

public:
    CWX_UINT32              m_uiBinLogVersion; ///<binlog�����ݰ汾������binlog�ķַ�
    CWX_UINT32              m_uiBinlogType;   ///<binlog����Ϣ�����ͣ�����binlog�ķַ�
    CwxKeyValueItemEx const*  m_pBinlogData; ///<binlog��data������binglog�ķַ�
    UnistorZkConf*          m_pZkConf;  ///<zk�����ö���
    UnistorZkLock*          m_pZkLock;  ///<zk������Ϣ
    CwxPackageReaderEx*       m_pReader; ///<���ݰ��Ľ������
    CwxPackageReaderEx*       m_pItemReader; ///<���ݰ��Ľ������
    CwxPackageReaderEx*       m_pEngineReader; ///<engineʹ�õ�reader���ⲿ����ʹ��
    CwxPackageReaderEx*       m_pEngineItemReader; ///<engineʹ�õ�reader���洢���治��ʹ��
    CwxPackageWriterEx*       m_pWriter; ///<���ݰ���pack����
    CwxPackageWriterEx*       m_pItemWriter; ///<һ����Ϣ�����ݰ���pack����
    CwxPackageWriterEx*       m_pEngineWriter; ///<engineʹ�õ�writer���ⲿ����ʹ��
    CwxPackageWriterEx*       m_pEngineItemWriter; ///<engineʹ�õ�writer���ⲿ����ʹ��
    char			        m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<�洢��key
    CWX_UINT32              m_uiThreadIndex; ///<�̵߳�������
    UnistorTssEngineObj*    m_engineConf;       ///<engine��conf��Ϣ���������ʹ��
    ///�����Ƿ���ͳ����Ϣ
    volatile CWX_UINT64     m_ullStatsGetNum; ///<get��ѯ������
    volatile CWX_UINT64     m_ullStatsGetReadCacheNum; ///<get��ѯʹ��Cache����
    volatile CWX_UINT64     m_ullStatsGetExistNum; ///<get��ѯ���ڽ��������
    volatile CWX_UINT64     m_ullStatsGetsNum; ///<gets��ѯ������
    volatile CWX_UINT64     m_ullStatsGetsKeyNum; ///<gets��key������
    volatile CWX_UINT64     m_ullStatsGetsKeyReadCacheNum; ///<gets��key��cache����
    volatile CWX_UINT64     m_ullStatsGetsKeyExistNum; ///<gets��key�Ĵ��ڵ�����
    volatile CWX_UINT64     m_ullStatsListNum; ///<list������
    volatile CWX_UINT64     m_ullStatsExistNum; ///<exist������
    volatile CWX_UINT64     m_ullStatsExistReadCacheNum; ///<exist��cache����
    volatile CWX_UINT64     m_ullStatsExistExistNum; ///<exist�Ĵ��ڵ�����
    volatile CWX_UINT64     m_ullStatsAddNum; ///<add������
    volatile CWX_UINT64     m_ullStatsAddReadCacheNum; ///<add��read cache����
    volatile CWX_UINT64     m_ullStatsAddWriteCacheNum; ///<add��write cache����
    volatile CWX_UINT64     m_ullStatsSetNum; ///<set������
    volatile CWX_UINT64     m_ullStatsSetReadCacheNum; ///<set��read cache����
    volatile CWX_UINT64     m_ullStatsSetWriteCacheNum; ///<set��write cache����
    volatile CWX_UINT64     m_ullStatsUpdateNum; ///<update������
    volatile CWX_UINT64     m_ullStatsUpdateReadCacheNum; ///<update��read cache����
    volatile CWX_UINT64     m_ullStatsUpdateWriteCacheNum; ///<update��write cache����
    volatile CWX_UINT64     m_ullStatsIncNum; ///<inc������
    volatile CWX_UINT64     m_ullStatsIncReadCacheNum; ///<inc��read cache����
    volatile CWX_UINT64     m_ullStatsIncWriteCacheNum; ///<inc��write cache����
    volatile CWX_UINT64     m_ullStatsDelNum; ///<del������
    volatile CWX_UINT64     m_ullStatsDelReadCacheNum; ///<del��read cache����
    volatile CWX_UINT64     m_ullStatsDelWriteCacheNum; ///<del��write cache����
    volatile CWX_UINT64     m_ullStatsImportNum; ///<del������
    volatile CWX_UINT64     m_ullStatsImportReadCacheNum; ///<del��read cache����
    volatile CWX_UINT64     m_ullStatsImportWriteCacheNum; ///<del��write cache����

private:
    UnistorWriteMsgArg*     m_writeMsgHead;  ///<�����Write��Ϣ������ͷ
    UnistorTssUserObj*    m_userObj;  ///�û�������
    char*                  m_szDataBuf; ///<����buf
    CWX_UINT32             m_uiDataBufLen; ///<����buf�Ŀռ��С
    char*                  m_szDataBuf1; ///<����buf1
    CWX_UINT32             m_uiDataBufLen1; ///<����buf1�Ŀռ��С
    char*                  m_szDataBuf2; ///<����buf2
    CWX_UINT32             m_uiDataBufLen2; ///<����buf2�Ŀռ��С
    char*                  m_szDataBuf3; ///<����buf3
    CWX_UINT32             m_uiDataBufLen3; ///<����buf3�Ŀռ��С
};









#endif
