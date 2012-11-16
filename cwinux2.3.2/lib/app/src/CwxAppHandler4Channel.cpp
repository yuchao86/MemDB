#include "CwxAppHandler4Channel.h"
#include "CwxAppChannel.h"
#include "CwxSockIo.h"

CWINUX_BEGIN_NAMESPACE


CwxAppHandler4Channel::CwxAppHandler4Channel(CwxAppChannel *channel)
:CwxAppHandler4Base(NULL)
{
    m_uiSendByte = 0;
    m_curSndingMsg = 0;
    m_waitSendMsgHead = NULL;
    m_waitSendMsgTail = NULL;
    m_channel = channel;
    m_bRedo = false;
}

///��������
CwxAppHandler4Channel::~CwxAppHandler4Channel()
{
    clear();
    if (getHandle() != CWX_INVALID_HANDLE)
    {
        ::close(getHandle());
        setHandle(CWX_INVALID_HANDLE);
    }
}


int CwxAppHandler4Channel::open (void * )
{
    //��������Ϊ������״̬
    if	(CwxSockIo::setNonblock(getHandle(), true) == -1)
    {
        CWX_ERROR(("Failure to set the connection for NONBLOCK.conn[%d]",
            getHandle()));
        return -1;
    }

    if (0 != channel()->registerHandler(getHandle(),
        this,
        CwxAppHandler4Base::PREAD_MASK))
    {
        CWX_ERROR(("Failure to register conn[%d] for read_mask",
            getHandle()));
        return -1;
    }
    return 0;
}

int CwxAppHandler4Channel::close(CWX_HANDLE)
{
    m_uiSendByte = 0;
    if (m_curSndingMsg)
    {
        if (m_curSndingMsg->send_ctrl().isFailNotice())
        {
            onFailSendMsg(m_curSndingMsg);
        }
        if (m_curSndingMsg) CwxMsgBlockAlloc::free(m_curSndingMsg);
    }
    m_curSndingMsg = NULL;
    while(m_waitSendMsgHead){
        m_curSndingMsg = m_waitSendMsgHead->m_next;
        if (m_waitSendMsgHead->send_ctrl().isFailNotice())
        {
            onFailSendMsg(m_waitSendMsgHead);
        }
        if (m_waitSendMsgHead) CwxMsgBlockAlloc::free(m_waitSendMsgHead);
        m_waitSendMsgHead = m_curSndingMsg;
    }
    m_waitSendMsgTail = NULL;

    int ret = onConnClosed();
	if (m_bRedo)
	{
		channel()->eraseRedoHander(this);
	}
    if (-1 == ret)
    {
        channel()->removeHandler(this);
        delete this;
		return 0;
    }
    else if (0 == ret)
    {
        channel()->removeHandler(this);
    }
    return 0;
}


/**
@brief ���������ϵĵ�������
@param [in] handle ���ӵ�handle
@return -1���رյ����ӣ� 0���������ݳɹ�
*/
int CwxAppHandler4Channel::handle_event(int event, CWX_HANDLE)
{
    if (CWX_CHECK_ATTR(event, CwxAppHandler4Base::WRITE_MASK))
    {
        if (0 != onOutput()) return -1;
    }
    if (CWX_CHECK_ATTR(event, CwxAppHandler4Base::READ_MASK))
    {
        if (0 != onInput()) return -1;
    }
    if (CWX_CHECK_ATTR(event, CwxAppHandler4Base::TIMEOUT_MASK))
    {
        if (0 != onTimeout( channel()->getCurTime())) return -1;
    }
    return 0;
}


int CwxAppHandler4Channel::onOutput ()
{
    int result = 0;
    bool bCloseConn = false;
    // The list had better not be empty, otherwise there's a bug!
    if (NULL == this->m_curSndingMsg)
    {
        CWX_ASSERT(m_uiSendByte==0);
        while(1)
        {
            result = this->getNextMsg();
            if (0 == result) return this->cancelWakeup();
            if (CWX_CHECK_ATTR(this->m_curSndingMsg->send_ctrl().getMsgAttr(), CwxMsgSendCtrl::BEGIN_NOTICE))
            {
                if (-1 == onStartSendMsg(m_curSndingMsg))
                {
                    CwxMsgBlockAlloc::free(this->m_curSndingMsg);
                    this->m_curSndingMsg = NULL;
                    continue; //next msg;
                }
            }//end if (this->m_curSndingMsg->IsBeginNotice())
            //it's a msg which need be sent.
            break; //break while
        }//end while
    }
    //has message
    result = this->nonBlockSend();
    if (0 == result)
    {// Partial send.
        CWX_ASSERT (errno == EWOULDBLOCK);
        return 0;
    }
    else if (1 == result)
    {//finish
        bCloseConn = CWX_CHECK_ATTR(this->m_curSndingMsg->send_ctrl().getMsgAttr(), CwxMsgSendCtrl::CLOSE_NOTICE);
        this->m_uiSendByte = 0;
        if (CWX_CHECK_ATTR(this->m_curSndingMsg->send_ctrl().getMsgAttr(), CwxMsgSendCtrl::FINISH_NOTICE)){
            CWX_UINT32 uiResume = onEndSendMsg(m_curSndingMsg);
            if (CwxMsgSendCtrl::RESUME_CONN == uiResume)
            {
                if (0 != channel()->resumeHandler(this, CwxAppHandler4Base::READ_MASK))
                {
                    CWX_ERROR(("Failure to resume handler with conn_id[%d]", getHandle()));
                    return -1;
                }
            }
            else if (CwxMsgSendCtrl::SUSPEND_CONN == uiResume)
            {
                if (0 != channel()->suspendHandler(this, CwxAppHandler4Base::READ_MASK))
                {
                    CWX_ERROR(("Failure to suspend handler with conn_id[%d]", getHandle()));
                    return -1;
                }
            }
        }
        if (bCloseConn)
        {
            return -1;
        }
        if (m_curSndingMsg)
        {
            CwxMsgBlockAlloc::free(m_curSndingMsg);
            this->m_curSndingMsg = NULL;
        }
        return 0;
    }
    else
    {//failure
        if (m_curSndingMsg->send_ctrl().isFailNotice())
        {
            onFailSendMsg(m_curSndingMsg);
        }
        if (m_curSndingMsg) CwxMsgBlockAlloc::free(m_curSndingMsg);
        this->m_curSndingMsg = NULL;
        return -1;
    }
    return -1;
}

/***
desc:
	recv data from socket.
param: 
	handle of the event.
return:
	-1 : failure, handle_close is invoked.
	0  : success.
***/
int CwxAppHandler4Channel::onInput ()
{
    return -1;
}

/***
desc:
	timeout evnet, it exec the noticeReconnect operation.
return:
	0  : success.
***/
int CwxAppHandler4Channel::onTimeout(CwxTimeValue const& )
{
    return 0;
}

int CwxAppHandler4Channel::onRedo()
{
    return 0;
}

//return -1��ȡ����Ϣ�ķ��͡� 0��������Ϣ��
int CwxAppHandler4Channel::onStartSendMsg(CwxMsgBlock* )
{
    return 0;
}
/*return 
CwxMsgSendCtrl::UNDO_CONN�����޸����ӵĽ���״̬
CwxMsgSendCtrl::RESUME_CONN�������Ӵ�suspend״̬��Ϊ���ݽ���״̬��
CwxMsgSendCtrl::SUSPEND_CONN�������Ӵ����ݽ���״̬��Ϊsuspend״̬
*/
CWX_UINT32 CwxAppHandler4Channel::onEndSendMsg(CwxMsgBlock*& )
{
    return CwxMsgSendCtrl::UNDO_CONN;
}

void CwxAppHandler4Channel::onFailSendMsg(CwxMsgBlock*&)
{


}
//return �����������ӣ�1������engine���Ƴ�ע�᣻0������engine���Ƴ�ע�ᵫ��ɾ��handler��-1����engine�н�handle�Ƴ���ɾ����
int CwxAppHandler4Channel::onConnClosed()
{
    return -1;
}


///����û����Ϣ���ͣ�ʹ���ӵķ��ͼ������.����ֵ�� -1: failure, 0: success
int CwxAppHandler4Channel::cancelWakeup()
{
    if(-1 == channel()->suspendHandler(this, CwxAppHandler4Base::WRITE_MASK)){
        CWX_ERROR(("Failure to cancel wakeup a connection. conn[%d]", getHandle()));
        return -1;
    }
    return 0;
}

///�������ӵĿ�д��أ��Է���δ������ϵ�����.����ֵ�� -1:failure�� 0:success��
int CwxAppHandler4Channel::wakeUp()
{
    if(-1 == channel()->resumeHandler(this, CwxAppHandler4Base::WRITE_MASK)){
        CWX_ERROR(("Failure to wakeup a connection. conn[%d]", getHandle()));
        return -1;
    }
    return 0;
}

int CwxAppHandler4Channel::recvPackage(CWX_HANDLE handle,
                                       CWX_UINT32& uiRecvHeadLen,
                                       CWX_UINT32& uiRecvDataLen,
                                       char*      szRecvHead,
                                       CwxMsgHead& header,
                                       CwxMsgBlock*& msg)
{
    ssize_t recv_size = 0;
    ssize_t need_size = 0;
    need_size = CwxMsgHead::MSG_HEAD_LEN - uiRecvHeadLen;
    if (need_size > 0 )
    {//not get complete head
        recv_size = CwxSocket::recv(handle, szRecvHead + uiRecvHeadLen, need_size);
        if (recv_size <=0 )
        { //error or signal
            if ((0==recv_size) || ((errno != EWOULDBLOCK) && (errno != EINTR)))
            {
                return -1; //error
            }
            else
            {//signal or no data
                return 0;
            }
        }
        uiRecvHeadLen += recv_size;
        if (recv_size < need_size)
        {
            return 0;
        }
        szRecvHead[uiRecvHeadLen] = 0x00;
        if (!header.fromNet(szRecvHead))
        {
            CWX_ERROR(("Msg header is error."));
            return -1;
        }
        msg = CwxMsgBlockAlloc::malloc(header.getDataLen());
        uiRecvDataLen = 0;
    }//end  if (need_size > 0)
    //recv data
    need_size = header.getDataLen() - uiRecvDataLen;
    if (need_size > 0)
    {//not get complete data
        recv_size = CwxSocket::recv(handle, msg->wr_ptr(), need_size);
        if (recv_size <=0 )
        { //error or signal
            if ((errno != EWOULDBLOCK)&&(errno != EINTR))
            {
                return -1; //error
            }
            else
            {//signal or no data
                return 0;
            }
        }
        //move write pointer
        msg->wr_ptr(recv_size);
        uiRecvDataLen += recv_size;
        if (recv_size < need_size)
        {
            return 0;
        }
    }
    return 1;
}


CWINUX_END_NAMESPACE

