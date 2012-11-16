#ifndef __CWX_INET_ADDR_H__
#define __CWX_INET_ADDR_H__
/*
版权声明：
    本软件遵循GNU GPL V3（http://www.gnu.org/licenses/gpl.html），
    联系方式：email:cwinux@gmail.com；微博:http://t.sina.com.cn/cwinux
*/

/**
@file CwxINetAddr.h
@brief 网络的地址对象的定义文件。
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxNetMacro.h"
#include "CwxAddr.h"
#include "CwxSocket.h"
#include <netdb.h>

CWINUX_BEGIN_NAMESPACE
/**
@class CwxINetAddr
@brief 网络地址对象。
*/
class CWX_API CwxINetAddr:public CwxAddr
{
public:
    /// Default constructor.
    CwxINetAddr (void);
    /// Copy constructor.
    CwxINetAddr (const CwxINetAddr &);
    /// Creates an CwxINetAddr from a sockaddr_in structure.
    CwxINetAddr (const sockaddr_in *addr, int len);
    /// Creates an CwxINetAddr from a @a unPort and the remote
    /// @a szHost. The port number is assumed to be in host byte order.
    /// To set a port already in network byte order, please @see set().
    /// Use iFamily to select IPv6 (PF_INET6) vs. IPv4 (PF_INET).
    CwxINetAddr (CWX_UINT16 unPort, char const* szHost, CWX_INT32 iFamily = AF_UNSPEC);
    /**
    * Initializes an CwxINetAddr from the @a szAddr, which can be
    * "ip-number:port-number" (e.g., "tango.cs.wustl.edu:1234" or
    * "128.252.166.57:1234").  If there is no ':' in the @a szAddr it
    * is assumed to be a port number, with the IP address being
    * INADDR_ANY.
    */
    explicit CwxINetAddr (char const* szAddr, CWX_INT32 iFamily = AF_UNSPEC);

    /**
    * Creates an CwxINetAddr from a @a unPort and an Internet
    * @a uiIpAddr.  This method assumes that @a unPort and @a uiIpAddr
    * are in host byte order. If you have addressing information in
    * network byte order, @see set().
    */
    explicit CwxINetAddr (CWX_UINT16 unPort, CWX_UINT32 uiIpAddr = INADDR_ANY);

    /// Uses getservbyname() to create an CwxINetAddr from a
    /// szPortName, the remote @a szHostName, and the @a protocol.
    CwxINetAddr (char const* szPortName, char const* szHostName, char const* protocol = "tcp");

    /**
    * Uses getservbyname() to create an CwxINetAddr from a
    * szPortName, an Internet @a uiIpAddr, and the @a protocol.  This
    * method assumes that @a uiIpAddr is in host byte order.
    */
    CwxINetAddr (char const* szPortName, CWX_UINT32 uiIpAddr, char const* protocol = "tcp");
    /// Default dtor.
    ~CwxINetAddr (void);


    // These methods are useful after the object has been constructed.

    /// Initializes from another CwxINetAddr.
    int set (const CwxINetAddr&);
    /// Creates an CwxINetAddr from a sockaddr_in structure.
    int set (const sockaddr_in *,  int len);

    /**
    * Initializes an CwxINetAddr from a @a unPort and the
    * remote @a szHost.  If @a bEncode is true then @a unPort is
    * converted into network byte order, otherwise it is assumed to be
    * in network byte order already and are passed straight through.
    * address_family can be used to select IPv4/IPv6 if the OS has
    * IPv6 capability (CWX_HAS_IPV6 is defined). To specify IPv6, use
    * the value AF_INET6. To specify IPv4, use AF_INET.
    */
    int set (CWX_UINT16 unPort,
        char const* szHost,
        bool bEncode = true,
        CWX_INT32 iFamily = AF_UNSPEC);
    /**
    * Initializes an CwxINetAddr from the @a szAddr, which can be
    * "ip-number:port-number" (e.g., "tango.cs.wustl.edu:1234" or
    * "128.252.166.57:1234").  If there is no ':' in the @a address it
    * is assumed to be a port number, with the IP address being
    * INADDR_ANY.
    */
    int set (char const* szAddr, CWX_INT32  iFamily = AF_UNSPEC);

    /**
    * Initializes an CwxINetAddr from a @a unPort and an Internet
    * @a uiIpAddr.  If @a bEncode is true then the port number and IP address
    * are converted into network byte order, otherwise they are assumed to be
    * in network byte order already and are passed straight through.
    *
    * If bMap is true and IPv6 support has been compiled in,
    * then this address will be set to the IPv4-mapped IPv6 address of it.
    */
    int set (CWX_UINT16 unPort,
        CWX_UINT32 uiIpAddr = INADDR_ANY,
        bool bEncode = true,
        bool bMap = false);

    /// Uses getservbyname() to initialize an CwxINetAddr from a
    /// szPortName, the remote @a szHostName, and the @a protocol.
    int set (char const* szPortName,
        char const* szHostName,
        char const* protocol = "tcp");

    /**
    * Uses getservbyname() to initialize an CwxINetAddr from a
    * szPortName, an @a uiIpAddr, and the @a protocol.  This assumes that
    * @a uiIpAddr is already in network byte order.
    */
    int set (char const* szPortName,
        CWX_UINT32 uiIpAddr = INADDR_ANY,
        char const* protocol = "tcp");

    /// Return a pointer to the underlying network address.
    virtual void *getAddr (void) const;
    /// Set a pointer to the address.
    virtual void setAddr (void *, CWX_INT32 iLen);
    /// Set a pointer to the address.
    virtual void setAddr (void *, CWX_INT32 iLen, bool bMap);

    /**
    * Sets the port number without affecting the host name.  If
    * @a bEncode is enabled then @a unPort is converted into network
    * byte order, otherwise it is assumed to be in network byte order
    * already and are passed straight through.
    */
    void setPort (CWX_UINT16 unPort, bool bEncode = true);

    /**
    * Sets the address without affecting the port number.  If
    * @a bEncode is enabled then @a szIpAddr is converted into network
    * byte order, otherwise it is assumed to be in network byte order
    * already and are passed straight through.  The size of the address
    * is specified in the @a len parameter.
    * If @a bMap is non-zero, IPv6 support has been compiled in, and
    * @a szIpAddr is an IPv4 address, then this address is set to the IPv4-mapped
    * IPv6 address of it.
    */
    int setAddr (char const * szIpAddr,
        int len,
        bool bEncode = true,
        bool bMap = false);
#if  (CWX_HAS_IPV6)
    /**
    * Sets the interface that should be used for this address. This only has
    * an effect when the address is link local, otherwise it does nothing.
    */
    int setInterface (char const * szIntfName);
#endif 

    /// Return the port number, converting it into host byte-order.
    CWX_UINT16 getPort(void) const;
    ///获取地址的类型
    int getAddrType() const;

    /**
    * Return the "dotted decimal" Internet address representation of
    * the hostname storing it in the @a szAddr (which is assumed to be
    * @a len bytes long).  
    */
    char const* getHostIp (char* szAddr, int len) const;


    /// Return the 4-byte IP address, converting it into host byte
    /// order.
    CWX_UINT32 getHostIp (void) const;

    /// Return @c true if the IP address is INADDR_ANY or IN6ADDR_ANY.
    bool isAny (void) const;

    /// Return @c true if the IP address is IPv4/IPv6 loopback address.
    bool isLoopback (void) const;

    /// Return @c true if the IP address is IPv4/IPv6 multicast address.
    bool isMulticast (void) const;

#if (CWX_HAS_IPV6)
    /// Return @c true if the IP address is IPv6 linklocal address.
    bool isLinklocal (void) const;

    /// Return @c true if the IP address is IPv4-mapped IPv6 address.
    bool isIpv4MapIpv6 (void) const;

    /// Return @c true if the IP address is IPv4-compatible IPv6 address.
    bool isIpv4CompatIpv6 (void) const;
#endif /* CWX_HAS_IPV6 */

    /// Compare two addresses for equality.  The addresses are considered
    /// equal if they contain the same IP address and port number.
    bool operator == (const CwxINetAddr &item) const;

    /// Compare two addresses for inequality.
    bool operator != (const CwxINetAddr &item) const;

private:
    /// Insure that @a hostname is properly null-terminated.
    int getHostName_i (char* hostname, size_t hostnamelen) const;
    // Methods to gain access to the actual address of
    // the underlying internet address structure.
    void * ip_addr_pointer (void) const;

    /// Initialize underlying inet_addr_ to default values
    void reset (void);

    /// Underlying representation.
    /// This union uses the knowledge that the two structures share the
    /// first member, sa_family (as all sockaddr structures do).
    union
    {
        sockaddr_in  in4_;
#if (CWX_HAS_IPV6)
        sockaddr_in6 in6_;
#endif /* CWX_HAS_IPV6 */
    } inet_addr_;
};

CWINUX_END_NAMESPACE

#include "CwxINetAddr.inl"
#include "CwxPost.h"

#endif
