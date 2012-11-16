#ifndef __UNISTOR_TSS_H__
#define __UNISTOR_TSS_H__


#include "UnistorMacro.h"
#include "CwxLogger.h"
#include "CwxTss.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "UnistorDef.h"
#include "CwxMsgBlock.h"



///定义EVENT类型
#define EVENT_ZK_CONNECTED      (CwxEventInfo::SYS_EVENT_NUM + 1) ///<建立了与zk的连接
#define EVENT_ZK_EXPIRED        (CwxEventInfo::SYS_EVENT_NUM + 2) ///<zk的连接失效
#define EVENT_ZK_FAIL_AUTH      (CwxEventInfo::SYS_EVENT_NUM + 3) ///<zk的认证失败
#define EVENT_ZK_SUCCESS_AUTH   (CwxEventInfo::SYS_EVENT_NUM + 4) ///<zk的认证成功 
#define EVENT_ZK_CONF_CHANGE    (CwxEventInfo::SYS_EVENT_NUM + 5) ///<ZK的conf节点配置变化
#define EVENT_ZK_LOCK_CHANGE    (CwxEventInfo::SYS_EVENT_NUM + 6) ///<ZK的锁变化
#define EVENT_ZK_LOST_WATCH     (CwxEventInfo::SYS_EVENT_NUM + 7) ///<失去watch
#define EVENT_ZK_ERROR          (CwxEventInfo::SYS_EVENT_NUM + 8) ///<zk错误
#define EVENT_ZK_SET_SID        (CwxEventInfo::SYS_EVENT_NUM + 9) ///<设置zk的sid
#define EVENT_SEND_MSG          (CwxEventInfo::SYS_EVENT_NUM + 10) ///<发送消息
#define EVENT_STORE_MSG_START   (CwxEventInfo::SYS_EVENT_NUM + 100)  ///<存储的消息起始值
#define EVENT_STORE_COMMIT       EVENT_STORE_MSG_START ///<存储引擎执行了一次commit
#define EVENT_STORE_DEL_EXPIRE   (EVENT_STORE_MSG_START + 2)  ///<删除指定的超时key
#define EVENT_STORE_DEL_EXPIRE_REPLY (EVENT_STORE_MSG_START + 3) ///<删除指定的超时key的回复

///tss的用户线程数据对象
class UnistorTssUserObj{
public:
    ///构造函数
    UnistorTssUserObj(){}
    ///析构函数
    virtual ~UnistorTssUserObj(){}
};

///为存储engine预留的配置对象信息
class UnistorTssEngineObj{
public:
    ///构造函数
    UnistorTssEngineObj(){}
    ///析构函数
    virtual ~UnistorTssEngineObj(){}

};

///定义Read线程向Write线程传递msg参数的对象
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
    CwxMsgBlock*              m_recvMsg; ///<Recv线程收到的写消息
    CwxMsgBlock*              m_replyMsg; ///<Write回复的消息
    UnistorWriteMsgArg*       m_next;  ///<空闲链表下一个
    CwxKeyValueItemEx         m_key; ///<操作对应的key参数
    CwxKeyValueItemEx         m_field; ///<操作的field参数
    CwxKeyValueItemEx         m_extra; ///<操作的extra参数
    CwxKeyValueItemEx         m_data; ///<操作的data参数
    CWX_INT64                 m_llNum; ///<操作的num参数
    CWX_INT64                 m_llMax; ///<操作的max参数
    CWX_INT64                 m_llMin; ///<操作的min参数
    CWX_UINT32                m_uiSign; ///<操作的sign参数
    CWX_UINT32                m_uiVersion; ///<操作的version参数
    CWX_UINT32                m_uiExpire; ///<操作的expire参数
    bool                      m_bCache;  ///<操作的cache参数
};

//unistor的recv线程池的tss
class UnistorTss:public CwxTss{
public:
    ///构造函数
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
        ///统计初始化
        resetStats();
    }
    ///析构函数
    ~UnistorTss();
