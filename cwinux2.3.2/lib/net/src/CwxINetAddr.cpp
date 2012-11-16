#include "CwxINetAddr.h"
#include <stdlib.h>
CWINUX_BEGIN_NAMESPACE
/// Default constructor.
CwxINetAddr::CwxINetAddr(void)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
}

/// Copy constructor.
inline CwxINetAddr::CwxINetAddr (const CwxINetAddr& sa):CwxAddr(sa.getType(), sa.getSize())
{
    this->reset();
    this->set(sa);
}

/// Creates an CwxINetAddr from a sockaddr_in structure.
CwxINetAddr::CwxINetAddr (const sockaddr_in *addr, int len)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
    this->reset();
    this->set(addr, len);
}

/// Creates an CwxINetAddr from a @a unPort and the remote
/// @a szHost. The port number is assumed to be in host byte order.
/// To set a port already in network byte order, please @see set().
/// Use iFamily to select IPv6 (PF_INET6) vs. IPv4 (PF_INET).
CwxINetAddr::CwxINetAddr(CWX_UINT16 unPort, char const* szHost, CWX_INT32 iFamily)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
    memset (&this->inet_addr_, 0, sizeof (this->inet_addr_));
    this->set(unPort, szHost, 1, iFamily);
}

/**
* Initializes an CwxINetAddr from the @a szAddr, which can be
* "ip-number:port-number" (e.g., "tango.cs.wustl.edu:1234" or
* "128.252.166.57:1234").  If there is no ':' in the @a szAddr it
* is assumed to be a port number, with the IP address being
* INADDR_ANY.
*/
CwxINetAddr::CwxINetAddr (char const* szAddr, CWX_INT32 iFamily)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
    this->reset ();
    this->set(szAddr, iFamily);
}

/**
* Creates an CwxINetAddr from a @a unPort and an Internet
* @a uiIpAddr.  This method assumes that @a unPort and @a uiIpAddr
* are in host byte order. If you have addressing information in
* network byte order, @see set().
*/
CwxINetAddr::CwxINetAddr(CWX_UINT16 unPort, CWX_UINT32 uiIpAddr)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
    this->reset ();
    this->set (unPort, uiIpAddr);

}

/// Uses <getservbyname> to create an CwxINetAddr from a
/// <szPortName>, the remote @a szHostName, and the @a protocol.
CwxINetAddr::CwxINetAddr(char const* szPortName, char const* szHostName, char const* protocol)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
    this->reset ();
    this->set(szPortName, szHostName, protocol);
}

/**
* Uses <getservbyname> to create an CwxINetAddr from a
* <szPortName>, an Internet @a uiIpAddr, and the @a protocol.  This
* method assumes that @a uiIpAddr is in host byte order.
*/
CwxINetAddr::CwxINetAddr(char const* szPortName, CWX_UINT32 uiIpAddr, char const* protocol)
:CwxAddr(getAddrType(), sizeof(inet_addr_))
{
    this->reset();
    this->set(szPortName, CWX_HTONL (uiIpAddr), protocol);
}
/// Default dtor.
CwxINetAddr::~CwxINetAddr (void)
{
}

int CwxINetAddr::set(CWX_UINT16 unPort,
         char const* szHost,
         bool bEncode,
         CWX_INT32 iFamily)
{
    CWX_ASSERT(szHost);
    memset (&this->inet_addr_, 0, sizeof this->inet_addr_);

#if (CWX_HAS_IPV6)
    struct addrinfo hints;
    struct addrinfo *res = 0;
    int error = 0;
    memset (&hints, 0, sizeof (hints));
# if (CWX_IPV4_IPV6_MIGRATION)
    if (iFamily == AF_UNSPEC && !CwxSocket::isEnableIpv6())
        iFamily = AF_INET;
# endif /* CWX_IPV4_IPV6_MIGRATION */
    if (iFamily == AF_UNSPEC || iFamily == AF_INET6)
    {
        hints.ai_family = AF_INET6;
        error = ::getaddrinfo (szHost, 0, &hints, &res);
        if (error)
        {
            if (iFamily == AF_INET6)
            {
                if (res) ::freeaddrinfo(res);
                errno = error;
                return -1;
            }
            iFamily = AF_INET;
        }
    }
    if (iFamily == AF_INET)
    {
        hints.ai_family = AF_INET;
        error = ::getaddrinfo (szHost, 0, &hints, &res);
        if (error)
        {
            if (res) ::freeaddrinfo(res);
            errno = error;
            return -1;
        }
    }
    this->setType(res->ai_family);
    this->setAddr(res->ai_addr, res->ai_addrlen);
    this->setPort(unPort, bEncode);
    ::freeaddrinfo (res);
    return 0;
#else /* CWX_HAS_IPV6 */

    // IPv6 not supported... insure the family is set to IPv4
    iFamily = AF_INET;
    this->setType(iFamily);
    this->inet_addr_.in4_.sin_family = static_cast<short> (iFamily);
#if (CWX_HAS_SOCKADDR_IN_SIN_LEN)
    this->inet_addr_.in4_.sin_len = sizeof (this->inet_addr_.in4_);
#endif
    struct in_addr addrv4;
    if (inet_aton(szHost,  &addrv4) == 1)
    {
        return this->set (unPort,
        bEncode ? CWX_NTOHL (addrv4.s_addr) : addrv4.s_addr,
        bEncode);
    }
    else
    {
        struct hostent hentry;
        struct hostent *hp;
        char buf[sizeof(hentry) * 100];
        int h_error = 0;  // Not the same as errno!

        int result = gethostbyname_r (szHost, &hentry, buf, sizeof(buf), &hp, &h_error);
        if (result == -1)
        {
            errno = h_error;
            return -1;
        }
        if (hp == 0)
        {
            return -1;
        }
        else
        {
            memcpy ((void *) &addrv4.s_addr,
                hp->h_addr,
                hp->h_length);
            return this->set(unPort,
                bEncode ? CWX_NTOHL (addrv4.s_addr) : addrv4.s_addr,
                bEncode);
        }
    }
#endif /*CWX_HAS_IPV6 */

}

