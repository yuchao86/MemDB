#ifndef __CWX_APP_HANDLER_4_IO_MSG_H__
#define __CWX_APP_HANDLER_4_IO_MSG_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4IoMsg.h
@brief �������msg�շ���IOͨ�š������Handle����CwxAppHandler4IoMsg��
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

CWINUX_BEGIN_NAMESPACE
/**
@class CwxAppHandler4IoMsg
@brief PIPE��������IOͨ�š�״̬�����Handle����
*/
class CwxAppFramework;

class CWX_API CwxAppHandler4IoMsg:public CwxAppHandler4Msg
{
public:
    ///���캯��
    CwxAppHandler4IoMsg(CwxAppFramework* pApp,
        CwxAppReactor *reactor);
    ///��������
    ~CwxAppHandler4IoMsg();
public:
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE);
public:
    CwxAppHandler4IoMsg*     m_next;
};
CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif
