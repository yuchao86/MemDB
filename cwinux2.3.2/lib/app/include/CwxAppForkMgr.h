#ifndef __CWX_APP_FORK_MGR_H__
#define __CWX_APP_FORK_MGR_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppForkMgr.h
@brief �첽fork�������Ķ���
@author cwinux@gmail.com
@version 0.1
@date 2009-11-25
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxStl.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"

CWINUX_BEGIN_NAMESPACE

class CwxAppFramework;
/**
@class CwxAppForkEnv
@brief �첽fork����Ϣ���󣬲�������child���̵Ĵ�����̡���fork��ʱ��
       �û���Ҫ���ش��࣬������ݵĴ�����child ���еĴ������
*/
class CWX_API CwxAppForkEnv
{
public:
    ///���캯��
    CwxAppForkEnv()
    {
    }
    ///��������
    virtual ~CwxAppForkEnv()
    {
    }
public:
    /**
    @brief ֪ͨ�ϲ�Ӧ���ڴ˵�ִ��fork������
           �û��ڴ�API�ڣ�ʵ��os��fork�Ĳ���������fork��صĲ�����
    @return   ���ڸ����̣��ɹ��򷵻��ӽ��̵�pid��ʧ�ܵ�ʱ�򷵻�-1��ʧ��ʱ�ӽ���û�д�����
    ����child���̣��򷵻�0��
    */
    virtual int onFork()=0;
    /**
    @brief child���̵���ڣ���ɴӸ�����clone��pApp��������ȡϵͳ��Դ�������ӵȡ�
           ��APIĬ��ʲôҲ����
    @param [in] pApp �Ӹ�����clone������app�����뿪��api��pApp�����ͷš�
    @return void
    */
    virtual void onChildEntry(CwxAppFramework* pApp)
    {
        CWX_UNUSED_ARG(pApp);
    }
    /**
    @brief child���̵����壬���child�Ĺ�������APIΪ���麯����Ӧ�ñ�������ʵ�֡�
    @return void
    */
    virtual void onChildMain()=0;
};


/**
@class CwxAppForkMgr
@brief �첽fork����Ϣ�������
*/
class CWX_API CwxAppForkMgr
{
public:
    ///���캯��
    CwxAppForkMgr()
    {
        m_bEvent = false;
    }
    ///��������
    ~CwxAppForkMgr()
    {
        list<CwxAppForkEnv*>::iterator iter = m_forks.begin();
        while(iter != m_forks.end())
        {
            delete *iter;
            iter++;
        }
        m_forks.clear();
    }
public:
    ///���һ��fork���¼�
    inline void append(CwxAppForkEnv* pForkEnv)
    {
        CwxMutexGuard<CwxMutexLock> lock(&m_lock);
        if (pForkEnv)
        {
            m_forks.push_back(pForkEnv);
            m_bEvent = true;
        }
    }
    ///��ȡ���е�fork�¼�
    inline void getForkEnvs(list<CwxAppForkEnv*>& forks)
    {
        CwxMutexGuard<CwxMutexLock> lock(&m_lock);
        forks = m_forks;
        m_bEvent = false;
        m_forks.clear();
    }
    ///��ȡ�Ƿ����fork���¼�������ֻ���жϱ�ǣ����Ϊ��Ч�ʲ�����
    inline bool isForkEvent() const
    {
        return m_bEvent;
    }
private:
    CwxMutexLock        m_lock;///<����������
    list<CwxAppForkEnv*> m_forks; ///<fork���¼�
    bool                m_bEvent; ///<�Ƿ���fork�¼���ֻ��һ����־�������б���
};

CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
