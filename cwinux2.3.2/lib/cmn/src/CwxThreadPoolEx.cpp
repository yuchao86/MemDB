#include "CwxThreadPoolEx.h"

CWINUX_BEGIN_NAMESPACE
///���캯��
CwxThreadPoolEx::CwxThreadPoolEx(CWX_UINT16 unGroupId,///<�̳߳ص�thread-group
                 CWX_UINT16 unThreadNum,///<�̳߳����̵߳�����
                 CwxThreadPoolMgr* mgr, ///<�̵߳Ĺ������
                 CwxCommander* commander,///<������Ϣ���ѵ�ȱʡcommander����ָ��func���Բ�ָ��
                 CWX_TSS_THR_FUNC func, ///<�û����߳�main����
                 void*            arg ///<func��void*����
                   ): CwxTpi(unGroupId, unThreadNum)
{
    m_commander = commander;
    m_func = func;
    m_arg = arg;
    m_mgr = mgr;
    m_threadArr =  NULL;
}

///��������
CwxThreadPoolEx::~CwxThreadPoolEx()
{
//    stop();
    if (m_threadArr)
    {
        for (CWX_UINT16 i=0; i<getThreadNum(); i++)
        {
            if (m_threadArr[i]) delete m_threadArr[i];
        }
        delete [] m_threadArr;
    }
}

int CwxThreadPoolEx::start(CwxTss** pThrEnv, size_t stack_size)
{
    CWX_UINT16 i = 0;
    if (m_mgr->isExist(getGroupId()))
    {
        CWX_ERROR(("Thread group[%u] exist.", getGroupId()));
        return -1;
    }
    m_mgr->add(getGroupId(), this);
    m_threadArr = new CwxThread*[getThreadNum()];
    for (i=0; i<getThreadNum(); i++)
    {
        m_threadArr[i] =  new CwxThread(getGroupId(),
            i,
            m_mgr,
            m_commander,
            m_func,
            m_arg);
        if (0 != m_threadArr[i]->start(pThrEnv?pThrEnv[i]:NULL, stack_size))
        {
            return -1;
        }
    }
    if (pThrEnv) delete [] pThrEnv;
    return 0;
}

void CwxThreadPoolEx::stop()
{
    for (CWX_UINT16 i=0; i<getThreadNum(); i++)
    {
        if (m_threadArr && m_threadArr[i]) m_threadArr[i]->stop();
    }
}

bool CwxThreadPoolEx::isStop()
{
    for (CWX_UINT16 i=0; i<getThreadNum(); i++)
    {
        if (m_threadArr && m_threadArr[i]) 
        {
            if (m_threadArr[i]->isStop()) return true;
        }
    }
    return false;
}

CwxTss* CwxThreadPoolEx::getTss(CWX_UINT16 unThreadIndex)
{
    if (m_threadArr && (unThreadIndex<getThreadNum()) && m_threadArr[unThreadIndex])
    {
        return m_threadArr[unThreadIndex]->getTss();
    }
    return NULL;
}
///��ס�����̳߳ء�����ֵ0���ɹ���-1��ʧ��
int CwxThreadPoolEx::lock()
{
    for (CWX_UINT16 i=0; i<getThreadNum(); i++)
    {
        if (m_threadArr && m_threadArr[i]) 
        {
            if (0 != m_threadArr[i]->lock())
            {
                for (CWX_UINT16 j=0; j<i; j++)
                {
                    if (m_threadArr[j]) m_threadArr[j]->unlock();
                }
                return -1;
            }
        }
    }
    return 0;
}
///��������̳߳ء�����ֵ0���ɹ���-1��ʧ��
int CwxThreadPoolEx::unlock()
{
    for (CWX_UINT16 i=0; i<getThreadNum(); i++)
    {
        if (m_threadArr && m_threadArr[i]) 
        {
            m_threadArr[i]->unlock();
        }
    }
    return 0;
}

CWINUX_END_NAMESPACE