public:
    ///tss的初始化，0：成功；-1：失败
    int init(UnistorTssUserObj* pUserObj=NULL);
    ///获取用户的数据
    UnistorTssUserObj* getUserObj() { return m_userObj;}
    ///获取一个write arg对象
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
    ///释放一个write arg对象
    inline void pushWriteMsgArg(UnistorWriteMsgArg* arg){
        arg->m_next = m_writeMsgHead;
        m_writeMsgHead = arg;

    }
    ///获取package的buf，返回NULL表示失败
    inline char* getBuf(CWX_UINT32 uiSize){
        if (m_uiDataBufLen < uiSize){
            delete [] m_szDataBuf;
            m_szDataBuf = new char[uiSize];
            m_uiDataBufLen = uiSize;
        }
        return m_szDataBuf;
    }
    ///获取package的buf，返回NULL表示失败
    inline char* getBuf1(CWX_UINT32 uiSize){
        if (m_uiDataBufLen1 < uiSize){
            delete [] m_szDataBuf1;
            m_szDataBuf1 = new char[uiSize];
            m_uiDataBufLen1 = uiSize;
        }
        return m_szDataBuf1;
    }
    ///获取package的buf，返回NULL表示失败
    inline char* getBuf2(CWX_UINT32 uiSize){
        if (m_uiDataBufLen2 < uiSize){
            delete [] m_szDataBuf2;
            m_szDataBuf2 = new char[uiSize];
            m_uiDataBufLen2 = uiSize;
        }
        return m_szDataBuf2;
    }
    ///获取package的buf，返回NULL表示失败
    inline char* getBuf3(CWX_UINT32 uiSize){
        if (m_uiDataBufLen3 < uiSize){
            delete [] m_szDataBuf3;
            m_szDataBuf3 = new char[uiSize];
            m_uiDataBufLen3 = uiSize;
        }
        return m_szDataBuf3;
    }
    ///是否是master idc
    bool isMasterIdc(){
        if (m_pZkConf && m_pZkConf->m_bMasterIdc) return true;
        return false;
    }
    ///获取master idc的名字
    char const* getMasterIdc() const{
        if (m_pZkConf && m_pZkConf->m_strMasterIdc.length()) return m_pZkConf->m_strMasterIdc.c_str();
        return "";
    }
    ///自己是否是master
    bool isMaster(){
        if (m_pZkLock && m_pZkLock->m_bMaster) return true;
        return false;
    }
    ///是否存在master
    bool isExistMaster(){
        if (m_pZkLock && m_pZkLock->m_strMaster.length()) return true;
        return false;
    }
    ///获取master的主机名
    char const* getMasterHost() const{
        if (m_pZkLock && m_pZkLock->m_strMaster.length()) return m_pZkLock->m_strMaster.c_str();
        return "";
    }
    ///获取idc内的上一个sync的主机名
    char const* getSyncHost() const{
        if (m_pZkLock){
            if (m_pZkLock->m_strPrev.length()) return m_pZkLock->m_strPrev.c_str();
            if (m_pZkLock->m_strMaster.length()) return m_pZkLock->m_strMaster.c_str();
        }
        return "";
    }
    inline void resetStats(){
        ///统计初始化
        m_ullStatsGetNum = 0; ///<get查询的数量
        m_ullStatsGetReadCacheNum = 0; ///<get查询使用Cache数量
        m_ullStatsGetExistNum = 0; ///<get查询存在结果的数量
        m_ullStatsGetsNum = 0; ///<gets查询的数量
        m_ullStatsGetsKeyNum = 0; ///<gets的key的数量
        m_ullStatsGetsKeyReadCacheNum = 0; ///<gets的key的cache数量
        m_ullStatsGetsKeyExistNum = 0; ///<gets的key的存在的数量
        m_ullStatsListNum = 0; ///<list的数量
        m_ullStatsExistNum = 0; ///<exist的数量
        m_ullStatsExistReadCacheNum = 0; ///<exist的cache数量
        m_ullStatsExistExistNum = 0; ///<exist的存在的数量
        m_ullStatsAddNum = 0; ///<add的数量
        m_ullStatsAddReadCacheNum = 0; ///<add的read cache数量
        m_ullStatsAddWriteCacheNum = 0; ///<add的write cache数量
        m_ullStatsSetNum = 0; ///<set的数量
        m_ullStatsSetReadCacheNum = 0; ///<set的read cache数量
        m_ullStatsSetWriteCacheNum = 0; ///<set的write cache数量
        m_ullStatsUpdateNum = 0; ///<update的数量
        m_ullStatsUpdateReadCacheNum = 0; ///<update的read cache数量
        m_ullStatsUpdateWriteCacheNum = 0; ///<update的write cache数量
        m_ullStatsIncNum = 0; ///<inc的数量
        m_ullStatsIncReadCacheNum = 0; ///<inc的read cache数量
        m_ullStatsIncWriteCacheNum = 0; ///<inc的write cache数量
        m_ullStatsDelNum = 0; ///<del的数量
        m_ullStatsDelReadCacheNum = 0; ///<del的read cache数量
        m_ullStatsDelWriteCacheNum = 0; ///<del的write cache数量
        m_ullStatsImportNum = 0; ///<del的数量
        m_ullStatsImportReadCacheNum = 0; ///<del的read cache数量
        m_ullStatsImportWriteCacheNum = 0; ///<del的write cache数量
    }

