CWINUX_BEGIN_NAMESPACE

inline CwxAppConnInfo& CwxAppHandler4Msg::getConnInfo()
{
    return m_conn;
}

inline CwxAppConnInfo const& CwxAppHandler4Msg::getConnInfo() const
{
    return m_conn;
}



///��ȡ��һ�������͵���Ϣ������ֵ��0��û�д�������Ϣ��1,�����һ����������Ϣ
inline int CwxAppHandler4Msg::getNextMsg()
{
    if (this->m_curSndingMsg) return 1;
    if (this->m_waitSendMsgHead == NULL) return 0;
    this->m_curSndingMsg = this->m_waitSendMsgHead;
    this->m_waitSendMsgHead = this->m_waitSendMsgHead->m_next;
    if (!this->m_waitSendMsgHead) this->m_waitSendMsgTail = NULL;
    m_curSndingMsg->m_next = NULL;
    return 1;
}

///��Ҫ���͵���Ϣ�Ŷ�
inline bool CwxAppHandler4Msg::putMsg(CwxMsgBlock* msg)
{
    if (m_conn.isWaitingMsgQueueFull())
    {
        CWX_DEBUG(("Connection is full, svr_id=%u, host_id=%u, conn_id=%u, msg_num=%u", 
            m_conn.getSvrId(),
            m_conn.getHostId(),
            m_conn.getConnId(),
            m_conn.getWaitingMsgNum()));
        return false;
    }
    msg->m_next = NULL;
    if (this->m_waitSendMsgTail){
        this->m_waitSendMsgTail->m_next = msg;
        this->m_waitSendMsgTail = msg;
        this->m_conn.setWaitingMsgNum(this->m_conn.getWaitingMsgNum()+1);
    }else{
        this->m_waitSendMsgTail = this->m_waitSendMsgHead = msg;
        this->m_conn.setWaitingMsgNum(1);
        this->wakeUp();
    }
    return true;
}

///����û����Ϣ���ͣ�ʹ���ӵķ��ͼ������.����ֵ�� -1: failure, 0: success
inline int CwxAppHandler4Msg::cancelWakeup()
{
    if(-1 == this->reactor()->suspendHandler(this, CwxAppHandler4Base::WRITE_MASK)){
        CWX_ERROR(("Failure to cancel wakeup a connection. conn[%u]", m_conn.getConnId()));
        return -1;
    }
    return 0;
}

///�������ӵĿ�д��أ��Է���δ������ϵ�����.����ֵ�� -1:failure�� 0:success��
inline int CwxAppHandler4Msg::wakeUp()
{
    if(-1 == this->reactor()->resumeHandler(this, CwxAppHandler4Base::WRITE_MASK)){
        CWX_ERROR(("Failure to wakeup a connection. conn[%u]", m_conn.getConnId()));
        return -1;
    }
    return 0;
}

///����Ƿ�suspend���ӵĿɶ�����д���
inline bool CwxAppHandler4Msg::isStopListen() const
{
    return m_bStopListen;
}

///����stop listen
inline void CwxAppHandler4Msg::setStopListen(bool bStop)
{
    m_bStopListen = bStop;
}

inline bool CwxAppHandler4Msg::isEmpty() const
{
    return (!this->m_curSndingMsg) && (!this->m_waitSendMsgHead);

}

///��ȡ����ʱ�Ĵ������
inline int CwxAppHandler4Msg::getConnErrNo() const
{
    return m_connErrNo;
}


///�Է������ķ�ʽ��������Ϣ������ֵ,-1: failure; 0: not send all;1:send a msg
inline int CwxAppHandler4Msg::nonBlockSend()
{
    CWX_ASSERT(NULL != this->m_curSndingMsg);
    char szAddr[256];
    CwxMsgBlock* data = this->m_curSndingMsg;

    //get the leaving data length to send.
    ssize_t len = data->length () - this->m_uiSendByte;
    //send data
    ssize_t n = CwxSocket::write(getHandle(), data->rd_ptr () + this->m_uiSendByte, len);
    if (n <= 0){
        if ((errno != EWOULDBLOCK)&&(errno != EINTR)){ //error
            CWX_ERROR(("Failure to send msg to IP[%s], port[%d], handle[%d], errno[%d].", 
                this->getRemoteAddr(szAddr, 255),
                this->getRemotePort(),
                (int)getHandle(),
                errno));
            return -1;
        }
        errno = EWOULDBLOCK;
        n = 0;
    }else if (n < len)
    {
        // Re-adjust pointer to skip over the part we did send.
        this->m_uiSendByte += n;
        errno = EWOULDBLOCK;
    }else // if (n == length)
    {
        // The whole event is sent, we now decrement the reference count
        // (which deletes itself with it reaches 0).
        this->m_uiSendByte = 0;
        errno = 0;
    }
    return EWOULDBLOCK == errno? 0:1;
}



CWINUX_END_NAMESPACE

