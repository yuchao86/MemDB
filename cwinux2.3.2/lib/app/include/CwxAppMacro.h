#ifndef __CWX_APP_MACRO_H__
#define __CWX_APP_MACRO_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxAppMacro.h
*@brief app��ȫ�ֺ궨��
*@author cwinux@gmail.com
*@version 0.1.0
*@date  2009-06-28
*@warning  ��.
*/
#include "CwxPre.h"
#include <signal.h>


#define CWX_APP_MAX_IO_NUM_BIT 19
#define CWX_APP_MAX_IO_NUM  (1<<19)
#define CWX_APP_MAX_IO_ID (CWX_APP_MAX_IO_NUM - 1)

#define CWX_APP_MAX_SIGNAL_ID _NSIG

///����ģʽ����
#define CWX_APP_MSG_MODE    1 ///<����msg������ģʽ
#define CWX_APP_EVENT_MODE  2 ///<����event������ģʽ

///��Ч����ID����
#define CWX_APP_INVALID_CONN_ID 0 ///<��Ч������ID
#define CWX_APP_MIN_CONN_ID     1 ///<��С������ID
#define CWX_APP_MAX_CONN_ID     0x7FFFFFFF ///<��������ID

///����Handle ����
#define CWX_APP_HANDLE_UNKNOW  0
#define CWX_APP_HANDLE_SIGNAL  1
#define CWX_APP_HANDLE_TIMEOUT 2


#define CWX_APP_MAX_TASK_NUM  4096 ///<Taskboard������Task����
#define CWX_APP_DEF_LOG_FILE_SIZE  1024 * 1024 * 20 ///<ȱʡ����־�ļ���С
#define CWX_APP_DEF_LOG_FILE_NUM   7 ///<ȱʡ����־�ļ�����

#define CWX_APP_DEF_BACKLOG_NUM   8192


#define CWX_APP_DEF_KEEPALIVE_IDLE 10//��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��
#define CWX_APP_DEF_KEEPALIVE_INTERNAL 5//����KeepAlive̽����ʱ����
#define CWX_APP_DEF_KEEPALIVE_COUNT   2//�ж��Ͽ�ǰ��KeepAlive̽�����

#include "CwxPost.h"
#endif
