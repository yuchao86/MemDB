#ifndef __CWX_APP_HANDLER_4_TCP_CONN_H__
#define __CWX_APP_HANDLER_4_TCP_CONN_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4TcpConn.h
@brief ����TCP��PIPE��������IOͨ�š������Handle����CwxAppHandler4TcpConn
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxMsgBlock.h"
#include "CwxMsgHead.h"
#include "CwxINetAddr.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxLogger.h"
#include "CwxAppHandler4Base.h"
#include "CwxAppReactor.h"
#include "CwxAppHandler4Msg.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxAppHandler4TcpConn
@brief TCP��PIPE��������IOͨ�š�״̬�����Handle����
*/

class CwxAppFramework;
class CwxAppHandlerCache;
class CwxAppTcpAcceptor;

class CWX_API CwxAppHandler4TcpConn:public CwxAppHandler4Msg
{
public:
    ///���캯��
    CwxAppHandler4TcpConn(CwxAppFramework* pApp, CwxAppReactor *reactor);
    ///��������
    ~CwxAppHandler4TcpConn();
public:
    /**
    @brief ��ʼ�����������ӣ�����Reactorע������
    @param [in] arg �������ӵ�acceptor��ΪNULL
    @return -1���������������ӣ� 0�����ӽ����ɹ�
    */
    virtual int open (void * arg= 0);
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE);
    ///��ʱ
    virtual void handle_timeout();
    /**
    @brief ��ȡ���ӵĶԶ˵�ַ��ֻ��STREAM_TYPE_TCP��STREAM_TYPE_UNIX��Ч
    @param [in,out] szBuf ���ص�ַ��buf,��ȡ�ɹ�����\\0������
    @param [in] unSize szBuf�Ĵ�С��
    @return ����szBuf
    */
    virtual char* getRemoteAddr(char* szBuf, CWX_UINT16 unSize);
    /**
    @brief ��ȡ���ӵĶԶ�port��ֻ��STREAM_TYPE_TCP��Ч
    @return ���ӶԶ˵�port
    */
    virtual CWX_UINT16 getRemotePort();
    /**
    @brief ��ȡ���ӱ��˵ĵ�ַ��ֻ��STREAM_TYPE_TCP��STREAM_TYPE_UNIX��Ч
    @param [in,out] szBuf ���ص�ַ��buf,��ȡ�ɹ�����\\0������
    @param [in] unSize szBuf�Ĵ�С��
    @return ����szBuf
    */
    virtual char* getLocalAddr(char* szBuf, CWX_UINT16 unSize);
    /**
    @brief ��ȡ���ӵı���port��ֻ��STREAM_TYPE_TCP��Ч
    @return ���ӶԶ˵�port
    */
    virtual inline CWX_UINT16 getLocalPort();
public:
    ///�����������ӵ����ӵ�ַ
    inline void setConnectAddr(char const* szAddr);
    ///��ȡ�������ӵ����ӵ�ַ
    inline string const& getConnectAddr() const;
    ///�����������ӵ����Ӷ˿�
    inline void setConnectPort(CWX_UINT16 unPort);
    ///��ȡ�������ӵ����Ӷ˿�
    inline CWX_UINT16 getConnectPort() const;
public:
    CwxAppHandler4TcpConn*    m_next; ///<next connection
private:
    string                  m_strConnectAddr;///<ip addr
    CWX_UINT16               m_unConnectPort;//</port for connect
    CwxINetAddr              m_remoteAddr;
};
CWINUX_END_NAMESPACE

#include "CwxAppHandler4TcpConn.inl"

#include "CwxPost.h"

#endif
