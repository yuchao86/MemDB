#ifndef __CWX_TAIL_MACRO_H__
#define __CWX_TAIL_MACRO_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxTailMacro.h
@brief ������������ĺ궨��
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"

///����˫����������
#define CWINUX_DTAIL_ENTRY(type)  \
type     *m_next;	/*<next item*/\
type     *m_prev	/*< previous next element */

///˫�����������ղ���
#define CWINUX_DTAIL_RESET()\
    m_next = NULL;\
    m_prev = NULL

///����ǰ����������
#define CWINUX_STAIL_ENTRY(type)  \
type     *m_next	/*<next item*/

///ǰ�������������ղ���
#define CWINUX_STAIL_RESET()  \
    m_next = NULL	/*<next item*/


#include "CwxPost.h"

#endif