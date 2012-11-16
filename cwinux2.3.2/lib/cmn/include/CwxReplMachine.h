#ifndef __CWX_REPL_MACHINE_H__
#define __CWX_REPL_MACHINE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxReplMachine.h
@brief ͨ��Trie��ʵ�ֵ��ַ��滻��CwxReplBase
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxTrieTree.h"
#include "CwxCommon.h"
#include "CwxCharset.h"
#include "CwxCharPool.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxReplBase
@brief �ַ��滻��Ļ��࣬���������API�����滻�Ǵ�Сд���еģ���ʹ�ò�����Case����API��<br>
       ����ʹ�ô�Case��API��
*/
class CWX_API CwxReplBase
{
public:
    ///���캯��
    CwxReplBase():m_uiReplNum(0)
    {

    }
    ///��������
    virtual ~CwxReplBase()
    {

    }
public:
    /**
    @brief ���滻������Ӵ�Сд���е��滻�ַ��ԣ�ֻ�д�Сд���е��滻��ʹ�ô˽ӿ�
    @param [in] szSrc Դ�ַ���
    @param [in] uiSrcLen Դ�ַ����ĳ���
    @param [in] szRepl �滻�ַ���
    @param [in] uiReplLen �滻�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addStr(char const* szSrc, CWX_UINT32 uiSrcLen, char const* szRepl, CWX_UINT32 uiReplLen)=0;
    /**
    @brief ���滻������Ӵ�Сд�����е��滻�ַ��ԣ�ֻ�д�Сд�����е��滻��ʹ�ô˽ӿ�
    @param [in] szSrc Դ�ַ���
    @param [in] uiSrcLen Դ�ַ����ĳ���
    @param [in] szRepl �滻�ַ���
    @param [in] uiReplLen �滻�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addCaseStr(char const* szSrc, CWX_UINT32 uiSrcLen, char const* szRepl, CWX_UINT32 uiReplLen)=0;
    /**
    @brief ִ�д�Сд���е��ַ��滻
    @param [in] szSrc ���滻��Դ�ַ���
    @param [in,out] uiSrcLen ���뱻�滻��Դ�ַ����ĳ��ȣ�ִ���滻�ĳ���
    @param [in] szOut �滻����ַ�����buffer����\\0����
    @param [in,out] uiOutLen ����szOut��buffer��С������滻����ַ����ĳ��ȣ�
    @return true���滻�ɹ���false���滻ʧ��
    */
    virtual bool replace(char const* szSrc, CWX_UINT32& uiSrcLen, char* szOut, unsigned int &uiOutLen) const=0;
    /**
    @brief ִ�д�Сд�����е��ַ��滻
    @param [in] szSrc ���滻��Դ�ַ���
    @param [in,out] uiSrcLen ���뱻�滻��Դ�ַ����ĳ��ȣ�ִ���滻�ĳ���
    @param [in] szOut �滻����ַ�����buffer����\\0����
    @param [in,out] uiOutLen ����szOut��buffer��С������滻����ַ����ĳ��ȣ�
    @return true���滻�ɹ���false���滻ʧ��
    */
    virtual bool caseReplace(char const* szSrc, CWX_UINT32& uiSrcLen, char* szOut, unsigned int &uiOutLen) const=0;
    ///��ȡ�滻�����ַ���
    virtual char const* charset() const = 0;
    ///����滻������ӵ��滻�ַ���
    virtual void clear() = 0;
public:
    ///��ȡ���õ��滻�ַ��Ե�����
    inline CWX_UINT32 getReplNum() const 
    {
        return m_uiReplNum;
    }
    ///�����滻�ַ��Ե�����
    inline void setReplNum(CWX_UINT32 uiReplNum)
    {
        m_uiReplNum = uiReplNum;
    }
    ///���滻�ַ��Ե�������һ
    inline void incReplNum() 
    {
        m_uiReplNum++;
    }
private:
    CWX_UINT32         m_uiReplNum;
};

/**
@class CwxReplMachine
@brief ֧�ֶ��ַ������ַ��滻��ģ����
*/
template<typename CHARSET>
class CwxReplMachine:public CwxReplBase
{
public:
    /**
    @brief ���캯��
    @param [in] uiMaxKeyNum �����滻Key������
    */
    CwxReplMachine(CWX_UINT32 uiMaxKeyNum)
        :m_charPool(1024, 2048), m_trieTree(uiMaxKeyNum/4)
    {

    }
public:
    /**
    @brief ���滻������Ӵ�Сд���е��滻�ַ��ԣ�ֻ�д�Сд���е��滻��ʹ�ô˽ӿ�
    @param [in] szSrc Դ�ַ���
    @param [in] uiSrcLen Դ�ַ����ĳ���
    @param [in] szRepl �滻�ַ���
    @param [in] uiReplLen �滻�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addStr(char const* szSrc, CWX_UINT32 uiSrcLen, char const* szRepl, CWX_UINT32 uiReplLen);
    /**
    @brief ���滻������Ӵ�Сд�����е��滻�ַ��ԣ�ֻ�д�Сд�����е��滻��ʹ�ô˽ӿ�
    @param [in] szSrc Դ�ַ���
    @param [in] uiSrcLen Դ�ַ����ĳ���
    @param [in] szRepl �滻�ַ���
    @param [in] uiReplLen �滻�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addCaseStr(char const* szSrc, CWX_UINT32 uiSrcLen, char const* szRepl, CWX_UINT32 uiReplLen);
    /**
    @brief ִ�д�Сд���е��ַ��滻
    @param [in] szSrc ���滻��Դ�ַ���
    @param [in] uiSrcLen ���滻��Դ�ַ����ĳ���
    @param [in] szOut �滻����ַ�����buffer����\\0����
    @param [in,out] uiOutLen ����szOut��buffer��С������滻����ַ����ĳ��ȣ�
    @return true���滻�ɹ���false���滻ʧ��
    */
    virtual bool replace(char const* szSrc, CWX_UINT32& uiSrcLen, char* szOut, unsigned int &uiOutLen) const;
    /**
    @brief ִ�д�Сд�����е��ַ��滻
    @param [in] szSrc ���滻��Դ�ַ���
    @param [in] uiSrcLen ���滻��Դ�ַ����ĳ���
    @param [in] szOut �滻����ַ�����buffer����\\0����
    @param [in,out] uiOutLen ����szOut��buffer��С������滻����ַ����ĳ��ȣ�
    @return true���滻�ɹ���false���滻ʧ��
    */
    virtual bool caseReplace(char const* szSrc, CWX_UINT32& uiSrcLen, char* szOut, unsigned int &uiOutLen) const;
    ///��ȡCHARSET���ַ���������
    virtual char const* charset() const;
    ///������õ��ַ��滻��
    virtual void clear();
private:
    CwxCharPool        m_charPool;///<�ַ�MEM POOL
    CwxTrieTree<CHARSET, CwxMultiString> m_trieTree;///<�ַ��滻��trie tree
};

CWINUX_END_NAMESPACE

#include "CwxReplMachine.inl"
#include "CwxPost.h"

#endif

