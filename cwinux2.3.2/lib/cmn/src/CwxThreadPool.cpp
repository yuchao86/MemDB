#include "CwxThreadPool.h"
#include "CwxLogger.h"

CWINUX_BEGIN_NAMESPACE
///���캯��
CwxThreadPool::CwxThreadPool(CWX_UINT16 unGroupId,///<�̳߳ص�thread-group
                 CWX_UINT16 unThreadNum,///<�̳߳����̵߳�����
                 CwxThreadPoolMgr* mgr, ///<�̵߳Ĺ������
                 CwxCommander* commander,///<������Ϣ���ѵ�ȱʡcommander����ָ��func���Բ�ָ��
                 CWX_TSS_THR_FUNC func, ///<�û����߳�main����
                 void*            arg ///<func��void*����
                 ):CwxTpi(unGroupId, unThreadNum)
{
    m_arrTssEnv = NULL;
    m_msgQueue = new CwxMsgQueue(1024*1024*200, 1024*1024*200);
    m_commander = commander;
    m_func = func;
    m_arg = arg;
    m_mgr = mgr;
    m_threadArr =  NULL;
}

///��������
CwxThreadPool::~CwxThreadPool()
{
    if (m_arrTssEnv) delete []m_arrTssEnv;
//    _stop();
    if (m_threadArr) {
        for (CWX_UINT16 i=0; i<getThreadNum(); i++)
        {
            if (m_threadArr[i]) delete m_threadArr[i];
        }
        delete [] m_threadArr;
    }
    if (m_msgQueue) delete m_msgQueue;
}

int CwxThreadPool::start(CwxTss** pThrEnv, size_t stack_size)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    if (m_mgr->isExist(getGroupId()))
    {
        CWX_ERROR(("Thread group[%u] exist.", getGroupId()));
        return -1;
    }
    m_mgr->add(getGroupId(), this);
    if (pThrEnv)
    {
        m_arrTssEnv = pThrEnv;
    }
    else
    {
        m_arrTssEnv = new CwxTss*[getThreadNum()];
        memset(m_arrTssEnv, 0x00, sizeof(CwxTss*) * getThreadNum());
    }
    ///�����߳�
    CWX_UINT16 index= 0;
    m_threadArr = new CwxThread*[getThreadNum()];
    for (index=0; index<getThreadNum(); index++)
    {
        m_threadArr[index] = new CwxThread(getGroupId(),
            index,
            m_mgr,
            m_commander,
            m_func,
            m_arg,
            m_msgQueue);
        if (0 != m_threadArr[index]->start(m_arrTssEnv[index], stack_size))
        {
           _stop();
           return -1;
        }
    }
    return 0;
}

void CwxThreadPool::stop()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    _stop();
}

bool CwxThreadPool::isStop() 
{
    return m_msgQueue->getState() == CwxMsgQueue::DEACTIVATED;
}

CwxTss* CwxThreadPool::getTss(CWX_UINT16 unThreadIndex)
{
    if (unThreadIndex >= getThreadNum()) return NULL;
    return m_arrTssEnv[unThreadIndex];
}

///��ס�����̳߳ء�����ֵ0���ɹ���-1��ʧ��
int CwxThreadPool::lock()
{
    return m_msgQueue->lock().acquire();
}
///��������̳߳ء�����ֵ0���ɹ���-1��ʧ��
int CwxThreadPool::unlock()
{
    return m_msgQueue->lock().release();
}


void CwxThreadPool::_stop()
{
    if (m_msgQueue->isActivate()){
        m_msgQueue->deactivate();
        if (m_threadArr){
            for(CWX_UINT16 i=0; i<getThreadNum(); i++){
                if (m_threadArr[i]){
                    m_threadArr[i]->stop();
                }
            }
        }
    }
    m_msgQueue->flush();
    m_mgr->remove(getGroupId());
}


CWINUX_END_NAMESPACE
