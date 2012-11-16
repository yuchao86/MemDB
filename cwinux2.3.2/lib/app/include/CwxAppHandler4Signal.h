#ifndef __CWX_APP_HANDLER_4_SIGNAL_H__
#define __CWX_APP_HANDLER_4_SIGNAL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4Signal.h
@brief signal��ӦHandle
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxAppMacro.h"
#include "CwxAppConfig.h"
#include "CwxAppHandler4Base.h"
#include "CwxAppReactor.h"

CWINUX_BEGIN_NAMESPACE

class CwxAppFramework;
/**
@class CwxAppHandler4Signal
@brief signal Handle
*/
class CWX_API CwxAppHandler4Signal:public CwxAppHandler4Base
{
public:
    ///���캯��
    CwxAppHandler4Signal(CwxAppFramework* pApp, CwxAppReactor* reactor, int sig);
    ///��������
    ~CwxAppHandler4Signal();
public:
    /**
    @brief handler open����reactor��ע�ᡣ
    @param [in] arg  ��Ч����
    @return -1��ʧ�ܣ� 0���ɹ�
    */
    virtual int open (void * arg= 0);
    /**
    @brief ����signal���¼�
    @param [in] event signal �¼�
    @param [in] handle  �������¼���handle��
    @return -1������ʧ�ܣ� 0������ɹ�
    */
    virtual int handle_event(int event, CWX_HANDLE handle=CWX_INVALID_HANDLE);
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE);
public:
    ///��ȡapp
    CwxAppFramework* getApp()
    {
        return m_pApp;
    }
private:
    CwxAppFramework* m_pApp;

};

CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
