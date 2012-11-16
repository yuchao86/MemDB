#ifndef __CWX_SOCK_STREAM_H__
#define __CWX_SOCK_STREAM_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxSockStream.h
@brief TCP������ͨ�����Ӷ���Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxMsgHead.h"
#include "CwxMsgBlock.h"
#include "CwxNetMacro.h"
#include "CwxINetAddr.h"
#include "CwxSockIo.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxSockStream
@brief ��������TCP���Ӷ���
*/
class CWX_API CwxSockStream:public CwxSockIo
{
public:
    ///Ĭ�Ϲ��캯��
    CwxSockStream();
    ///���캯�������������趨����handle
    CwxSockStream(CWX_HANDLE handle);
    ///��������.
    ~CwxSockStream(void);
public:
    /**
    @brief ����len�ֽڵ����ݡ�
    @param [out] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] flags recv()��ϵͳOS api��flags������
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] bytes_transferred ����Ϊ0���򷵻�ʵ���Ѿ����յ�����������������ʧ�ܵ�����¡�
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t recv_n (void *buf, size_t len, int flags, CwxTimeouter  *timeout = 0, size_t* bytes_transferred = 0) const;
    /**
    @brief ����len�ֽڵ����ݡ�
    @param [out] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] bytes_transferred ����Ϊ0���򷵻�ʵ���Ѿ����յ�����������������ʧ�ܵ�����¡�
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t recv_n (void *buf, size_t len, CwxTimeouter  *timeout = 0, size_t *bytes_transferred = 0) const;

    /**
    @brief ����len�ֽڵ����ݡ�
    @param [out] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] bytes_transferred ����Ϊ0���򷵻�ʵ���Ѿ����յ�����������������ʧ�ܵ�����¡�
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t read_n (void *buf, size_t len, CwxTimeouter  *timeout = 0, size_t *bytes_transferred = 0) const;

    /**
    @brief һ��cwinux�����ݰ���
    @param [out] head ���ݰ��İ�ͷ��
    @param [out] msg ���յ����ݰ�����ʧ�ܣ��򷵻�NULL��
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t read (CwxMsgHead& head, CwxMsgBlock*& msg, CwxTimeouter  *timeout = 0) const;
    /**
    @brief һ��cwinux�����ݰ���
    @param [out] head ���ݰ��İ�ͷ��
    @param [out] msg ���յ����ݰ�����ʧ�ܣ��򷵻�NULL��
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t recv (CwxMsgHead& head, CwxMsgBlock*& msg, CwxTimeouter  *timeout = 0) const;


    /**
    @brief ����len�ֽڵ����ݡ�
    @param [in] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] bytes_transferred ����Ϊ0���򷵻�ʵ���Ѿ����͵�����������������ʧ�ܵ�����¡�
    @return -1�����ʹ���>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t write_n (const void *buf, size_t len, CwxTimeouter  *timeout = 0, size_t *bytes_transferred = 0) const;

    /**
    @brief ����len�ֽڵ����ݡ�
    @param [in] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] flags send()��ϵͳOS api��flags������
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] bytes_transferred ����Ϊ0���򷵻�ʵ���Ѿ����͵�����������������ʧ�ܵ�����¡�
    @return -1�����ʹ���>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t send_n (const void *buf, size_t len, int flags, CwxTimeouter  *timeout = 0, size_t *bytes_transferred = 0) const;

    /**
    @brief ����len�ֽڵ����ݡ�
    @param [in] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] bytes_transferred ����Ϊ0���򷵻�ʵ���Ѿ����͵�����������������ʧ�ܵ�����¡�
    @return -1�����ʹ���>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    ssize_t send_n (const void *buf, size_t len, CwxTimeouter  *timeout = 0,  size_t *bytes_transferred = 0) const;
    /**
    @brief ����urgent���ݡ�
    @param [out] ptr Ҫ���͵�urgent���ݡ�
    @param [in] len urgent���ݵĳ��ȡ�
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����ʹ���>0������len��
    */
    ssize_t send_urg (const void *ptr, size_t len = sizeof (char), CwxTimeouter  *timeout = 0) const;
    /**
    @brief ����urgent���ݡ�
    @param [out] ptr ����urgent���ݵ�buf��
    @param [in] len ����urgent���ݵĳ��ȡ�
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����մ���0�����ӹرգ�>0������len��
    */
    ssize_t recv_urg (void *ptr, size_t len = sizeof (char), CwxTimeouter  *timeout = 0) const;
    /// �ر����ӵĶ�������ֵ��0���ɹ���-1��ʧ�ܡ�
    int close_reader (void);
    /// �ر�����д������ֵ��0���ɹ���-1��ʧ�ܡ�
    int close_writer (void);
private:
};

CWINUX_END_NAMESPACE

#include "CwxSockStream.inl"
#include "CwxPost.h"

#endif
