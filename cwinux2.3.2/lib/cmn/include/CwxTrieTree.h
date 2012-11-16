#ifndef __CWX_TRIE_TREE_H__
#define __CWX_TRIE_TREE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxTrieTree.h
@brief ֧�ֶ��ַ�����ģ��trie���Ķ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxTypePoolEx.h"

CWINUX_BEGIN_NAMESPACE
template<typename CHARSET, typename DATA>
class CwxTrieTree;
/**
@class CwxTrieNode
@brief Trie ���ڵ��ģ���ࡣĩ�ڵ��ϵ����õ���������ͨ��ģ�����DATA���塣
*/
template<typename DATA>
class CwxTrieNode
{
public:
    enum{
        SIGN_WORD_END_BIT = 0,///<�ڵ���һ���ַ���ĩ�ڵ�
        SIGN_KEY_END_BIT = 1,///<�ڵ���trie �����ַ�����ĩ�ڵ�
    };
public:
    ///���캯��
    CwxTrieNode()
    {
        memset(this, 0x00, sizeof(CwxTrieNode));
    }
public:
    /**
    @brief �Ա��ڵ�Ϊ���ڵ㣬����szWord�����·���ϵĽڵ㡣
    @param [in] szWord ���ӵĽڵ��Ӧ���ַ���
    @param [in] uiWordLen �ַ����ĳ���
    @param [in] pool ����Ҫ�����½ڵ㣬���½ڵ���pool���з���
    @return NULL�����ʧ�ܣ�����ΪszWord�ַ�����ĩ�ڵ㡣
    */
    inline CwxTrieNode<DATA>* add(char const* szWord, CWX_UINT32 uiWordLen, CwxTypePoolEx<CwxTrieNode<DATA> >& pool);
    ///���ñ��ڵ�Ϊһ���ַ�����ĩ�ڵ�
    inline CwxTrieNode<DATA>* endKey();
    /**
    @brief ����szWord�ַ���Ӧ�ĺ��ӽ�㡣
    @param [in] szWord ���ӽ���Ӧ��word
    @param [in] uiWordLen word�ĳ���
    @return NULL���ڵ㲻���ڻ���word�ڵ㣻����ΪszWord��Ӧ�Ľڵ㡣
    */
    inline CwxTrieNode<DATA> const* word(char const* szWord, CWX_UINT32 uiWordLen) const;
    /**
    @brief ������szKeyΪ·���ĵ�һ���ַ����ڵ㣬Ϊ��Сƥ�䡣
    @param [in] szKey �ڵ��·��
    @param [in] uiKeyLen key�ĳ���
    @return NULL���ڵ㲻���ڣ�����Ϊ��szKeyΪ·���ĵ�һ���ַ����ڵ㡣
    */
    inline CwxTrieNode<DATA> const* firstKey(char const* szKey, CWX_UINT32& uiKeyLen) const;
    /**
    @brief ������szKeyΪ·�������һ���ַ����ڵ㣬Ϊ���ƥ��
    @param [in] szKey �ڵ��·��
    @param [in] uiKeyLen key�ĳ���
    @return NULL���ڵ㲻���ڣ�����Ϊ��szKeyΪ·�������һ���ַ����ڵ㡣
    */
    inline CwxTrieNode<DATA> const* lastKey(char const* szKey, CWX_UINT32& uiKeyLen) const;
public:
    ///��ȡ�ڵ�ĸ��ڵ�
    inline CwxTrieNode<DATA> const* getParent() const{ return m_parent;}
    ///���ýڵ�ĸ��ڵ�
    inline void setParent(CwxTrieNode<DATA>* parent) { m_parent = parent;}
    ///��ȡ�ڵ��DATA���͵����ݵ�ַ
    DATA* getData() const { return m_data;}
    ///���ýڵ��DATA���͵����ݵ�ַ
    void setData(DATA* data) { m_data = data;}
    ///��ȡ�ڵ���û����ԡ�
    CWX_UINT8 getAttr() const { return m_ucAttr;}
    ///���ýڵ���û�����
    void setAttr(CWX_UINT8 ucAttr) { m_ucAttr = ucAttr;}
    ///�жϽڵ��Ƿ�Ϊһ��word�ڵ�
    bool isWordEnd() const { return CWX_CHECK_BIT(m_ucSign, SIGN_WORD_END_BIT);}
    ///�жϽڵ��Ƿ�Ϊһ���ַ����ڵ�
    bool isKeyEnd() const { return CWX_CHECK_BIT(m_ucSign, SIGN_KEY_END_BIT);}
    ///���KEY���SIGN_KEY_END_BIT
    void clrKeyEnd() { CWX_CLR_BIT(m_ucSign, SIGN_KEY_END_BIT);}
private:
    template <class A1,class A2>  friend class CwxTrieTree;
private:
    CwxTrieNode<DATA>*      m_child[16];///<�ڵ�ĺ��ӽ��
    CwxTrieNode<DATA>*      m_parent;///<�ڵ�ĸ��ڵ�
    DATA*                  m_data;///<�ڵ��ϵ�����
    CWX_UINT8              m_ucAttr;///<�ڵ���û�����
    CWX_UINT8              m_ucSign;///<�ڵ������
};