public:
    CWX_UINT32              m_uiBinLogVersion; ///<binlog的数据版本，用于binlog的分发
    CWX_UINT32              m_uiBinlogType;   ///<binlog的消息的类型，用于binlog的分发
    CwxKeyValueItemEx const*  m_pBinlogData; ///<binlog的data，用于binglog的分发
    UnistorZkConf*          m_pZkConf;  ///<zk的配置对象
    UnistorZkLock*          m_pZkLock;  ///<zk的锁信息
    CwxPackageReaderEx*       m_pReader; ///<数据包的解包对象
    CwxPackageReaderEx*       m_pItemReader; ///<数据包的解包对象
    CwxPackageReaderEx*       m_pEngineReader; ///<engine使用的reader，外部不能使用
    CwxPackageReaderEx*       m_pEngineItemReader; ///<engine使用的reader，存储引擎不能使用
    CwxPackageWriterEx*       m_pWriter; ///<数据包的pack对象
    CwxPackageWriterEx*       m_pItemWriter; ///<一个消息的数据包的pack对象
    CwxPackageWriterEx*       m_pEngineWriter; ///<engine使用的writer，外部不能使用
    CwxPackageWriterEx*       m_pEngineItemWriter; ///<engine使用的writer，外部不能使用
    char			        m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<存储的key
    CWX_UINT32              m_uiThreadIndex; ///<线程的索引号
    UnistorTssEngineObj*    m_engineConf;       ///<engine的conf信息，引擎可以使用
    ///下面是访问统计信息
    volatile CWX_UINT64     m_ullStatsGetNum; ///<get查询的数量
    volatile CWX_UINT64     m_ullStatsGetReadCacheNum; ///<get查询使用Cache数量
    volatile CWX_UINT64     m_ullStatsGetExistNum; ///<get查询存在结果的数量
    volatile CWX_UINT64     m_ullStatsGetsNum; ///<gets查询的数量
    volatile CWX_UINT64     m_ullStatsGetsKeyNum; ///<gets的key的数量
    volatile CWX_UINT64     m_ullStatsGetsKeyReadCacheNum; ///<gets的key的cache数量
    volatile CWX_UINT64     m_ullStatsGetsKeyExistNum; ///<gets的key的存在的数量
    volatile CWX_UINT64     m_ullStatsListNum; ///<list的数量
    volatile CWX_UINT64     m_ullStatsExistNum; ///<exist的数量
    volatile CWX_UINT64     m_ullStatsExistReadCacheNum; ///<exist的cache数量
    volatile CWX_UINT64     m_ullStatsExistExistNum; ///<exist的存在的数量
    volatile CWX_UINT64     m_ullStatsAddNum; ///<add的数量
    volatile CWX_UINT64     m_ullStatsAddReadCacheNum; ///<add的read cache数量
    volatile CWX_UINT64     m_ullStatsAddWriteCacheNum; ///<add的write cache数量
    volatile CWX_UINT64     m_ullStatsSetNum; ///<set的数量
    volatile CWX_UINT64     m_ullStatsSetReadCacheNum; ///<set的read cache数量
    volatile CWX_UINT64     m_ullStatsSetWriteCacheNum; ///<set的write cache数量
    volatile CWX_UINT64     m_ullStatsUpdateNum; ///<update的数量
    volatile CWX_UINT64     m_ullStatsUpdateReadCacheNum; ///<update的read cache数量
    volatile CWX_UINT64     m_ullStatsUpdateWriteCacheNum; ///<update的write cache数量
    volatile CWX_UINT64     m_ullStatsIncNum; ///<inc的数量
    volatile CWX_UINT64     m_ullStatsIncReadCacheNum; ///<inc的read cache数量
    volatile CWX_UINT64     m_ullStatsIncWriteCacheNum; ///<inc的write cache数量
    volatile CWX_UINT64     m_ullStatsDelNum; ///<del的数量
    volatile CWX_UINT64     m_ullStatsDelReadCacheNum; ///<del的read cache数量
    volatile CWX_UINT64     m_ullStatsDelWriteCacheNum; ///<del的write cache数量
    volatile CWX_UINT64     m_ullStatsImportNum; ///<del的数量
    volatile CWX_UINT64     m_ullStatsImportReadCacheNum; ///<del的read cache数量
    volatile CWX_UINT64     m_ullStatsImportWriteCacheNum; ///<del的write cache数量

private:
    UnistorWriteMsgArg*     m_writeMsgHead;  ///<缓存的Write消息参数的头
    UnistorTssUserObj*    m_userObj;  ///用户的数据
    char*                  m_szDataBuf; ///<数据buf
    CWX_UINT32             m_uiDataBufLen; ///<数据buf的空间大小
    char*                  m_szDataBuf1; ///<数据buf1
    CWX_UINT32             m_uiDataBufLen1; ///<数据buf1的空间大小
    char*                  m_szDataBuf2; ///<数据buf2
    CWX_UINT32             m_uiDataBufLen2; ///<数据buf2的空间大小
    char*                  m_szDataBuf3; ///<数据buf3
    CWX_UINT32             m_uiDataBufLen3; ///<数据buf3的空间大小
};









#endif
