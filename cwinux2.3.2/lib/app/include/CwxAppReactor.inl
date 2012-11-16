
CWINUX_BEGIN_NAMESPACE

///ע��IO�¼�����handle
inline int CwxAppReactor::registerHandler (CWX_HANDLE io_handle,
                                    CwxAppHandler4Base *event_handler,
                                    int mask,
                                    CWX_UINT32 uiConnId,
                                    CWX_UINT32 uiMillSecond)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret =  _registerHandler(io_handle, event_handler, mask, uiConnId, uiMillSecond);
        }
        m_rwLock.release();
        return ret;
    }
    return _registerHandler(io_handle, event_handler, mask, uiConnId, uiMillSecond);
}
///ɾ��io�¼�����handle
inline int CwxAppReactor::removeHandler (CwxAppHandler4Base *event_handler, bool bRemoveConnId)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _removeHandler(event_handler, bRemoveConnId);
        }
        m_rwLock.release();
        return ret;
    }
    return _removeHandler(event_handler, bRemoveConnId);
}

///ע��IO�¼�����handle
inline int CwxAppReactor::suspendHandler (CwxAppHandler4Base *event_handler, int suspend_mask)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _suspendHandler(event_handler, suspend_mask);
        }
        m_rwLock.release();
        return ret;
    }
    return _suspendHandler(event_handler, suspend_mask);
}
///ɾ��io�¼�����handle
inline int CwxAppReactor::resumeHandler (CwxAppHandler4Base *event_handler, int resume_mask)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _resumeHandler(event_handler, resume_mask);
        }
        m_rwLock.release();
        return ret;
    }
    return _resumeHandler(event_handler, resume_mask);
}


inline CwxAppHandler4Base* CwxAppReactor::removeHandler (CWX_HANDLE handle, bool bRemoveConnId)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        CwxAppHandler4Base* ret = NULL;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _removeHandler(handle, bRemoveConnId);
        }
        m_rwLock.release();
        return ret;
    }
    return _removeHandler(handle, bRemoveConnId);
}

inline int CwxAppReactor::suspendHandler (CWX_HANDLE handle,
                    int suspend_mask)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret =  _suspendHandler(handle, suspend_mask);
        }
        m_rwLock.release();
        return ret;
    }
    return _suspendHandler(handle, suspend_mask);
}

inline int CwxAppReactor::resumeHandler (CWX_HANDLE handle,
                   int resume_mask)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _resumeHandler(handle, resume_mask);
        }
        m_rwLock.release();
        return ret;
    }
    return _resumeHandler(handle, resume_mask);

}

inline CwxAppHandler4Base* CwxAppReactor::removeHandlerByConnId (CWX_UINT32 uiConnId, bool bRemoveConnId)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        CwxAppHandler4Base* ret = NULL;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _removeHandlerByConnId(uiConnId, bRemoveConnId);
        }
        m_rwLock.release();
        return ret;
    }
    return _removeHandlerByConnId(uiConnId, bRemoveConnId);
}

inline int CwxAppReactor::suspendHandlerByConnId (CWX_UINT32 uiConnId,
                            int suspend_mask)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _suspendHandlerByConnId(uiConnId, suspend_mask);
        }
        m_rwLock.release();
        return ret;
    }
    return _suspendHandlerByConnId(uiConnId, suspend_mask);
}

inline int CwxAppReactor::resumeHandlerByConnId (CWX_UINT32 uiConnId,
                           int resume_mask)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret =  _resumeHandlerByConnId(uiConnId, resume_mask);
        }
        m_rwLock.release();
        return ret;

    }
    return _resumeHandlerByConnId(uiConnId, resume_mask);
}

///��Conn mapɾ��ָ����Handler����ʱ�����ӱ���û��ע�ᡣ
inline CwxAppHandler4Base* CwxAppReactor::removeFromConnMap(CWX_UINT32 uiConnId)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_connMapMutex);
    hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*����*/>::iterator iter = m_connMap.find(uiConnId);
    if (iter == m_connMap.end())
    {
        CWX_DEBUG(("ConnId[%u]'s handler doesn't exist in conn-map", uiConnId));
        return NULL ;
    }
    CwxAppHandler4Base* handler = iter->second;
    if (handler->getHandle() != CWX_INVALID_HANDLE)
    {
        if (m_engine->m_eHandler[handler->getHandle()].m_handler)
        {
            CWX_ERROR(("ConnId[%u]'s handler[%d] is in register state, can't remove from conn-map",
                uiConnId,
                (int)handler->getHandle()));
            return NULL;
        }
    }
    m_connMap.erase(iter);
    return handler;
}

