
CWINUX_BEGIN_NAMESPACE

inline CwxAppHandler4TcpConn* CwxAppHandlerCache::fetchTcpHandle()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    if (!m_uiFreeTcpHandleNum) return NULL;
    CwxAppHandler4TcpConn* pHandle = m_freeTcpHandles;
    m_freeTcpHandles = m_freeTcpHandles->m_next;
    m_uiFreeTcpHandleNum--;
    return pHandle;
}
inline void CwxAppHandlerCache::cacheTcpHandle(CwxAppHandler4TcpConn* pHandle)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    pHandle->setHandle(CWX_INVALID_HANDLE);
    if (m_uiFreeTcpHandleNum>MAX_FREE_HANDLE_NUM){
        delete pHandle;
    }else{
        pHandle->m_next = m_freeTcpHandles;
        m_freeTcpHandles = pHandle;
        m_uiFreeTcpHandleNum++;
    }
}

inline CwxAppHandler4UnixConn* CwxAppHandlerCache::fetchUnixHandle()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    if (!m_uiFreeUnixHandleNum) return NULL;
    CwxAppHandler4UnixConn* pHandle = m_freeUnixHandles;
    m_freeUnixHandles = m_freeUnixHandles->m_next;
    m_uiFreeUnixHandleNum--;
    return pHandle;
}
inline void CwxAppHandlerCache::cacheUnixHandle(CwxAppHandler4UnixConn* pHandle)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    pHandle->setHandle(CWX_INVALID_HANDLE);
    if (m_uiFreeUnixHandleNum>MAX_FREE_HANDLE_NUM){
        delete pHandle;
    }else{
        pHandle->m_next = m_freeUnixHandles;
        m_freeUnixHandles = pHandle;
        m_uiFreeUnixHandleNum++;
    }
}

///��ȡһ��IO MSG���͵����Ӷ��󣬷���NULL��ʾû��CACHE�����Ӷ���
inline CwxAppHandler4IoMsg* CwxAppHandlerCache::fetchIoMsgHandle()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    if (!m_uiFreeIoMsgHandleNum) return NULL;
    CwxAppHandler4IoMsg* pHandle = m_freeIoMsgHandles;
    m_freeIoMsgHandles = m_freeIoMsgHandles->m_next;
    m_uiFreeIoMsgHandleNum--;
    return pHandle;
}
///Cacheһ���رյ�Io msg���͵����Ӷ���
inline void CwxAppHandlerCache::cacheIoMsgHandle(CwxAppHandler4IoMsg* pHandle)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    pHandle->setHandle(CWX_INVALID_HANDLE);
    if (m_uiFreeIoMsgHandleNum>MAX_FREE_HANDLE_NUM){
        delete pHandle;
    }else{
        pHandle->m_next = m_freeIoMsgHandles;
        m_freeIoMsgHandles = pHandle;
        m_uiFreeIoMsgHandleNum++;
    }
}



///��ȡһ��io event handler���͵����Ӷ��󣬷���NULL��ʾû��CACHE�����Ӷ���
inline CwxAppHandler4IoEvent* CwxAppHandlerCache::fetchIoEventHandle()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    if (!m_uiFreeIoEventHandleNum) return NULL;
    CwxAppHandler4IoEvent* pHandle = m_freeIoEventHandles;
    m_freeIoEventHandles = m_freeIoEventHandles->m_next;
    m_uiFreeIoEventHandleNum--;
    return pHandle;

}
///Cacheһ���رյ�io event handler���͵����Ӷ���
inline void CwxAppHandlerCache::cacheIoEventHandle(CwxAppHandler4IoEvent* pHandle)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    pHandle->setHandle(CWX_INVALID_HANDLE);
    if (m_uiFreeIoEventHandleNum>MAX_FREE_HANDLE_NUM){
        delete pHandle;
    }else{
        pHandle->m_next = m_freeIoEventHandles;
        m_freeIoEventHandles = pHandle;
        m_uiFreeIoEventHandleNum++;
    }
}

CWINUX_END_NAMESPACE

