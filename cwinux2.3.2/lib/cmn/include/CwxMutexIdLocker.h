#ifndef __CWX_MUTEX_ID_LOCKER_H__
#define __CWX_MUTEX_ID_LOCKER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxMutexIdLocker.h
*@brief ʵ�ֻ���Id��������������ɶ�һ��64λ���ֵļ���
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-05-30
*@warning  ��.
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "CwxSingleton.h"

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxMutexIdLockMgr
*
* @brief 64λ���ֵ�����ID������������
*/
class CWX_API CwxMutexIdLockMgr : public CwxSingleton
{
public:
    enum{
        MAX_FREE_MUTEX_NUM = 32,///<��������������
        HASH_BUCKET_SIZE = 2048///<��HASH�Ĵ�С
    };
    ///ID��д������
    class CwxMutexObj
    {
    public:
        CwxMutexObj():m_unNum(0)
        {

        }
        ~CwxMutexObj()
        {

        }
    public:
        CwxMutexLock        m_lock;///<��д��
        CWX_UINT16       m_unNum;///<�ȴ���д�����߳�����
    };
public:
public:
    ///����signleton CwxRwIdLockMgr ����
    static CwxMutexIdLockMgr* instance();
public:
    ///��ID��������, 0���ɹ��� -1��ʧ��
    int lock(CWX_UINT64  id);
    ///��ID����, 0���ɹ��� -1��ʧ��
    int unlock(CWX_UINT64 id);
private:
    ///ID�������ĳ�ʼ��
    void  init();
    ///��ֹ�ⲿ��������ʵ��
    CwxMutexIdLockMgr();
    ///��ֹ�ⲿɾ������
    virtual ~CwxMutexIdLockMgr();

private:
    static CwxMutexIdLockMgr* m_pInstance; ///<��ʵ�����
    static CwxMutexLock   m_lock;///<��ʵ���ı�����
    CWX_UINT16        m_unFreeMutexNum;///<������������
    list<CwxMutexObj*>*  m_pListLocks;///<���������б�
    CwxMutexLock      m_mutex;///<ȫ����Ϣ������ͬ����
    hash_map<CWX_UINT64, CwxMutexObj* /*lock*/, CwxNumHash<CWX_UINT64> >*   m_pLockMap;///<����hash
};


/**
* @class CwxMutexIdLocker
*
* @brief ��һ��64λ���֣�����������
*/
class CWX_API CwxMutexIdLocker
{
public:
public:
    ///��id��������
    CwxMutexIdLocker(CWX_UINT64 id)
    {
        m_ullLockID = id;
        CwxMutexIdLockMgr::instance()->lock(m_ullLockID);
    }
    ///ͨ��������������ɶ�ID���������ͷ�.
    ~CwxMutexIdLocker()
    {
        CwxMutexIdLockMgr::instance()->unlock(m_ullLockID);
    }
private:
    CWX_UINT64              m_ullLockID;///<������ID
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif
