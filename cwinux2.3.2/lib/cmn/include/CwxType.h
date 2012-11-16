#ifndef __CWX_TYPE_H__
#define __CWX_TYPE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
/**
*@file  CwxType.h
*@brief define the base data type and ntol serial macro.
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-06-30
*@warning  none.
*/

#include "CwxPre.h"

#include "CwxConfig.h"
#include "CwxGlobalMacro.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/time.h>
///�����������Ͷ���
typedef int8_t             CWX_INT8;
typedef uint8_t            CWX_UINT8;
typedef int16_t            CWX_INT16;
typedef uint16_t           CWX_UINT16;
typedef int32_t            CWX_INT32;
typedef uint32_t           CWX_UINT32;
typedef int64_t            CWX_INT64;
typedef uint64_t           CWX_UINT64;

///�����ֽ��򡢱����ֽ���任����
#define CWX_NTOHS(x)  ntohs(x)
#define CWX_NTOHL(x)  ntohl(x)
#define CWX_HTONS(x)  htons(x)
#define CWX_HTONL(x)  htonl(x)

///IO������Ͷ���
typedef int CWX_HANDLE;
///��ЧIO������Ͷ���
#define CWX_INVALID_HANDLE  -1


#include "CwxPost.h"

#endif
