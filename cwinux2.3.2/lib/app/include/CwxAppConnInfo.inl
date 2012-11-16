
CWINUX_BEGIN_NAMESPACE

inline CWX_UINT32 CwxAppConnInfo::getSvrId() const 
{
    return m_uiSvrId;
}

inline void CwxAppConnInfo::setSvrId(CWX_UINT32 uiSvrId)
{
    m_uiSvrId = uiSvrId;
}

///��ȡ���ӵ�HOST-ID
inline CWX_UINT32 CwxAppConnInfo::getHostId() const 
{
    return m_uiHostId;
}

inline void CwxAppConnInfo::setHostId(CWX_UINT32 uiHostId)
{
    m_uiHostId = uiHostId;
}
///��ȡ���ӵ�����ID
inline CWX_UINT32 CwxAppConnInfo::getConnId() const 
{
    return m_uiConnId;
}
///�������ӵ�����ID
inline void CwxAppConnInfo::setConnId(CWX_UINT32 uiConnId)
{
    m_uiConnId = uiConnId;
}
///���ñ������ӵ�Listen ID
inline CWX_UINT32 CwxAppConnInfo::getListenId() const 
{
    return m_uiListenId;
}
///��ȡ���ӳ�ʱʱ��
inline CWX_UINT32 CwxAppConnInfo::getConnectTimeout() const
{
	return m_uiConnectTimeout;
}
///�������ӳ�ʱʱ��
inline void CwxAppConnInfo::setConnectTimeout(CWX_UINT32 uiTimeout)
{
	m_uiConnectTimeout = uiTimeout;
}

inline void CwxAppConnInfo::setListenId(CWX_UINT32 uiListenId)
{
    m_uiListenId = uiListenId;
}


///��ȡ���ӵ�״̬
inline CWX_UINT16 CwxAppConnInfo::getState() const
{
    return m_unState;
}
inline void CwxAppConnInfo::setState(CWX_UINT16 unState)
{
    m_unState = unState;
}

///��ȡ���ӵĴ���ʱ��
inline time_t CwxAppConnInfo::getCreateTime() const 
{
    return m_ttCreateTime;
}
inline void CwxAppConnInfo::setCreateTime(time_t ttTime)
{
    m_ttCreateTime = ttTime;
}

///��ȡ��������ʧ�ܵĴ���
inline CWX_UINT32 CwxAppConnInfo::getFailConnNum() const
{
    return m_uiFailConnNum;
}
///��������ʧ�����Ӵ���
inline void CwxAppConnInfo::setFailConnNum(CWX_UINT32 uiNum)
{
    m_uiFailConnNum = uiNum;
}
///��������ʧ�����Ӵ���
inline CWX_UINT32 CwxAppConnInfo::incFailConnNum()
{
    m_uiFailConnNum++;
    return m_uiFailConnNum;
}

///��ȡʧЧ����������С������ʱ����
inline CWX_UINT16 CwxAppConnInfo::getMinRetryInternal() const 
{
    return m_unMinRetryInternal;
}
inline void CwxAppConnInfo::setMinRetryInternal(CWX_UINT16 unInternal)
{
    m_unMinRetryInternal = unInternal;
}

///��ȡʧЧ�����������������ʱ����
inline CWX_UINT16 CwxAppConnInfo::getMaxRetryInternal() const 
{
    return m_unMaxRetryInternal;
}
inline void CwxAppConnInfo::setMaxRetryInternal(CWX_UINT16 unInternal)
{
    m_unMaxRetryInternal = unInternal;
}

///��ȡ�����Ƿ�Ϊ��������
inline bool CwxAppConnInfo::isActiveConn() const 
{
    return m_bActiveConn;
}
inline void CwxAppConnInfo::setActiveConn(bool bActive)
{
    m_bActiveConn = bActive;
}

///��ȡ�����Ƿ������ر�
inline bool CwxAppConnInfo::isActiveClose() const 
{
    return m_bActiveClose;
}
inline void CwxAppConnInfo::setActiveClose(bool bActive)
{
    m_bActiveClose = bActive;
}


///��ȡ���ӵ����ݰ��Ƿ��а�ͷ
inline bool CwxAppConnInfo::isRawData() const 
{
    return m_bRawData;
}
inline void CwxAppConnInfo::setRawData(bool bRaw)
{
    m_bRawData = bRaw;
}



///��ȡ���������յ���Ϣ��ʱ��
inline time_t  CwxAppConnInfo::getLastRecvMsgTime() const 
{
    return m_ttLastRecvMsgTime;
}
inline void CwxAppConnInfo::setLastRecvMsgTime(time_t ttTime)
{
    m_ttLastRecvMsgTime = ttTime;
}

///��ȡ�������·�����Ϣ��ʱ��
inline time_t  CwxAppConnInfo::getLastSendMsgTime() const 
{
    return m_ttLastSendMsgTime;
}
inline void CwxAppConnInfo::setLastSendMsgTime(time_t ttTime)
{
    m_ttLastSendMsgTime = ttTime;
}


///��ȡ���ӵ��û�����
inline void*  CwxAppConnInfo::getUserData() const
{
    return  m_pUserData;
}
///�������ӵ��û�����
inline void CwxAppConnInfo::setUserData(void* userData)
{
    m_pUserData = userData;
}

