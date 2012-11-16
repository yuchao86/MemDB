#ifndef __CWX_LOCK_GUARD_H__
#define __CWX_LOCK_GUARD_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxLockGuard.h
*@brief CWINUX����������д����ģ�����������CwxMutexGuard
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-05-30
*@warning  ��.
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxMutexGuard
*
* @brief �����������ڹ��캯���м����������������н���
*/
template <class LOCK>
class CwxMutexGuard
{
public:
    ///���캯������lock��Ϊ�գ������
    CwxMutexGuard (LOCK * lock):m_pLock(lock)
    {
        if (m_pLock) m_pLock->acquire();
    }
    ///������������������Ϊ�գ������
    ~CwxMutexGuard ()
    {
        if (m_pLock) m_pLock->release();
    }
private:
    ///��ֹ�����������Ĭ�Ϲ��캯��.
    CwxMutexGuard (): m_pLock (0) {}
    ///�������ָ��
    LOCK*      m_pLock;

};


/**
* @class CwxReadLockGuard
*
* @brief This data structure is meant to be used within a method or
* function...  It performs automatic aquisition and release of
* a parameterized synchronization object \<LOCK\>.
*
* The \<LOCK\> class given as an actual parameter must provide at
* the very least the \<acquire_read\>,  \<release\> method
*/
template <class LOCK>
class CwxReadLockGuard
{
public:
    // = Initialization and termination methods.
    CwxReadLockGuard (LOCK * lock):m_pLock(lock){
        if (m_pLock) m_pLock->acquire_read();
    }
    /// Implicitly release the lock.
    ~CwxReadLockGuard (){
        if (m_pLock) m_pLock->release();
    }
private:
    /// Helper, meant for subclass only.
    CwxReadLockGuard (): m_pLock (0) {}
    /// Pointer to the LOCK we're guarding.
    LOCK*      m_pLock;

};

/**
* @class CwxWriteLockGuard
*
* @brief This data structure is meant to be used within a method or
* function...  It performs automatic aquisition and release of
* a parameterized synchronization object \<LOCK\>.
*
* The \<LOCK\> class given as an actual parameter must provide at
* the very least the \<acquire_write\>,  \<release\> method
*/
template <class LOCK>
class CwxWriteLockGuard
{
public:
    // = Initialization and termination methods.
    CwxWriteLockGuard (LOCK * lock):m_pLock(lock){
        if (m_pLock) m_pLock->acquire_write();
    }
    /// Implicitly release the lock.
    ~CwxWriteLockGuard (){
        if (m_pLock) m_pLock->release();
    }
private:
    /// Helper, meant for subclass only.
    CwxWriteLockGuard (): m_pLock (0) {}
    /// Pointer to the LOCK we're guarding.
    LOCK*      m_pLock;

};
CWINUX_END_NAMESPACE

#include "CwxPost.h"
#endif

