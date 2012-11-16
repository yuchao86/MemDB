#include "CwxAppChannel.h"

CWINUX_BEGIN_NAMESPACE

CwxAppChannel::CwxAppChannel()
{
    m_owner = CwxThread::self();
    m_bStop = true;
    m_noticeFd[0] = CWX_INVALID_HANDLE;
    m_noticeFd[1] = CWX_INVALID_HANDLE;
    ///����notice pipe����
    m_pNoticeHandler = NULL;
    //�¼�����
    m_engine= NULL;
    //
    m_pCurRedoSet = &m_redoHandlers_1;
}


CwxAppChannel::~CwxAppChannel()
{
    close();
}

///��reactor��return -1��ʧ�ܣ�0���ɹ�
int CwxAppChannel::open()
{
    if (!m_bStop)
    {
        CWX_ERROR(("Can't re-open the openning reactor"));
        return -1;
    }
    close();
    ///����reactor��owner
    m_owner = CwxThread::self();
    ///����notice pipe����
    m_pNoticeHandler = new NoticeHanlder();
    ///����engine
    if (m_engine)
    {
        delete m_engine;
        m_engine = NULL;
    }
    m_engine = new CwxAppEpoll(false);
    if (0 != m_engine->init()) return -1;
    //ע��notice handler
    if (0 != pipe(m_noticeFd))
    {
        CWX_ERROR(("Failure to invokde pipe to create signal fd, errno=%d", errno));
        return -1;
    }
    if (0 != CwxIpcSap::setCloexec(m_noticeFd[0], true))
    {
        CWX_ERROR(("Failure to set notice handle[0]'s m_noticeFd sign, errno=%d", errno));
        return -1;
    }
    if (0 != CwxIpcSap::setCloexec(m_noticeFd[1], true))
    {
        CWX_ERROR(("Failure to set notice handle[1]'s cloexec sign, errno=%d", errno));
        return -1;
    }
    if (0 != CwxIpcSap::setNonblock(m_noticeFd[0], true))
    {
        CWX_ERROR(("Failure to set notice handle[0]'s noblock sign, errno=%d", errno));
        return -1;
    }
    if (0 != CwxIpcSap::setNonblock(m_noticeFd[1], true))
    {
        CWX_ERROR(("Failure to set notice handle[1]'s noblock sign, errno=%d", errno));
        return -1;
    }
    m_pNoticeHandler->setHandle(m_noticeFd[0]);
    //ע���ź�fd�Ķ�
    if (0 != m_engine->registerHandler(m_noticeFd[0], m_pNoticeHandler, CwxAppHandler4Base::PREAD_MASK))
    {
        CWX_ERROR(("Failure to register notice handle to engine"));
        return -1;
    }
    
    CWX_DEBUG(("Success to open CwxAppChannel"));
    m_bStop = false;
    return 0;
}

///�ر�reactor��return -1��ʧ�ܣ�0���ɹ�
int CwxAppChannel::close()
{
    if (!m_bStop)
    {
        CWX_ERROR(("CwxAppChannel::close must be invoked after stop"));
        return -1;
    }
    m_redoHandlers_1.clear();
    m_redoHandlers_2.clear();
    m_pCurRedoSet= &m_redoHandlers_1;
    if (m_engine) delete m_engine;
    m_engine = NULL;
    if (m_noticeFd[0] != CWX_INVALID_HANDLE)::close(m_noticeFd[0]);
    m_noticeFd[0] = CWX_INVALID_HANDLE;
    if (m_noticeFd[1] != CWX_INVALID_HANDLE)::close(m_noticeFd[1]);
    m_noticeFd[1] = CWX_INVALID_HANDLE;
    return 0;
}

/**
@brief �ܹ��¼���ѭ������API��ʵ����Ϣ�ķַ���
@return -1��ʧ�ܣ�0�������˳�
*/
int CwxAppChannel::dispatch(CWX_UINT32 uiMiliTimeout)
{
    int ret = 0;
    if (!m_engine)
    {
        CWX_ERROR(("CwxAppChannel::open() must be invoke before CwxAppReactor::run()"));
        return -1;
    }
    if (m_bStop)
    {
        CWX_ERROR(("Channel is stopped."));
        return -1;
    }
    ///���channel��owner
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        CWX_ERROR(("CwxAppChannel::dispatch() must be invoked by CwxAppChannel::open()'s thread"));
        return -1;
    }
    {
        {
            ///����ִ��event-loop
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = m_engine->poll(CwxAppChannel::callback, this, uiMiliTimeout);
            if (m_pCurRedoSet->size())
            {
                set<CwxAppHandler4Channel*>* pCurRedoSet = m_pCurRedoSet;
                if (m_pCurRedoSet == &m_redoHandlers_1)
                {
                    m_pCurRedoSet = &m_redoHandlers_2;
                }
                else
                {
                    m_pCurRedoSet = &m_redoHandlers_1;
                }
                m_pCurRedoSet->clear();

                CwxAppHandler4Channel* redoHander = NULL;
                set<CwxAppHandler4Channel*>::iterator iter = pCurRedoSet->begin();
                while(iter != pCurRedoSet->end())
                {
                    redoHander = *iter;
                    redoHander->m_bRedo = false;
                    if (-1 == redoHander->onRedo())
                    {
                        redoHander->close();
                    }
                    iter++;
                }
                pCurRedoSet->clear();
            }

        }
        if (m_bStop)
        {
            CWX_DEBUG(("Stop running for stop"));
            return 0;
        }
        if (0 != ret)
        {
            if ((-1 == ret) && (EINTR != errno))
            {
                CWX_ERROR(("Failure to running epoll with -1, errno=%d", errno));
                return -1;
            }
        }
        //wait other thread
        m_rwLock.acquire_write();
        m_rwLock.release();
    };
    return 0;
}

/**
@brief ֹͣ�ܹ��¼���ѭ������
@return -1��ʧ�ܣ�0�������˳�
*/
int CwxAppChannel::stop()
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            return _stop();
        }
        m_rwLock.release();
    }
    return _stop();
}

void CwxAppChannel::callback(CwxAppHandler4Base* handler, int mask, bool , void *)
{
    int ret = handler->handle_event(mask, handler->getHandle());
    if (-1 == ret)
    {
        handler->close(handler->getHandle());
    }
}

CWINUX_END_NAMESPACE
