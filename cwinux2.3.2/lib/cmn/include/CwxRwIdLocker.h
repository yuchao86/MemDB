#ifndef __CWX_RW_ID_LOCKER_H__
#define __CWX_RW_ID_LOCKER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxRwIdLocker.h
*@brief ʵ�ֻ���Id�Ķ�д��������ɶ�һ��64λ���ֵļ���
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
#include "CwxRwLock.h"
#include "CwxLockGuard.h"
#include "CwxSingleton.h"

CWINUX_BEGIN_NAMESPACE
/**
* @class CwxRwIdLockMgr
*
* @brief 64λ���ֵĶ�дID������������
*/
class CWX_API CwxRwIdLockMgr : public CwxSingleton
{
public:
    enum{
        MAX_FREE_MUTEX_NUM = 32,///<��������������
        HASH_BUCKET_SIZE = 2048///<��HASH�Ĵ�С
    };
    ///ID��д������
    class CwxRwObj{
    public:
        CwxRwObj():m_unNum(0)
        {

        }
        ~CwxRwObj()
        {

        }
    public:
        CwxRwLock        m_lock;///<��д��
        CWX_UINT16       m_unNum;///<�ȴ���д�����߳�����
    };
public:
public:
    ///����signleton CwxRwIdLockMgr ����
    static CwxRwIdLockMgr* instance();
public:
    ///��ID�Ӷ���, 0���ɹ��� -1��ʧ��
    int rdLock(CWX_UINT64  id);
    ///��ID��д��, 0���ɹ��� -1��ʧ��
    int wrLock(CWX_UINT64  id);
    ///��ID����, 0���ɹ��� -1��ʧ��
    int unlock(CWX_UINT64 id);
private:
    ///ID�������ĳ�ʼ��
    void  init();
    ///��ֹ�ⲿ��������ʵ��
    CwxRwIdLockMgr();
    ///��ֹ�ⲿɾ������
    virtual ~CwxRwIdLockMgr();

private:
    static CwxRwIdLockMgr* m_pInstance; ///<��ʵ�����
    static CwxMutexLock   m_lock;///<��ʵ���ı�����
    CWX_UINT16        m_unFreeMutexNum;///<������������
    list<CwxRwObj*>*  m_pListLocks;///<���������б�
    CwxMutexLock      m_mutex;///<ȫ����Ϣ������ͬ����
    hash_map<CWX_UINT64, CwxRwObj* /*lock*/, CwxNumHash<CWX_UINT64> >*   m_pLockMap;///<����hash
};

/**
* @class CwxRwIdLocker
*
* @brief ��һ��64λ���֣��Ӷ�д����
*/
class CWX_API CwxRwIdLocker
{
public:
    ///��id�Ӷ�д����bWriteΪtrue��ʾ��д��������Ϊ����
    CwxRwIdLocker(CWX_UINT64 id, bool bWrite=true)
    {
        m_ullLockID = id;
        if (bWrite)
            CwxRwIdLockMgr::instance()->wrLock(m_ullLockID);
        else
            CwxRwIdLockMgr::instance()->rdLock(m_ullLockID);
    }
    ///ͨ��������������ɶ�ID��д�����ͷ�.
    ~CwxRwIdLocker()
    {
        CwxRwIdLockMgr::instance()->unlock(m_ullLockID);
    }
private:
    CWX_UINT64              m_ullLockID;///<������ID
};

CWINUX_END_NAMESPACE

#include "CwxRwIdLocker.inl"
#include "CwxPost.h"

#endif