///ע��signal�¼�����handle
inline int CwxAppReactor::registerSignal(int signum,
                                  CwxAppHandler4Base *event_handler
                                  )
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret =  _registerSignal(signum, event_handler);
        }
        m_rwLock.release();
        return ret;
    }
    return _registerSignal(signum, event_handler);
}

///ɾ��signal�¼�����handle
inline int CwxAppReactor::removeSignal(CwxAppHandler4Base *event_handler)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _removeSignal(event_handler);
        }
        m_rwLock.release();
        return ret;
    }
    return _removeSignal(event_handler);
}

inline CwxAppHandler4Base* CwxAppReactor::removeSignal(int sig)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        CwxAppHandler4Base* handler = NULL;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            handler = _removeSignal(sig);
        }
        m_rwLock.release();
        return handler;
    }
    return _removeSignal(sig);
}

///���ö�ʱ����handle
inline int CwxAppReactor::scheduleTimer (CwxAppHandler4Base *event_handler,
                                   CwxTimeValue const&interval)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _scheduleTimer(event_handler, interval);
        }
        m_rwLock.release();
        return ret;
    }
    return _scheduleTimer(event_handler, interval);
}

///ȡ����ʱ����handle
inline int CwxAppReactor::cancelTimer (CwxAppHandler4Base *event_handler)
{
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        int ret = 0;
        m_rwLock.acquire_read();
        this->notice();
        {
            CwxMutexGuard<CwxMutexLock> lock(&m_lock);
            ret = _cancelTimer(event_handler);
        }
        m_rwLock.release();
        return ret;
    }
    return _cancelTimer(event_handler);
}

inline int CwxAppReactor::notice(CwxAppNotice* notice)
{
    int ret = 0;
    if (notice)
    {
        ret = m_pNoticePipe->notice(notice);
    }
    ret =  m_pNoticePipe->noticeDummy();
    if (0 != ret)
    {
        CWX_ERROR(("Failure to notice the event pipe, stop framework"));
        m_bStop = true;
    }
    return ret;
}

inline void CwxAppReactor::noticed(CwxAppNotice*& head)
{
    return m_pNoticePipe->noticed(head);
}


inline CWX_UINT32 CwxAppReactor::getNextConnId()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_connMapMutex);
    CWX_UINT32 uiConnId = m_uiCurConnId + 1;
    while(1)
    {
        if (m_connMap.find(uiConnId) == m_connMap.end()) break;
        uiConnId++;
        if (uiConnId > CWX_APP_MAX_CONN_ID) uiConnId = CWX_APP_MIN_CONN_ID;
    }
    m_uiCurConnId = uiConnId;
    return uiConnId;
}

/**
@brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ
@return true��ע�᣻false��û��ע��
*/
inline bool CwxAppReactor::isRegIoHandle(CWX_HANDLE handle)
{
    return _isRegIoHandle(handle);
}


///����conn id��ȡ��Ӧ��handler
inline CwxAppHandler4Base* CwxAppReactor::getHandlerByConnId(CWX_UINT32 uiConnId)
{
    CWX_ASSERT(CwxThread::equal(m_owner, CwxThread::self()));
    if (!CwxThread::equal(m_owner, CwxThread::self()))
    {
        CWX_ERROR(("Only owner thread can invoke CwxAppReactor::getHandlerByConnId()"));
        return NULL;
    }
    {
        CwxMutexGuard<CwxMutexLock> lock(&m_connMapMutex);
        hash_map<CWX_UINT32, CwxAppHandler4Base*>::iterator iter = m_connMap.find(uiConnId);
        if (iter == m_connMap.end()) return NULL;
        return iter->second;
    }
}

/**
@brief ���ָ��sig��handle�Ƿ��Ѿ�ע�ᡣ
@return true��ע�᣻false��û��ע��
*/
inline bool CwxAppReactor::isRegSigHandle(int sig)
{
    return _isRegSigHandle(sig);
}
/**
@brief ��ȡָ��sig��Ӧ��event handler��
@return ����handle��Ӧ��event handler��NULL��ʾ������
*/
inline CwxAppHandler4Base* CwxAppReactor::getSigHandler(int sig)
{
    return _getSigHandler(sig);
}
///Return the ID of the "owner" thread.
inline pthread_t CwxAppReactor::getOwner() const
{
    return m_owner;
}
///�Ƿ�stop
inline bool CwxAppReactor::isStop()
{
    return m_bStop;
}

///��ȡ��ǰ��ʱ��
inline CwxTimeValue const& CwxAppReactor::getCurTime() const
{
    return m_engine->getCurTime();
}

///io handle�Ƿ�����ָ����mask
inline bool CwxAppReactor::isMask(CWX_HANDLE handle, int mask)
{
    return CWX_CHECK_ATTR(m_engine->m_eHandler[handle].m_mask, mask);
}


///ע��IO�¼�����handle
inline int CwxAppReactor::_registerHandler (CWX_HANDLE io_handle,
                                    CwxAppHandler4Base *event_handler,
                                    int mask,
                                    CWX_UINT32 uiConnId,
                                    CWX_UINT32 uiMillSecond)
{
    int ret = 0;
    if (_isRegIoHandle(io_handle))
    {
        CWX_ERROR(("Handle[%d] exists, conn_id[%d]", (int)io_handle, uiConnId));
        return -1;
    }
    if ((CWX_APP_INVALID_CONN_ID != uiConnId) 
        && !enableRegConnMap(uiConnId, event_handler))
    {
        CWX_ERROR(("Conn handler with conn_id[%u] exists.", uiConnId));
        return -1;
    }

    event_handler->setRegType(REG_TYPE_IO);
    event_handler->setHandle(io_handle);
    mask &=CwxAppHandler4Base::IO_MASK; ///ֻ֧��READ��WRITE��PERSIST��TIMEOUT��������
    ret = m_engine->registerHandler(io_handle,
        event_handler,
        mask,
        uiMillSecond);
    if (0 == ret)
    {
        if (uiConnId != CWX_APP_INVALID_CONN_ID)
        {
            addRegConnMap(uiConnId, event_handler);
            m_connId[io_handle] = uiConnId;
        }
    }
    else
    {
        CWX_ERROR(("Failure to add event handler to event-base, handle[%d], conn_id[%u], errno=%d",
            (int)io_handle, 
            uiConnId,
            errno));
    }
    return ret==0?0:-1;
}

///ɾ��io�¼�����handle
inline int CwxAppReactor::_removeHandler (CwxAppHandler4Base *event_handler, bool bRemoveConnId)
{
    return _removeHandler(event_handler->getHandle(), bRemoveConnId)?0:-1;
}

///ע��IO�¼�����handle
inline int CwxAppReactor::_suspendHandler (CwxAppHandler4Base *event_handler, int suspend_mask)
{
    int ret=0;
    if (!_isRegIoHandle(event_handler))
    {
        CWX_ERROR(("event handle[%d] doesn't exist", (int)event_handler->getHandle()));
        return -1;
    }
    if (event_handler->getRegType() != REG_TYPE_IO)
    {
        CWX_ERROR(("event handle[%d] isn't io handle, it's [%d]",
            (int)event_handler->getHandle(),
            event_handler->getType()));
        return -1;
    }
    ret = m_engine->suspendHandler(event_handler->getHandle(), suspend_mask);
    if (0 != ret)
    {
        CWX_ERROR(("Failure to suspend handle[%d], errno=%d",
            (int)event_handler->getHandle(),
            errno));
        return -1;
    }
    return 0;
}

///ɾ��io�¼�����handle
inline int CwxAppReactor::_resumeHandler (CwxAppHandler4Base *event_handler, int resume_mask)
{
    int ret = 0;
    if (!_isRegIoHandle(event_handler))
    {
        CWX_ERROR(("event handle[%d] doesn't exist", (int)event_handler->getHandle()));
        return -1;
    }
    if (event_handler->getRegType() != REG_TYPE_IO)
    {
        CWX_ERROR(("event handle[%d] isn't io handle, it's [%d]",
            (int)event_handler->getHandle(),
            event_handler->getType()));
        return -1;
    }
    
    ret = m_engine->resumeHandler(event_handler->getHandle(), resume_mask);
    if (0 != ret)
    {
        CWX_ERROR(("Failure to resume handle[%d] from event base, errno=%d",
            (int)event_handler->getHandle(),
            errno));
        return -1;
    }
    return 0;
}


///ɾ��io�¼�����handle��
inline CwxAppHandler4Base* CwxAppReactor::_removeHandler (CWX_HANDLE handle, bool bRemoveConnId)
{
    if (handle >= CWX_APP_MAX_IO_NUM)
    {
        CWX_ERROR(("Handle[%d] exceed the max handle-no[%d]",
            (int)handle,
            CWX_APP_MAX_IO_NUM));
        return NULL;
    }
    CwxAppHandler4Base* handler= m_engine->removeHandler(handle);
    if (!handler)
    {
//        CWX_DEBUG(("Handle[%d] doesn't exist", (int)handle));
        return NULL;
    }
    if (bRemoveConnId && 
        (m_connId[handle] != CWX_APP_INVALID_CONN_ID))
    {
        removeRegConnMap(m_connId[handle]);
    }
    m_connId[handle] = CWX_APP_INVALID_CONN_ID;

    return handler;
}

///suspend io�¼�����handle��
inline int CwxAppReactor::_suspendHandler (CWX_HANDLE handle,
                     int suspend_mask)
{
    if (handle >= CWX_APP_MAX_IO_NUM)
    {
        CWX_ERROR(("Handle[%d] exceed the max handle-no[%d]",
            (int)handle,
            CWX_APP_MAX_IO_NUM));
        return -1;
    }
    return m_engine->suspendHandler(handle, suspend_mask);
}

///resume io�¼�����handle��
inline int CwxAppReactor::_resumeHandler (CWX_HANDLE handle,
                    int resume_mask)
{
    if (handle >= CWX_APP_MAX_IO_NUM)
    {
        CWX_ERROR(("Handle[%d] exceed the max handle-no[%d]",
            (int)handle,
            CWX_APP_MAX_IO_NUM));
        return -1;
    }
    return m_engine->resumeHandler(handle, resume_mask);
}

///ɾ��io�¼�����handle��
inline CwxAppHandler4Base* CwxAppReactor::_removeHandlerByConnId (CWX_UINT32 uiConnId,
                                                                  bool bRemoveConnId)
{
    hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
    if (iter == m_connMap.end())
    {
        CWX_DEBUG(("ConnId[%u] doesn't exist", uiConnId));
        return NULL;
    }
    CwxAppHandler4Base* handler = iter->second;
    _removeHandler(handler, bRemoveConnId);
    return handler;
}

///suspend io�¼�����handle��
inline int CwxAppReactor::_suspendHandlerByConnId (CWX_UINT32 uiConnId,
                             int suspend_mask)
{
    hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
    if (iter == m_connMap.end())
    {
        CWX_DEBUG(("ConnId[%u] doesn't exist", uiConnId));
        return -1;
    }
    CwxAppHandler4Base* handler = iter->second;
    return _suspendHandler(handler, suspend_mask);
}

/// resume io�¼�����handle��
inline int CwxAppReactor::_resumeHandlerByConnId (CWX_UINT32 uiConnId,
                            int resume_mask)
{
    hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
    if (iter == m_connMap.end())
    {
        CWX_DEBUG(("ConnId[%u] doesn't exist", uiConnId));
        return -1;
    }
    CwxAppHandler4Base* handler = iter->second;
    return _resumeHandler(handler, resume_mask);
}


///ע��signal�¼�����handle
inline int CwxAppReactor::_registerSignal(int signum,
                                  CwxAppHandler4Base *event_handler
                                  )
{
    if (_isRegSigHandle(signum))
    {
        CWX_ERROR(("Sig[%d] has been registered", signum));
        return -1;
    }
    event_handler->setRegType(REG_TYPE_SIG);
    event_handler->setHandle(signum);
    int ret = m_engine->registerSignal(signum, event_handler);
    if (0 != ret)
    {
        CWX_ERROR(("Failure to register Sig[%d] handler to event base, errno=%d",
            signum,
            errno));
        return -1;
    }
    return 0;
}

///ɾ��signal�¼�����handle
inline int CwxAppReactor::_removeSignal(CwxAppHandler4Base *event_handler)
{
    int signum = event_handler->getHandle();
    if (!_isRegSigHandle(signum))
    {
        CWX_DEBUG(("Sig[%d] doesn't registered", signum));
        return 0;
    }
    if (event_handler->getRegType() != REG_TYPE_SIG)
    {
        CWX_ERROR(("Handler with handle[%d] is not signal handler, it's type[%d]",
            (int)event_handler->getHandle(),
            event_handler->getType()));
        return -1;
    }
    if (!m_engine->removeSignal(event_handler->getHandle()))
    {
        CWX_ERROR(("Failure to remvoe signal[%d] handle from event-base, errno=%d", 
            event_handler->getHandle(),
            errno));
        return -1;
    }
    return 0;
}

inline CwxAppHandler4Base* CwxAppReactor::_removeSignal(int sig)
{
    if (sig <0 || sig>CWX_APP_MAX_SIGNAL_ID)
    {
        CWX_ERROR(("Sig[%d] exceed the max sig-no[%d]",
            sig,
            CWX_APP_MAX_SIGNAL_ID));
        return NULL;
    }
    return m_engine->removeSignal(sig);
}


///���ö�ʱ����handle
inline int CwxAppReactor::_scheduleTimer (CwxAppHandler4Base *event_handler,
                                   CwxTimeValue const&interval)
{
    int ret = m_engine->scheduleTimer(event_handler, interval);
    if (0 != ret)
    {
        CWX_ERROR(("Failure to register timer handler to event base, errno=%d",
            errno));
        return -1;
    }
    return 0;

}
///ȡ����ʱ����handle
inline int CwxAppReactor::_cancelTimer (CwxAppHandler4Base *event_handler)
{

    int ret = m_engine->cancelTimer(event_handler);
    if (-1 == ret)
    {
        CWX_ERROR(("Failure to remvoe timer handle from event-base, errno=%d", 
            errno));
        return -1;
    }
    return 0;
}

inline bool CwxAppReactor::_isRegIoHandle(CWX_HANDLE handle)
{
    if (handle >= CWX_APP_MAX_IO_NUM) return true;
    return m_engine->m_eHandler[handle].m_handler != NULL;
}

inline bool CwxAppReactor::_isRegIoHandle(CwxAppHandler4Base* handler)
{
    if (handler->getHandle() >= CWX_APP_MAX_IO_NUM) return true;
    return m_engine->m_eHandler[handler->getHandle()].m_handler == handler;
}


/**
@brief ֹͣ�ܹ��¼���ѭ������
@return -1��ʧ�ܣ�0�������˳�
*/
inline int CwxAppReactor::_stop()
{
    m_bStop = true;
    return 0;
}


/**
@brief ��ȡָ��handle��Ӧ��event handler��
@param bLock  true��api�ڲ�lock��false���ⲿ����
@return ����handle��Ӧ��event handler��NULL��ʾ������
*/
inline CwxAppHandler4Base* CwxAppReactor::_getIoHandler(CWX_HANDLE handle)
{
    if (handle >= CWX_APP_MAX_IO_NUM) return NULL;
    return m_engine->m_eHandler[handle].m_handler;
}
/**
@brief ���ָ��sig��handle�Ƿ��Ѿ�ע�ᡣ
@return true��ע�᣻false��û��ע��
*/
inline bool CwxAppReactor::_isRegSigHandle(int sig)
{
    if (sig > CWX_APP_MAX_SIGNAL_ID) return false;
    return m_engine->m_sHandler[sig] != NULL;
}
/**
@brief ���ָ��IO��handle�Ƿ��Ѿ�ע�ᡣ
@return true��ע�᣻false��û��ע��
*/
inline bool CwxAppReactor::_isRegSigHandle(CwxAppHandler4Base* handler)
{
    if (handler->getHandle() > CWX_APP_MAX_SIGNAL_ID) return false;
    return m_engine->m_sHandler[handler->getHandle()] == handler;
}
/**
@brief ��ȡָ��sig��Ӧ��event handler��
@return ����handle��Ӧ��event handler��NULL��ʾ������
*/
inline CwxAppHandler4Base* CwxAppReactor::_getSigHandler(int sig)
{
    if (sig > CWX_APP_MAX_SIGNAL_ID) return NULL;
    return m_engine->m_sHandler[sig];

}


inline bool CwxAppReactor::enableRegConnMap(CWX_UINT32 uiConnId, CwxAppHandler4Base* handler)
{
    CWX_ASSERT(uiConnId != CWX_APP_INVALID_CONN_ID);
    {
        CwxMutexGuard<CwxMutexLock> lock(&m_connMapMutex);
        hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*����*/>::iterator iter = m_connMap.find(uiConnId);
        return ((iter==m_connMap.end())||(iter->second == handler))?true:false;
    }
}
inline void CwxAppReactor::addRegConnMap(CWX_UINT32 uiConnId, CwxAppHandler4Base* handler)
{
    CWX_ASSERT(uiConnId != CWX_APP_INVALID_CONN_ID);
    {
        CwxMutexGuard<CwxMutexLock> lock(&m_connMapMutex);
        m_connMap[uiConnId] = handler;
    }
}

inline CwxAppHandler4Base* CwxAppReactor::removeRegConnMap(CWX_UINT32 uiConnId)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_connMapMutex);
    CwxAppHandler4Base* handler = NULL;
    hash_map<CWX_UINT32/*conn id*/, CwxAppHandler4Base*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
    if (iter != m_connMap.end())
    {
        handler = iter->second;
        m_connMap.erase(iter);
    }
    return handler;
}


CWINUX_END_NAMESPACE
