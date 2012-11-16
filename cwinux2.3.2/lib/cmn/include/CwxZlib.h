#ifndef __CWX_ZLIB_H__
#define __CWX_ZLIB_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxZlib.h
*@brief ZLIB�ļ򵥷�װ������ʵ��CWINUXƽ̨��ѹ��
*@author cwinux@gmail.com
*@version 1.0
*@date  2009-10-25
*@warning  nothing
*@bug    
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include <zlib.h>
CWINUX_BEGIN_NAMESPACE
/**
*@class  CwxZlib
*@brief  zlib��ļ򻯷�װ��ʵ��CWXINUX������ѹ������ѹ���ܡ�
*
*/
class CwxZlib
{
public:
    /**
    *@brief ��szSrc����ѹ��������ѹ��������ݱ�����szDest��
    *@param [out] szDest ѹ���������.
    *@param [in, out] ulDestLen ����szDest�ĳ��ȣ�����ѹ�������ݵĳ���.
    *@param [in] szSrc ѹ��ǰ������.
    *@param [in] ulSrcLen ѹ��ǰ���ݵĳ���.
    *@param [in] ucLevel ѹ���ļ���0~9��0��ʾ��ѹ��,����ԽС��ѹ����Խ�ͣ��ٶ�Խ�죬Z_DEFAULT_COMPRESSIONΪȱʡ��
    *@return true:success; false:failure.
    */
    static inline bool zip(unsigned char* szDest,
        unsigned long& ulDestLen,
        const unsigned char* szSrc,
        unsigned long ulSrcLen,
        int level=Z_DEFAULT_COMPRESSION)
    {
        int ret = compress2(szDest, &ulDestLen, szSrc, ulSrcLen, level);
        return Z_OK == ret;
    }
    /**
    *@brief ��ѹ�������ݽ�ѹ
    *@param [out] szDest ��ѹ�������.
    *@param [in, out] ulDestLen ����szDest�ĳ��ȣ����ؽ�ѹ�����ݵĳ���.
    *@param [in] szSrc ��ѹǰ������.
    *@param [in] ulSrcLen ��ѹǰ���ݵĳ���.
    *@return true:success; false:failure.
    */
    static inline bool unzip(unsigned char* szDest,
        unsigned long& ulDestLen,
        const unsigned char* szSrc,
        unsigned long ulSrcLen)
    {
        int ret = uncompress(szDest,&ulDestLen,szSrc,ulSrcLen);
        return Z_OK == ret;
    }
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif
