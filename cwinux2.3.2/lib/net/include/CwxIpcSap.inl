CWINUX_BEGIN_NAMESPACE
///enable��disable asynchronous������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setSigio(bool enable) const
{
    return setSigio(handle_, enable);
}
///enable��disable  non-blocking I/O������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setNonblock(bool enable) const
{
    return setNonblock(handle_, enable);
}
///enable��disable   close-on-exec������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setCloexec (bool enable) const
{
    return setCloexec(handle_, enable);
}
///enable��disable   sigurg������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setSigurg (bool enable) const
{
    return setSigurg(handle_, enable);
}
///�Ƿ�������sigio, -1��ʧ�ܣ�0��û�У�1������
inline int  CwxIpcSap::isSigio() const
{
    return isSigio(handle_);
}
///�Ƿ�������nonblock, -1��ʧ�ܣ�0��û�У�1������
inline int CwxIpcSap::isNonBlock() const
{
    return isNonBlock(handle_);
}
///�Ƿ�������Cloexec, -1��ʧ�ܣ�0��û�У�1������
inline int  CwxIpcSap::isCloexec() const
{
    return isCloexec(handle_);
}
///�Ƿ�������Sigurg, -1��ʧ�ܣ�0��û�У�1������
inline int  CwxIpcSap::isSigurg() const
{
    return isSigurg(handle_);
}

/// Get the underlying handle.
inline CWX_HANDLE CwxIpcSap::getHandle (void) const
{
    return handle_;
}
/// Set the underlying handle.
inline void CwxIpcSap::setHandle (CWX_HANDLE handle)
{
    handle_ = handle;
}

///ͨ��fcntl����״̬������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setFlags (CWX_HANDLE handle, int flags)
{
    int val = ::fcntl (handle, F_GETFL, 0);
    if (val == -1) return -1;
    // Turn on flags.
    CWX_SET_ATTR(val, flags);
    if (::fcntl (handle, F_SETFL, val) == -1)
        return -1;
    else
        return 0;
}

inline int CwxIpcSap::clrFlags (CWX_HANDLE handle, int flags)
{
    int val = ::fcntl (handle, F_GETFL, 0);

    if (val == -1)  return -1;

    // Turn flags off.
    CWX_CLR_ATTR (val, flags);

    if (::fcntl (handle, F_SETFL, val) == -1)
        return -1;
    else
        return 0;
}

inline int CwxIpcSap::isFlags(CWX_HANDLE handle, int flags)
{
    int val = ::fcntl (handle, F_GETFL, 0);
    if (val == -1)  return -1;
    return CWX_CHECK_ATTR(val, flags)?true:false;
}

///enable��disable asynchronous������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setSigio(CWX_HANDLE handle, bool enable)
{
    if (enable)
    {
        if ((-1==::fcntl (handle, F_SETOWN, getpid())) ||
            (-1 == setFlags(handle, O_ASYNC)))
            return -1;
    }
    else
    {
        if ((-1 == ::fcntl (handle, F_SETOWN, 0)) ||
            (-1 == clrFlags (handle,  O_ASYNC)))
            return -1;
    }
    return 0;
}

///enable��disable  non-blocking I/O������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setNonblock(CWX_HANDLE handle, bool enable)
{
    if (enable)
    {
        if (-1 == setFlags (handle, O_NONBLOCK)) return -1;
    }
    else
    {
        if (-1 == clrFlags (handle, O_NONBLOCK)) return -1;
    }
    return 0;
}
///enable��disable   close-on-exec������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setCloexec (CWX_HANDLE handle, bool enable)
{
    if (enable)
    {
        if (-1 == ::fcntl (handle, F_SETFD, 1)) return -1;
    }
    else
    {
        if (-1 == ::fcntl (handle, F_SETFD, 0)) return -1;
    }
    return 0;
}
///enable��disable   sigurg������ֵ��0�ɹ���-1��ʧ�ܡ�
inline int CwxIpcSap::setSigurg (CWX_HANDLE handle, bool enable)
{
    if (enable)
    {
        if (-1 == ::fcntl (handle, F_SETOWN, getpid())) return -1;
    }
    else
    {
        if (-1 == ::fcntl (handle, F_SETOWN, 0)) return -1;
    }
    return 0;
}


///�Ƿ�������sigio, -1��ʧ�ܣ�0��û�У�1������
inline int CwxIpcSap::isSigio(CWX_HANDLE handle) const
{
    int ret = ::fcntl (handle, F_GETOWN);
    if (-1 == ret) return -1;
    if (getpid()!=ret) return 0;
    return isFlags(handle, O_ASYNC);

}
///�Ƿ�������nonblock, -1��ʧ�ܣ�0��û�У�1������
inline int CwxIpcSap::isNonBlock(CWX_HANDLE handle) const
{
    return isFlags(handle, O_NONBLOCK);

}
///�Ƿ�������Cloexec, -1��ʧ�ܣ�0��û�У�1������
inline int CwxIpcSap::isCloexec(CWX_HANDLE handle) const
{
    int ret = ::fcntl(handle, F_GETFD);
    return 0==ret?0:((-1 == ret)?-1:1);
}
///�Ƿ�������Sigurg, -1��ʧ�ܣ�0��û�У�1������
inline int CwxIpcSap::isSigurg(CWX_HANDLE handle) const
{
    int pid = ::fcntl (handle, F_GETOWN);
    if (-1 == pid) return -1;
    return getpid()==pid?1:0;
}


CWINUX_END_NAMESPACE
