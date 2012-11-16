#ifndef __CWX_APP_HANDLER_4_UNIX_CONN_H__
#define __CWX_APP_HANDLER_4_UNIX_CONN_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4UnixConn.h
@brief ����UNIX-DOMAIN����ͨ�ŵ�Handle����CwxAppHandler4UnixConn
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
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxLogger.h"
#include "CwxAppHandler4Msg.h"
#include "CwxAppReactor.h"

CWINUX_BEGIN_NAMESPACE

class CwxAppFramework;
class CwxAppHandlerCache;
/**
@class CwxAppHandler4UnixConn
@brief UNIX-DOMAIN���ӵ�Handle���󣬸���UNIX-DOMAIN���ӵĶ���
*/
class CWX_API CwxAppHandler4UnixConn:public CwxAppHandler4Msg
{
public:
    ///���캯��
    CwxAppHandler4UnixConn(CwxAppFramework* pApp, CwxAppReactor *reactor);
    ///��������
    ~CwxAppHandler4UnixConn();
public:
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
    void setConnectPathFile(char const* szPathFile);
    string const& getConnectPathFile() const;

public:
    CwxAppHandler4UnixConn*    m_next; ///<next connection
private:
    string                m_strConnectPathFile;///path file for connect
};
CWINUX_END_NAMESPACE

#include "CwxAppHandler4UnixConn.inl"
#include "CwxPost.h"

#endif
