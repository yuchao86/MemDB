#ifndef __CWX_RW_LOCK_H__
#define __CWX_RW_LOCK_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxRwLock.h
@brief ʵ���˶�д���Ľӿڡ�
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>

CWINUX_BEGIN_NAMESPACE

/**
@class CwxRwLock
@brief ��д������֧���̶߳�д������̶�д����
*/
class CWX_API CwxRwLock
{
public:
    ///���캯��������һ���̶߳�д��
    CwxRwLock(pthread_rwlockattr_t *arg = NULL);
    ///���캯��������һ�����̼��Ķ�д��
///    CwxRwLock(char* path, void *arg = NULL);
    ///��������
    ~CwxRwLock();
public:
    ///��ȡд����0���ɹ���-1��ʧ��
    int acquire();
    ///����Ƿ�ɼ�д����0���ɣ�-1����
    int tryacquire();
    ///�Ƿ�����0���ɹ���-1��ʧ��
    int release();
    ///��ȡ������0���ɹ���-1��ʧ��
    int acquire_read();
    ///��ȡд����0���ɹ���-1��ʧ��
    int acquire_write();
    ///����Ƿ�ɼӶ�����0���ɣ�-1����
    int tryacquire_read();
    ///����Ƿ�ɼ�д����0���ɣ�-1����
    int tryacquire_write();
    ///��ȡ�ڲ������������ΪOS�������
    pthread_rwlock_t const &lock() const;
private:
    pthread_rwlock_t*  m_rwLock;///<��д�����
};


CWINUX_END_NAMESPACE
#include "CwxRwLock.inl"
#include "CwxPost.h"

#endif
