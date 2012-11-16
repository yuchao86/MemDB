#ifndef __CWX_APP_HANDLER_4_IO_EVENT_H__
#define __CWX_APP_HANDLER_4_IO_EVENT_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4IoEvent.h
@brief IO�¼����Handle�Ķ���
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "CwxAppMacro.h"
#include "CwxAppConfig.h"
#include "CwxAppHandler4Base.h"
#include "CwxAppReactor.h"

CWINUX_BEGIN_NAMESPACE

class CwxAppFramework;

/**
@class CwxAppHandler4IoEvent
@brief ����IO HANDLE�¼���⡣
*/
class CWX_API CwxAppHandler4IoEvent : public CwxAppHandler4Base
{
public:
    ///���캯��
    CwxAppHandler4IoEvent(CwxAppFramework* app,
        CwxAppReactor *reactor);
    ///��������
    ~CwxAppHandler4IoEvent();
public:
    /**
    @brief handler open����reactor��ע�ᡣ
    @param [in] arg  ��Ч����
    @return -1��ʧ�ܣ� 0���ɹ�
    */
    virtual int open (void * arg= 0);
    /**
    @brief ���������¼�������
    @param [in] event �����ϵ��¼�
    @param [in] handle  �������¼���handle��
    @return -1������ʧ�ܣ� 0������ɹ�
    */
    virtual int handle_event(int event, CWX_HANDLE handle=CWX_INVALID_HANDLE);
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE);
public:
    ///get svr id
    inline CWX_UINT32 getSvrId() const
    {
        return m_uiSvrId;
    };
    ///set svr id
    inline void setSvrId(CWX_UINT32 uiSvrId)
    {
        m_uiSvrId = uiSvrId;
    }
    ///get host id
    inline CWX_UINT32 getHostId() const
    {
        return m_uiHostId;
    }
    ///set host id
    inline void setHostId(CWX_UINT32 uiHostId)
    {
        m_uiHostId = uiHostId;
    }
    ///get io event mask
    inline int getIoEventMask() const
    {
        return m_ioEventMask;
    }
    ///set io event mask
    inline void setIoEventMask(int mask)
    {
        m_ioEventMask = mask&CwxAppHandler4Base::RW_MASK;
    }
    ///get user data
    inline void* getUserData() const
    {
        return m_userData;
    }
    ///set user data
    inline void setUserData(void* userData)
    {
        m_userData = userData;
    }
    ///��ȡapp
    CwxAppFramework* getApp()
    {
        return m_pApp;
    }

public:
    CwxAppHandler4IoEvent* m_next;
private:
    CwxAppFramework* m_pApp;
    CWX_UINT32   m_uiSvrId;///<Handle��svr id
    CWX_UINT32   m_uiHostId;///<Handle��host id
    CWX_UINT16   m_ioEventMask;///<Handle���¼��������
    void*       m_userData;///<Handle���û�����
};

CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
