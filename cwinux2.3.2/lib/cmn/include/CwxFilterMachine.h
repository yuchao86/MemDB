#ifndef __CWX_FILTER_MACHINE_H__
#define __CWX_FILTER_MACHINE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxFilterMachine.h
@brief ͨ��Trie����ʵ����һ�����ݹ����֧࣬��stopword��֧�ֶ��ַ���(utf8, gbk)��
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
#include "CwxCharPool.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxFilterBase
@brief ������Ļ��࣬�����˹������API���������Ǵ�Сд���еģ���ʹ�ò�����Case����API��<br>
       ����ʹ�ô�Case��API��
*/
class CWX_API CwxFilterBase
{
public:
    enum{
        ATTR_FILTER_LEVEL1 = 0x01,
        ATTR_FILTER_LEVEL2 = 0x02,
        ATTR_FILTER_LEVEL3 = 0x04,
        ATTR_FILTER_LEVEL4 = 0x08,
        ATTR_FILTER_LEVEL5 = 0x10,
        ATTR_FILTER_LEVEL6 = 0x20,
        ATTR_FILTER_LEVEL7 = 0x40,
        ATTR_FILTER_LEVEL8 = 0x80
    };
    ///���캯��
    CwxFilterBase():m_uiFilterNum(0)
    {

    }
    ///��������
    virtual ~CwxFilterBase()
    {

    }
public:
    /**
    @brief ����������Ӵ�Сд���е�stopword��ֻ�д�Сд���еĹ�������ʹ�ô˽ӿ�
    @param [in] szStopWord stopword���ַ���
    @param [in] uiStopWordLen stopword�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addStopWord(char const* szStopWord, CWX_UINT32 uiStopWordLen)  =0;
    /**
    @brief ����������Ӵ�Сд�����е�stopword��ֻ�д�Сд�����еĹ�������ʹ�ô˽ӿ�
    @param [in] szStopWord stopword���ַ���
    @param [in] uiStopWordLen stopword�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addCaseStopWord(char const* szStopWord, CWX_UINT32 uiStopWordLen) = 0;
    /**
    @brief ����������Ӵ�Сд���еĹ��˴ʣ�ֻ�д�Сд���еĹ�������ʹ�ô˽ӿ�
    @param [in] szFilterStr ���˴��ַ���
    @param [in] uiFilterLen ���˴��ַ����ĳ���
    @param [in] ucFilterLevel �����˵ļ���һ���ʿ������ö�����˼������Ϊ8����
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addFilterStr(char const* szFilterStr, CWX_UINT32 uiFilterLen, CWX_UINT8 ucFilterLevel) = 0;
    /**
    @brief ����������Ӵ�Сд�����еĹ��˴ʣ�ֻ�д�Сд�����еĹ�������ʹ�ô˽ӿ�
    @param [in] szFilterStr ���˴��ַ���
    @param [in] uiFilterLen ���˴��ַ����ĳ���
    @param [in] ucFilterLevel �����˵ļ���һ���ʿ������ö�����˼������Ϊ8����
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addCaseFilterStr(char const* szFilterStr, CWX_UINT32 uiFilterLen, CWX_UINT8 ucFilterLevel) = 0;
    /**
    @brief ���մ�Сд���еķ�ʽ�����szContent���������Ƿ�������õĹ��˴ʡ�ֻ�д�Сд���еĹ�����ʹ�ô�API��
    @param [in] szContent ����������
    @param [in] uiContentLen ���������ݵĳ���
    @param [out] uiStart ��level����0���򷵻ض�Ӧlevel�ĴʵĿ�ʼλ��
    @param [out] uiLen ��level����0���򷵻ض�Ӧlevel�Ĵʵ��ֽڳ���
    @return 0�������ڹ��˴ʣ�>0����ߵĹ���LEVEL
    */
    virtual CWX_UINT8 filterStr(char const* szContent, CWX_UINT32 uiContentLen, CWX_UINT32& uiStart, CWX_UINT32& uiLen) const = 0;
    /**
    @brief ���մ�Сд�����еķ�ʽ�����szContent���������Ƿ�������õĹ��˴ʡ�ֻ�д�Сд�����еĹ�����ʹ�ô�API��
    @param [in] szContent ����������
    @param [in] uiContentLen ���������ݵĳ���
    @param [out] uiStart ��level����0���򷵻ض�Ӧlevel�ĴʵĿ�ʼλ��
    @param [out] uiLen ��level����0���򷵻ض�Ӧlevel�Ĵʵ��ֽڳ���
    @return 0�������ڹ��˴ʣ�>0����ߵĹ���LEVEL
    */
    virtual CWX_UINT8 filterCaseStr(char const* szContent, CWX_UINT32 uiContentLen, CWX_UINT32& uiStart, CWX_UINT32& uiLen) const = 0;
    ///���ع��������ַ���
    virtual char const* charset() const = 0;
    ///��λ��������������õĹ����ַ�����stopword
    virtual void clear() = 0;
public:
    ///��ȡ��ӵĹ����ַ���������
    CWX_UINT32 getFilterNum() const 
    {
        return m_uiFilterNum;
    }
    ///������ӵĹ����ַ���������
    void setFilterNum(CWX_UINT32 uiNum)
    {
        m_uiFilterNum = uiNum;
    }
    ///�������ַ�����������1
    void incFilterNum()
    {
        m_uiFilterNum ++;
    }
private:
    CWX_UINT32         m_uiFilterNum;///<���˴ʵ�����
};

