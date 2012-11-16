#ifndef __CWX_CHR_TYPE_H__
#define __CWX_CHR_TYPE_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxChrType.h
@brief �ַ�����ʶ�����Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-02
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include <ctype.h>

CWINUX_BEGIN_NAMESPACE

/**
@class CwxChrType
@brief ʵ�����ַ�����ʶ��ľ�̬������
*/
class CWX_API CwxChrType
{
public:
    ///����ַ�c�Ƿ�Ϊ�ַ������֣���Ч��isAlpha()||isDigit()��
    inline static bool isAlnum(int c);
    ///����ַ�c�Ƿ�Ϊ�ַ�������"C" locale����Ч��isUpper()||isLower()
    inline static bool  isAlpha(int c);
    ///����ַ�c�Ƿ�7-bit unsigned char��
    inline static bool  isAscii(int c);
    ///����ַ�c�Ƿ��ǿո��tab
    inline static bool  isBlank(int c);
    ///���zifuc�Ƿ�Ϊ�����ַ�
    inline static bool  isCntrl(int c);
    ///����ַ�c�Ƿ�Ϊ0~9������
    inline static bool  isDigit(int c);
    ///����ַ�c�Ƿ�Ϊ�ɴ�ӡ�ַ������ո����
    inline static bool  isGraph(int c);
    ///����ַ�c�Ƿ�ΪСд
    inline static bool  isLower(int c);
    ///����ַ�c�Ƿ�Ϊ�ɴ�ӡ�ַ��������ո�
    inline static bool  isPrint(int c);
    ///����ַ�c�Ƿ�Ϊ���ո����֡��ַ���Ŀɴ�ӡ�ַ�
    inline static bool  isPunct(int c);
    ///����ַ�c�Ƿ�Ϊ�ո񣬶��ڡ�C"��"POSIX����locale�����˿ո��⣬\\f \\n \\r \\t \\v������
    inline static bool  isSpace(int c);
    ///����ַ�c�Ƿ�Ϊ��д
    inline static bool  isUpper(int c);
    ///����ַ�c�Ƿ�16���Ƶ��ַ�
    inline static bool  isXdigit(int c);
private:
    ///��ֹ��������
    CwxChrType(){}
    ~CwxChrType(){}
};

CWINUX_END_NAMESPACE

#include "CwxChrType.inl"
#include "CwxPre.h"

#endif
