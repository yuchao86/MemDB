#ifndef __CWX_GBK_UNICODE_MAP_H__
#define __CWX_GBK_UNICODE_MAP_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxGbkUnicodeMap.h
@brief ������GBK��UTF-8��UTF-16ת����̬��CwxGbkUnicodeMap��
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxCharset.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxGbkUnicodeMap
@brief �ṩ������̬API��ʵ��GBK��UTF-8��UTF-16����໥ת��
*/
class CWX_API CwxGbkUnicodeMap
{
public:
    /**
    @brief ��һ��UTF16���ַ���ת��ΪUTF8���ַ�
    @param [in] szUtf16 UTF16���ַ���
    @param [in,out] uiUtf16Len ����UTF16���ַ����ĳ��ȣ�����ת����UTF16���ֽ�����
    @param [out] szUtf8 �����UTF8���ַ������γɵ�UTF8�ַ�����\\0������
    @param [in,out] uiUtf8Len ����szUtf8��buffer��С������szUtf8�ĳ���
    @return void
    */
    static void utf16ToUtf8(char const* szUtf16, size_t& uiUtf16Len, char* szUtf8, size_t& uiUtf8Len);
    /**
    @brief ��һ��UTF16���ַ���ת��ΪGBK���ַ�
    @param [in] szUtf16 UTF16���ַ���
    @param [in,out] uiUtf16Len ����UTF16���ַ����ĳ��ȣ�����ת����UTF16���ֽ�����
    @param [out] szGbk �����GBK���ַ������γɵ�GBK�ַ�����\\0������
    @param [in,out] uiGbkLen ����szGbk��buffer��С������szGbk�ĳ���
    @return void
    */
    static void utf16ToGbk(char const* szUtf16, CWX_UINT32& uiUtf16Len, char* szGbk, CWX_UINT32& uiGbkLen);
    /**
    @brief ��һ��UTF8���ַ���ת��ΪUTF16���ַ�
    @param [in] szUtf8 UTF8���ַ���
    @param [in,out] uiUtf8Len ����UTF8���ַ����ĳ��ȣ�����ת����UTF8���ֽ�����
    @param [out] szUtf16 �����UTF16���ַ������γɵ�UTF16�ַ�����\\0������
    @param [in,out] uiUtf16Len ����szUtf16��buffer��С������szUtf16�ĳ���
    @return void
    */
    static void utf8ToUtf16(char const* szUtf8, size_t& uiUtf8Len, char* szUtf16, size_t& uiUtf16Len);
    /**
    @brief ��һ��UTF8���ַ���ת��ΪGBK���ַ�
    @param [in] szUtf8 UTF8���ַ���
    @param [in,out] uiUtf8Len ����UTF8���ַ����ĳ��ȣ�����ת����UTF8���ֽ�����
    @param [out] szGbk �����GBK���ַ������γɵ�GBK�ַ�����\\0������
    @param [in,out] uiGbkLen ����szGbk��buffer��С������szGbk�ĳ���
    @return void
    */
    static void utf8ToGbk(char const* szUtf8, CWX_UINT32& uiUtf8Len, char* szGbk, CWX_UINT32& uiGbkLen);
    /**
    @brief ��һ��GBK���ַ���ת��ΪUTF8���ַ�
    @param [in] szGbk GBK���ַ���
    @param [in,out] uiGbkLen ����GBK���ַ����ĳ��ȣ�����ת����GBK���ֽ�����
    @param [out] szUtf8 �����UTF8���ַ������γɵ�utf8�ַ�����\\0������
    @param [in,out] uiUtf8Len ����szUtf8��buffer��С������szUtf8�ĳ���
    @return void
    */
    static void gbkToUtf8(char const* szGbk, CWX_UINT32& uiGbkLen, char* szUtf8, CWX_UINT32& uiUtf8Len);
    /**
    @brief ��һ��GBK���ַ���ת��ΪUTF16���ַ�
    @param [in] szGbk GBK���ַ���
    @param [in,out] uiGbkLen ����GBK���ַ����ĳ��ȣ�����ת����GBK���ֽ�����
    @param [out] szUtf16 �����UTF16���ַ������γɵ�utf16�ַ�����\\0������
    @param [in,out] uiUtf16Len ����szUtf16��buffer��С������szUtf16�ĳ���
    @return void
    */
    static void gbkToUtf16(char const* szGbk, CWX_UINT32& uiGbkLen, char* szUtf16, CWX_UINT32& uiUtf16Len);
    /**
    @brief ��һ��utf16���ַ������Ϊһ��UTF8���ַ�
    @param [in] unUtf16 utf16�ַ�����
    @param [out] szUtf8 �����UTF8���ַ�
    @param [in,out] unUtf8Len ����UTF8��buffer��С������unUtf16��Ӧ��UTF8�ַ��ĳ���
    @return void
    */
    static void utf16ChrToUtf8(CWX_UINT16 unUtf16, char* szUtf8, CWX_UINT8& unUtf8Len);
    /**
    @brief ��һ��utf8���ַ���Ϊһ��UTF16���ַ�
    @param [in] szUtf8 һ��utf8�ַ�
    @param [in] unUtf8Len һ��UTF8���ַ��ĳ���
    @param [out] unUtf16 UTF8�ַ���Ӧ��UTF16����
    @return void
    */
    static void utf8ChrToUtf16(char const* szUtf8, CWX_UINT16 unUtf8Len, CWX_UINT16& unUtf16);
    ///����GBK�ı��룬��ȡ��Ӧ��UTF16���룬��Ϊ0��ʾ�����ڣ��˽�����˫�ֽڵ�GBK����
    static CWX_UINT16 gbkToUtf16 (CWX_UINT16 unGbk);
    ///����UTF16�ı��룬��ȡ��Ӧ��GBK���룬��Ϊ0��ʾ�����ڣ��˽�����˫�ֽڵ�GBK����
    static CWX_UINT16 utf16ToGbk(CWX_UINT16 unUtf16);
private:
    static unsigned short  m_gbkUtf16Map[65536];
    static unsigned short  m_utf16GbkMap[65536];
private:
    ///���캯������ֹʵ����
    CwxGbkUnicodeMap(){}
    ///������������ֹɾ��
    ~CwxGbkUnicodeMap(){}
};

CWINUX_END_NAMESPACE

#include "CwxGbkUnicodeMap.inl"
#include "CwxPost.h"

#endif
