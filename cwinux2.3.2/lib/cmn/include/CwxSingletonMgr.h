#ifndef __CWX_SINGLETON_MGR_H__
#define __CWX_SINGLETON_MGR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxSingletonMgr.h
@brief ��ʵ������Ĺ�����Ķ��壬���ڹ�����Щ��Ҫ�ͷŵĵ�ʵ������
@author cwinux@gmail.com
@version 0.1
@date 2009-10-02
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxSingleton.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxSingletonMgr
@brief ��ʵ������Ĺ����࣬���ڹ�����Щ��Ҫ�ͷŵĵ�ʵ������
       CwxSingletonMgrͬ��Ҳ��singleton����
*/

class CWX_API CwxSingletonMgr
{
public:
    ///��ȡCwxSingletonMgr�����ʵ������û�д����򴴽�
    static CwxSingletonMgr* instance();
    ///�ͷ�CwxSignletonMgr��Singleton���󡣴�API��ע����os��atexit��
    static void destroy();
public:
    ///ע����Ҫ�ͷŵ�singleton����
    void  reg(CwxSingleton* pObj);
private:
    ///���캯������ֻ֤����instance() API������
    CwxSingletonMgr();
    ///������������ֻ֤����destroy() API�ͷŴ�����
    ~CwxSingletonMgr();
private:
    static CwxSingletonMgr*    m_pInstance;///<Mgr��singleton����
    static CwxMutexLock        m_lock;///<singleton����ı�����
    CwxSingleton*      m_pObjs;///<ע���singleton���������
};

CWINUX_END_NAMESPACE


#include "CwxPost.h"

#endif
