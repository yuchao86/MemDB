#ifndef __CWX_THREAD_POOL_MGR_H__
#define __CWX_THREAD_POOL_MGR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxThreadPoolMgr.h
@brief �̳߳ؼ����̵߳�TSS�Ĺ�������ʵ��
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxStl.h"
#include "CwxType.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxTss.h"
#include "CwxTpi.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxThreadPoolMgr
@brief �̳߳ؼ����̵߳�TSS�Ĺ������
*/
class CWX_API CwxThreadPoolMgr
{
public:
    ///���캯��
    CwxThreadPoolMgr();
    ///��������
    ~CwxThreadPoolMgr();
public:
    /**
    @brief �����������һ���̳߳�
    @param [in] unGroupId �̳߳ص��߳���ID
    @param [in] pThreadPool �̳߳ض���
    @return false��unGroupIdָ�����̳߳��Ѿ����ڣ� true���ɹ�
    */
    bool add(CWX_UINT16 unGroupId, CwxTpi* pThreadPool);
    /**
    @brief �ӹ�������ɾ��һ���̳߳�
    @param [in] unGroupId ɾ���̳߳ص��߳���ID
    @return false�������ڣ� true���ɹ�ɾ��
    */
    bool remove(CWX_UINT16 unGroupId);
    /**
    @brief ���ָ����unGroupId���߳����ڹ��������Ƿ����
    @return false�������ڣ� true���ɹ�ɾ��
    */
    bool isExist(CWX_UINT16 unGroupId);
    ///��ȡ������̳߳ص�����
    CWX_UINT16 getNum();
    /**
    @brief ���̳߳ع����������һ���̵߳�TSS
    @param [in] pTss �̵߳�TSS
    @return false�����TSS��GroupId��ThreadId��ͬ��Tss�Ѿ����ڣ� true���ɹ�
    */
    bool addTss(CwxTss* pTss);
    /**
    @brief ����������������̵߳�TSS
    @param [in] arrTss �����̵߳�TSS��arrTss[i]Ϊһ���߳�����̵߳�TSS����һ�������Ԫ�ذ�GroupId���򣬵ڶ�����ThreadId����
    @return void
    */
    void getTss(vector<vector<CwxTss*> >& arrTss);
    /**
    @brief ����ָ��Thread GroupId��TSS
    @param [in] unGroup �̵߳�GroupId
    @param [in] arrTss ���߳�����̵߳�TSS�����鰴���̵߳�ThreadId����
    @return void
    */
    void getTss(CWX_UINT16 unGroup, vector<CwxTss*>& arrTss);
    /**
    @brief ����һ���̵߳�Tss
    @param [in] unGroup �̵߳�GroupId
    @param [in] unThreadId �̵߳�ThreadId��
    @return NULL�������ڣ�����Ϊ�̵߳�TSS
    */
    CwxTss* getTss(CWX_UINT16 unGroup, CWX_UINT16 unThreadId);
private:
    CwxMutexLock        m_lock;///<thread lock for sync.
    map<CWX_UINT16, CwxTpi*>  m_threadPoolMap; ///<�̳߳ص�MAP
    map<CWX_UINT16, map<CWX_UINT16, CwxTss*> >  m_threadPoolTss;///�߳�Tss��map
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"


#endif
