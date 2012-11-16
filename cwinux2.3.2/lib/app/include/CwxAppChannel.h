#ifndef __CWX_APP_CHANNEL_H__
#define __CWX_APP_CHANNEL_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppChannel.h
@brief �¼�channel����
@author cwinux@gmail.com
@version 1.0
@date 2011-04-15
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxTimeValue.h"
#include "CwxRwLock.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxAppHandler4Base.h"
#include "CwxAppNoticePipe.h"
#include "CwxThread.h"
#include "CwxLogger.h"
#include "CwxAppEpoll.h"
#include "CwxAppHandler4Channel.h"
#include "CwxStl.h"

CWINUX_BEGIN_NAMESPACE


/**
@class CwxAppChannel
@brief �ܹ���Channel����ʵ��IO�¼���ѭ��
*/

class CWX_API CwxAppChannel
{
public:
    typedef int (*CHANNEL_EVENT_HOOK)(void *);
    ///���캯��
    CwxAppChannel();
    ///��������
    ~CwxAppChannel();
public:
    /**
    @brief ��reactor�������߳�Ϊreactor��owner��
    @param [in] noticeHandler notice handler
    @return -1��ʧ�ܣ�0�������˳�
    */
    int open();
    /**
    @brief �ر�reactor������Ϊowner thread�����ͷ����е�handler
    @return -1��ʧ�ܣ�0�������˳�
    */
    int close();
    /**
    @brief �ܹ��¼���ѭ������API��ʵ����Ϣ�ķַ��������߳�Ϊchannel��owner��
    @param [in] uiMiliTimeout ��ʱ�ĺ�������0��ʾһֱ�������¼�������
    @return -1��ʧ�ܣ�0�������˳�
    */
    int dispatch(CWX_UINT32 uiMiliTimeout=0);
    /**
    @brief ֹͣ�ܹ��¼���ѭ���������̰߳�ȫ�������̶߳����Ե��á�
    @return -1��ʧ�ܣ�0�������˳�
    */
    int stop();
    /**
    @brief ע��IO�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] io_handle ����IO handle
    @param [in] event_handler io handle��Ӧ��event handler��
    @param [in] mask ע����¼����룬ΪREAD_MASK��WRITE_MASK��PERSIST_MASK���
    @param [in] uiMillSecond ��ʱ��������0��ʾ�����г�ʱ��⡣
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int registerHandler (CWX_HANDLE io_handle,
        CwxAppHandler4Channel *CwxAppHandler4Channel,
        int mask,
        CWX_UINT32 uiMillSecond = 0);
    /**
    @brief ɾ��io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler �Ƴ���event-handler
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int removeHandler (CwxAppHandler4Channel *event_handler);
    /**
    @brief suspend io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler suspend��event-handler
    @param [in] suspend_mask suspend���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int suspendHandler (CwxAppHandler4Channel *event_handler,
        int suspend_mask);
    /**
    @brief resume io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler resume��event-handler
    @param [in] resume_mask resume���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int resumeHandler (CwxAppHandler4Channel *event_handler,
        int resume_mask);

    /**
    @brief ɾ��io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] handle �Ƴ��� io handle
    @return NULL�������ڣ����򣺳ɹ���
    */
    CwxAppHandler4Channel* removeHandler (CWX_HANDLE handle);
    /**
    @brief suspend io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] handle suspend io handle
    @param [in] suspend_mask suspend���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�0���ɹ�
    */
    int suspendHandler (CWX_HANDLE handle,
        int suspend_mask);
    /**
    @brief resume io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] handle resume io handle
    @param [in] resume_mask resume���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int resumeHandler (CWX_HANDLE handle,
        int resume_mask);
    /**
    @brief ���ö�ʱ����handle��timeout������persist���ԡ����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler timer��event handler
    @param [in] interval ��ʱ�ļ��
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int scheduleTimer (CwxAppHandler4Channel *event_handler,
        CwxTimeValue const &interval);
    ///ȡ����ʱ����handle�����̰߳�ȫ�������̶߳����Ե��á�
    int cancelTimer (CwxAppHandler4Channel *event_handler);
    /**
    @brief ע��redo handler��redo��handler��dispatch�ᱻִ��һ�β��Ƴ������̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler redo��event handler
    @return true���ɹ���false�����ڣ�
    */
    bool regRedoHander(CwxAppHandler4Channel* event_handler);
    /**
    @brief ɾ��redo handler�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler redoe��event handler
    @return true���ɹ���false�������ڣ�
    */
    bool eraseRedoHander(CwxAppHandler4Channel* event_handler);
    /**
    @brief notice reactor�����̰߳�ȫ�������̶߳����Ե��á�
    @return -1��ʧ�ܣ�  0���ɹ���
    */
    int notice();
public:
    /**
    */
public:
    /**
    @brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ���̰߳�ȫ�������̶߳����Ե��á�
    @return true��ע�᣻false��û��ע��
    */
    bool isRegIoHandle(CWX_HANDLE handle);
    ///Return the ID of the "owner" thread.
    pthread_t getOwner() const;
    ///�Ƿ�stop
    bool isStop();
    ///��ȡ��ǰ��ʱ��
    CwxTimeValue const& getCurTime() const;
    ///��ȡ��ǰ��cacheʱ��
    ///io handle�Ƿ�����ָ����mask
    bool isMask(CWX_HANDLE handle, int mask);
private:
    ///ע��IO�¼�����handle
    int _registerHandler (CWX_HANDLE io_handle,
        CwxAppHandler4Channel *event_handler,
        int mask,
        CWX_UINT32 uiMillSecond = 0);
    ///ɾ��io�¼�����handle
    int _removeHandler (CwxAppHandler4Channel *event_handler);
    ///ע��IO�¼�����handle
    int _suspendHandler (CwxAppHandler4Channel *event_handler,
        int suspend_mask);
    ///ɾ��io�¼�����handle
    int _resumeHandler (CwxAppHandler4Channel *event_handler,
        int resume_mask);
    ///ɾ��io�¼�����handle��
    CwxAppHandler4Channel* _removeHandler (CWX_HANDLE handle);
    ///suspend io�¼�����handle��
    int _suspendHandler (CWX_HANDLE handle,
        int suspend_mask);
    ///resume io�¼�����handle��
    int _resumeHandler (CWX_HANDLE handle,
        int resume_mask);
    ///���ö�ʱ����handle
    int _scheduleTimer (CwxAppHandler4Channel *event_handler,
        CwxTimeValue const &interval );
    ///ȡ����ʱ����handle
    int _cancelTimer (CwxAppHandler4Channel *event_handler);
    ///���redo handler
    bool _regRedoHander(CwxAppHandler4Channel* event_handler);
    ///ɾ��redo handler��
    bool _eraseRedoHander(CwxAppHandler4Channel* event_handler);

