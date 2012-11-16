#include "CwxAppHandler4UnixConn.h"
#include "CwxAppFramework.h"

CWINUX_BEGIN_NAMESPACE

///���캯��
CwxAppHandler4UnixConn::CwxAppHandler4UnixConn(CwxAppFramework* pApp, CwxAppReactor *reactor)
:CwxAppHandler4Msg(pApp, reactor)
{
    m_next = NULL;
}
///��������
CwxAppHandler4UnixConn::~CwxAppHandler4UnixConn()
{
    if (getHandle() != CWX_INVALID_HANDLE)
    {
        ::close(getHandle());
        setHandle(CWX_INVALID_HANDLE);
    }
}


///handle close
int CwxAppHandler4UnixConn::close(CWX_HANDLE )
{
    CWX_UINT16 ucLocState = m_conn.getState();
    char const* szState;
    int iCloseReturn=0;

    if (getApp()->isStopped())
    {
		if (-1 != this->index()) reactor()->cancelTimer(this);
        delete this;
        return 0;
    }

    switch(ucLocState)
    {
    case CwxAppConnInfo::IDLE:
        szState = "IDLE"; ///һ��û����reactorע��
        break;
    case CwxAppConnInfo::CONNECTING:
        szState = "CONNECTING";///������������ʱ����reactorע��
        CWX_ERROR(("Failure to connect to unix-file:%s, errno=%d",
            m_strConnectPathFile.c_str(),
            getConnErrNo()));
        if (reactor()) reactor()->removeHandler(this);
        break;
    case CwxAppConnInfo::TIMEOUT:
        szState = "TIMEOUT"; ///����ע����timeout
        if (-1 != this->index()) reactor()->cancelTimer(this);
        break;
    case CwxAppConnInfo::ESTABLISHING:
        szState = "ESTABLISHING";///����ע������Ϣ�շ�
        if (CWX_INVALID_HANDLE != getHandle())
            if (reactor()) reactor()->removeHandler(this, false);
        break;
    case CwxAppConnInfo::ESTABLISHED:
        szState = "ESTABLISHED";///һ��ע������Ϣ�շ�
        if (CWX_INVALID_HANDLE != getHandle())
            if (reactor()) reactor()->removeHandler(this, false);
        break;
    case CwxAppConnInfo::FAILED:
        szState = "FAILED";///һ��û��ע����Ϣ�շ�
        break;
    default:
        szState = "Unknown";
        CWX_ASSERT(0);
        break;
    }
    if (CWX_INVALID_HANDLE != getHandle())
    {
        CWX_DEBUG(("Connection closed. State = %s, unix file=%s, Active-Close=%s", 
            szState,
            m_strConnectPathFile.c_str(),
            m_conn.isActiveClose()?"yes":"no"));
    }

    m_conn.setState(CwxAppConnInfo::FAILED);

    //reconnection or close
    if (CwxAppConnInfo::ESTABLISHED == ucLocState)
    {
        //re-dispatch all msg
        while(this->getNextMsg() == 1)
        {
            if (this->m_curSndingMsg && m_curSndingMsg->send_ctrl().isFailNotice())
            {
                this->getApp()->onFailSendMsg(m_curSndingMsg);
            }
            if (m_curSndingMsg) CwxMsgBlockAlloc::free(m_curSndingMsg);
            this->m_curSndingMsg = NULL;
        }
        iCloseReturn = this->getApp()->connClosed(*this);
        m_conn.setFailConnNum(1);
        //remove recieved data.
        if (this->m_recvMsgData) CwxMsgBlockAlloc::free(this->m_recvMsgData);
        this->m_recvMsgData = NULL;
    }
    else if (m_conn.isActiveConn())
    {
        m_conn.setFailConnNum(m_conn.getFailConnNum() + 1);
        iCloseReturn = this->getApp()->onFailConnect(*this);
    }

    if (getHandle () != CWX_INVALID_HANDLE)
    {
        ::close(getHandle());
        setHandle(CWX_INVALID_HANDLE);
    }

    if (m_conn.isNeedReconnect() && (iCloseReturn>=0))
    {
        CWX_UINT32 uiInternal = 0;
        if (m_conn.isReconn())
        {
            uiInternal = m_conn.getReconnDelay();
            m_conn.setReconn(false);
        }
        else
        {
            if (iCloseReturn > 0)
            {
                uiInternal = iCloseReturn;
            }
            else
            {
                uiInternal = 2 * m_conn.getMinRetryInternal() * getConnInfo().getFailConnNum();
                if (uiInternal > getConnInfo().getMaxRetryInternal()) uiInternal = getConnInfo().getMaxRetryInternal();
                uiInternal *= 1000;
            }
        }

        CWX_DEBUG(("Reconnect to address %s after %d mili-second.", m_strConnectPathFile.c_str(), uiInternal));
        if (uiInternal)
        {
            m_conn.setState(CwxAppConnInfo::TIMEOUT);
            if (this->reactor()->scheduleTimer(this, CwxTimeValue(uiInternal/1000, (uiInternal%1000) * 1000)) == -1)
            {
                CWX_ERROR(("Failure schedule_timer to noticeReconnect for conn id[%u]", m_conn.getConnId()));
                if (reactor()) reactor()->removeHandlerByConnId(m_conn.getConnId());
                delete this;
                return 0;
            }
        }
        else
        {
            if (-1 == getApp()->getUnixConnector()->connect(this,
                m_strConnectPathFile.c_str()))
            {
                CWX_ERROR(("Failure to reconnect unix-file=%s, errno=%d",
                    m_strConnectPathFile.c_str(),
                    errno));
                this->close();
            }
        }

    }
    else
    {
        reactor()->removeFromConnMap(m_conn.getConnId());
        getApp()->getHandlerCache()->cacheUnixHandle(this);
    }

    return 0;
}

