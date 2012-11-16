#ifndef __CWX_APP_NOTICE_PIPE_H__
#define __CWX_APP_NOTICE_PIPE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxAppNoticePipe.h
*@brief Framework��Notice��Ϣ�Ĺܵ�
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-05-30
*@warning  ��.
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxAppMacro.h"
#include "CwxAppConfig.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxLogger.h"
#include <unistd.h>
#include <sys/socket.h>
CWINUX_BEGIN_NAMESPACE

/**
@class CwxAppNotice
@brief Framework��Notice�������ڱ���߳���ͨ���̵߳�ͨ��
*/
class CWX_API CwxAppNotice
{
public:
    ///notice���Ͷ���
    enum{
        DUMMY = 0,///<dummy����
        SEND_MSG_BY_CONN = 1,///<�����ӷ�����Ϣ
        SEND_MSG_BY_SVR = 2, ///<��ָ����SVR���鷢����Ϣ
        TCP_CONNECT = 3,///<����TCP����
        UNIX_CONNECT = 4,///<����UNIX DOMAIN����
        ADD_IO_HANDLE = 5,///<IO HANDLE���ݽ���
        TCP_LISTEN = 6,///<TCP listen
        UNIX_LISTEN = 7,///<unix-domain listen
        CLOSE_LISTEN = 8,///<�رռ���
        CLOSE_CONNECT = 9,///<�ر�����
        RECONNECTION = 10,///<��������
        ALL_NOTICE_NUM = 11///<Notice���͵�����
    };
public:
    ///���캯��
    CwxAppNotice();
    ///��ն���
    void reset();
public:
    CWX_UINT16              m_unNoticeType;///<notice msg type
    void*                  m_noticeArg; ///<notice's arguement
    void*                  m_noticeArg1;///<notice's arguement2
    CwxAppNotice*           m_next;///<����notice�����ָ��
};


/**
*@class  CwxAppNoticePipe
*@brief  Framework��Notice��Ϣ�Ĺܵ�������ͨ���߳��������̼߳��ͨ�š�
*/
class CWX_API CwxAppNoticePipe
{
public:
    ///���캯��
    CwxAppNoticePipe();
    ///��������
    ~CwxAppNoticePipe();
public:
    ///�ܵ���ʼ��������ֵ��-1��ʧ�ܣ�0���ɹ���
    int init();
    /**
    @brief ���ܵ�����pushһ��notice��Ϣ
    @param [in] pItem ֪ͨ����Ϣ
    @return -1��д�ܵ�ʧ�ܣ� 0���ɹ�
    */
    int notice(CwxAppNotice* pItem);
    /**
    @brief ���ܵ�����pushһ��dummy notice��Ϣ����Ŀ���ǣ���ͨ���߳���������֤����ͨ���̡߳�
    @return -1��д�ܵ�ʧ�ܣ� 0���ɹ�
    */
    int noticeDummy();
    /**
    @brief ͨ���̻߳�ȡ��ѹ��Notice��Ϣ��
    @return void
    */
    void noticed(CwxAppNotice*& head);
    ///��ȡ�ܵ��Ķ����
    CWX_HANDLE getPipeReader();
    ///��ȡ�ܵ���д���
    CWX_HANDLE getPipeWriter();
private:
    ///�ͷŶ������Դ
    void clear();
private:
    CwxMutexLock   m_lock;///<thread lock for sync.
    bool           m_bPipeEmpty; ///<notice pipe empty sign.
    CwxAppNotice*  m_noticeHead; ///<notice's head.
    CwxAppNotice*  m_noticeTail; ///<notice's tail
    CWX_HANDLE      m_pipeReader; ///<pipe handle for read
    CWX_HANDLE      m_pipeWriter; ///<pipe handle for write
};

CWINUX_END_NAMESPACE

#include "CwxAppNoticePipe.inl"
#include "CwxPost.h"

#endif