int CwxINetAddr::set(char const* szAddr, CWX_INT32  iFamily)
{
    int result;
    char *ip_buf = strdup(szAddr);
    char *ip_addr = ip_buf;
    // We use strrchr because of IPv6 addresses.
    char *port_p =strrchr (ip_addr, ':');
#if (CWX_HAS_IPV6)
    // Check for extended IPv6 format : '[' <ipv6 address> ']' ':' <port>
    if (ip_addr[0] == '[')
    {
        // find closing bracket
        char *cp_pos = strchr (ip_addr, ']');
        // check for port separator after closing bracket
        // if not found leave it, error will come later
        if (cp_pos)
        {
            *cp_pos = '\0'; // blank out ']'
            ++ip_addr; // skip over '['
            if (cp_pos[1] == ':')
                port_p = cp_pos + 1;
            else
                port_p = cp_pos; // leads to error on missing port
        }
    }
#endif /* CWX_HAS_IPV6 */
    if (port_p == 0) // Assume it's a port number.
    {
        char *endp = 0;
        long const port = strtol (ip_addr, &endp, 10);

        if (*endp == '\0')    // strtol scanned the entire string - all digits
        {
            if (port < 0 || port > CWX_MAX_DEFAULT_PORT)
                result = -1;
            else
                result = this->set (u_short (port), CWX_UINT32 (INADDR_ANY));
        }
        else// port name
        { 
            result = this->set (ip_addr, CWX_UINT32 (INADDR_ANY));
        }
    }
    else
    {
        *port_p = '\0'; ++port_p; // skip over ':'

        char *endp = 0;
        long port = strtol (port_p, &endp, 10);

        if (*endp == '\0')    // strtol scanned the entire string - all digits
        {
            if (port < 0 || port > CWX_MAX_DEFAULT_PORT)
                result = -1;
            else
                result = this->set(u_short (port), ip_addr, 1, iFamily);
        }
        else
            result = this->set (port_p, ip_addr);
    }

    free(ip_buf);
    return result;
}

int CwxINetAddr::set (CWX_UINT16 unPort, CWX_UINT32 uiIpAddr, bool bEncode, bool bMap)
{
    int ret = this->setAddr (reinterpret_cast<const char *> (&uiIpAddr),
        sizeof uiIpAddr,
        bEncode, bMap);
    this->setPort(unPort, bEncode);
    return ret;
    
}

int CwxINetAddr::set (char const* szPortName, char const* szHostName, char const* protocol)
{
    int port_number = CwxSocket::getSvrPort(szPortName, protocol);
    if (port_number == -1)
    {
        errno = ENOTSUP;
        return -1;
    }
    int address_family = AF_UNSPEC;
#  if (CWX_HAS_IPV6)
    if (strcmp (protocol, "tcp6") == 0)
        address_family = AF_INET6;
#  endif /* CWX_HAS_IPV6 */
    return this->set(static_cast<u_short> (port_number), szHostName, true, address_family);
}

int CwxINetAddr::set (char const* szPortName, CWX_UINT32 uiIpAddr, char const* protocol)
{
    int port_number = CwxSocket::getSvrPort(szPortName, protocol);
    if (port_number == -1)
    {
        errno = ENOTSUP;
        return -1;
    }
    return this->set(static_cast<u_short> (port_number), uiIpAddr, true, false);
}



