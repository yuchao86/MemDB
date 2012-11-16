#ifndef __CWX_CONFIG_H__
#define __CWX_CONFIG_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file cwx_config.h
@brief ���ݶ���
@author cwinux@gmail.com
@version 0.1
@date 2010-10-04
@warning
@bug
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <arpa/inet.h>
#ifdef __cplusplus
extern "C" {
#endif

///CWX_UINT64 �������Ͷ���
#define CWX_UINT64   uint64_t
///CWX_INT64 �������Ͷ���
#define CWX_INT64   int64_t
///CWX_UINT32 �������Ͷ���
#define CWX_UINT32   uint32_t
///CWX_INT32 �������Ͷ���
#define CWX_INT32   int32_t
///CWX_UINT16 �������Ͷ���
#define CWX_UINT16   uint16_t
///CWX_INT16 �������Ͷ���
#define CWX_INT16   int16_t
///CWX_UINT8 �������Ͷ���
#define CWX_UINT8   uint8_t
///CWX_INT8 �������Ͷ���
#define CWX_INT8   int8_t

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

#ifdef __cplusplus
}
#endif

#endif
