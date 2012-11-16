#ifndef __CWX_IPC_SAP_H__
#define __CWX_IPC_SAP_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxIpcSap.h
@brief ���̼�ͨ��handle����Ļ��ࡣ
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include <unistd.h>
#include <fcntl.h>
CWINUX_BEGIN_NAMESPACE

/**
* @class CwxIpcSap
*
* @brief Defines the member functions for the base class of the
* CwxIpcSap abstraction
*/
class CWX_API CwxIpcSap
{
public:
    ///enable��disable asynchronous������ֵ��0�ɹ���-1��ʧ�ܡ�
    int setSigio(bool enable=true) const;
    ///enable��disable  non-blocking I/O������ֵ��0�ɹ���-1��ʧ�ܡ�
    int setNonblock(bool enable=true) const;
    ///enable��disable   close-on-exec������ֵ��0�ɹ���-1��ʧ�ܡ�
    int setCloexec (bool enable=true) const;
    ///enable��disable   sigurg������ֵ��0�ɹ���-1��ʧ�ܡ�
    int setSigurg (bool enable=true) const;
    ///�Ƿ�������sigio, -1��ʧ�ܣ�0��û�У�1������
    int  isSigio() const;
    ///�Ƿ�������nonblock, -1��ʧ�ܣ�0��û�У�1������
    int  isNonBlock() const;
    ///�Ƿ�������Cloexec, -1��ʧ�ܣ�0��û�У�1������
    int  isCloexec() const;
    ///�Ƿ�������Sigurg, -1��ʧ�ܣ�0��û�У�1������
    int  isSigurg() const;
    /// Get the underlying handle.
    CWX_HANDLE getHandle (void) const;
    /// Set the underlying handle.
    void setHandle (CWX_HANDLE handle);
public:
    ///ͨ��fcntl����F_SETFL״̬������ֵ��0�ɹ���-1��ʧ�ܡ�
    static int setFlags (CWX_HANDLE handle, int flags);
    ///ͨ��fcntl���F_SETFL״̬������ֵ��0�ɹ���-1��ʧ�ܡ�
    static int clrFlags (CWX_HANDLE handle, int flags);
    ///�ж��Ƿ�������flag��-1��ʧ�ܣ�0��û�У�1������
    static int isFlags(CWX_HANDLE handle, int flags);
    ///enable��disable asynchronous������ֵ��0�ɹ���-1��ʧ�ܡ�
    static int setSigio(CWX_HANDLE handle, bool enable=true);
    ///enable��disable  non-blocking I/O������ֵ��0�ɹ���-1��ʧ�ܡ�
    static int setNonblock(CWX_HANDLE handle, bool enable=true);
    ///enable��disable   close-on-exec������ֵ��0�ɹ���-1��ʧ�ܡ�
    static int setCloexec (CWX_HANDLE handle, bool enable=true);
    ///enable��disable   sigurg������ֵ��0�ɹ���-1��ʧ�ܡ�
    static int setSigurg (CWX_HANDLE handle, bool enable=true);
    ///�Ƿ�������sigio, -1��ʧ�ܣ�0��û�У�1������
    int  isSigio(CWX_HANDLE handle) const;
    ///�Ƿ�������nonblock, -1��ʧ�ܣ�0��û�У�1������
    int  isNonBlock(CWX_HANDLE handle) const;
    ///�Ƿ�������Cloexec, -1��ʧ�ܣ�0��û�У�1������
    int  isCloexec(CWX_HANDLE handle) const;
    ///�Ƿ�������Sigurg, -1��ʧ�ܣ�0��û�У�1������
    int  isSigurg(CWX_HANDLE handle) const;
protected:
    // = Ensure that CwxIpcSap is an abstract base class.
    /// Default constructor.
    CwxIpcSap (void);

    /// Protected destructor.
    /**
    * Not a virtual destructor.  Protected destructor to prevent
    * operator delete() from being called through a base class
    * CwxIpcSap pointer/reference.
    */
    ~CwxIpcSap (void);

private:
    /// Underlying I/O handle.
    CWX_HANDLE handle_;
};


CWINUX_END_NAMESPACE

#include "CwxIpcSap.inl"
#include "CwxPost.h"

#endif