///��ȡ���ӵȴ����͵������Ϣ��������0��ʾû������
inline CWX_UINT32 CwxAppConnInfo::getMaxWaitingMsgNum() const
{
    return m_uiMaxWaitingMsgNum;
}
///�����������ĵȴ����͵���Ϣ������Ĭ��0��ʾû������
inline void CwxAppConnInfo::setMaxWaitingMsgNum(CWX_UINT32 uiNum)
{
    m_uiMaxWaitingMsgNum = uiNum;
}
///�ж��Ƿ����Ӵ����Ͷ�������
inline bool CwxAppConnInfo::isWaitingMsgQueueFull() const
{
    return m_uiMaxWaitingMsgNum && (m_uiMaxWaitingMsgNum <= m_uiWaitingMsgNum);
}


///��ȡ���ӵȴ����͵���Ϣ������
inline CWX_UINT32 CwxAppConnInfo::getWaitingMsgNum() const
{
    return m_uiWaitingMsgNum;
}
///�������ӵȴ����͵���Ϣ������
inline void CwxAppConnInfo::setWaitingMsgNum(CWX_UINT32 uiNum)
{
    m_uiWaitingMsgNum = uiNum;
}
///�������ӵȴ����͵���Ϣ������
inline CWX_UINT32 CwxAppConnInfo::incWaitingMsgNum()
{
    m_uiWaitingMsgNum++;
    return m_uiWaitingMsgNum;
}
///�������ӵȴ����͵���Ϣ������
inline CWX_UINT32 CwxAppConnInfo::decWaitingMsgNum()
{
    if (m_uiWaitingMsgNum) m_uiWaitingMsgNum--;
    return m_uiWaitingMsgNum;
}

///��ȡ�����Ѿ��������յ�����Ϣ��������
inline CWX_UINT32 CwxAppConnInfo::getContinueRecvNum() const
{
    return m_uiContinueRecvNum;
}
inline void CwxAppConnInfo::setContinueRecvNum(CWX_UINT32 uiNum)
{
    m_uiContinueRecvNum = uiNum;
}
///��ȡ�������͵���Ϣ����
inline CWX_UINT32 CwxAppConnInfo::getContinueSendNum() const
{
    return m_uiContinueSendNum;
}
///�����������͵���Ϣ����
inline void CwxAppConnInfo::setContinueSendNum(CWX_UINT32 uiNum)
{
    m_uiContinueSendNum = uiNum;
}


///�ж϶Ͽ��������Ƿ���Ҫ����
inline bool CwxAppConnInfo::isNeedReconnect() const 
{
    return !m_bActiveClose && m_bActiveConn;
}


///�Ƿ����CwxAppFramework::onCreate
inline bool CwxAppConnInfo::isInvokeCreate() const
{
    return m_bInvokeCreate;
}
///�����Ƿ����CwxAppFramework::onCreate
inline void CwxAppConnInfo::setInvokeCreate(bool bInvoke)
{
    m_bInvokeCreate = bInvoke;
}

///�Ƿ���������
inline bool CwxAppConnInfo::isReconn() const
{
    return m_bReconn;
}
///�����Ƿ�����
inline void CwxAppConnInfo::setReconn(bool bReconnect)
{
    m_bReconn = bReconnect;
}
///��ȡ����������ʱ�ĺ�����
inline CWX_UINT32 CwxAppConnInfo::getReconnDelay() const
{
    return m_uiReconnDelay;
}
///��������������ʱ�ĺ�����
inline void CwxAppConnInfo::setReconnDelay(CWX_UINT32 uiDelay)
{
    m_uiReconnDelay = uiDelay;
}

///��ȡsocket���õ�function
inline CWX_NET_SOCKET_ATTR_FUNC CwxAppConnInfo::getSockFunc() const
{
    return m_fn;
}
///����socket���õ�function
inline void CwxAppConnInfo::setSockFunc(CWX_NET_SOCKET_ATTR_FUNC fn)
{
    m_fn = fn;
}

///��ȡsocket����function��arg
inline void* CwxAppConnInfo::getSockFuncArg() const
{
    return m_fnArg;
}
///����socket����function��arg
inline void CwxAppConnInfo::setSockFuncArg(void* arg)
{
    m_fnArg = arg;
}




///��ȡ���Ӷ�Ӧ��handler
inline CwxAppHandler4Msg* CwxAppConnInfo::getHandler()
{
    return m_pHandler;
}
///�������Ӷ�Ӧ��handler
inline void CwxAppConnInfo::setHandler(CwxAppHandler4Msg*  pHandler)
{
    m_pHandler = pHandler;
}



inline void CwxAppConnInfo::reset()
{
    m_bActiveClose = false;
    m_uiWaitingMsgNum = 0;
    m_uiMaxWaitingMsgNum = 0;
    m_uiContinueRecvNum = 0;
    m_uiContinueSendNum = 0;
//    m_bInvokeCreate = true;
    m_bReconn = false;
    m_uiReconnDelay = 0;
}

CWINUX_END_NAMESPACE

