CWINUX_BEGIN_NAMESPACE

///��ȡapp
///����handle��reactor
inline void CwxAppHandler4Base::reactor (CwxAppReactor *reactor)
{
    m_reactor = reactor;
}
///��ȡhandle��reactor.
inline CwxAppReactor *CwxAppHandler4Base::reactor(void)
{
    return m_reactor;
}

///��ȡIo handle
inline CWX_HANDLE CwxAppHandler4Base::getHandle(void) const
{
    return m_handler;
}
///����IO handle
inline void CwxAppHandler4Base::setHandle (CWX_HANDLE handle)
{
    m_handler = handle;
}

///����handle type
inline void CwxAppHandler4Base::setType(int type)
{
    m_type = type;
}
///��ȡhandle type
inline int CwxAppHandler4Base::getType() const
{
    return m_type;
}

///��ȡע������
inline int CwxAppHandler4Base::getRegType() const
{
    return m_regType;
}
///����ע������
inline void CwxAppHandler4Base::setRegType(int type)
{
    m_regType = type;
}
///��ȡ��ʱʱ��
inline CWX_UINT64 CwxAppHandler4Base::getTimeout() const
{
    return m_ullTimeout;
}
///���ó�ʱʱ��
inline void CwxAppHandler4Base::setTimeout(CWX_UINT64 ullTimeout)
{
    m_ullTimeout = ullTimeout;
}
///��ȡheap�е�index
inline int CwxAppHandler4Base::index() const
{
    return m_index;
}
///����heap�е�index
inline void CwxAppHandler4Base::index(int index)
{
    m_index = index;
}


///��ʱ�ȽϺ���
inline bool CwxAppHandler4Base::operator>(CwxAppHandler4Base const& base) const
{
    return m_ullTimeout>base.m_ullTimeout;
}


CWINUX_END_NAMESPACE

