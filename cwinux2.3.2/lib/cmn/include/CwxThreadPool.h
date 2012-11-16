#ifndef __CWX_THREAD_POOL_H__
#define __CWX_THREAD_POOL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxThreadPool.h
@brief �޷����Ƶ����̵߳���ͨ�̳߳ض���ʵ��
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxMsgBlock.h"
#include "CwxTss.h"
#include "CwxTpi.h"
#include "CwxThread.h"

CWINUX_BEGIN_NAMESPACE

/**
@class CwxThreadPool
@brief ��ͨ���̳߳أ��̳߳��е������߳���Ϊһ��������֣��޷��������ء�
       ���̳߳ر������һ�������������߶��С�
*/
class CWX_API CwxThreadPool:public CwxTpi
{
public :
    ///���캯��
    CwxThreadPool(CWX_UINT16 unGroupId,///<�̳߳ص�thread-group
        CWX_UINT16 unThreadNum,///<�̳߳����̵߳�����
        CwxThreadPoolMgr* mgr, ///<�̵߳Ĺ������
        CwxCommander* commander,///<������Ϣ���ѵ�ȱʡcommander����ָ��func���Բ�ָ��
        CWX_TSS_THR_FUNC func=NULL, ///<�û����߳�main����
        void*            arg=NULL
        );
    ///��������
    ~CwxThreadPool();
public:
    /**
    @brief �����̳߳�
    @param [in] pThrEnv �̳߳ص��߳�Tss�����飬����ָ������ͨ��onThreadCreated������
    @param [in] stack_size �̶߳�ջ�Ĵ�С����Ϊ0�������ϵͳĬ�ϴ�С��
    @return -1��ʧ�ܣ� 0���ɹ�
    */
    virtual int start(CwxTss** pThrEnv=NULL, size_t stack_size= 0);
    ///ֹͣ�̳߳�
    virtual void stop();
    ///check thread �Ƿ�ֹͣ������Ҫ�ı���Ĺ��������ش�API
    virtual bool isStop();
    ///��ȡ�̵߳�TSS����Thread env
    virtual CwxTss* getTss(CWX_UINT16 unThreadIndex);
    ///��ס�����̳߳ء�����ֵ0���ɹ���-1��ʧ��
    virtual int lock();
    ///��������̳߳ء�����ֵ0���ɹ���-1��ʧ��
    virtual int unlock();
public:
    ///��ȡ�̵߳���Ϣ�����Ŷ���Ϣ��
    inline size_t getQueuedMsgNum();
    /**
    @brief ���̵߳���Ϣ�������һ������Ϣ��
    @param [in] pMsg append����Ϣ
    @return -1��ʧ�ܣ�>=0�������Ŷӵ���Ϣ����
    */
    inline int  append(CwxMsgBlock* pMsg);
    /**
    @brief ���̵߳���Ϣ����ͷ���һ������Ϣ���Ա㼰ʱ���ѡ�
    @param [in] pMsg append����Ϣ
    @return -1��ʧ�ܣ�>=0�������Ŷӵ���Ϣ����
    */
    int  appendHead(CwxMsgBlock* pMsg);

    /**
    @brief ���̵߳���Ϣ���л�ȡһ���Ŷ���Ϣ��������Ϊ�գ���������
    @param [out] pMsg pop����Ϣ��
    @return -1��ʧ�ܣ�>=0�������Ŷӵ���Ϣ����
    */
    inline int  pop(CwxMsgBlock*& pMsg);
private:
    ///ֹͣ�̳߳�
    void _stop();
protected:
    CwxMutexLock            m_lock;
    CwxTss**                m_arrTssEnv;///<�̵߳�tss
    CwxMsgQueue*           m_msgQueue; ///<����
    CwxCommander*          m_commander; ///<commander
    CWX_TSS_THR_FUNC       m_func; ///<�û�ָ����thread main function
    CwxThreadPoolMgr*      m_mgr; ///<�̵߳Ĺ������
    void*                  m_arg; ///<�̵߳Ĳ���
    CwxThread**            m_threadArr;  ///<thead������
};

CWINUX_END_NAMESPACE

#include "CwxThreadPool.inl"
#include "CwxPost.h"
#endif