/**
@class CwxTrieTree
@brief Trie ����ģ���ࡣ�ַ�����CHARSET���壬ĩ�ڵ��ϵ����õ���������ͨ��ģ�����DATA���塣<br>
       Trie ��֧���ַ����Ƿ��Сд���У�һ������ʵ��Ҫô��Сд���У�Ҫô��Сд�����С�
*/
template<typename CHARSET, typename DATA>
class CwxTrieTree
{
public:
    /**
    @brief ���캯����
    @param [in] uiNodePerPool �������ڴ�ص�ÿ��trunk�Ľڵ���
    */
    CwxTrieTree(CWX_UINT32 uiNodePerPool):m_pool(uiNodePerPool), m_uiDataNum(0){}
public:
    /**
    @brief ��Trie Tree���szKeyword����Ϊ��Сд���С�
    @param [in] szKey ��ӵ��ַ���
    @param [in] uiKeyLen �ַ����ĳ���
    @return NULL�����ʧ�ܣ�����ΪszKey�ַ�����ĩ�ڵ㡣
    */
    inline CwxTrieNode<DATA>* addKey(char const* szKey, CWX_UINT32 uiKeyLen);
    /**
    @brief ��Trie Tree���szKeyword����Ϊ��Сд�����С�
    @param [in] szKey ��ӵ��ַ���
    @param [in] uiKeyLen �ַ����ĳ���
    @return NULL�����ʧ�ܣ�����ΪszKey�ַ�����ĩ�ڵ㡣
    */
    inline CwxTrieNode<DATA>* addCaseKey(char const* szKey, CWX_UINT32 uiKeyLen);
    /**
    @brief ɾ��ָ����Key�ڵ㣬�˽ڵ����Ϊһ��SIGN_KEY_END_BIT�ڵ㣬�����к��ӣ���ֻɾ��SIGN_KEY_END_BIT��ǡ�
           ������Ϊ�գ���Ӵ˽ڵ�һֱ��parent������ɾ�����еĸ��ڵ�ֱ��������һ�µĽڵ㣺
           1��SIGN_KEY_END_BIT�ڵ㣬����ζ�������һ��keyword����
           2��һ���ڵ��к��ӡ�
    @param [in] pNode Ҫɾ����node
    @return true���ɹ�ɾ����false����Ч�Ľڵ㣬��ʱ���ڵ㲻��SIGN_KEY_END_BIT�ڵ㡣
    */
    inline bool eraseNode(CwxTrieNode<DATA>* pNode);
    /**
    @brief ������szKeyΪ·���ĵ�һ���ַ����ڵ㣬Ϊ��Сƥ�䡣��Ϊ��Сд���С�
    @param [in] szKey �ڵ��·��
    @param [in,out] uiKeyLen ����key�ĳ��ȣ��������Ǹ�λ�÷����˵�һ���ַ����ڵ�
    @return NULL���ڵ㲻���ڣ�����Ϊ��szKeyΪ·���ĵ�һ���ַ����ڵ㡣
    */
    inline CwxTrieNode<DATA> const* firstKey(char const* szKey, CWX_UINT32& uiKeyLen) const;
    /**
    @brief ������szKeyΪ·���ĵ�һ���ַ����ڵ㣬Ϊ��Сƥ�䡣��Ϊ��Сд�����С�
    @param [in] szKey �ڵ��·��
    @param [in,out] uiKeyLen ����key�ĳ��ȣ��������Ǹ�λ�÷����˵�һ���ַ����ڵ�
    @return NULL���ڵ㲻���ڣ�����Ϊ��szKeyΪ·���ĵ�һ���ַ����ڵ㡣
    */
    inline CwxTrieNode<DATA> const* firstCaseKey(char const* szKey, CWX_UINT32& uiKeyLen) const;
    /**
    @brief ������szKeyΪ·�������һ���ַ����ڵ㣬Ϊ���ƥ�䡣��Ϊ��Сд���С�
    @param [in] szKey �ڵ��·��
    @param [in,out] uiKeyLen ����key�ĳ��ȣ��������Ǹ�λ�÷��������һ���ַ����ڵ�
    @return NULL���ڵ㲻���ڣ�����Ϊ��szKeyΪ·�������һ���ַ����ڵ㡣
    */
    inline CwxTrieNode<DATA> const* lastKey(char const* szKey, CWX_UINT32& uiKeyLen) const;
    /**
    @brief ������szKeyΪ·�������һ���ַ����ڵ㣬Ϊ���ƥ�䡣��Ϊ��Сд�����С�
    @param [in] szKey �ڵ��·��
    @param [in,out] uiKeyLen ����key�ĳ��ȣ��������Ǹ�λ�÷��������һ���ַ����ڵ�
    @return NULL���ڵ㲻���ڣ�����Ϊ��szKeyΪ·�������һ���ַ����ڵ㡣
    */
    inline CwxTrieNode<DATA> const* lastCaseKey(char const* szKey, CWX_UINT32& uiKeyLen) const;
    ///��ȡTrie Tree�ĸ��ڵ�
    inline CwxTrieNode<DATA> const* getRoot() const { return &m_root;}
    ///��ȡTrie Tree���ַ���������
    inline CWX_UINT32 getDataNum() const { return m_uiDataNum;}
    ///���Trie Tree
    inline void clear();
private:
    CwxTrieNode<DATA>  m_root; ///<Trie Tree�ĸ��ڵ�
    CwxTypePoolEx<CwxTrieNode<DATA> > m_pool;///<Trie Tree�ڵ������ڴ��
    CWX_UINT32        m_uiDataNum;///<Trie Tree���ַ���������
};

CWINUX_END_NAMESPACE

#include "CwxTrieTree.inl"
#include "CwxPost.h"

#endif

