CWINUX_BEGIN_NAMESPACE
///���캯��
inline CwxAppNotice::CwxAppNotice()
:m_unNoticeType(DUMMY),m_noticeArg(NULL),m_noticeArg1(NULL),m_next(NULL)
{
}

///��ն���
inline void CwxAppNotice::reset()
{
    m_unNoticeType =  0;
    m_noticeArg = NULL;
    m_noticeArg1 = NULL;
    m_next = NULL;
}



///�ܵ���ʼ��������ֵ��-1��ʧ�ܣ�0���ɹ���
inline int CwxAppNoticePipe::init()
{
    clear();
    CWX_HANDLE  pipes[2];
    if (-1 == ::pipe(pipes))
    {
        CWX_ERROR(("Failure to invoke pipe(), errno=%d", errno));
        return -1;
    }
    m_pipeReader = pipes[0];
    m_pipeWriter = pipes[1];
    return 0;
}

/**
@brief ���ܵ�����pushһ��notice��Ϣ
@param [in] pItem ֪ͨ����Ϣ
@return -1��д�ܵ�ʧ�ܣ� 0���ɹ�
*/
inline int CwxAppNoticePipe::notice(CwxAppNotice* pItem)
{
    CwxMutexGuard<CwxMutexLock> lock(&this->m_lock);
    pItem->m_next = NULL;
    if (m_bPipeEmpty)
    {
        CWX_ASSERT(!m_noticeHead);
        CWX_ASSERT(!m_noticeTail);
        if (1 != write(m_pipeWriter, "n", 1))
        {
            CWX_ERROR(("Failure to write msg to notice pipe, errno%d", errno));
            return -1;                
        }
        m_noticeHead = m_noticeTail = pItem;
        m_bPipeEmpty = false;
    }
    else
    {
        if (m_noticeTail)
        {
            m_noticeTail->m_next = pItem;
            m_noticeTail = pItem;
        }
        else
        {
            CWX_ASSERT(!m_noticeHead);
            m_noticeHead = m_noticeTail = pItem;
        }
    }
    return 0;
}
/**
@brief ���ܵ�����pushһ��dummy notice��Ϣ����Ŀ���ǣ���ͨ���߳���������֤����ͨ���̡߳�
@return -1��д�ܵ�ʧ�ܣ� 0���ɹ�
*/
inline int CwxAppNoticePipe::noticeDummy()
{
    CwxMutexGuard<CwxMutexLock> lock(&this->m_lock);
    if (m_bPipeEmpty)
    {
        if (1 != ::write(m_pipeWriter, "n", 1))
        {
            CWX_ERROR(("Failure to write msg to notice pipe, errno=%d", errno));
            return -1;                
        }
        m_bPipeEmpty = false;
    }
    return 0;
}
/**
@brief ͨ���̻߳�ȡ��ѹ��Notice��Ϣ��
@return void
*/
inline void CwxAppNoticePipe::noticed(CwxAppNotice*& head)
{
    CwxMutexGuard<CwxMutexLock> lock(&this->m_lock);
    head = m_noticeHead;
    m_noticeHead = m_noticeTail = NULL;
    m_bPipeEmpty = true;
}
///��ȡ�ܵ��Ķ����
inline CWX_HANDLE CwxAppNoticePipe::getPipeReader()
{
    return m_pipeReader;
}
///��ȡ�ܵ���д���
inline CWX_HANDLE CwxAppNoticePipe::getPipeWriter()
{
    return m_pipeWriter;
}
///�ͷŶ������Դ
inline void CwxAppNoticePipe::clear()
{
    if (CWX_INVALID_HANDLE != m_pipeReader)
    {
        ::close(m_pipeReader);
        m_pipeReader = CWX_INVALID_HANDLE;
    }
    if (CWX_INVALID_HANDLE != m_pipeWriter)
    {
        ::close(m_pipeWriter);
        m_pipeWriter = CWX_INVALID_HANDLE;
    }
    CwxAppNotice* item;
    while(m_noticeHead){
        item = m_noticeHead->m_next;
        delete m_noticeHead;
        m_noticeHead = item;
    }
    m_noticeTail = NULL;
    m_bPipeEmpty = true;
}

CWINUX_END_NAMESPACE
