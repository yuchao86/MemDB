CWINUX_BEGIN_NAMESPACE

///����UNIX-DOMAIN��·���ļ�
inline void CwxAppHandler4UnixConn::setConnectPathFile(char const* szPathFile)
{
    m_strConnectPathFile = szPathFile;
}
///��ȡUNIX-DOMAIN��·���ļ�
inline string const& CwxAppHandler4UnixConn::getConnectPathFile() const
{
    return m_strConnectPathFile;
}


CWINUX_END_NAMESPACE

