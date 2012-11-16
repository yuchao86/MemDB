#ifndef __CWX_APP_REACTOR_H__
#define __CWX_APP_REACTOR_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppReactor.h
@brief �ܹ���reactor����
@author cwinux@gmail.com
@version 0.1
@date 2009-11-25
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

CWINUX_BEGIN_NAMESPACE


/**
@class CwxAppReactor
@brief �ܹ���reactor����ʵ���¼���ѭ��
*/

class CWX_API CwxAppReactor
{
public:
    enum{
        REG_TYPE_UNKNOWN=0,
        REG_TYPE_IO = 1,
        REG_TYPE_SIG = 2
    };
    typedef int (*REACTOR_EVENT_HOOK)(void *);
    ///���캯��
    CwxAppReactor(bool bEnableSig=true);
    ///��������
    ~CwxAppReactor();
public:
    /**
    @brief ��reactor�������߳�Ϊreactor��owner��
    @param [in] noticeHandler notice handler
    @return -1��ʧ�ܣ�0�������˳�
    */
    int open(CwxAppHandler4Base* noticeHandler);
    /**
    @brief �ر�reactor������Ϊowner thread�����ͷ����е�handler
    @return -1��ʧ�ܣ�0�������˳�
    */
    int close();
    /**
    @brief �ܹ��¼���ѭ������API��ʵ����Ϣ�ķַ��������߳�Ϊreactor��owner��
    @param [in] hook hook����
    @param [in] arg hook�����Ĳ�����
    @param [in] bOnce �Ƿ�ֻ����һ�Ρ�
    @param [in] uiMiliTimeout ��ʱ�ĺ�������0��ʾһֱ�������¼�������
    @return -1��ʧ�ܣ�0�������˳�
    */
    int run(REACTOR_EVENT_HOOK hook=NULL,
        void* arg=NULL,
        bool  bOnce=false,
        CWX_UINT32 uiMiliTimeout=0);
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
    @param [in] uiConnId ����ID��������ID��ΪCWX_APP_INVALID_CONN_ID������������ID���й���
    @param [in] uiMillSecond ��ʱ��������0��ʾ�����г�ʱ��⡣
    @return -1��ʧ�ܣ�
            0���ɹ���
    */
    int registerHandler (CWX_HANDLE io_handle,
        CwxAppHandler4Base *event_handler,
        int mask,
        CWX_UINT32 uiConnId=CWX_APP_INVALID_CONN_ID,
        CWX_UINT32 uiMillSecond = 0);
    /**
    @brief ɾ��io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler �Ƴ���event-handler
    @param [in] bRemoveConnId �Ƿ�Ҳ��conn-id��map��ɾ��
    @return -1��ʧ�ܣ�
             0���ɹ���
    */
    int removeHandler (CwxAppHandler4Base *event_handler, bool bRemoveConnId=true);
    /**
    @brief suspend io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler suspend��event-handler
    @param [in] suspend_mask suspend���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int suspendHandler (CwxAppHandler4Base *event_handler,
        int suspend_mask);
    /**
    @brief resume io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler resume��event-handler
    @param [in] resume_mask resume���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int resumeHandler (CwxAppHandler4Base *event_handler,
        int resume_mask);

    /**
    @brief ɾ��io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] handle �Ƴ��� io handle
    @param [in] bRemoveConnId �Ƿ�Ҳ��conn-id��map��ɾ��
    @return NULL�������ڣ����򣺳ɹ���
    */
    CwxAppHandler4Base* removeHandler (CWX_HANDLE handle, bool bRemoveConnId=true);
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
    @brief ɾ��io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] uiConnId ����ID
    @param [in] bRemoveConnId �Ƿ�Ҳ��conn-id��map��ɾ��
    @return NULL��ʧ�ܣ����򣺳ɹ���
    */
    CwxAppHandler4Base* removeHandlerByConnId (CWX_UINT32 uiConnId, bool bRemoveConnId=true);
    /**
    @brief suspend io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] uiConnId ����ID
    @param [in] suspend_mask suspend���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int suspendHandlerByConnId (CWX_UINT32 uiConnId,
        int suspend_mask);
    /**
    @brief resume io�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] uiConnId ����ID
    @param [in] resume_mask resume���¼�,ΪREAD_MASK��WRITE_MASK���
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int resumeHandlerByConnId (CWX_UINT32 uiConnId,
        int resume_mask);
    ///��Conn mapɾ��ָ����Handler����ʱ�����ӱ���û��ע�ᡣ���̰߳�ȫ�������̶߳����Ե��á�
    CwxAppHandler4Base* removeFromConnMap(CWX_UINT32 uiConnId);

    /**
    @brief ע��signal�¼�����handle���źž���PERSIST���ԡ����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] signum �ź�
    @param [in] event_handler signal��event handler
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int registerSignal(int signum,
        CwxAppHandler4Base *event_handler);
    /**
    @brief ɾ��signal�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler signal��event handler
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int removeSignal(CwxAppHandler4Base *event_handler);
    /**
    @brief ɾ��signal�¼�����handle�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] sig signal
    @return NULL�������ڣ����򷵻�signal��handler
    */
    CwxAppHandler4Base* removeSignal(int sig);

    /**
    @brief ���ö�ʱ����handle��timeout������persist���ԡ����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] event_handler timer��event handler
    @param [in] interval ��ʱ�ļ��
    @return -1��ʧ�ܣ�
    0���ɹ���
    */
    int scheduleTimer (CwxAppHandler4Base *event_handler,
        CwxTimeValue const &interval);
    ///ȡ����ʱ����handle�����̰߳�ȫ�������̶߳����Ե��á�
    int cancelTimer (CwxAppHandler4Base *event_handler);
    /**
    @brief notice reactor�����̰߳�ȫ�������̶߳����Ե��á�
    @param [in] notice notice�Ķ��󣬿���Ϊ��
    @return -1��ʧ�ܣ�  0���ɹ���
    */
    int notice(CwxAppNotice* notice= NULL);
    /**
    @brief ��ȡnotice���¼������̰߳�ȫ�������̶߳����Ե��á�
    @param [in] head notice���������ͷ
    @return void
    */
    void noticed(CwxAppNotice*& head);
    ///fork��re-init����������ֵ��0���ɹ���-1��ʧ��
    int forkReinit();
