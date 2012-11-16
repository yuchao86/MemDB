
CWINUX_BEGIN_NAMESPACE

inline size_t CwxThreadPoolEx::getQueuedMsgNum()
{
    size_t num = 0;
    for (CWX_UINT16 i=0; i<getThreadNum(); i++){
        if (m_threadArr && m_threadArr[i]) {
            num += m_threadArr[i]->getQueuedMsgNum();
        }
    }
    return num;
}
///��ȡ�̵߳���Ϣ�����Ŷ���Ϣ��
inline size_t CwxThreadPoolEx::getQueuedMsgNum(CWX_UINT16 unThreadIndex)
{
    if (!m_threadArr || (unThreadIndex >= getThreadNum())) return 0;
    return m_threadArr[unThreadIndex]->getQueuedMsgNum();
}

/**
@brief ���̵߳���Ϣ�������һ������Ϣ��
@param [in] pMsg append����Ϣ
@param [in] uiThread ��Ϣ���̶߳��У��ڲ������Thread��������������������̡߳�
@return -1��ʧ�ܣ�>=0Ϊ��Ӧ�̵߳Ķ������Ŷӵ���Ϣ����
*/
inline int CwxThreadPoolEx::append(CwxMsgBlock* pMsg, CWX_UINT32 uiThread)
{
    CWX_UINT16 uiIndex = uiThread%getThreadNum();
    return m_threadArr[uiIndex]->append(pMsg);
}

inline int CwxThreadPoolEx::appendHead(CwxMsgBlock* pMsg, CWX_UINT32 uiThread)
{
    CWX_UINT16 uiIndex = uiThread%getThreadNum();
    return m_threadArr[uiIndex]->appendHead(pMsg);
}

CWINUX_END_NAMESPACE

