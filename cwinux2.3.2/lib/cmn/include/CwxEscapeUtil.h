#ifndef __CWX_ESCAPE_UTIL_H__
#define __CWX_ESCAPE_UTIL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxEscapeUtil.h
@brief �����˸��ֳ��ϵ�escape�����Ķ��壬ֻ��Է���UTF8�ı����ַ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-02
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxChrType.h"
#include "CwxStl.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "CwxSingleton.h"
CWINUX_BEGIN_NAMESPACE
/**
@class CwxEscapeUtil
@brief ����һ���ַ��Ƿ���Ҫ����encode�������б��봦��ֻ��Է���UTF8�ı����ַ���
*/
class CWX_API CwxEscapeUtil:public CwxSingleton
{
public:
    enum{
        URL_ESCAPE = 0x01///<url��escape����
    };
public:
    ///����signleton CwxEscapes ����
    static CwxEscapeUtil* instance();
public:
    /**
    @brief �ж��ַ�chr��unEscapeType���͵�escape�У��Ƿ���Ҫ��escape��
    @param [in] chr �����ַ�
    @param [in] unEscapeType escape�����ͣ�escape������Ϊenum��ֵ
    @return true����Ҫ��escape�任��false������Ҫ��escape�任
    */
    bool isEncode(CWX_UINT8 chr, CWX_UINT16 unEscapeType) const;
    /**
    @brief ��һ���ַ�what��2��16���Ƶ��ַ���ʾ��
    @param [in] what ��Ҫ��Ϊ16���Ʊ�ʾ���ַ�
    @param [in] prefix 16���Ƶ�ǰ׺�ַ�����%��0��ʾû��ǰ׺
    @param [in] out �γɵĴ�ǰ׺��16�����ַ���
    @return out�ַ�������һ��д��λ�á�
    */
    char* c2x(CWX_UINT8 what, CWX_UINT8 prefix, char* out) const;
    /**
    @brief ������16���Ƶ��ַ�����ת��Ϊһ���ַ���
    @param [in] szStr ��Ҫ��Ϊ16���Ʊ�ʾ���ַ�
    @return ת������ַ���
    */
    CWX_UINT8 x2c(char const* szStr) const;
    /**
    @brief ����URL�ı�����򣬶�url���ַ������б��봦��
    @param [in] szUrl ��Ҫ����url������ַ���
    @param [out] szOut ����url�������ַ������γ��ַ�����\\0������
    @param [in,out] uiOutLen ����szOut�Ŀռ��С����������ĳ���
    @return true���ɹ���false:szOut�Ŀռ䲻�㡣
    */
    bool urlEncode(char const* szUrl, char* szOut, CWX_UINT32& uiOutLen) const;
    /**
    @brief ����URL�ı�����򣬶�url�ı�����ַ������н��봦��
    @param [in] szUrl ��Ҫ����url������ַ���
    @param [out] szOut ����url�������ַ������γ��ַ�����\\0������
    @param [in,out] uiOutLen ����szOut�Ŀռ��С����������ĳ���
    @return true���ɹ���false:szOut�Ŀռ䲻�㡣
    */
    bool urlDecode(char const* szUrl, char* szOut, CWX_UINT32& uiOutLen) const;
    /**
    @brief ����URL�ı�����򣬶�url���ַ������б��봦��
    @param [in] szUrl ��Ҫ����url������ַ���
    @param [out] strOut ����url�������ַ�����
    @return void��
    */
    void urlEncode(char const* szUrl, string& strOut) const;
    /**
    @brief ����URL�ı�����򣬶�url�ı�����ַ������н��봦��
    @param [in] szUrl ��Ҫ����url������ַ���
    @param [out] strOut ����url�������ַ�����
    @return void��
    */
    void urlDecode(char const* szUrl, string & strOut) const;

private:
    ///��ʼ����Ҫ���ַ��任��map��
    void init();
    ///��ֹ�������󣬱�֤singleton
    CwxEscapeUtil();
    ~CwxEscapeUtil();
private:
    CWX_UINT16    m_chrMap[256];
    static CwxMutexLock  m_lock;
    static CwxEscapeUtil*   m_pInstance;
};

CWINUX_END_NAMESPACE

#include "CwxEscapeUtil.inl"
#include "CwxPost.h"

#endif
