#ifndef __UNISTOR_DEF_H__
#define __UNISTOR_DEF_H__
#include "UnistorMacro.h"
#include "CwxStl.h"
#include "CwxHostInfo.h"


///idc间数据同步的传输信息定义
class UnistorTransInfo{
public:
    ///构造函数
    UnistorTransInfo(){
        m_bZip = false;
        m_unConnNum = UNISTOR_DEF_CONN_NUM;
        m_uiChunkSize = 0;
    }
    
    UnistorTransInfo(UnistorTransInfo const& item){
        m_ips = item.m_ips;
        m_unPort = item.m_unPort;
        m_strUser = item.m_strUser;
        m_strPasswd = item.m_strPasswd;
        m_bZip = item.m_bZip;
        m_strSign = item.m_strSign;
        m_unConnNum = item.m_unConnNum;
        m_uiChunkSize = item.m_uiChunkSize;
    }

    UnistorTransInfo& operator = (UnistorTransInfo const& item){
        if (this != &item){
            m_ips = item.m_ips;
            m_unPort = item.m_unPort;
            m_strUser = item.m_strUser;
            m_strPasswd = item.m_strPasswd;
            m_bZip = item.m_bZip;
            m_strSign = item.m_strSign;
            m_unConnNum = item.m_unConnNum;
            m_uiChunkSize = item.m_uiChunkSize;
        }
        return *this;
    }

    bool operator == (UnistorTransInfo const& item) const{
        if (m_ips != item.m_ips) return false;
        if (m_unPort != item.m_unPort) return false;
        if (m_strUser != item.m_strUser) return false;
        if (m_strPasswd != item.m_strPasswd) return false;
        if (m_bZip != item.m_bZip) return false;
        if (m_strSign != item.m_strSign) return false;
        if (m_unConnNum != item.m_unConnNum) return false;
        if (m_uiChunkSize != item.m_uiChunkSize) return false;
        return true;
    }

public:
    list<string>        m_ips; ///<可用的ip列表
    CWX_UINT16          m_unPort; ///<端口号
    string              m_strUser; ///<同步的用户名
    string              m_strPasswd; ///<同步的用户口令
    bool                m_bZip; ///<是否压缩
    string              m_strSign; ///<签名方式
    CWX_UINT16          m_unConnNum; ///<并发连接的数量
    CWX_UINT32          m_uiChunkSize; ///<chunk的大小
};

///ZK conf配置节点的信息
class UnistorZkConf{
public:
    UnistorZkConf(){
        m_bMasterIdc = false;
        m_ullVersion = 0;
    }
public:
    UnistorZkConf(UnistorZkConf const& item){
        m_bMasterIdc = item.m_bMasterIdc;
        m_strMasterIdc = item.m_strMasterIdc;
        m_transInfo = item.m_transInfo;
        m_ullVersion = item.m_ullVersion;
    }
    UnistorZkConf& operator = (UnistorZkConf const& item){
        if (this != &item){
            m_bMasterIdc = item.m_bMasterIdc;
            m_strMasterIdc = item.m_strMasterIdc;
            m_transInfo = item.m_transInfo;
            m_ullVersion = item.m_ullVersion;
        }
        return *this;
    }
    bool operator ==(UnistorZkConf const& item) const{
        return (m_strMasterIdc == item.m_strMasterIdc)&& (m_transInfo == item.m_transInfo);
    }
public:
    bool                m_bMasterIdc; ///<是否为master idc
    string              m_strMasterIdc; ///<master idc的名字
    UnistorTransInfo    m_transInfo;   ///<转发的主机信息
    CWX_UINT64          m_ullVersion; ///<版本号
};

///zookeeper锁的信息
class UnistorZkLock{
public:
    UnistorZkLock(){
        m_bMaster = false;
        m_ullPreMasterMaxSid = 0;
        m_ullVersion = 0;
    }
    UnistorZkLock(UnistorZkLock const& item){
        m_bMaster = item.m_bMaster;
        m_strMaster = item.m_strMaster;
        m_strPrev = item.m_strPrev;
        m_ullPreMasterMaxSid = item.m_ullPreMasterMaxSid;
        m_ullVersion = item.m_ullVersion;
    }
    UnistorZkLock& operator=(UnistorZkLock const& item){
        if (this != &item){
            m_bMaster = item.m_bMaster;
            m_strMaster = item.m_strMaster;
            m_strPrev = item.m_strPrev;
            m_ullPreMasterMaxSid = item.m_ullPreMasterMaxSid;
            m_ullVersion = item.m_ullVersion;
        }
        return *this;
    }
    bool operator==(UnistorZkLock const& item) const{
        return (m_strMaster == item.m_strMaster)&&(m_strPrev == item.m_strPrev);
    }
public:
    bool        m_bMaster; ///<是否获得锁
    string      m_strMaster; ///<Master主机
    string      m_strPrev;  ///<前一个主机
    CWX_UINT64  m_ullPreMasterMaxSid; ///<前一个master的最大sid
    CWX_UINT64          m_ullVersion; ///<版本号
};

///连接属性定义
class UnistorConnAttr{
public:
    UnistorConnAttr(){
        m_bNoDelay = false;
        m_bLinger = false;
        m_uiRecvBuf = 0;
        m_uiSendBuf = 0;
        m_bKeepalive = false;
    }
public:
    bool        m_bNoDelay; ///<是否nodelay
    bool        m_bLinger; ///<是否设置LINGER
    CWX_UINT32  m_uiRecvBuf; ///<接收buf大小,0表示不设置
    CWX_UINT32  m_uiSendBuf; ///<发送buf大小，0表示不设置
    bool        m_bKeepalive; ///<是否设置keepalive
};



///解析ip:port格式的内容。返回值：true：成功；false：失败
bool parseHostPort(string const& strHostPort,///<格式为ip/host:port的字符串
                   CwxHostInfo& host ///<返回host的对象
                   );


#endif
