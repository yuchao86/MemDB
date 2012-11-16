#ifndef __CWX_SOCK_CONNECTOR_H__
#define __CWX_SOCK_CONNECTOR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
/**
@file CwxSockConnector.h
@brief TCP�������ӵ�����������Ķ��塣
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
#include "CwxSockBase.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxSockConnector
@brief TCP�������ӵ�����������
*/
class CWX_API CwxSockConnector
{
public:
    ///Ĭ�Ϲ��캯��
    CwxSockConnector();
    ///��������.
    ~CwxSockConnector(void);
public:
    /**
    @brief ����TCP�������ӡ�
    @param [out] stream �������ӡ�
    @param [in] remoteAddr ���ӵ�Զ�̵�ַ��
    @param [in] localAddr ���ӵı��ص�ַ����Ϊ�գ�����connect�Լ����䡣
    @param [in] timeout ���ӵĳ�ʱʱ�䣬��NULL����ʾû�г�ʱ���ơ���ֵΪ0�����ʾ��û�������ϣ��������ء�
    @param [in] protocol ���ӵ�Э�飬����socket()��protocol����������һ��Э������socket ���ͣ���protocolҲ��Ψһ�ģ�Ϊ0�Ϳ����ˡ�
    @param [in] reuse_addr �Ƿ����ñ��ص�ַ��
    @param [in] fn socket�������õ�function��
    @return -1������errno��¼�����ԭ��0���ɹ���
    */
    int connect (CwxSockStream& stream,
        CwxAddr const& remoteAddr,
        CwxAddr const& localAddr=CwxAddr::sap_any,
        CwxTimeouter* timeout=0,
        int protocol = 0,
        bool reuse_addr = false,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL, ///<socket���õ�function
        void* fnArg=NULL
    );
    /**
    @brief ����������ɺ�����ӱ�־��
    @param [out] stream �������ӡ�
    @param [in] remoteAddr ���ӵ�Զ�̵�ַ������Ϊ�����ȡԶ�˵�ַ��
    @param [in] timeout ���ӵĳ�ʱʱ�䣬��NULL����ʾû�г�ʱ���ơ���ֵΪ0�����ʾ��û�������ϣ��������ء�
    @return -1������errno��¼�����ԭ��0���ɹ���
    */
    int complete (CwxSockStream &stream,
        CwxAddr *remoteAddr = 0,
        CwxTimeouter* timeout=0);

private:
};

CWINUX_END_NAMESPACE

#include "CwxSockConnector.inl"
#include "CwxPost.h"

#endif
