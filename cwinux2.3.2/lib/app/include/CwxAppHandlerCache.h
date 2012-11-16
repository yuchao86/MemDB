#ifndef __CWX_APP_HANDLER_CACHE_H__
#define __CWX_APP_HANDLER_CACHE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandlerCache.h
@brief �رյ�TCP/Unix-domain�����Ӿ����CACHE����ʵ�־��������
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxAppHandler4TcpConn.h"
#include "CwxAppHandler4UnixConn.h"
#include "CwxAppHandler4IoMsg.h"
#include "CwxAppHandler4IoEvent.h"
CWINUX_BEGIN_NAMESPACE

/**
@class CwxAppHandlerCache
@brief �رյ�TCP/Unix-domain�����Ӷ����CACHE��ʵ�����Ӷ��������
*/
class CWX_API CwxAppHandlerCache
{
public:
    enum{
        MAX_FREE_HANDLE_NUM = 64///<cache�Ĺر����Ӷ��������
    };
public:
    ///���캯��
    CwxAppHandlerCache();
    ///��������
    ~CwxAppHandlerCache();
public:
    ///��ȡһ��TCP���͵����Ӷ��󣬷���NULL��ʾû��CACHE�����Ӷ���
    inline CwxAppHandler4TcpConn* fetchTcpHandle();
    ///Cacheһ���رյ�TCP���͵����Ӷ���
    inline void cacheTcpHandle(CwxAppHandler4TcpConn* pHandle);
    ///��ȡһ��UNIX-domain���͵����Ӷ��󣬷���NULL��ʾû��CACHE�����Ӷ���
    inline CwxAppHandler4UnixConn* fetchUnixHandle();
    ///Cacheһ���رյ�UNIX-domain���͵����Ӷ���
    inline void cacheUnixHandle(CwxAppHandler4UnixConn* pHandle);
    ///��ȡһ��IO MSG���͵����Ӷ��󣬷���NULL��ʾû��CACHE�����Ӷ���
    inline CwxAppHandler4IoMsg* fetchIoMsgHandle();
    ///Cacheһ���رյ�Io msg���͵����Ӷ���
    inline void cacheIoMsgHandle(CwxAppHandler4IoMsg* pHandle);
    ///��ȡһ��io event handler���͵����Ӷ��󣬷���NULL��ʾû��CACHE�����Ӷ���
    inline CwxAppHandler4IoEvent* fetchIoEventHandle();
    ///Cacheһ���رյ�io event handler���͵����Ӷ���
    inline void cacheIoEventHandle(CwxAppHandler4IoEvent* pHandle);
private:
    ///�ͷ�����cache�����Ӷ���
    void destroy();
private:
    CwxMutexLock                 m_lock;///<Cache����
    CWX_UINT32                   m_uiFreeTcpHandleNum;///<cache��TCP���Ӷ�������
    CWX_UINT32                   m_uiFreeUnixHandleNum;///<cache��UNIX-DOMAIN���Ӷ�������
    CWX_UINT32                   m_uiFreeIoMsgHandleNum; ///<cache msg handle���Ӷ�������
    CWX_UINT32                   m_uiFreeIoEventHandleNum; ///<cache io event���Ӷ�������
    CwxAppHandler4TcpConn*        m_freeTcpHandles;///<cache��TCP���Ӷ��������
    CwxAppHandler4UnixConn*       m_freeUnixHandles;///<cache��UNIX-DOMAIN���Ӷ�������
    CwxAppHandler4IoMsg*          m_freeIoMsgHandles; ///<cache msg handle���Ӷ��������
    CwxAppHandler4IoEvent*        m_freeIoEventHandles;///<cache��Io event���Ӷ��������
};

CWINUX_END_NAMESPACE

#include "CwxAppHandlerCache.inl"

#include "CwxPost.h"

#endif
