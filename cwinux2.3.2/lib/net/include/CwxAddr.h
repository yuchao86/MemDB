#ifndef __CWX_ADDR_H__
#define __CWX_ADDR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAddr.h
@brief ����ĵ�ַ����Ķ����ļ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxNetMacro.h"
#include <net/if.h>

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxAddr
*
* @brief Defines the base class for the "address family independent"
* address format.
*/
class CWX_API CwxAddr
{
public:
    ///���캯��
    CwxAddr (CWX_INT32 iType = -1, CWX_INT32 iSize = -1);
    /// ����
    virtual ~CwxAddr (void);
    ///��ȡ��ַ��size
    CWX_INT32 getSize(void) const;
    ///���õ�ַ��size.
    void setSize (CWX_INT32 iSize);
    ///��ȡ��ַ����
    CWX_INT32 getType (void) const;
    /// ���õ�ַ����
    void setType (CWX_INT32 iType);
    ///���ص�ַ����.
    virtual void * getAddr (void) const;
    ///���õ�ַ����.
    virtual void setAddr (void *, CWX_INT32 iLen);
    ///����ַ�Ƿ����.
    bool operator == (const CwxAddr &sap) const;
    ///����ַ�Ƿ����.
    bool operator != (const CwxAddr &sap) const;
    ///Initializes instance variables.
    void baseSet(CWX_INT32 iType, CWX_INT32 iSize);
    /// Wild-card address.
    static const CwxAddr sap_any;
private:
    /// e.g., AF_UNIX, AF_INET, AF_SPIPE, etc.
    CWX_INT32  m_iAddrType;
    ///Number of bytes in the address.
    CWX_INT32  m_iAddrSize;
};


CWINUX_END_NAMESPACE

#include "CwxAddr.inl"
#include "CwxPost.h"

#endif