///��ʱ
void CwxAppHandler4UnixConn::handle_timeout()
{
    if (getApp()->isStopped()) return ;
    if (-1 == getApp()->getUnixConnector()->connect(this,
        m_strConnectPathFile.c_str()))
    {
        this->close();
    }
}

/**
@brief ��ȡ���ӵĶԶ˵�ַ��ֻ��STREAM_TYPE_TCP��STREAM_TYPE_UNIX��Ч
@param [in,out] szBuf ���ص�ַ��buf,��ȡ�ɹ�����\0������
@param [in] unSize szBuf�Ĵ�С��
@return ����szBuf
*/
char* CwxAppHandler4UnixConn::getRemoteAddr(char* szBuf, CWX_UINT16 unSize)
{
    CwxCommon::copyStr(szBuf, unSize, m_strConnectPathFile.c_str() , m_strConnectPathFile.length());
    return szBuf;
}
/**
@brief ��ȡ���ӵĶԶ�port��ֻ��STREAM_TYPE_TCP��Ч
@return ���ӶԶ˵�port
*/
CWX_UINT16 CwxAppHandler4UnixConn::getRemotePort()
{
    return 0;
}
/**
@brief ��ȡ���ӱ��˵ĵ�ַ��ֻ��STREAM_TYPE_TCP��STREAM_TYPE_UNIX��Ч
@param [in,out] szBuf ���ص�ַ��buf,��ȡ�ɹ�����\0������
@param [in] unSize szBuf�Ĵ�С��
@return ����szBuf
*/
char* CwxAppHandler4UnixConn::getLocalAddr(char* szBuf, CWX_UINT16 unSize)
{
    CwxCommon::copyStr(szBuf, unSize, m_strConnectPathFile.c_str() , m_strConnectPathFile.length());
    return szBuf;
}
/**
@brief ��ȡ���ӵı���port��ֻ��STREAM_TYPE_TCP��Ч
@return ���ӶԶ˵�port
*/
CWX_UINT16 CwxAppHandler4UnixConn::getLocalPort()
{
    return 0;
}


CWINUX_END_NAMESPACE

