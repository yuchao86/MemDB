#ifndef __CWX_APP_TCP_ACCEPTOR_H__
#define __CWX_APP_TCP_ACCEPTOR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppTcpAcceptor.h
@brief Tcp��������Ķ���
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxINetAddr.h"
#include "CwxSockAcceptor.h"
#include "CwxSockStream.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxAppHandler4TcpConn.h"
#include "CwxAppHandler4Base.h"
#include "CwxAppReactor.h"


CWINUX_BEGIN_NAMESPACE

class CwxAppFramework;
/**
@class CwxAppTcpAcceptor
@brief �ܹ���TCP ���ӵļ����������ÿ������ʵ������һ��TCP �˿ڵļ�����
*/
class CWX_API CwxAppTcpAcceptor  :public CwxAppHandler4Base
{
public:
    ///��������
    enum{
        MAX_PER_RECIEVE_CONNECT  = 512 ///<tcp listen's backlog value
    };
public:
    ///���캯��
    CwxAppTcpAcceptor(CwxAppFramework* pApp,///<
        CwxAppReactor *reactor,
        char const* szAddr, ///<listen addr
        CWX_UINT32 unPort, ///<listen port
        CWX_UINT32 uiSvrId, ///<connect's svr-id
        CWX_UINT32 uiListenId, ///<acceptor's listen id
        bool       bRawData = false, ///<connect's msg having header
        CWX_UINT16 unMode = CWX_APP_MSG_MODE,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* fnArg=NULL);

    ///��������
    ~CwxAppTcpAcceptor();
public:
    int accept(CwxINetAddr const& addr);
    ///handle open
    virtual int open (void * parent= 0);
    /// �¼��Ļص�����������-1��ʧ�ܣ��Զ��ͷ�handle��0���ɹ�
    virtual int handle_event(int event, CWX_HANDLE handle=CWX_INVALID_HANDLE);
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE);
public:
    ///get listen addr
    string const& getAddr() const
    {
        return m_strAddr;
    }
    ///get port
    CWX_UINT16 getPort() const
    {
        return m_unPort;
    }
    ///get svr id
    CWX_UINT32	getSvrId() const
    {
        return m_uiSvrId;
    }
    ///get Listen id
    CWX_UINT32 getListenId() const
    {
        return m_uiListenId;
    }
    ///get sign for msg-header
    bool isRawData() const
    {
        return m_bRawData;
    }
    ///get conn mode
    CWX_UINT16 getConnMode() const
    {
        return m_unMode;
    }
    ///get close all sign
    bool isCloseAll() const
    {
        return m_bCloseAll;
    }
    ///set close all sign
    void setCloseAll(bool bCloseAll)
    {
        m_bCloseAll = bCloseAll;
    }
    ///��ȡapp
    CwxAppFramework* getApp()
    {
        return m_pApp;
    }
private:
    CwxAppHandler4TcpConn* makeHander();
private:
    CwxAppFramework* m_pApp;
    string         m_strAddr;///<�����ĵ�ַ
    CWX_UINT16     m_unPort;///<�����Ķ˿ں�
    CWX_UINT32	    m_uiSvrId;///<�������ӵ�svr-id
    CWX_UINT32     m_uiListenId;///<TCP ���Ӽ����ļ���ID
    bool           m_bRawData;///<�����������Ƿ��޸�ʽ�������ķ�ʽ����
    CWX_UINT16     m_unMode;///<�ܹ��������������ݽ��գ�����ֻ������ӵ��¼�
    bool           m_bCloseAll;///<listen�رյ�ʱ���Ƿ�ر���������������
    CWX_NET_SOCKET_ATTR_FUNC m_fn;
    void*          m_fnArg;
    CwxSockAcceptor m_acceptor; ///<������
    CwxSockStream   m_stream;
};
CWINUX_END_NAMESPACE


#include "CwxPost.h"

#endif