    /**
    @brief ֹͣ�ܹ��¼���ѭ������
    @return -1��ʧ�ܣ�0�������˳�
    */
    int _stop();
    /**
    @brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ
    @return true��ע�᣻false��û��ע��
    */
    bool _isRegIoHandle(CWX_HANDLE handle);
    /**
    @brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ
    @return true��ע�᣻false��û��ע��
    */
    bool _isRegIoHandle(CwxAppHandler4Channel* handler);
    /**
    @brief ��ȡָ��handle��Ӧ��event handler��
    @return ����handle��Ӧ��event handler��NULL��ʾ������
    */
    CwxAppHandler4Channel* _getIoHandler(CWX_HANDLE handle);
    static void callback(CwxAppHandler4Base* handler, int mask, bool bPersist, void *arg);
private:
    class NoticeHanlder:public CwxAppHandler4Base
    {
    public:
        NoticeHanlder():CwxAppHandler4Base(NULL)
        {
        }
        ~NoticeHanlder(){}
    public:
        virtual int open (void *){ return 0;}
        virtual int handle_event(int , CWX_HANDLE handle)
        {
            char sigBuf[64];
            read(handle, sigBuf, 63);
            return 0;
        }
        virtual int close(CWX_HANDLE ){return 0;}
    };

private:
    CwxMutexLock            m_lock; ///<ȫ����
    CwxRwLock               m_rwLock; ///<��д��
    pthread_t               m_owner; ///<reactor��owner �߳�
    set<CwxAppHandler4Channel*>* m_pCurRedoSet;
    set<CwxAppHandler4Channel*>  m_redoHandlers_1; ///<��set�е�handler��ÿ��dispatch��ִ��һ��,���Ƴ���
    set<CwxAppHandler4Channel*>  m_redoHandlers_2; ///<��set�е�handler��ÿ��dispatch��ִ��һ��,���Ƴ���
    bool                    m_bStop; ///<reactor�Ƿ��Ѿ�ֹͣ
    int                     m_noticeFd[2]; ///<notice�Ķ�дhandle
    CwxAppEpoll*            m_engine; ///<epoll��engine
    NoticeHanlder*          m_pNoticeHandler; ///<notice��handler
};


CWINUX_END_NAMESPACE
#include "CwxAppChannel.inl"
#include "CwxPost.h"

#endif
