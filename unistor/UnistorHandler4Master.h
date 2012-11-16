#ifndef __UNISTOR_HANDLER_4_MASTER_H__
#define __UNISTOR_HANDLER_4_MASTER_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"
#include "UnistorConnector.h"

///前置声明对象
class UnistorApp;

///binlog同步的session
class UnistorSyncSession{
public:
    ///构造函数
    UnistorSyncSession(CWX_UINT32 uiHostId){
        m_uiHostId = uiHostId;
        m_ullSessionId = 0;
        m_ullNextSeq = 0;
        m_unPort = 0;
        m_bZip = false;
        m_uiChunkSize = 0;
        m_unConnNum = 0;
        m_uiReportDatetime = 0;
    }
    ~UnistorSyncSession(){
        map<CWX_UINT64/*seq*/, CwxMsgBlock*>::iterator iter =  m_msg.begin();
        while(iter != m_msg.end()){
            CwxMsgBlockAlloc::free(iter->second);
            iter++;
        }
    }
public:
    ///接收新消息，返回已经收到的消息列表
    bool recv(CWX_UINT64 ullSeq, CwxMsgBlock* msg, list<CwxMsgBlock*>& finished){
        map<CWX_UINT32,  bool>::iterator iter = m_conns.find(msg->event().getConnId());
        if ( (iter == m_conns.end()) || !iter->second ) return false;
        finished.clear();
        if (ullSeq == m_ullNextSeq){
            finished.push_back(msg);
            m_ullNextSeq++;
            map<CWX_UINT64/*seq*/, CwxMsgBlock* >::iterator iter =  m_msg.begin();
            while(iter != m_msg.end()){
                if (iter->first == m_ullNextSeq){
                    finished.push_back(iter->second);
                    m_ullNextSeq++;
                    m_msg.erase(iter);
                    iter = m_msg.begin();
                    continue;
                }
                break;
            }
            return true;
        }
        m_msg[ullSeq] = msg;
        msg->event().setTimestamp((CWX_UINT32)time(NULL));
        return true;
    }

    //检测是否超时
    bool isTimeout(CWX_UINT32 uiTimeout) const{
        if (!m_msg.size()) return false;
        CWX_UINT32 uiNow = time(NULL);
        return m_msg.begin()->second->event().getTimestamp() + uiTimeout < uiNow;
    }
public:
    CWX_UINT64              m_ullSessionId; ///<session id
    CWX_UINT64              m_ullNextSeq; ///<下一个待接收的sid
    CWX_UINT32              m_uiHostId; ///<当前的host id
    string                  m_strHost; ///<当前同步的主机
    CWX_UINT16              m_unPort; ///<同步的端口号
    string                  m_strUser; ///<同步的用户名
    string                  m_strPasswd; ///<当前同步的用户口令
    bool                    m_bZip;  ///<是否压缩
    string                  m_strSign; ///<签名
    CWX_UINT32              m_uiChunkSize; ///<chunk的size
    CWX_UINT16              m_unConnNum;   ///<conn 的数量
    map<CWX_UINT64/*seq*/, CwxMsgBlock*>  m_msg;   ///<等待排序的消息
    map<CWX_UINT32,  bool/*是否已经report*/>  m_conns; ///<建立的连接
    CWX_UINT32              m_uiReportDatetime; ///<报告的时间戳，若过了指定的时间没有回复，则关闭
};

///slave从master接收binlog的处理handle
class UnistorHandler4Master : public CwxCmdOp{
public:
    ///构造函数
    UnistorHandler4Master(UnistorApp* pApp):m_pApp(pApp){
        m_unzipBuf = NULL;
        m_uiBufLen = 0;
        m_syncSession = NULL;
        m_uiCurHostId = 0;
    }
    ///析构函数
    virtual ~UnistorHandler4Master(){
        if (m_unzipBuf) delete [] m_unzipBuf;
        if (m_syncSession) delete m_syncSession;
    }
public:
    /**
    @brief 连接关闭事件的的函数。
    @param [in] msg 连接关闭的事件对象。
    @param [in] pThrEnv 线程的TSS对象。
    @return -1：处理失败，0：不处理此事件，1：处理此事件。
    */
    virtual int onConnClosed(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief 收到通信数据包事件的处理函数。
    @param [in] msg 收到通信数据包的事件对象。
    @param [in] pThrEnv 线程的TSS对象。
    @return -1：处理失败，0：不处理此事件，1：处理此事件。
    */
    virtual int onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv);
public:
    ///zk配置改变处理函数
    void configChange(UnistorTss* pTss);
    ///时钟定时检查函数
    void timecheck(UnistorTss* pTss);
private:
    ///收到一条消息的处理函数。返回值：0:成功；-1：失败
    int recvMsg(CwxMsgBlock*& msg, ///<收到的消息
        list<CwxMsgBlock*>& msgs ///<接收池中返回的可处理的消息。在list按照先后次序排序
        );

    ///处理Sync report的reply消息。返回值：0：成功；-1：失败
    int dealSyncReportReply(CwxMsgBlock*& msg, ///<收到的消息
        UnistorTss* pTss ///<tss对象
        );

    ///处理收到的sync data。返回值：0：成功；-1：失败
    int dealSyncData(CwxMsgBlock*& msg, ///<收到的消息
        UnistorTss* pTss ///<tss对象
        );

    //处理收到的chunk模式下的sync data。返回值：0：成功；-1：失败
    int dealSyncChunkData(CwxMsgBlock*& msg, ///<收到的消息
        UnistorTss* pTss ///<tss对象
        );

    //处理错误消息。返回值：0：成功；-1：失败
    int dealErrMsg(CwxMsgBlock*& msg,  ///<收到的消息
        UnistorTss* pTss ///<tss对象
        );

    //保存binlog。返回值：0：成功；-1：失败
    int saveBinlog(UnistorTss* pTss, ///<tss对象
        char const* szBinLog, ///<收到的一条binlog数据
        CWX_UINT32 uiLen  ///<binlog数据的长度
        );

    ///检查数据的签名。返回值：true：成功；false：失败
    bool checkSign(char const* data, ///<binlog数据
        CWX_UINT32 uiDateLen, ///<binlog数据的长度
        char const* szSign,   ///<签名类型，为md5或crc32
        char const* sign  ///<签名值
        );

    //获取unzip的buf。返回值：true：成功；false：失败
    bool prepareUnzipBuf();

    //关闭已有连接
    void closeSession();

    ///创建与master同步的连接。返回值：0：成功；-1：失败
    int createSession(UnistorTss* pTss, ///<tss对象
        string const& strHost, ///<要连接的主机
        CWX_UINT16 unPort, ///<连接的端口号
        CWX_UINT16 unConnNum, ///<建立连接的数量
        char const* user, ///<连接的用户
        char const* passwd, ///<连接的用户口令
        CWX_UINT32 uiChunk, ///<binlog同步的chunk大小
        char const* subscribe, ///<binlog的订阅规则
        char const* sign, ///<消息的签名方式，为md5或crc32
        bool  bZip  ///<是否压缩
        );

    ///检查是否需要重建连接
    bool isNeedReconnect(UnistorTransInfo const& trans);

private:
    UnistorApp*             m_pApp;  ///<app对象
    CwxPackageReaderEx        m_reader; ///<解包的reader
    unsigned char*          m_unzipBuf; ///<解压的buffer
    CWX_UINT32              m_uiBufLen; ///<解压buffer的大小，其为trunk的20倍，最小为20M。
    UnistorSyncSession*     m_syncSession; ///<数据同步的session
    CWX_UINT32              m_uiCurHostId; ///<当前的host id
};

#endif 
