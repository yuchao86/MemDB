#ifndef __CWX_UNIX_ADDR_H__
#define __CWX_UNIX_ADDR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxUnixAddr.h
@brief UNIX domain�ĵ�ַ����Ķ����ļ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxCommon.h"
#include "CwxNetMacro.h"
#include "CwxAddr.h"
#include "CwxSocket.h"
#include <sys/un.h>
CWINUX_BEGIN_NAMESPACE
/**
@class CwxUnixAddr
@brief Unix domain��ַ����
*/
class CWX_API CwxUnixAddr:public CwxAddr
{
public:
    ///Ĭ�Ϲ��캯��
    CwxUnixAddr();
    /**
    @brief ����szPathFile��Unix domain��ַ��
    @param [in] szPathFile Unix domain��Ӧ���ļ���
    */
    CwxUnixAddr(char const* szPathFile);
    /**
    @brief ����sockaddr_un�Ľṹ������Unix domain��ַ��
    @param [in] addr ��ַ�ṹ��
    @param [in] len addr�ĳ��ȡ�
    */
    CwxUnixAddr (sockaddr_un const* addr, int len);
    ///��������
    CwxUnixAddr(CwxUnixAddr const& addr);
    ///��������
    ~CwxUnixAddr();
public:
    /**
    @brief ����ֵ��
    @param [in] addr ��ֵ�Ķ���
    @return 0���ɹ���-1��ʧ�ܡ�
    */
    int set (CwxUnixAddr const& addr);
    /**
    @brief �޸ĵ�ַ��Ӧ��PathFile��
    @param [in] szPathFile Unix domain ��Ӧ��path file��
    @return 0���ɹ���-1��ʧ�ܡ�
    */
    int set(char const* szPathFile);
    /**
    @brief ����sockaddr_un�Ľṹ���ݣ��޸ĵ�ַ��
    @param [in] addr ��ַ�ṹ��
    @param [in] len addr�ĳ��ȡ�
    @return 0���ɹ���-1��ʧ�ܡ�
    */
    int set(sockaddr_un const* addr, int len);
    /// Return a pointer to the underlying network address.
    virtual void *getAddr (void) const;
    /// Set a pointer to the underlying network address.
    virtual void setAddr (void *addr, int len);
public:
    /**
    @brief ��ȡUnix domain��path file��
    @return NULL��ʧ�ܣ�NOT NULL��Unix domain��Path file��
    */
    char const* getPathFile() const;
    /// Compare two addresses for equality.
    bool operator == (const CwxUnixAddr & sap) const;

    /// Compare two addresses for inequality.
    bool operator != (const CwxUnixAddr & sap) const;

private:
    /// Underlying socket address.
    sockaddr_un unix_addr_;
};

CWINUX_END_NAMESPACE

#include "CwxUnixAddr.inl"
#include "CwxPost.h"

#endif
