#ifndef __CWX_UNIX_STREAM_H__
#define __CWX_UNIX_STREAM_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxUnixStream.h
@brief UNix domain������ͨ�����Ӷ���Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-12
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxNetMacro.h"
#include "CwxUnixAddr.h"
#include "CwxSockStream.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxUnixStream
@brief ��������Unix domain���Ӷ���
*/
class CWX_API CwxUnixStream:public CwxSockStream
{
public:
    ///Ĭ�Ϲ��캯��
    CwxUnixStream();
    ///���캯�������������趨����handle
    CwxUnixStream(CWX_HANDLE handle);
    ///��������.
    ~CwxUnixStream(void);
public:
    /**
    * Return the address of the remotely connected peer (if there is
    * one), in the referenced CwxAddr. Returns 0 if successful, else
    * -1.
    */
    int getRemoteAddr (CwxAddr &) const;
};

CWINUX_END_NAMESPACE

#include "CwxUnixStream.inl"
#include "CwxPost.h"

#endif