/**
@class CwxFilterMachine
@brief ֧�ֶ��ַ����Ĺ�����ģ���ࡣCHARSET��ΪCwxCharset.h�ж�����ַ�������
*/
template<typename CHARSET>
class CwxFilterMachine:public CwxFilterBase{
public:
    /**
    @brief ���캯��
    @param [in] uiMaxFilteNum ���˴ʵ������Ŀ
    @param [in] uiMaxStopword stopword�������Ŀ
    */
    CwxFilterMachine(CWX_UINT32 uiMaxFilteNum, CWX_UINT32 uiMaxStopword):
        m_charPool(1024, 2048), m_filterTree(uiMaxFilteNum/4),m_stopwordTree(uiMaxStopword/4){
            m_ucMaxFilterLevel = 0;
    }
    ///��������
    ~CwxFilterMachine(){}
public:
    /**
    @brief ����������Ӵ�Сд���е�stopword��ֻ�д�Сд���еĹ�������ʹ�ô˽ӿ�
    @param [in] szStopWord stopword���ַ���
    @param [in] uiStopWordLen stopword�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addStopWord(char const* szStopWord, CWX_UINT32 uiStopWordLen);
    /**
    @brief ����������Ӵ�Сд�����е�stopword��ֻ�д�Сд�����еĹ�������ʹ�ô˽ӿ�
    @param [in] szStopWord stopword���ַ���
    @param [in] uiStopWordLen stopword�ַ����ĳ���
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addCaseStopWord(char const* szStopWord, CWX_UINT32 uiStopWordLen);
    /**
    @brief ����������Ӵ�Сд���еĹ��˴ʣ�ֻ�д�Сд���еĹ�������ʹ�ô˽ӿ�
    @param [in] szFilterStr ���˴��ַ���
    @param [in] uiFilterLen ���˴��ַ����ĳ���
    @param [in] ucFilterLevel �����˵ļ���һ���ʿ������ö�����˼������Ϊ8����
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addFilterStr(char const* szFilterStr, CWX_UINT32 uiFilterLen, CWX_UINT8 ucFilterLevel);
    /**
    @brief ����������Ӵ�Сд�����еĹ��˴ʣ�ֻ�д�Сд�����еĹ�������ʹ�ô˽ӿ�
    @param [in] szFilterStr ���˴��ַ���
    @param [in] uiFilterLen ���˴��ַ����ĳ���
    @param [in] ucFilterLevel �����˵ļ���һ���ʿ������ö�����˼������Ϊ8����
    @return true����ӳɹ���false�����ʧ��
    */
    virtual bool addCaseFilterStr(char const* szFilterStr, CWX_UINT32 uiFilterLen, CWX_UINT8 ucFilterLevel);
    /**
    @brief ���մ�Сд���еķ�ʽ�����szContent���������Ƿ�������õĹ��˴ʡ�ֻ�д�Сд���еĹ�����ʹ�ô�API��
    @param [in] szContent ����������
    @param [in] uiContentLen ���������ݵĳ���
    @param [out] uiStart ��level����0���򷵻ض�Ӧlevel�ĴʵĿ�ʼλ��
    @param [out] uiLen ��level����0���򷵻ض�Ӧlevel�Ĵʵ��ֽڳ���
    @return 0�������ڹ��˴ʣ�>0����ߵĹ���LEVEL
    */
    virtual CWX_UINT8 filterStr(char const* szContent, CWX_UINT32 uiContentLen, CWX_UINT32& uiStart, CWX_UINT32& uiLen) const;
    /**
    @brief ���մ�Сд�����еķ�ʽ�����szContent���������Ƿ�������õĹ��˴ʡ�ֻ�д�Сд�����еĹ�����ʹ�ô�API��
    @param [in] szContent ����������
    @param [in] uiContentLen ���������ݵĳ���
    @param [out] uiStart ��level����0���򷵻ض�Ӧlevel�ĴʵĿ�ʼλ��
    @param [out] uiLen ��level����0���򷵻ض�Ӧlevel�Ĵʵ��ֽڳ���
    @return 0�������ڹ��˴ʣ�>0����ߵĹ���LEVEL
    */
    virtual CWX_UINT8 filterCaseStr(char const* szContent, CWX_UINT32 uiContentLen, CWX_UINT32& uiStart, CWX_UINT32& uiLen) const;
    ///���ء�CHARSET����Ӧ���ַ���������
    virtual char const* charset() const;
    ///��λ��������������õĹ����ַ�����stopword
    virtual void clear();
private:
    ///���szContent�Ŀ�ʼ�ַ����Ƿ���stopword����ͨ��uiContentLen����stopword�ĳ���
    bool isStopWord(char const* szContent, CWX_UINT32& uiContentLen) const;
    ///���szContent�Ŀ�ʼ�ַ����Ƿ���stopword����ͨ��uiContentLen����stopword�ĳ���
    bool isCaseStopWord(char const* szContent, CWX_UINT32& uiContentLen) const;
    ///��ucLevel�м������ߵ�level
    CWX_UINT8 maxFilterLevel(CWX_UINT8 ucLevel) const;
private:
    CwxCharPool        m_charPool;///<�ַ����ڴ��
    CwxTrieTree<CHARSET, char> m_filterTree;///<���˴ʵ�trie��
    CwxTrieTree<CHARSET, char> m_stopwordTree;///<stopword��trie��
    CWX_UINT8          m_ucMaxFilterLevel;///<���õĹ��˴��е�������Level
};

CWINUX_END_NAMESPACE

#include "CwxFilterMachine.inl"
#include "CwxPost.h"

#endif
