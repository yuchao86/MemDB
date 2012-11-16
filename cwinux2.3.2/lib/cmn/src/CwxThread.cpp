#include "CwxThread.h"
#include "CwxLogger.h"

CWINUX_BEGIN_NAMESPACE

///���캯��
CwxThread::CwxThread(CWX_UINT16 unGroupId,///<�̵߳�group id
             CWX_UINT16 unThreadId,///<�߳����߳�group�е����
             CwxThreadPoolMgr* mgr, ///<�̵߳Ĺ������
             CwxCommander* commander,///<������Ϣ���ѵ�ȱʡcommander����ָ��func���Բ�ָ��
             CWX_TSS_THR_FUNC func, ///<�û����߳�main����
             void*            arg, ///<func��void*����
             CwxMsgQueue* queue///<�̳߳صĶ��У�����ָ������ϵͳĬ�ϴ���
             )
{
    m_pTssEnv = NULL;
    m_unThreadId = unThreadId;
    m_unGroupId = unGroupId;
    if (!queue)
    {
        m_msgQueue = new CwxMsgQueue(1024*1024*200, 1024*1024*200);
        m_bOwnQueue = true;
    }
    else
    {
        m_msgQueue = queue;
        m_bOwnQueue = false;
    }
    m_commander = commander;
    m_func = func;
    m_arg = arg;
    m_mgr = mgr;
}

CwxThread::~CwxThread()
{
    if (m_bOwnQueue && m_msgQueue) delete m_msgQueue;
    m_msgQueue = NULL;
}



int CwxThread::start(CwxTss* pThrEnv, size_t stack_size)
{
    if (m_mgr->getTss(m_unGroupId, m_unThreadId))
    {
        CWX_ERROR(("Thread[group:%u, id=%u] exists, exit.", m_unGroupId, m_unThreadId));
        return -1;
    }
    m_pTssEnv = pThrEnv;
    if (-1 ==  CwxThread::spawn(
        threadFunc,
        this,
        THREAD_NEW_LWP | THREAD_JOINABLE | THREAD_INHERIT_SCHED,
        &m_tid,
        CWX_DEFAULT_THREAD_PRIORITY,
        NULL,
        stack_size        
        ))
    {
        CWX_ERROR(("Failure to spawn thread[group:%u, id=%u], errno=%d", m_unGroupId, m_unThreadId, errno));
        return -1;
    }
    return 0;
}

void CwxThread::stop()
{
    if (m_msgQueue->isActivate())
    {
        m_msgQueue->deactivate();
    }
    join(m_tid, NULL);
    if (m_bOwnQueue)
        m_msgQueue->flush();
}



void CwxThread::threadMain()
{
    time_t ttTime = time(NULL);

    {//ע��tss
        if (!m_pTssEnv)
        {
            m_pTssEnv = new CwxTss;
        }
        m_pTssEnv->getThreadInfo().setThreadGroup(m_unGroupId);
        m_pTssEnv->getThreadInfo().setThreadNo(m_unThreadId);
        if (!m_mgr->addTss(m_pTssEnv))
        {
            CWX_ERROR(("Failure to add thread[group:%u, id=%u] to thread mgr, it exists, exit.", m_unGroupId, m_unThreadId));
            return;
        }
    }
    m_pTssEnv->getThreadInfo().setStopped(false);
    CwxTss::regTss(m_pTssEnv);
    m_pTssEnv->getThreadInfo().setThreadId(self());
    m_pTssEnv->getThreadInfo().setStartTime(ttTime);
    m_pTssEnv->getThreadInfo().setUpdateTime(ttTime);
    m_pTssEnv->getThreadInfo().setRecvMsgNum(0);
    m_pTssEnv->getThreadInfo().setQueuedMsgNum(0);
    do
    {
        if (m_func)
        {
            m_func(m_pTssEnv, m_msgQueue, m_arg);
        }
        else if (m_commander)
        {
            int iRet;
            time_t ttTime = time(NULL);
            CwxMsgBlock* block;
            CWX_UINT32 uiEventType = 0;
            CWX_UINT32 uiSvrId = 0;
            m_pTssEnv->getThreadInfo().setBlocked(true);
            while( (iRet = this->pop(block)) != -1)
            {//block until has query message
                m_pTssEnv->getThreadInfo().setBlocked(false);
                ttTime = time(NULL);
                m_pTssEnv->getThreadInfo().setUpdateTime(ttTime);
                m_pTssEnv->getThreadInfo().setQueuedMsgNum(iRet);
                m_pTssEnv->getThreadInfo().incRecvMsgNum();
                iRet = 0;
                uiEventType = block->event().getEvent();
                uiSvrId = block->event().getSvrId();
                if (!m_commander->dispatch(block, m_pTssEnv, iRet))
                {
                    CWX_DEBUG(("No handle to deal with event: event_type=%u, svr_id=%u", uiEventType, uiSvrId));
                }
                if (1 != iRet)
                {
                    if (0 == iRet)
                    {
                        CWX_DEBUG(("No care the event for  event_type=%u, svr_id=%u", uiEventType, uiSvrId));
                    }
                    else
                    {
                        CWX_DEBUG(("Failure to act the event for  event_type=%u, svr_id=%u", uiEventType, uiSvrId));
                    }
                }
                if (block) CwxMsgBlockAlloc::free(block);
                m_pTssEnv->getThreadInfo().setBlocked(true);
            }
        }
        else
        {
            CWX_ERROR(("Thread[group:%u, id:%u] has neither commander nor own func, exit.", m_unGroupId, m_unThreadId));
        }

    }while(0);
    m_pTssEnv->getThreadInfo().setStopped(true);
}


bool CwxThread::isStop() 
{
    return m_msgQueue->getState() == CwxMsgQueue::DEACTIVATED;
}

CwxTss* CwxThread::getTss()
{
    return m_pTssEnv;
}

///��ס�̡߳�����ֵ0���ɹ���-1��ʧ��
int CwxThread::lock()
{
    return m_msgQueue->lock().acquire();
}
///��������̡߳�����ֵ0���ɹ���-1��ʧ��
int CwxThread::unlock()
{
    return m_msgQueue->lock().release();
}



int CwxThread::spawn(CWX_OS_THR_FUNC func,
                   void *args,
                   long flags,
                   pthread_t *thr_id,
                   long priority,
                   void *stack,
                   size_t stacksize)
{
    if (!CWX_CHECK_ATTR(flags,THREAD_DETACHED) &&
        !CWX_CHECK_ATTR(flags,THREAD_JOINABLE))
    {
        CWX_SET_ATTR(flags, THREAD_JOINABLE);
    }

    pthread_t tmp_thr;
    if (!thr_id) thr_id = &tmp_thr;

    int result;
    pthread_attr_t attr;

    result = ::pthread_attr_init(&attr);
    if (result != 0)
    {
        errno = result;
        return -1;
    }
    if (stacksize != 0)
    {
        size_t size = stacksize;
#if defined (PTHREAD_STACK_MIN)
        if (size < static_cast <size_t> (PTHREAD_STACK_MIN))
            size = PTHREAD_STACK_MIN;
#endif /* PTHREAD_STACK_MIN */
        if (stack != 0)
            result =::pthread_attr_setstack (&attr, stack, size);
        else
            result =::pthread_attr_setstacksize (&attr, size);
        if (result != 0)
        {
            errno = result;
            ::pthread_attr_destroy (&attr);
            return -1;
        }
    }
    if (flags != 0)
    {
        if (CWX_CHECK_ATTR(flags,THREAD_DETACHED) ||
            CWX_CHECK_ATTR(flags,THREAD_JOINABLE))
        {
            int dstate = PTHREAD_CREATE_JOINABLE;
            if (CWX_CHECK_ATTR(flags,THREAD_DETACHED))
                dstate = PTHREAD_CREATE_DETACHED;
            result = ::pthread_attr_setdetachstate (&attr, dstate);
            if (0 != result){
                errno = result;
                ::pthread_attr_destroy (&attr);
                return -1;
            }
        }
        if (priority != CWX_DEFAULT_THREAD_PRIORITY)
        {
            CWX_SET_ATTR(flags, THREAD_EXPLICIT_SCHED);
            if (!CWX_CHECK_ATTR(flags, THREAD_SCHED_FIFO) &&
                !CWX_CHECK_ATTR(flags, THREAD_SCHED_RR) &&
                !CWX_CHECK_ATTR(flags, THREAD_SCHED_DEFAULT))
            {
                CWX_SET_ATTR(flags, THREAD_SCHED_DEFAULT);
            }
        }

        if (CWX_CHECK_ATTR(flags, THREAD_SCHED_FIFO) ||
            CWX_CHECK_ATTR(flags, THREAD_SCHED_RR) ||
            CWX_CHECK_ATTR(flags, THREAD_SCHED_DEFAULT))
        {
            int spolicy;
            CWX_SET_ATTR(flags, THREAD_EXPLICIT_SCHED);
            if (CWX_CHECK_ATTR(flags, THREAD_SCHED_DEFAULT))
                spolicy = SCHED_OTHER;
            else if (CWX_CHECK_ATTR(flags, THREAD_SCHED_FIFO))
                spolicy = SCHED_FIFO;
            else if (CWX_CHECK_ATTR(flags, THREAD_SCHED_IO))
            {
                errno = ENOSYS;
                return -1;
            }
            else
                spolicy = SCHED_RR;

            result = ::pthread_attr_setschedpolicy (&attr, spolicy);
            if (result != 0)
            {
                errno = result;
                ::pthread_attr_destroy (&attr);
                return -1;
            }
        }
        if ((CWX_CHECK_ATTR(flags, THREAD_SCHED_FIFO) ||
             CWX_CHECK_ATTR(flags, THREAD_SCHED_RR) ||
             CWX_CHECK_ATTR(flags, THREAD_SCHED_DEFAULT))
            &&(priority == CWX_DEFAULT_THREAD_PRIORITY))
        {
            if (CWX_CHECK_ATTR(flags, THREAD_SCHED_FIFO))
                priority = (sched_get_priority_min(1) + sched_get_priority_max(1))/2;
            else if (CWX_CHECK_ATTR(flags, THREAD_SCHED_RR))
                priority = (sched_get_priority_min(2) + sched_get_priority_max(2))/2;
            else
                priority = (sched_get_priority_min(0) + sched_get_priority_max(0))/2;
        }
        if (priority != CWX_DEFAULT_THREAD_PRIORITY)
        {
            struct sched_param sparam;
            memset ((void *) &sparam, 0, sizeof sparam);
            if (CWX_CHECK_ATTR(flags, THREAD_SCHED_FIFO))
                sparam.__sched_priority = CWX_MIN(sched_get_priority_max(1), CWX_MAX(sched_get_priority_min(1), priority));
            else if (CWX_CHECK_ATTR(flags, THREAD_SCHED_RR))
                sparam.__sched_priority = CWX_MIN(sched_get_priority_max(2), CWX_MAX(sched_get_priority_min(2), priority));
            else
                sparam.__sched_priority = CWX_MIN(sched_get_priority_max(0), CWX_MAX(sched_get_priority_min(0), priority));
            result = ::pthread_attr_setschedparam (&attr, &sparam);
            if (0 != result)
            {
                errno = result;
                ::pthread_attr_destroy (&attr);
                return -1;
            }
        }
        if (CWX_CHECK_ATTR(flags, THREAD_INHERIT_SCHED) ||
            CWX_CHECK_ATTR(flags, THREAD_EXPLICIT_SCHED))
        {
            int sched = PTHREAD_EXPLICIT_SCHED;
            if (CWX_CHECK_ATTR(flags, THREAD_INHERIT_SCHED))
                sched = PTHREAD_INHERIT_SCHED;
            result = ::pthread_attr_setinheritsched (&attr, sched);
            if (0 != result)
            {
                errno = result;
                ::pthread_attr_destroy (&attr);
                return -1;
            }
        }

        if (CWX_CHECK_ATTR(flags, THREAD_SCOPE_SYSTEM) ||
            CWX_CHECK_ATTR(flags, THREAD_SCOPE_PROCESS))
        {

            int scope = PTHREAD_SCOPE_SYSTEM;
            if (CWX_CHECK_ATTR(flags, THREAD_SCOPE_SYSTEM))
                scope = PTHREAD_SCOPE_SYSTEM;
            result = ::pthread_attr_setscope (&attr, scope);
            if (0 != result)
            {
                errno = result;
                ::pthread_attr_destroy (&attr);
                return -1;
            }

        }
        if (CWX_CHECK_ATTR(flags, THREAD_NEW_LWP))
        {
            int lwps = ::pthread_getconcurrency();
            if (lwps == -1)
            {
                if (errno == ENOTSUP)
                    // Suppress the ENOTSUP because it's harmless.
                    errno = 0;
                else
                {
                    ::pthread_attr_destroy (&attr);
                    return -1;
                }
            }
            else if (::pthread_setconcurrency(lwps + 1) == -1)
            {
                if (errno == ENOTSUP)
                {
                    // Unlikely: ::thr_getconcurrency () is supported
                    // but ::thr_setconcurrency () is not?
                }
                else
                {
                    ::pthread_attr_destroy (&attr);
                    return -1;
                }
            }
        }
    }
    result = ::pthread_create (thr_id, &attr, func, args);
    ::pthread_attr_destroy (&attr);
    if (0 != result){
        errno = result;
        return -1;
    }
    return 0;
}

void* CwxThread::threadFunc(void * thread)
{
    CwxThread* pThread = (CwxThread*)thread;
    pThread->threadMain();
    return thread;
}


CWINUX_END_NAMESPACE
