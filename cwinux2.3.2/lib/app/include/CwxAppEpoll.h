#ifndef __CWX_APP_EPOLL_H__
#define __CWX_APP_EPOLL_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppEpoll.h
@brief �ܹ���epoll�¼��������
@author cwinux@gmail.com
@version 0.1
@date 2011-04-13
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxTimeValue.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxAppHandler4Base.h"
#include "CwxThread.h"
#include "CwxLogger.h"
#include "CwxMinHeap.h"
#include "CwxIpcSap.h"
#include "CwxDate.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

CWINUX_BEGIN_NAMESPACE

class CwxAppReactor;
class CwxAppChannel;

/**
@class CwxAppEpoll
@brief �ܹ���epoll�¼�����
*/
typedef void (*REACTOR_CALLBACK)(CwxAppHandler4Base* handler, int mask, bool bPersist, void *arg);

class CWX_API CwxAppEpoll
{
public:
    enum
    {
        CWX_EPOLL_INIT_HANDLE = 81920
    };
public:
    ///���캯��
    CwxAppEpoll(bool bEnableSignal=true);
    ///��������
    ~CwxAppEpoll();
public:
    /**
    @brief ��ʼ��epoll���档
    @return -1��ʧ�ܣ�0���ɹ�
    */
    int init();
    /**
    @brief ע��IO�¼�����handle��
    @param [in] handle ����IO handle
    @param [in] event_handler io handle��Ӧ��event handler��
    @param [in] mask ע����¼����룬ΪREAD_MASK��WRITE_MASK��PERSIST_MASK��TIMEOUT_MASK���
    @param [in] uiMillSecond ���ٺ��볬ʱ��0��ʾû�г�ʱ���á�
    @param [in] bForkAdd �Ƿ���fork��������ӡ�
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int registerHandler(CWX_HANDLE handle,
        CwxAppHandler4Base *event_handler,
        int mask,
        CWX_UINT32 uiMillSecond = 0);
    /**
    @brief ɾ��io�¼�����handle��
    @param [in] handle �Ƴ��� io handle
    @return NULL�������ڣ����򣺳ɹ���
    */
    CwxAppHandler4Base* removeHandler (CWX_HANDLE handle);
    /**
    @brief suspend io�¼�����handle��
    @param [in] handle suspend io handle
    @param [in] suspend_mask suspend���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�0���ɹ�
    */
    int suspendHandler (CWX_HANDLE handle,
        int suspend_mask);
    /**
    @brief resume io�¼�����handle��
    @param [in] handle resume io handle
    @param [in] resume_mask resume���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int resumeHandler (CWX_HANDLE handle,
        int resume_mask);
    /**
    @brief ע��signal�¼�����handle���źž���PERSIST���ԡ�
    @param [in] signum �ź�
    @param [in] event_handler signal��event handler
    @return -1��ʧ�ܣ� 0���ɹ���
    */
    int registerSignal(int signum,
        CwxAppHandler4Base *event_handler);
    /**
    @brief ɾ��signal�¼�����handle��
    @param [in] event_handler signal��event handler
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int removeSignal(CwxAppHandler4Base *event_handler);
    /**
    @brief ɾ��signal�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] sig signal
    @return NULL�������ڣ����򷵻�signal��handler
    */
    CwxAppHandler4Base* removeSignal(int sig);

    /**
    @brief ���ö�ʱ����handle��timeout������persist���ԡ�
    @param [in] event_handler timer��event handler
    @param [in] interval ��ʱ�ļ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int scheduleTimer (CwxAppHandler4Base *event_handler,
        CwxTimeValue const &interval);
    ///ȡ����ʱ����handle��
    int cancelTimer (CwxAppHandler4Base *event_handler);
    ///fork��re-init����������ֵ��0���ɹ���-1��ʧ��
    int forkReinit();
    /**
    @brief ����¼���
    @param [in] callback �¼��Ļص�����
    @param [in] arg �ص������Ĳ���
    @param [in] uiMiliTimeout ��ʱ�ĺ�������0��ʾһֱ�������¼�������
    @return -1��ʧ�ܣ�0���ɹ�
    */
    int poll(REACTOR_CALLBACK callback, void* arg, CWX_UINT32 uiMiliTimeout=0);
    ///ֹͣ����
    void stop();
    ///��ȡ��ǰ��ʱ��
    CwxTimeValue const& getCurTime() const;
private:
    ///��ȡ��һ����epoll�ĳ�ʱʱ��
    void timeout(CWX_UINT64& ullTime);
    ///��� epoll���,maskΪREAD_MASK��WRITE_MASK����ϡ�
    int addEvent(int fd, int mask);
    ///ɾ�����ڵ�mask��maskΪREAD_MASK��WRITE_MASK����ϡ�
    int delEvent(int fd, int mask);
    ///�ź�handle
    static void sigAction(int , siginfo_t *info, void *);
private:
    class EventHandle
    {
    public:
        EventHandle()
        {
            m_mask = 0;
            m_handler = NULL;
        }
        inline bool isReg() { return (m_mask&CwxAppHandler4Base::RW_MASK) != 0;}
    public:
        int         m_mask;
        CwxAppHandler4Base* m_handler;
    };
    class SignalHanlder:public CwxAppHandler4Base
    {
    public:
        SignalHanlder():CwxAppHandler4Base(NULL)
        {
        }
        ~SignalHanlder(){}
    public:
        virtual int open (void *){ return 0;}
        virtual int handle_event(int , CWX_HANDLE handle)
        {
            char sigBuf[64];
            read(handle, sigBuf, 64);
            return 0;
        }
        virtual int close(CWX_HANDLE ){return 0;}
    };
private:
    friend class CwxAppReactor;
    friend class CwxAppChannel;
private:
    int                             m_epfd;     ///<epoll��fd
    struct epoll_event              m_events[CWX_APP_MAX_IO_NUM]; ///<epoll��event ����
    EventHandle                     m_eHandler[CWX_APP_MAX_IO_NUM]; ///<epoll��event handler
    static int                      m_signalFd[2]; ///<�źŵĶ�дhandle
    static sig_atomic_t             m_arrSignals[CWX_APP_MAX_SIGNAL_ID + 1];///<signal������
    static volatile sig_atomic_t    m_bSignal; ///<�Ƿ����ź�
    bool                            m_bEnableSignal; ///<�Ƿ�֧��signal
    CwxAppHandler4Base*             m_sHandler[CWX_APP_MAX_SIGNAL_ID + 1];///<signal handler������
    CwxMinHeap<CwxAppHandler4Base>  m_timeHeap; ///<ʱ���
    SignalHanlder*                  m_sigHandler; ///<��ȡsignal�¼���handle
    bool                            m_bStop; ///<�Ƿ�ֹͣ
    CwxTimeValue                    m_current; ///<��ǰ��ʱ��
};


CWINUX_END_NAMESPACE
#include "CwxAppEpoll.inl"
#include "CwxPost.h"

#endif
