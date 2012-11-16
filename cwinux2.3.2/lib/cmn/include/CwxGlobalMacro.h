#ifndef __CWX_GLOBAL_MACRO_H__
#define __CWX_GLOBAL_MACRO_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxGlobalMacro.h
*@brief ϵͳ��ȫ�ֺ궨��
*@author cwinux@gmail.com
*@version 0.1.0
*@date  2009-06-28
*@warning  ��.
*/
#include "CwxPre.h"
#include <assert.h>

#define _CWX_COMPILE_DATE(x)                      #x
#define CWX_COMPILE_DATE(x)                      _CWX_COMPILE_DATE(x)


#if defined(CWX_WIN32) && defined(CWX_DLL)
#if defined(CWX_EXPORTS)
#define CWX_API __declspec(dllexport)
#else
#define CWX_API __declspec(dllimport)
#endif
#endif

#if !defined(CWX_API)
#define CWX_API
#endif


///define the namespace for cwinux
#define CWINUX_BEGIN_NAMESPACE namespace cwinux{
#define CWINUX_END_NAMESPACE }
#define CWINUX_USING_NAMESPACE using namespace cwinux;

///�������
#define CWX_ASSERT(a)  assert(a)

///���Եļ��
# define CWX_CHECK_ATTR(WORD, ATTR) (((WORD) & (ATTR)) != 0)
///���Ե�����
# define CWX_SET_ATTR(WORD, ATTR) (WORD |= (ATTR))
///���Ե����
# define CWX_CLR_ATTR(WORD, ATTR) (WORD &= ~(ATTR))

///λ���
# define CWX_CHECK_BIT(WORD, BIT) (((WORD) & (1<<(BIT))) != 0)
///λ����
# define CWX_SET_BIT(WORD, BIT) ((WORD) |= (1<<(BIT)))
///λ���
# define CWX_CLR_BIT(WORD, BIT) ((WORD) &= ~((1<<(BIT))))
///��С
# define CWX_MIN(a, b) ((b) > (a) ? (a) : (b))
///���
# define CWX_MAX(a, b) ((b) > (a) ? (b) : (a))

#define CWX_UNUSED_ARG(a) (void)(a)

#define CWX_MSG_SIZE_MAX  1024 * 1024 * 64

#include "CwxPost.h"
#endif
