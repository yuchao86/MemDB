#ifndef __CWX_SOCKET_H__
#define __CWX_SOCKET_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxSocket.h
@brief ����socket API�ķ�װ�ࡣ
@author cwinux@gmail.com
@version 0.1
@date 2010-5-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxTimeValue.h"
#include "CwxMsgHead.h"
#include "CwxMsgBlock.h"
#include "CwxNetMacro.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
CWINUX_BEGIN_NAMESPACE
/**
@class CwxSocket
@brief Socket API�ķ�װ�ࡣ
*/
class CWX_API CwxSocket
{
public:
    /**
    @brief ��װ��setsockopt api������socket handle�Ĳ�����
    @param [in] handle socket ��handle��
    @param [in] level setsockopt��level
    @param [in] option setsockopt��option
    @param [in] optval setsockopt��optval
    @param [in] optlen setsockopt��optlen��
    @return -1��failure��0���ɹ�����setsockoptһ��
    */
    static int setOption (CWX_HANDLE handle, int level,
        int option,
        void *optval,
        socklen_t optlen);

    /**
    @brief �򿪡��ر����ӵ�keep-alive���ء�
    @param [in] handle socket ��handle��
    @param [in] bKeepalive ��keep-alive
    @param [in] idle ��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��(second��
    @param [in] inter ����KeepAlive̽����ʱ����(second)
    @param [in] count �ж��Ͽ�ǰ��KeepAlive̽�������
    @return -1��failure��0���ɹ�����setsockoptһ��
    */
    static int setKeepalive(CWX_HANDLE handle,
        bool bKeepalive,
        int idle,
        int inter,
        int count);

    /**
    @brief ��װ��getsockopt api����ȡsocket handle�Ĳ�����
    @param [in] handle socket ��handle��
    @param [in] level setsockopt��level
    @param [in] option setsockopt��option
    @param [in] optval setsockopt��optval
    @param [in] optlen setsockopt��optlen��
    @return -1��failure��0���ɹ�����getsockoptһ��
    */
    static int getOption (CWX_HANDLE handle, int level,
        int option,
        void *optval,
        socklen_t *optlen);

    /**
    @brief ����д�¼��Ƿ�������select�ķ�װ��
    @param [in] width ����handle+1��
    @param [in] rfds ����handle����
    @param [in] wfds д��handle����
    @param [in] efds exeption��handle����
    @param [in] timeout timeout�Ķ�ʱ������ΪNULL��ʾ������
    @param [in] bRestart ���ڵȴ������б��ź��жϣ��Ƿ�����ȴ���true�������ȴ���false�����ȴ���
    @return -1��failure��0����ʱ��>0��ready��handle��������selectһ�¡�
    */
    static int select (int width,
        fd_set *rfds,
        fd_set *wfds = NULL,
        fd_set *efds = NULL,
        CwxTimeouter  *timeout = NULL,
        bool bRestart = true
        );
    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��read�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len buf�ĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t read (CWX_HANDLE handle,
        void *buf,
        size_t len,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��recv�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len buf�ĳ���
    @param [in] flags os��recv api��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recv (CWX_HANDLE handle,
        void *buf,
        size_t len,
        int flags,
        CwxTimeouter *timeout = NULL);
    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��read�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len buf�ĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recv (CWX_HANDLE handle,
        void *buf,
        size_t len,
        CwxTimeouter *timeout = NULL);
    /**
    @brief �������Ͻ���n���ֽڡ��Ƕ�OS��read�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len buf�ĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] accepted ʧ��ʱ��ȡ���ֽڵ�������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t read_n (CWX_HANDLE handle,
        void *buf,
        size_t len,
        CwxTimeouter *timeout = NULL,
        size_t* accepted=NULL);

    /**
    @brief ��len���ֽڡ��Ƕ�OS��recv�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len ��ȡ���ݵĳ���
    @param [in] flags os��recv api��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] accepted ʧ��ʱ��ȡ���ֽڵ�������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recv_n (CWX_HANDLE handle,
        void *buf,
        size_t len,
        int flags,
        CwxTimeouter *timeout = NULL,
        size_t* accepted=NULL);

    /**
    @brief ��len���ֽڡ��Ƕ�OS��read�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len ��ȡ���ݵĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] accepted ʧ��ʱ��ȡ���ֽڵ�������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recv_n (CWX_HANDLE handle,
        void *buf,
        size_t len,
        CwxTimeouter *timeout = NULL,
        size_t* accepted=NULL);
    /**
    @brief һ��cwinux�����ݰ���
    @param [in] handle ���ľ����
    @param [out] head ���ݰ��İ�ͷ��
    @param [out] msg ���յ����ݰ�����ʧ�ܣ��򷵻�NULL��
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    static ssize_t read (CWX_HANDLE handle,
        CwxMsgHead& head,
        CwxMsgBlock*& msg,
        CwxTimeouter  *timeout = 0) ;
    /**
    @brief һ��cwinux�����ݰ���
    @param [in] handle ���ľ����
    @param [out] head ���ݰ��İ�ͷ��
    @param [out] msg ���յ����ݰ�����ʧ�ܣ��򷵻�NULL��
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1�����մ���0�����ӹرգ�>0��Ӧ�õ���len����ʾ�ɹ�������len���ȵ��ֽ�����
    */
    static ssize_t recv (CWX_HANDLE handle,
        CwxMsgHead& head,
        CwxMsgBlock*& msg,
        CwxTimeouter  *timeout = 0) ;
    /**
    @brief �������Ͻ���һ��msg����Ϊ���������ӵġ��Ƕ�OS��recvmsg�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] msg �������ݵ�buf
    @param [in] flags recvmsg��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recvmsg(CWX_HANDLE handle,
        struct msghdr *msg,
        int flags,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ͻ���һ��msg����Ϊ���������ӵġ��Ƕ�OS��recvfrom�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len buf�Ŀռ�
    @param [in] flags recvfrom��flag
    @param [out] from ��Ϣ����Դ��ַ
    @param [out] fromlen ��Դ��ַ�ĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recvfrom (CWX_HANDLE handle,
        char *buf,
        int len,
        int flags,
        struct sockaddr *from,
        socklen_t *fromlen,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ͻ������ݡ��Ƕ�OS��readv�ӿڵķ�װ
    @param [in] handle ���ľ����
    @param [in] iov �������ݵĿռ������
    @param [in] iovcnt ����Ĵ�С
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ��0���Զ˹رգ�>0����ȡ���ֽ�����
    */
    static ssize_t recvv (CWX_HANDLE handle,
        iovec *iov,
        int iovcnt,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��write�ӿڵķ�װ
    @param [in] handle д�ľ����
    @param [in] buf ���ݵ�buf
    @param [in] len ���ݵĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t write (CWX_HANDLE handle,
        void const *buf,
        size_t len,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��send�ӿڵķ�װ
    @param [in] handle д�ľ����
    @param [in] buf ���ݵ�buf
    @param [in] len ���ݵĳ���
    @param [in] flags os��send api��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t send (CWX_HANDLE handle,
        void const *buf,
        size_t len,
        int flags,
        CwxTimeouter *timeout = NULL);
    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��write�ӿڵķ�װ
    @param [in] handle д�ľ����
    @param [in] buf �������ݵ�buf
    @param [in] len �������ݵĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊд�ֽڵ�����
    */
    static ssize_t send (CWX_HANDLE handle,
        void const *buf,
        size_t len,
        CwxTimeouter *timeout = NULL);
    /**
    @brief ����len���ֽڡ��Ƕ�OS��write�ӿڵķ�װ
    @param [in] handle д�ľ����
    @param [in] buf ���ݵ�buf
    @param [in] len ���ݵĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] sent ʧ��ʱ���͵��ֽڵ�������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t write_n (CWX_HANDLE handle,
        void const*buf,
        size_t len,
        CwxTimeouter *timeout = NULL,
        size_t* sent=NULL);
    /**
    @brief ����len���ֽڡ��Ƕ�OS��send�ӿڵķ�װ
    @param [in] handle д�ľ����
    @param [in] buf ���ݵ�buf
    @param [in] len ���ݵĳ���
    @param [in] flags os��send api��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] sent ʧ��ʱ���͵��ֽڵ�������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t send_n (CWX_HANDLE handle,
        void const*buf,
        size_t len,
        int flags,
        CwxTimeouter *timeout = NULL,
        size_t* sent=NULL);

    /**
    @brief ����len���ֽڡ��Ƕ�OS��write�ӿڵķ�װ
    @param [in] handle д�ľ����
    @param [in] buf ���ݵ�buf
    @param [in] len ���ݵĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [out] accepted ʧ��ʱ���͵��ֽڵ�������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t send_n (CWX_HANDLE handle,
        void const*buf,
        size_t len,
        CwxTimeouter *timeout = NULL,
        size_t* accepted=NULL);

    /**
    @brief �������Ϸ���һ��msg����Ϊ���������ӵġ��Ƕ�OS��sendmsg�ӿڵķ�װ
    @param [in] handle д�����
    @param [in] msg ���ݵ�buf
    @param [in] flags sendmsg��flag
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t sendmsg(CWX_HANDLE handle,
        struct msghdr const *msg,
        int flags,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ϸ������ݣ���Ϊ���������ӵġ��Ƕ�OS��sendto�ӿڵķ�װ
    @param [in] handle д�����
    @param [in] buf ���ݵ�buf
    @param [in] len ���ݵ�len
    @param [in] flags sendto��flag
    @param [in] to sendto�ĵ�ַ
    @param [in] tolen sendto�ĵ�ַ�ĳ���
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t sendto (CWX_HANDLE handle,
        char const *buf,
        int len,
        int flags,
        struct sockaddr *to,
        socklen_t tolen,
        CwxTimeouter *timeout = NULL);

    /**
    @brief �������Ϸ������ݡ��Ƕ�OS��writev�ӿڵķ�װ
    @param [in] handle д�����
    @param [in] iov ���ݵ�buf����
    @param [in] iovcnt ����Ĵ�С
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @return -1��failure��������ʱ������Ϊ���͵��ֽ�����
    */
    static ssize_t sendv (CWX_HANDLE handle,
        iovec const*iov,
        int iovcnt,
        CwxTimeouter *timeout = NULL);


    /**
    @brief ���һ��io handle�Ƿ����д������ready��
    @param [in] handle ���ľ����
    @param [in] timeout timeout��ֵ����ΪNULL��ʾ������
    @param [in] bRead �Ƿ����
    @param [in] bWrite �Ƿ���д
    @param [in] bException �Ƿ�������
    @param [in] bRestart ���ڵȴ������б��ź��жϣ��Ƿ�����ȴ���true�������ȴ���false�����ȴ���
    @return -1��failure��0����ʱ��>0��handle ready���¼�����
    */
    static int handleReady (CWX_HANDLE handle,
        CwxTimeouter  *timeout = NULL,
        bool bRead = true,
        bool bWrite = false,
        bool bException = false,
        bool bRestart = true);



    ///�Ƿ�֧��IPV6
    static bool isEnableIpv6(void);
    ///�Ƿ�֧��IPV4
    static bool isEnableIpv4(void);
    ///��ȡ����Ķ˿ں�, -1ʧ�ܡ�
    static int getSvrPort(char const* szSvrName, char const* szProtocol);
private:
    //forbiden new instance
    CwxSocket();
#if (CWX_HAS_IPV6)
    //����Ƿ�֧��ָ����Э����
    static int ipvn_check(int &ipvn_enabled, int pf);
#endif
private:
    static int  m_enableIpv6; ///<ipv6֧�ֵı�־, -1:δ��飻0����֧�֣�1:֧��
    static int  m_enableIpv4; ///<ipv4֧�ֵı�־, -1:δ��飻0����֧�֣�1��֧��
};

CWINUX_END_NAMESPACE

#include "CwxSocket.inl"
#include "CwxPost.h"

#endif