public:
    ///��ȡ��һ�����õ�����Id�����̰߳�ȫ�������̶߳����Ե��á�
    CWX_UINT32 getNextConnId();
    /**
    @brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ���̰߳�ȫ�������̶߳����Ե��á�
    @return true��ע�᣻false��û��ע��
    */
    bool isRegIoHandle(CWX_HANDLE handle);
    ///����conn id��ȡ��Ӧ��handler��������owner thread
    CwxAppHandler4Base* getHandlerByConnId(CWX_UINT32 uiConnId);
    /**
    @brief ���ָ��sig��handle�Ƿ��Ѿ�ע�ᡣ
    @return true��ע�᣻false��û��ע��
    */
    bool isRegSigHandle(int sig);
    /**
    @brief ��ȡָ��sig��Ӧ��event handler��
    @return ����handle��Ӧ��event handler��NULL��ʾ������
    */
    CwxAppHandler4Base* getSigHandler(int sig);
    ///Return the ID of the "owner" thread.
    pthread_t getOwner() const;
    ///�Ƿ�stop
    bool isStop();
    ///��ȡ��ǰ��ʱ��
    CwxTimeValue const& getCurTime() const;
    ///io handle�Ƿ�����ָ����mask
    bool isMask(CWX_HANDLE handle, int mask);
