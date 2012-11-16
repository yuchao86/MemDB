#ifndef __CWX_APP_LISTEN_MGR_H__
#define __CWX_APP_LISTEN_MGR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file CwxAppListenMgr.h
@brief �ܹ���LISTEN�Ĺ���
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxLogger.h"
#include "CwxAppUnixAcceptor.h"
#include "CwxAppTcpAcceptor.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxAppListenObj
@brief ʵ��TCP/UNIX-DOMAIN��Accept�����ͳһ��װ��
*/
class CWX_API CwxAppListenObj
{
public:
    enum{
        TCP_LISTEN = 1,///<Tcp��Listen
        UNIX_LISTEN = 2///<Unix-domain��Listen
    };
    ///Ĭ�Ϲ��캯��
    CwxAppListenObj()
    {
        m_unListenType = TCP_LISTEN;
        m_listenObject = NULL;
    }
public:
    CWX_UINT16  m_unListenType;///<listen���������
    void*      m_listenObject;///<listen����ĵ�ַָ��
};

///����Listen�����MAP
typedef map<CWX_UINT32, CwxAppListenObj> CwxAppIdListenMap;

/**
@class CwxAppListenMgr
@brief Listen����Ĺ���������Ķ��塣
*/
class CWX_API CwxAppListenMgr
{
public:
    ///listen id��Χ����
    enum{
        MIN_LISTEN_ID = 1,///<��С��Listen ID
        MAX_LISTEN_ID = 0x3FFFFFFF///<����Listen id
    };
public:
    ///���캯��
    CwxAppListenMgr(CwxAppFramework* pApp);
    ///��������
    ~CwxAppListenMgr();
public:
    ///�����������һ��TCP��Listen���󣬷���ֵ��false��Listen-id��Listen�����Ѿ�����
    bool add(CwxAppTcpAcceptor* acceptor);
    ///�����������һ��UNIX-DOMAIN��Listen���󣬷���ֵ��false��Listen-id��Listen�����Ѿ�����
    bool add(CwxAppUnixAcceptor* acceptor);
    ///����listen-id�ر�һ��Listen���󡣷���ֵ��false,listen id�ļ������󲻴���
    bool close(CWX_UINT32 uiListenId);
    ///resumeһ��listen����ļ���������ֵ��false,listen id�ļ������󲻴���
    bool resume(CWX_UINT32 uiListenId);
    ///suspendһ��listen����ļ���������ֵ��false,listen id�ļ������󲻴���
    bool suspend(CWX_UINT32 uiListenId);
    ///��ȡ��һ�����õ�Listen-id
    CWX_UINT32 getNextListenId();
    ///��ȡlisten��id
    void getAllListens(vector<CWX_UINT32>& listens);
private:
    CwxMutexLock        m_lock;///<����������
    CWX_UINT32          m_uiListenId;///<�Ӵ�listen-id��ʼ������һ������ID
    CwxAppIdListenMap*  m_pListenMap;///<listen��map
    CwxAppFramework*    m_pApp;
};

CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
