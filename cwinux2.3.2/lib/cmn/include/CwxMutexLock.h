#ifndef __CWX_MUTEX_LOCK_H__
#define __CWX_MUTEX_LOCK_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxMutexLock.h
@brief ʵ�����������Ľӿڡ�
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
CWINUX_BEGIN_NAMESPACE
/**
@class CwxMutexLock
@brief ����������֧���߳��������������������
*/
class CWX_API CwxMutexLock
{
public:
    ///���캯��������һ���߳�������
    CwxMutexLock(pthread_mutexattr_t *arg = 0);
    ///���캯��������һ������������
//    CwxMutexLock(char const* path, pthread_mutexattr_t *arg = 0);
    ///��������
    ~CwxMutexLock();
public:
    ///��ȡ��������0���ɹ���-1��ʧ��
    int acquire();
    ///����������Ƿ���У�0�����У�-1��æ
    int tryacquire();
    ///�ͷ�����0���ɹ���-1��ʧ��
    int release();
    ///��ȡ��������0���ɹ���-1��ʧ��
    int acquire_read();
    ///��ȡ��������0���ɹ���-1��ʧ��
    int acquire_write();
    ///����������Ƿ���У�0�����У�-1��æ
    int tryacquire_read();
    ///����������Ƿ���У�0�����У�-1��æ
    int tryacquire_write();
    ///��ȡ�������ΪOS�������������
    pthread_mutex_t &lock() ;
private:
    pthread_mutex_t*  m_mutex;///<������
    int              m_shm;///<��������shm fd

};


CWINUX_END_NAMESPACE
#include "CwxMutexLock.inl"
#include "CwxPost.h"
#endif