/// Return a pointer to the underlying network address.
void * CwxINetAddr::getAddr (void) const
{
    return (void*)&this->inet_addr_;
}
/// Set a pointer to the address.
void CwxINetAddr::setAddr (void * addr, CWX_INT32 iLen)
{
    this->setAddr(addr, iLen, 0);
}
/// Set a pointer to the address.
void CwxINetAddr::setAddr (void *addr, CWX_INT32 , bool bMap)
{
    struct sockaddr_in *getfamily = static_cast<struct sockaddr_in *> (addr);

    if (getfamily->sin_family == AF_INET)
    {
#if (CWX_HAS_IPV6)
        if (bMap)
            this->setType (AF_INET6);
        else
#endif /* CWX_HAS_IPV6 */
        this->setType (AF_INET);
        this->setPort(getfamily->sin_port, 0);
        this->setAddr (reinterpret_cast<const char*> (&getfamily->sin_addr),
            sizeof (getfamily->sin_addr),
            0, bMap);
    }
#if (CWX_HAS_IPV6)
    else if (getfamily->sin_family == AF_INET6)
    {
        struct sockaddr_in6 *in6 = static_cast<struct sockaddr_in6*> (addr);
        this->setPort(in6->sin6_port, 0);
        this->setAddr(reinterpret_cast<const char*> (&in6->sin6_addr),
            sizeof (in6->sin6_addr),
            0);
        this->inet_addr_.in6_.sin6_scope_id = in6->sin6_scope_id;
    }
#endif // CWX_HAS_IPV6
}


int CwxINetAddr::setAddr (char const * szIpAddr, int len, bool bEncode, bool bMap)
{
    // This is really intended for IPv4. If the object is IPv4, or the type
    // hasn't been set but it's a 4-byte address, go ahead. If this is an
    // IPv6 object and <encode> is requested, refuse.
    if (bEncode && (len != 4))
    {
        errno = EAFNOSUPPORT;
        return -1;
    }

    if (len == 4)
    {
        CWX_UINT32 ip4 = *reinterpret_cast<const CWX_UINT32 *> (szIpAddr);
        if (bEncode) ip4 = CWX_HTONL (ip4);
        if (this->getType () == AF_INET && (bMap == 0)) {
            this->baseSet(AF_INET, sizeof (this->inet_addr_.in4_));
#if (CWX_HAS_SOCKADDR_IN_SIN_LEN)
            this->inet_addr_.in4_.sin_len = sizeof (this->inet_addr_.in4_);
#endif
            this->inet_addr_.in4_.sin_family = AF_INET;
            memcpy (&this->inet_addr_.in4_.sin_addr, &ip4, len);
        }
#if (CWX_HAS_IPV6)
        else if (!bMap)
        {
#if (CWX_HAS_SOCKADDR_IN_SIN_LEN)
            this->inet_addr_.in4_.sin_len = sizeof (this->inet_addr_.in4_);
#endif
            this->inet_addr_.in4_.sin_family = AF_INET;
            this->baseSet (AF_INET, sizeof (this->inet_addr_.in4_));
            memcpy (&this->inet_addr_.in4_.sin_addr, &ip4, len);
        }
        // If given an IPv4 address to copy to an IPv6 object, map it to
        // an IPv4-mapped IPv6 address.
        else
        {
            this->baseSet (AF_INET6, sizeof (this->inet_addr_.in6_));
#if (CWX_HAS_SOCKADDR_IN6_SIN6_LEN)
            this->inet_addr_.in6_.sin6_len = sizeof (this->inet_addr_.in6_);
#endif
            this->inet_addr_.in6_.sin6_family = AF_INET6;
            if (ip4 == CWX_HTONL (INADDR_ANY))
            {
                in6_addr const ip6 = in6addr_any;
                memcpy (&this->inet_addr_.in6_.sin6_addr, &ip6, sizeof (ip6));
                return 0;
            }
            // Build up a 128 bit address.  An IPv4-mapped IPv6 address
            // is defined as 0:0:0:0:0:ffff:IPv4_address.  This is defined
            // in RFC 1884 */
            memset (&this->inet_addr_.in6_.sin6_addr, 0, 16);
            this->inet_addr_.in6_.sin6_addr.s6_addr[10] =
                this->inet_addr_.in6_.sin6_addr.s6_addr[11] = 0xff;
            memcpy(&this->inet_addr_.in6_.sin6_addr.s6_addr[12], &ip4, 4);
        }
#endif /* CWX_HAS_IPV6 */
        return 0;
    }   /* end if (len == 4) */
#if (CWX_HAS_IPV6)
    else if (len == 16)
    {
        if (this->getType () != PF_INET6)
        {
            errno = EAFNOSUPPORT;
            return -1;
        }
        // We protect ourselves up above so IPv6 must be possible here.
        this->baseSet (AF_INET6, sizeof (this->inet_addr_.in6_));
        this->inet_addr_.in6_.sin6_family = AF_INET6;
#if (CWX_HAS_SOCKADDR_IN6_SIN6_LEN)
        this->inet_addr_.in6_.sin6_len = sizeof (this->inet_addr_.in6_);
#endif
        memcpy (&this->inet_addr_.in6_.sin6_addr, szIpAddr, len);
        return 0;
    } /* end len == 16 */
#endif /* CWX_HAS_IPV6 */
    // Here with an unrecognized length.
    errno = EAFNOSUPPORT;
    return -1;

}



CWINUX_END_NAMESPACE

