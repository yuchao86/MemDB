#ifndef __CWX_HOST_INFO_H__
#define __CWX_HOST_INFO_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxHostInfo.h
@brief ����cwinux�ܹ�Host������
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

CWINUX_BEGIN_NAMESPACE

/**
@class CwxHostInfo
@brief Host���ݶ��󣬶���HOST�ĳ�����Ϣ
*/
class CWX_API CwxHostInfo
{
public:
    enum{
        DEF_MIN_INTERNAL = 2,
        DEF_MAX_INTERNAL = 16,
        DEF_RAW_DATA_LEN = 2048
    };
public:
    ///����Ĭ�Ϻ���
    CwxHostInfo()
    {
        reset();
    }
    ///���캯��
    CwxHostInfo(string const& strHost,        
        string const& strUser,
        CWX_UINT16 unPort)
    {
        m_strHost = strHost;
        m_strUser = strUser;
        m_unPort = unPort;
        m_bKeepAlive = true;
        m_bRawData = false;
        m_uiRawDataLen = DEF_RAW_DATA_LEN;
        m_unMinInternal = DEF_MIN_INTERNAL;
        m_unMaxInternal = DEF_MAX_INTERNAL;
    }
    ///�������캯��
    CwxHostInfo(const CwxHostInfo& obj)
    {
        m_strHost = obj.m_strHost;
        m_strUnixPathFile = obj.m_strUnixPathFile;
        m_strUser = obj.m_strUser;
        m_unPort = obj.m_unPort;
        m_unGroupId = obj.m_unGroupId;
        m_uiSvrId = obj.m_uiSvrId;
        m_uiHostId = obj.m_uiHostId;
        m_uiConnId = obj.m_uiConnId;
        m_bKeepAlive = obj.m_bKeepAlive;
        m_bRawData = obj.m_bRawData;
        m_uiRawDataLen = obj.m_uiRawDataLen;
        m_unMinInternal = obj.m_unMinInternal;
        m_unMaxInternal = obj.m_unMaxInternal;
    }
    ///��ֵ����
    CwxHostInfo& operator=(const CwxHostInfo& obj) 
    {
        if(this != & obj)
        {
            m_strHost = obj.m_strHost;
            m_strUnixPathFile = obj.m_strUnixPathFile;
            m_strUser = obj.m_strUser;
            m_unPort = obj.m_unPort;
            m_unGroupId = obj.m_unGroupId;
            m_uiSvrId = obj.m_uiSvrId;
            m_uiHostId = obj.m_uiHostId;
            m_uiConnId = obj.m_uiConnId;
            m_bKeepAlive = obj.m_bKeepAlive;
            m_bRawData = obj.m_bRawData;
            m_uiRawDataLen = obj.m_uiRawDataLen;
            m_unMinInternal = obj.m_unMinInternal;
            m_unMaxInternal = obj.m_unMaxInternal;
        }
        return *this;
    }
    ///���������Ϣ
    void reset()
    {
        m_strHost.erase();
        m_strUnixPathFile.erase();
        m_strUser.erase();
        m_unPort = 0;
        m_unGroupId = 0;
        m_uiSvrId = 0;
        m_uiHostId = 0;
        m_uiConnId = 0;
        m_bKeepAlive = true;
        m_bRawData = false;
        m_uiRawDataLen = DEF_RAW_DATA_LEN;
        m_unMinInternal = DEF_MIN_INTERNAL;
        m_unMaxInternal = DEF_MAX_INTERNAL;
    }

public:
    ///��ȡ���������ֻ�unix-domain file
    string const& getHostName() const
    {
        return m_strHost;
    }
    ///�������������ֻ�unix-domain file
    void setHostName(string const& strHost)
    { 
        m_strHost = strHost; 
    }
    ///��ȡunix domain path file
    string const& getUnixDomain() const 
    { 
        return m_strUnixPathFile;
    }
    ///����unix domain path file
    void setUnixDomain(string const& strPathFile) 
    { 
        m_strUnixPathFile = strPathFile;
    }
    ///��ȡ�������û���
    string const& getUser() const 
    {
        return m_strUser;
    }
    ///�����������û���
    void setUser(string const& strUser)
    {
        m_strUser = strUser;
    }
    ///��ȡ�����Ŀ���
    string const& getPasswd() const
    {
        return m_strPasswd;
    }
    ///���������Ŀ���
    void setPassword(string const& strPasswd)
    {
        m_strPasswd = strPasswd;
    }
    ///��ȡ�����Ķ˿ں�
    CWX_UINT16 getPort() const 
    { 
        return m_unPort;
    }
    ///���������Ķ˿ں�
    void setPort(CWX_UINT16 unPort)
    { 
        m_unPort = unPort;
    }
    ///��ȡ�����ķ���ID
    CWX_UINT16 getGroupId() const 
    { 
        return m_unGroupId;
    }
    ///���������ķ���ID
    void setGroupId(CWX_UINT16 unGroupId) 
    { 
        m_unGroupId = unGroupId;
    }
    ///��ȡ�����ķ���ID
    CWX_UINT32 getSvrId() const 
    { 
        return m_uiSvrId;
    }
    ///���������ķ���ID
    void setSvrId(CWX_UINT32 uiSvrId) 
    {
        m_uiSvrId = uiSvrId;
    }
    ///��ȡ������HOST ID
    CWX_UINT32 getHostId() const 
    { 
        return m_uiHostId;
    }
    ///����������Host ID
    void setHostId(CWX_UINT32 uiHostId) 
    {
        m_uiHostId = uiHostId;
    }
    ///��ȡ����������ID
    CWX_UINT32 getConnId() const 
    {
        return m_uiConnId;
    }
    ///��������������ID
    void setConnID(CWX_UINT32 uiConnId) 
    {
        m_uiConnId = uiConnId;
    }
    ///��ȡ�Ƿ�ִ��keep-alive
    bool isKeepAlive() const 
    {
        return m_bKeepAlive;
    }
    ///�����Ƿ�ִ��keep-alive
    void setKeepAlive(bool bKeepAlive) 
    { 
        m_bKeepAlive = bKeepAlive;
    }
    ///��ȡ�Ƿ����raw���ݵĸ�ʽͨ��
    bool isRawData() const 
    {
        return m_bRawData;
    }
    ///�����Ƿ����raw���ݵĸ�ʽͨ��
    void setRawData(bool bRawData)
    {
        m_bRawData = bRawData;
    }
    ///��ȡraw���ݵ�ÿ�������ճ���
    CWX_UINT32 getRawDataLen() const 
    {
        return m_uiRawDataLen;
    }
    ///����raw���ݵ�ÿ�������ճ���
    void setRawDataLen(CWX_UINT32 uiLen) 
    {
        m_uiRawDataLen = uiLen;
    }
    ///��ȡ��������ʧЧʱ����С�������
    CWX_UINT16 getMinInternal() const 
    {
        return m_unMinInternal;
    }
    ///������������ʧЧʱ����С�������
    void setMinInternal(CWX_UINT16 unInternal) 
    {
        m_unMinInternal = unInternal;
    }
    ///��ȡ��������ʧЧʱ������������
    CWX_UINT16 getMaxInternal() const 
    {
        return m_unMaxInternal;
    }
    ///������������ʧЧʱ������������
    void setMaxInternal(CWX_UINT16 unInternal)
    {
        m_unMaxInternal = unInternal;
    }
private:
    string			    m_strHost;///<������IP
    string             m_strUnixPathFile; ///<unix-domain file
    string			    m_strUser;///<�������û���
    string              m_strPasswd; ///<�����û��Ŀ���
    CWX_UINT16         m_unPort;///<���������˿ں�
    CWX_UINT16         m_unGroupId;///<���������ķ���ID
    CWX_UINT32         m_uiSvrId;///<���������ķ�������ID
    CWX_UINT32         m_uiHostId;///<������host ID
    CWX_UINT32         m_uiConnId;///<����������ID
    bool               m_bKeepAlive; ///<�Ƿ�keep-alive
    bool               m_bRawData; ///<�Ƿ���Raw����
    CWX_UINT32         m_uiRawDataLen; ///<raw ���ݵ������ճ���
    CWX_UINT16         m_unMinInternal; ///<��С���������
    CWX_UINT16         m_unMaxInternal; ///<�����������
};


CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif

