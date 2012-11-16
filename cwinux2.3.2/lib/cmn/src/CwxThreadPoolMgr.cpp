#include "CwxThreadPoolMgr.h"

CWINUX_BEGIN_NAMESPACE
CwxThreadPoolMgr::CwxThreadPoolMgr()
{
}

CwxThreadPoolMgr::~CwxThreadPoolMgr()
{
    m_threadPoolMap.clear();
    map<CWX_UINT16, map<CWX_UINT16, CwxTss*> >::iterator iter =m_threadPoolTss.begin();
    while(iter != m_threadPoolTss.end())
    {
        map<CWX_UINT16, CwxTss*>::iterator iter_tss = iter->second.begin();
        while(iter_tss != iter->second.end())
        {
            delete iter_tss->second;
            iter_tss++;
        }
        iter++;
    }
    m_threadPoolTss.clear();
}

bool CwxThreadPoolMgr::add(CWX_UINT16 unGroupId, CwxTpi* pThreadPool)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    if (m_threadPoolMap.find(unGroupId) != m_threadPoolMap.end()) return false;
    m_threadPoolMap[unGroupId] = pThreadPool;
    return true;
}

bool CwxThreadPoolMgr::remove(CWX_UINT16 unGroupId)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    return m_threadPoolMap.erase(unGroupId)>0?true:false;
}

bool CwxThreadPoolMgr::isExist(CWX_UINT16 unGroupId)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    return m_threadPoolMap.find(unGroupId) != m_threadPoolMap.end();
}


CWX_UINT16 CwxThreadPoolMgr::getNum()
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    return m_threadPoolMap.size();
}

bool CwxThreadPoolMgr::addTss(CwxTss* pTss)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    CWX_UINT16 unGroup = pTss->getThreadInfo().getThreadGroup();
    CWX_UINT16 unThreadNo = pTss->getThreadInfo().getThreadNo();
    map<CWX_UINT16, map<CWX_UINT16, CwxTss*> >::iterator iter = m_threadPoolTss.find(unGroup);
    if (iter == m_threadPoolTss.end())
    {
        m_threadPoolTss[unGroup] = map<CWX_UINT16, CwxTss*>();
        iter = m_threadPoolTss.find(unGroup);
    }
    if (iter->second.find(unThreadNo) != iter->second.end())
    {
        return false;
    }
    iter->second[unThreadNo] = pTss;
    return true;
}

void CwxThreadPoolMgr::getTss(vector<vector<CwxTss*> >& arrTss)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    vector<CwxTss*> poolTss;
    arrTss.clear();
    map<CWX_UINT16, map<CWX_UINT16, CwxTss*> >::iterator iter = m_threadPoolTss.begin();
    while (iter != m_threadPoolTss.end())
    {
        poolTss.clear();
        map<CWX_UINT16, CwxTss*>::iterator iter_tss = iter->second.begin();
        while(iter_tss != iter->second.end())
        {
            poolTss.push_back(iter_tss->second);
            iter_tss++;
        }
        arrTss.push_back(poolTss);
        iter++;
    }
}


void CwxThreadPoolMgr::getTss(CWX_UINT16 unGroup, vector<CwxTss*>& arrTss)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    arrTss.clear();
    map<CWX_UINT16, map<CWX_UINT16, CwxTss*> >::iterator iter = m_threadPoolTss.find(unGroup);
    if (iter != m_threadPoolTss.end())
    {
        map<CWX_UINT16, CwxTss*>::iterator iter_tss = iter->second.begin();
        while(iter_tss != iter->second.end())
        {
            arrTss.push_back(iter_tss->second);
            iter_tss++;
        }
    }
}

CwxTss* CwxThreadPoolMgr::getTss(CWX_UINT16 unGroup, CWX_UINT16 unThreadId)
{
    CwxMutexGuard<CwxMutexLock> lock(&m_lock);
    map<CWX_UINT16, map<CWX_UINT16, CwxTss*> >::iterator iter = m_threadPoolTss.find(unGroup);
    if (iter != m_threadPoolTss.end())
    {
        map<CWX_UINT16, CwxTss*>::iterator iter_tss = iter->second.find(unThreadId);
        if (iter_tss != iter->second.end()) return iter_tss->second;
    }
    return NULL;
}


CWINUX_END_NAMESPACE
