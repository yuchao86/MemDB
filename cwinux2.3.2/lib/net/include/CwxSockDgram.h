#ifndef __CWX_SOCK_DGRAM_H__
#define __CWX_SOCK_DGRAM_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxSockDgram.h
@brief UDP������ͨ�Ŷ���Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-11
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxTimeValue.h"
#include "CwxNetMacro.h"
#include "CwxINetAddr.h"
#include "CwxSockBase.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxSockDgram
@brief �������UDPͨ�Ŷ���
*/
class CWX_API CwxSockDgram : public CwxSockBase
{
public:
    ///Ĭ�Ϲ��캯��
    CwxSockDgram();
    ///���캯�������������趨����handle
    CwxSockDgram(CWX_HANDLE handle);
    ///��������.
    ~CwxSockDgram(void);
public:
    /**
    @brief ���޶���ʱ���ڣ�����len�ֽڵ����ݡ�
    @param [in] buf �������ݵ�buf��
    @param [in] len �������ݵ��ֽ�����
    @param [in] addr ���͵ĵ�ַ��
    @param [in] flags sendto()��ϵͳOS api��flags������
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����ʹ����ʱ(errno==ETIME��������Ϊ���͵��ֽ���������len
    */
    ssize_t send(const void *buf,  size_t len,  const CwxAddr &addr, int flags, CwxTimeouter  *timeout=0) const;
    /**
    @brief ���޶���ʱ���ڣ�����һ�����ݰ���
    @param [in] buf �������ݵ�buf��
    @param [in] len ����BUF�Ĵ�С��
    @param [in] addr ���ݵ���Դ�ĵ�ַ��
    @param [in] flags recvfrom()��ϵͳOS api��flags������
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����մ����ʱ(errno==ETIME��������Ϊ���յ��ֽ���
    */
    ssize_t recv (void *buf,  size_t len,  CwxAddr &addr, int flags, CwxTimeouter  *timeout=0) const;
private:
    /**
    * Return the address of the remotely connected peer (if there is
    * one), in the referenced CwxAddr. Returns 0 if successful, else
    * -1.
    */
    int getRemoteAddr (CwxAddr &) const;
};

CWINUX_END_NAMESPACE

#include "CwxSockDgram.inl"
#include "CwxPost.h"

#endif