private:
    ///ע��IO�¼�����handle
    int _registerHandler (CWX_HANDLE io_handle,
        CwxAppHandler4Base *event_handler,
        int mask,
        CWX_UINT32 uiConnId=CWX_APP_INVALID_CONN_ID,
        CWX_UINT32 uiMillSecond = 0);
    ///ɾ��io�¼�����handle
    int _removeHandler (CwxAppHandler4Base *event_handler, bool bRemoveConnId=true);
    ///ע��IO�¼�����handle
    int _suspendHandler (CwxAppHandler4Base *event_handler,
        int suspend_mask);
    ///ɾ��io�¼�����handle
    int _resumeHandler (CwxAppHandler4Base *event_handler,
        int resume_mask);
    ///ɾ��io�¼�����handle��
    CwxAppHandler4Base* _removeHandler (CWX_HANDLE handle, bool bRemoveConnId=true);
    ///suspend io�¼�����handle��
    int _suspendHandler (CWX_HANDLE handle,
        int suspend_mask);
    ///resume io�¼�����handle��
    int _resumeHandler (CWX_HANDLE handle,
        int resume_mask);
    ///ɾ��io�¼�����handle��
    CwxAppHandler4Base* _removeHandlerByConnId (CWX_UINT32 uiConnId, bool bRemoveConnId=true);
    ///suspend io�¼�����handle��
    int _suspendHandlerByConnId (CWX_UINT32 uiConnId,
        int suspend_mask);
    /// resume io�¼�����handle��
    int _resumeHandlerByConnId (CWX_UINT32 uiConnId,
        int resume_mask);
    ///ע��signal�¼�����handle
    int _registerSignal(int signum,
        CwxAppHandler4Base *event_handler
        );
    ///ɾ��signal�¼�����handle
    int _removeSignal(CwxAppHandler4Base *event_handler);
    /**
    @brief ɾ��signal�¼�����handle��
    @param [in] sig signal
    @return NULL�������ڣ����򷵻�signal��handler
    */
    CwxAppHandler4Base* _removeSignal(int sig);

    ///���ö�ʱ����handle
    int _scheduleTimer (CwxAppHandler4Base *event_handler,
        CwxTimeValue const &interval );
    ///ȡ����ʱ����handle
    int _cancelTimer (CwxAppHandler4Base *event_handler);
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
    bool _isRegIoHandle(CwxAppHandler4Base* handler);
    /**
    @brief ��ȡָ��handle��Ӧ��event handler��
    @return ����handle��Ӧ��event handler��NULL��ʾ������
    */
    CwxAppHandler4Base* _getIoHandler(CWX_HANDLE handle);
    /**
    @brief ���ָ��sig��handle�Ƿ��Ѿ�ע�ᡣ
    @return true��ע�᣻false��û��ע��
    */
    bool _isRegSigHandle(int sig);
    /**
    @brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ
    @return true��ע�᣻false��û��ע��
    */
    bool _isRegSigHandle(CwxAppHandler4Base* handler);
    /**
    @brief ��ȡָ��sig��Ӧ��event handler��
    @return ����handle��Ӧ��event handler��NULL��ʾ������
    */
    CwxAppHandler4Base* _getSigHandler(int sig);

    ///���handler��Ӧ��conn-id�Ƿ��ܼ���conn-map��
    bool enableRegConnMap(CWX_UINT32 uiConnId, CwxAppHandler4Base* handler);
    void addRegConnMap(CWX_UINT32 uiConnId, CwxAppHandler4Base* handler);
    CwxAppHandler4Base* removeRegConnMap(CWX_UINT32 uiConnId);
    static void callback(CwxAppHandler4Base* handler, int mask, bool bPersist, void *arg);

private:
    bool                    m_bEnableSig;///<�Ƿ�ע��signal
    CwxMutexLock            m_lock; ///<ȫ����
    CwxRwLock               m_rwLock; ///<��д��
    pthread_t               m_owner; ///<reactor��owner �߳�
    bool                    m_bStop; ///<reactor�Ƿ��Ѿ�ֹͣ
    CWX_UINT32              m_connId[CWX_APP_MAX_IO_NUM]; ///<handler id��conn-id��ӳ��
    CWX_UINT32             m_uiCurConnId; ///<�ϴη��������ID
    CwxMutexLock           m_connMapMutex;///<m_connMap����������m_lock�Ϳ��Ա�������Ϊ�����getNextConnId()������˫������
    hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*���Ӷ���*/> m_connMap; ///<����Conn id������Map
    CwxAppNoticePipe*       m_pNoticePipe;
    ///�������Դ
    CwxAppEpoll*            m_engine; ///<epoll��engine


};


CWINUX_END_NAMESPACE
#include "CwxAppReactor.inl"
#include "CwxPost.h"

#endif
