#ifndef __CWX_SOCK_IO_H__
#define __CWX_SOCK_IO_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxSockIo.h
@brief CwxSockIo��Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2010-06-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxTimeValue.h"
#include "CwxSockBase.h"
CWINUX_BEGIN_NAMESPACE

/**
* @class CwxSockIo 
*
* @brief  Defines the methods for the  socket wrapper I/O routines��
* (e.g., send/recv).
* 
*/
class CWX_API CwxSockIo : public CwxSockBase
{
public:

public:
    // = Initialization and termination methods.

    /// Constructor.
    CwxSockIo (void);

    /// Destructor.
    ~CwxSockIo (void);

    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��recv�ӿڵķ�װ
    @param [in] buf �������ݵ�buf
    @param [in] n buf�ĳ���
    @param [in] flags os��recv api��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    ssize_t recv (void *buf,
        size_t n,
        int flags,
        CwxTimeouter *timeout = 0) const;

    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��read�ӿڵķ�װ
    @param [in] buf �������ݵ�buf
    @param [in] n buf�ĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    ssize_t recv (void *buf,
        size_t n,
        CwxTimeouter *timeout = 0) const;

    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��readv�ӿڵķ�װ
    @param [in] iov �������ݵĿռ������
    @param [in] n ����Ĵ�С
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    ssize_t recvv (iovec iov[],
        int n,
        CwxTimeouter  *timeout = 0) const;
    /**
    @brief ������len�ֽڵ����ݡ�
    @param [out] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    ssize_t read (void *buf, size_t len, CwxTimeouter  *timeout = 0) const;

    /**
    @brief ��෢��len�ֽڵ����ݡ�
    @param [in] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    ssize_t write (const void *buf, size_t len, CwxTimeouter  *timeout = 0) const;

    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��send�ӿڵķ�װ
    @param [in] buf ���ݵ�buf
    @param [in] n ���ݵĳ���
    @param [in] flags os��send api��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    ssize_t send (const void *buf,
        size_t n,
        int flags,
        CwxTimeouter  *timeout = 0) const;

    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��write�ӿڵķ�װ
    @param [in] buf �������ݵ�buf
    @param [in] n �������ݵĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊд�ֽڵ�����
    */
    ssize_t send (const void *buf,
        size_t n,
        CwxTimeouter  *timeout = 0) const;

    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��writev�ӿڵķ�װ
    @param [in] iov ���ݵ�buf����
    @param [in] n ����Ĵ�С
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    ssize_t sendv (const iovec iov[],
        int n,
        CwxTimeouter  *timeout = 0) const;
};


CWINUX_END_NAMESPACE

#include "CwxSockIo.inl"
#include "CwxPost.h"

#endif
