#ifndef __CWX_SOCK_ACCEPTOR_H__
#define __CWX_SOCK_ACCEPTOR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
/**
@file CwxSockAcceptor.h
@brief TCP�������ӽ��ն���Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-11
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxErrGuard.h"
#include "CwxNetMacro.h"
#include "CwxINetAddr.h"

#include "CwxSockStream.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxSockAcceptor
@brief TCP�������ӽ��ն���
*/
class CWX_API CwxSockAcceptor:public CwxSockBase
{
public:
    enum{
        DEF_BACK_LOG = 64
    };
    ///Ĭ�Ϲ��캯��
    CwxSockAcceptor();
    ///�򿪼����˿ڵĹ��캯��
    CwxSockAcceptor(CwxAddr const& addr,
        bool reuse= 0,
        int backlog = DEF_BACK_LOG,
        int domain = PF_INET,
        int protocol = 0,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* fnArg = NULL);
    ///��������.
    ~CwxSockAcceptor(void);
public:
    /**
    @brief ����TCP�������ӡ�
    @param [in] addr Accept�ı��ص�ַ��
    @param [in] reuse ��������ַû���ͷŵĻ����Ƿ�re-use��true���ǣ�false�����ǡ�
    @param [in] backlog listen����Ŷӵ�������
    @param [in] domain Э���壬����socket()�е�domain��PF_UNSPEC��ʾ����addr�е�Э���塣
    @param [in] protocol ���ӵ�Э�飬����socket()��protocol����������һ��Э������socket ���ͣ���protocolҲ��Ψһ�ģ�Ϊ0�Ϳ����ˡ�
    @param [in] fn socket ���õ�function��
    @return -1������errno��¼�����ԭ��0���ɹ���
    */
    int listen(CwxAddr const& addr,
        bool reuse= 0,
        int backlog = DEF_BACK_LOG,
        int domain = PF_INET,
        int protocol = 0,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* fnArg=NULL);
    /**
    @brief ����һ������TCP���ӡ�
    @param [in] stream ���ؽ��յ��ı������ӡ�
    @param [in] remote_addr ����Ϊ�գ��򷵻ؽ��յ��ĶԶ�host�ĵ�ַ��
    @param [in] timeout accept�ĳ�ʱʱ�䣬��ΪNULL�������޵ȴ���
    @param [in] bRestart ���ڵȴ������б��ź��жϣ��Ƿ�����ȴ���true�������ȴ���false�����ȴ���
    @return -1������errno��¼�����ԭ��0���ɹ���
    */
    int accept (CwxSockStream &stream,
        CwxAddr* remote_addr = 0,
        CwxTimeouter* timeout = NULL,
        bool bRestart = true) const;
public:
    int close (void);
private:
    int getRemoteAddr(CwxAddr &) const;
};

CWINUX_END_NAMESPACE

#include "CwxSockAcceptor.inl"
#include "CwxPost.h"

#endif
