
CWINUX_BEGIN_NAMESPACE



///�����������ӵ����ӵ�ַ
inline void CwxAppHandler4TcpConn::setConnectAddr(char const* szAddr)
{
    m_strConnectAddr = szAddr;
}

///��ȡ�������ӵ����ӵ�ַ
inline string const& CwxAppHandler4TcpConn::getConnectAddr() const
{
    return m_strConnectAddr;
}

///�����������ӵ����Ӷ˿�
inline void CwxAppHandler4TcpConn::setConnectPort(CWX_UINT16 unPort)
{
    m_unConnectPort = unPort;
}

///��ȡ�������ӵ����Ӷ˿�
inline CWX_UINT16 CwxAppHandler4TcpConn::getConnectPort() const 
{
    return m_unConnectPort;
}




CWINUX_END_NAMESPACE

