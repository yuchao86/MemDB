#ifndef __UNISTOR_DEF_H__
#define __UNISTOR_DEF_H__
#include "UnistorMacro.h"
#include "CwxStl.h"
#include "CwxHostInfo.h"


///idc������ͬ���Ĵ�����Ϣ����
class UnistorTransInfo{
public:
    ///���캯��
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
    list<string>        m_ips; ///<���õ�ip�б�
    CWX_UINT16          m_unPort; ///<�˿ں�
    string              m_strUser; ///<ͬ�����û���
    string              m_strPasswd; ///<ͬ�����û�����
    bool                m_bZip; ///<�Ƿ�ѹ��
    string              m_strSign; ///<ǩ����ʽ
    CWX_UINT16          m_unConnNum; ///<�������ӵ�����
    CWX_UINT32          m_uiChunkSize; ///<chunk�Ĵ�С
};

///ZK conf���ýڵ����Ϣ
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
    bool                m_bMasterIdc; ///<�Ƿ�Ϊmaster idc
    string              m_strMasterIdc; ///<master idc������
    UnistorTransInfo    m_transInfo;   ///<ת����������Ϣ
    CWX_UINT64          m_ullVersion; ///<�汾��
};

///zookeeper������Ϣ
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
    bool        m_bMaster; ///<�Ƿ�����
    string      m_strMaster; ///<Master����
    string      m_strPrev;  ///<ǰһ������
    CWX_UINT64  m_ullPreMasterMaxSid; ///<ǰһ��master�����sid
    CWX_UINT64          m_ullVersion; ///<�汾��
};

///�������Զ���
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
    bool        m_bNoDelay; ///<�Ƿ�nodelay
    bool        m_bLinger; ///<�Ƿ�����LINGER
    CWX_UINT32  m_uiRecvBuf; ///<����buf��С,0��ʾ������
    CWX_UINT32  m_uiSendBuf; ///<����buf��С��0��ʾ������
    bool        m_bKeepalive; ///<�Ƿ�����keepalive
};



///����ip:port��ʽ�����ݡ�����ֵ��true���ɹ���false��ʧ��
bool parseHostPort(string const& strHostPort,///<��ʽΪip/host:port���ַ���
                   CwxHostInfo& host ///<����host�Ķ���
                   );


#endif
