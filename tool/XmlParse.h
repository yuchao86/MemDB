#ifndef __XML_PARSE_H__
#define __XML_PARSE_H__


#include "CwxStl.h"
#include "CwxCommon.h"
#include "CwxCharPool.h"
#include "CwxGbkUnicodeMap.h"
#include <expat.h>
#include <string.h>
#include <stdio.h>
#include "CwxEncodeXml.h"
#include "CwxPackageReaderEx.h"
CWINUX_USING_NAMESPACE

/**
@class XmlTreeNode
@brief ��״�����ݽڵ�������ڱ�ʾXML��JSON�����ݽڵ㡣
*/
class XmlTreeNode
{
public:
    ///���캯��
    XmlTreeNode()
    {
        m_pChildHead = NULL;
        m_pChildTail = NULL;
        m_prev = NULL;
        m_next = NULL;
        m_pParent = NULL;
    }
    ///��������
    ~XmlTreeNode()
    {
        if (m_pChildHead) delete m_pChildHead;
        if (m_next) delete m_next;
    }
public:
    char*   m_szElement;///<�ڵ������
    list<char*>   m_listData; ///<XML��\<aaa\>aaaaa\</aaa\>���ͽڵ������
    list<pair<char*, char*> > m_lsAttrs;///<�ڵ����Ե�key,value��
    XmlTreeNode* m_pChildHead;///<�ڵ�ĺ��ӽ���ͷ
    XmlTreeNode* m_pChildTail;///<�ڵ�ĺ��ӽ���β
    XmlTreeNode* m_prev;///<�ڵ��ǰһ���ֵܽڵ�
    XmlTreeNode* m_next;///<�ڵ����һ���ֵܽڵ�
    XmlTreeNode* m_pParent;///<�ڵ�ĸ��ڵ�
};

/**
@class XmlParser
@brief ����expatʵ�ֵ�XML���������󡣳���֧��expatĬ��֧�ֵ��ַ�������֧��GBK��gb2312�ַ���
*/
class XmlParser
{
public:
    enum{
        DEF_TRUCK_BUF_SIZE = 16 * 1024, ///<ȱʡ�����ݿ�Ĵ�С
        PATH_BUF_ALIGN = 1024 ///���ݿ�ı߽�����С
    };
public:
    /**
    @brief ���캯����
    @param [in] uiBufSize ���ݿ�Ĵ�С
    */
    XmlParser(CWX_UINT32 uiBufSize=DEF_TRUCK_BUF_SIZE);
    ///��������
    virtual ~XmlParser(void);
public:
    ///����XMLʵ�壬�û����ԶԴ�API�������ء�
    virtual bool parse();
    ///expat��XML���������Ƿ��ʼ��
    bool isReady(void) const
    {
        return m_isReady;
    }
    ///��ȡXML�Ľ���������Ϣ
    XML_Error getLastError(void) const
    {
        return m_lastError;
    }
    ///��ȡXML������״̬����
    XML_Status getStatus(void) const
    {
        return m_status; 
    }
    ///��ȡXML���������ݿ�
    XML_Char *getBuf(void) const
    {
        return m_szXmlBuf; 
    }
    ///��ȡ���ݿ�Ĵ�С
    CWX_UINT32 getBufSize(void) const
    {
        return m_uiBufSize; 
    }
    ///��ȡ��ǰ��������XML·��
    XML_Char const* getXmlPath() const
    {
        return m_szXmlPath;
    }
    ///�ж�XML�Ƿ�ΪGBK�ı���
    bool isGbk() const
    {
        return m_bGbk;
    }
    /**
    @brief ���ַ���ΪGBK��gb2312����expat��UTF-8�����ΪGBK��gb2312�ı����ʽ��
    @param [in] value expat�����UTF-8���ַ���
    @param [in] uiValueLen value�ĳ���
    @return ����GBK��gb2312������ַ���
    */
    char const* charsetValue(XML_Char const* value, CWX_UINT32 uiValueLen);
protected:
    ///����expat�������ready״̬
    void setReady(bool isReady)
    {
        m_isReady = isReady;
    }
    ///����xml������״̬��
    void setStatus(XML_Status status)
    {
        m_status = status; 
    }
    ///����XML�����Ĵ�����Ϣ
    void setLastError(XML_Error lastError)
    {
        m_lastError = lastError;
    }
    ///׼��xml�����Ļ������̳���������ش�API
    virtual bool prepare();
    ///��ȡXML��������һ�����ݿ飬��������Ҫ���ش�API��Ϊexpat�����ṩ������
    virtual ssize_t readBlock(void);
    /**
    @brief ֪ͨ����һ��XML�����ݽڵ㡣
    @param [in] name XML�ڵ������
    @param [in] atts XML�ڵ�����ԣ�atts[2n]Ϊ���Ե����֣�atts[2n+1]Ϊ���Ե�ֵ����atts[2n]ΪNULL����ʾ���Խ���
    @return void
    */
    virtual void startElement(const XML_Char *name, const XML_Char **atts);
    /**
    @brief ֪ͨ�뿪һ��XML�����ݽڵ㡣
    @param [in] name XML�ڵ������
    @return void
    */
    virtual void endElement(const XML_Char *name);
    /**
    @brief ֪ͨһ���ڵ��ڵ����ݡ�
    @param [in] s ���ݵ����ݣ������ΪUTF8�ı���
    @param [in] len ���ݵ����ݵĳ��ȡ�
    @return void
    */
    virtual void characterData(const XML_Char *s, int len);
    /**
    @brief ֪ͨXML��instructions.
    @param [in] target instruction�ĵ�һ��word.
    @param [in] data ��һ��word��ȥ�����пո���ַ�����
    @return void
    */
    virtual void processingInstruction(const XML_Char *target, const XML_Char *data);
    ///xml�е�ע��
    virtual void commentData(const XML_Char *data);
    ///xml��ȱʡ���ݴ�����
    virtual void defaultHandler(const XML_Char *s, int len);
    ///֪ͨ����XML��CDATA�﷨
    virtual void startCData(void);
    ///֪ͨ�뿪XML��CDATA�﷨
    virtual void endCData(void);

private:
    ///ע�����е�expat���¼�������
    void regDefHandlers();
    ///����һ��XML�ڵ���¼�������
    static XMLCALL void elementStartHandler(void *userData,
        const XML_Char *name, const XML_Char **atts);
    ///�뿪һ��XML�ڵ���¼�������
    static XMLCALL void elementEndHandler(void *userData,
        const XML_Char *name);
    ///�ڵ��ڲ����ݵĽ��ܺ���
    static XMLCALL void characterDataHandler(void *userData,
        const XML_Char *s, int len);
    ///XML instruction�Ľ��ܺ���
    static XMLCALL void processingInstrHandler(void *userData,
        const XML_Char *target, const XML_Char *data);
    ///ע�͵Ľ��ܺ���
    static XMLCALL void commentHandler(void *userData,
        const XML_Char *data);
    ///ȱʡ�¼��Ĵ�����
    static XMLCALL void defaultHandler(void *userData,
        const XML_Char *s, int len);
    ///����CDATA���¼�����
    static XMLCALL void startCDatahandler(void *userData);
    ///�뿪CDATA���¼�����
    static XMLCALL void endCDatahandler(void *userData);
    ///GBK��gb2312���ַ���ת��API
    static XMLCALL int convert(void* data, char const* s);
    ///GBK��gb2312�ַ�����ת�����¼�����
    static XMLCALL int encodingHandler(void* userData, XML_Char const* name, XML_Encoding* info);
private:
    XML_Parser  m_expatParser;///<expat������
    XML_Char *  m_szXmlBuf; ///<�ڲ���ʱBUF
    CWX_UINT32  m_uiBufSize;///<��ʱBUF�Ĵ�С
    bool    m_isReady;///<�����ʼ��״̬���
    XML_Status m_status;///<XML������״̬��
    XML_Error m_lastError;///<XML�����Ĵ�����Ϣ
    XML_Char*  m_szXmlPath;///<��ǰXML�ڵ��ȫ·��
    CWX_UINT32 m_uiPathBufLen;///<XML�ڵ�ȫ·����BUF�ĳ���
    CWX_UINT32 m_uiPathLen;///<m_szXmlPath�еĽڵ�·������
    bool      m_bGbk;///<�Ƿ������ı���
    XML_Char*  m_szGbkBuf;///<����GBK����ת�����ڴ�
    CWX_UINT32 m_uiGbkBufLen;///<m_szGbkBuf���ڴ泤��
};


/**
@class XmlFileParser
@brief ����XmlParser����ʵ��XML�ļ���������
*/
class  XmlFileParser : public XmlParser
{
public:
    ///�����ļ���strFileNameΪҪ������XML���ļ���
    XmlFileParser(string const& strFileName);
    ///��������
    virtual ~XmlFileParser();
protected:
    ///XML�ļ�������׼��
    virtual bool prepare();
    ///��XML�ļ��ж�ȡ��һ�������������ݿ飬-1����ʾ�ļ�β���ļ�������ͨ��status��ʶ��>=0����ȡ�����ݵĳ���
    virtual ssize_t readBlock(void);
private:
    FILE *  m_fd; ///<XML�ļ��ľ��
    string  m_strFileName;///<XML�ļ�������

};

/**
@class XmlConfigParser
@brief ��XML��BUF������XmlTreeNode��֯�Ľڵ���������֧��expatĬ��֧�ֵ��ַ�������֧��GBK��gb2312�ַ���
*/
class XmlConfigParser
{
public:
    /**
    @brief ���캯����
    @param [in] uiAvgTokenLen XML�е����ݽڵ��ƽ������
    @param [in] uiAvgXmlSize Ҫ������XML��ƽ����С
    */
    XmlConfigParser(CWX_UINT32 uiAvgTokenLen=1024, CWX_UINT32 uiAvgXmlSize=4096);
    ///��������
    ~XmlConfigParser();
public:
    /**
    @brief ��szXml�����XML�ı���������XmlTreeNode�Ľڵ�����
    @param [in] szXml XML
    @return true�������ɹ���false������ʧ��
    */
    bool parse(char const* szXml);
    /**
    @brief ��ȡһ��XML�ڵ�����ԡ�
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @param [in] szAttr �ڵ��������
    @return NULL�������ڣ�����Ϊ�ڵ����Ե���ֵ
    */
    char const* getElementAttr(char const* szPath, char const* szAttr) const;
    /**
    @brief ��ȡһ��XML�ڵ���������ԡ�
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @param [in] attrs �ڵ��������������pair��firstΪ��������secondΪ���Ե�ֵ
    @return false���ڵ㲻���ڣ����򷵻ؽڵ�������б�
    */
    bool getElementAttrs(char const* szPath, list<pair<char*, char*> >& attrs) const;
    /**
    @brief ��ȡ[\<aa\>xxxx\</aa\>]����ʽ�Ľڵ������xxxx��
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @param [in] strData �ڵ������
    @return false���ڵ㲻���ڻ���\<aa\>xxxx\</aa\>�ĸ�ʽ���ڵ���ڶ���Ϊ�˸�ʽ
    */
    bool getElementData(char const* szPath, string& strData) const;
    /**
    @brief ��ȡ�ڵ��Tree Node��
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @return NULL���ڵ㲻���ڣ�·���Ľڵ�
    */
    XmlTreeNode const* getElementNode(char const* szPath) const;
    ///��ȡ�ڵ�ĸ�
    XmlTreeNode const * getRoot() const
    {
        return m_root; 
    }
    ///�ж�xml�ı����Ƿ�ΪGBK
    bool isGbk() const 
    {
        return m_bGbk;
    }
private:
    /**
    @brief ֪ͨ����һ��XML�����ݽڵ㡣
    @param [in] name XML�ڵ������
    @param [in] atts XML�ڵ�����ԣ�atts[2n]Ϊ���Ե����֣�atts[2n+1]Ϊ���Ե�ֵ����atts[2n]ΪNULL����ʾ���Խ���
    @return void
    */
    void startElement(const XML_Char *name, const XML_Char **atts);
    /**
    @brief ֪ͨ�뿪һ��XML�����ݽڵ㡣
    @param [in] name XML�ڵ������
    @return void
    */
    void endElement(const XML_Char *name);
    /**
    @brief ֪ͨһ���ڵ��ڵ����ݡ�
    @param [in] s ���ݵ����ݣ������ΪUTF8�ı���
    @param [in] len ���ݵ����ݵĳ��ȡ�
    @return void
    */
    void characterData(const XML_Char *s, int len);
    /**
    @brief ���ַ���ΪGBK��gb2312����expat��UTF-8�����ΪGBK��gb2312�ı����ʽ��
    @param [in] value expat�����UTF-8���ַ���
    @param [in] uiValueLen value�ĳ���
    @return ����GBK��gb2312������ַ���
    */
    char const* charsetValue(XML_Char const* value, CWX_UINT32 uiValueLen);
private:
    ///ע�����е�expat���¼�������
    void regDefHandlers(void);
    ///����һ��XML�ڵ���¼�������
    static XMLCALL void elementStartHandler(void *userData,
        const XML_Char *name, const XML_Char **atts);
    ///�뿪һ��XML�ڵ���¼�������
    static XMLCALL void elementEndHandler(void *userData,
        const XML_Char *name);
    ///�ڵ��ڲ����ݵĽ��ܺ���
    static XMLCALL void characterDataHandler(void *userData,
        const XML_Char *s, int len);
    ///GBK��gb2312���ַ���ת��API
    static XMLCALL int convert(void* data, char const* s);
    ///GBK��gb2312�ַ�����ת�����¼�����
    static XMLCALL int encodingHandler(void* userData, XML_Char const* name, XML_Encoding* info);
private:
    XML_Parser  m_expatParser;///<expat������
    CwxCharPool m_memPool;///<�ַ��ڴ��
    XmlTreeNode* m_root;///<���ڵ�
    XmlTreeNode* m_pCur;///<���������еĵ�ǰ�ڵ�
    bool      m_bGbk;///<�Ƿ�ΪGBK����
    XML_Char*  m_szGbkBuf;///<GBK����ת������ʱBUF
    CWX_UINT32 m_uiGbkBufLen;///<m_szGbkBuf�Ŀռ��С
};

/**
@class XmlFileConfigParser
@brief ��XML���ļ�������XmlTreeNode��֯�Ľڵ���������֧��expatĬ��֧�ֵ��ַ�������֧��GBK��gb2312�ַ���
*/
class  XmlFileConfigParser
{
public:
    /**
    @brief ���캯����
    @param [in] uiAvgTokenLen XML�е����ݽڵ��ƽ������
    @param [in] uiAvgXmlSize Ҫ������XML��ƽ����С
    */
    XmlFileConfigParser(CWX_UINT32 uiAvgTokenLen=1024, CWX_UINT32 uiAvgXmlSize=4096);
    ///��������
    virtual ~XmlFileConfigParser(void);
public:
    /**
    @brief ��szXml�����XML�ı���������XmlTreeNode�Ľڵ�����
    @param [in] strFileName XML�ļ���
    @return true�������ɹ���false������ʧ��
    */
    bool parse(string const& strFileName);
    /**
    @brief ��ȡһ��XML�ڵ�����ԡ�
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @param [in] szAttr �ڵ��������
    @return NULL�������ڣ�����Ϊ�ڵ����Ե���ֵ
    */
    char const* getElementAttr(char const* szPath, char const* szAttr) const 
    {
        return m_parser.getElementAttr(szPath, szAttr);
    }
    /**
    @brief ��ȡһ��XML�ڵ���������ԡ�
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @param [in] attrs �ڵ��������������pair��firstΪ��������secondΪ���Ե�ֵ
    @return false���ڵ㲻���ڣ����򷵻ؽڵ�������б�
    */
    bool getElementAttrs(char const* szPath, list<pair<char*, char*> >& attrs) const
    {
        return m_parser.getElementAttrs(szPath, attrs);
    }
    /**
    @brief ��ȡ[\<aa\>xxxx\</aa\>]����ʽ�Ľڵ������xxxx��
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @param [in] strData �ڵ������
    @return false���ڵ㲻���ڻ���\<aa\>xxxx\</aa\>�ĸ�ʽ���ڵ���ڶ���Ϊ�˸�ʽ
    */
    bool getElementData(char const* szPath, string& strData) const 
    {
        return m_parser.getElementData(szPath, strData);
    }
    /**
    @brief ��ȡ�ڵ��Tree Node��
    @param [in] szPath XML�Ľڵ㣬����key:key1:key2�ĸ�ʽ�������ڵ��ԡ�:���ָ�
    @return NULL���ڵ㲻���ڣ�·���Ľڵ�
    */
    XmlTreeNode const* getElementNode(char const* szPath) const 
    {
        return m_parser.getElementNode(szPath);
    }
    ///��ȡ�ڵ�ĸ�
    XmlTreeNode const * getRoot() const
    { 
        return m_parser.getRoot(); 
    }
    ///�ж�xml�ı����Ƿ�ΪGBK
    bool isGbk() const
    { 
        return m_parser.isGbk();
    }
private:
    FILE *  m_fd;///<xml�ļ���FD
    string  m_strFileName;///<xml�ļ�������
    char*   m_szBuf;///<XML�ļ���ȡBUF
    XmlConfigParser  m_parser;///<XmlConfigParser�࣬���XML �ڴ�Ľ���

};


/**
@class CwxXmlPackageConv
@brief ʵ��XML��Package���໥ת����
*/
class CWX_API CwxXmlPackageConv
{
public:
    ///���캯��������ΪXML��encode�滻�������ⲿ���й���
    CwxXmlPackageConv(CwxEncodeXml const* encode)
        :m_xmlEncode(encode)
    {
        memset(m_szErrMsg, 0x00, 512);
    }
    ///��������
    ~CwxXmlPackageConv()
    {

    }
public:
    /**
    *@brief  ��XMLת��ΪPackage.
    *@param [in] szSrc XML��
    *@param [out] szOut �����PACKAGE��
    *@param [in,out] uiOutLen ����szDesc������������γɵ�Package�Ĵ�С��
    *@return false��ʧ�ܣ�true���ɹ�.
    */ 
    bool xmlToPackage(char const * szSrc,
        char* szOut,
        CWX_UINT32& uiOutLen);
    /**
    *@brief  ��Packageת��Ϊ������XML��Ĭ�ϲ���UTF8����
    *@param [in] szRootName �γɵ�XML���ڵ������,���Ϊ�գ����ʾû�и��ڵ㡣
    *@param [in] szSrc package��buf��
    *@param [in] uiSrcLen Package�Ĵ�С��
    *@param [out] szOut �����XML��
    *@param [in,out] uiOutLen ����XML��BUF��С������γɵ�XML�ĳ��ȡ�
    *@param [in] szXmlTitile XML�ı��⣬����ָ������Ϊ��<?xml version='1.0' encoding=\"utf-8\" ?>��
    *@return false��ʧ�ܣ�true���ɹ�.
    */ 
    bool packageToXml(char const* szRootName,
        char const * szSrc,
        CWX_UINT32 uiSrcLen,
        char* szOut,
        CWX_UINT32& uiOutLen,
        char const* szXmlTitile=NULL);
    /**
    *@brief  ��Packageת��Ϊ������XML��Ĭ�ϲ���UTF8����
    *@param [in] szRootName �γɵ�XML���ڵ������,���Ϊ�գ����ʾû�и��ڵ㡣
    *@param [in] package package��
    *@param [out] szOut �����XML��
    *@param [in,out] uiOutLen ����XML��BUF��С������γɵ�XML�ĳ��ȡ�
    *@param [in] szXmlTitile XML�ı��⣬����ָ������Ϊ��<?xml version='1.0' encoding=\"utf-8\" ?>��
    *@return false��ʧ�ܣ�true���ɹ�.
    */ 
    bool packageToXml(char const* szRootName,
        CwxPackageReaderEx& package,
        char* szOut,
        CWX_UINT32& uiOutLen,
        char const* szXmlTitile=NULL);
    /**
    *@brief  ��packageת��Ϊ��szNodeNameΪ�ڵ��XMLƬ��.
    *@param [in] szNodeName �γɵ�XML�Ľڵ������,���Ϊ�գ����ʾû�и��ڵ㡣
    *@param [in] szSrc package��buf��
    *@param [in] uiSrcLen PACKAGE�ĳ��ȡ�
    *@param [out] szOut �����XML��
    *@param [in,out] uiOutLen ����XML��BUF��С������γɵ�XML�ĳ��ȡ�
    *@return false��ʧ�ܣ�true���ɹ�.
    */ 
    bool packageToXmlNode(char const* szNodeName,
        char const * szSrc,
        CWX_UINT32 uiSrcLen,
        char* szOut,
        CWX_UINT32& uiOutLen);
    /**
    *@brief  ��packageת��Ϊ��szNodeNameΪ�ڵ��XMLƬ��.
    *@param [in] szNodeName �γɵ�XML�Ľڵ������,���Ϊ�գ����ʾû�и��ڵ㡣
    *@param [in] package package��
    *@param [out] szOut �����XML��
    *@param [in,out] uiOutLen ����XML��BUF��С������γɵ�XML�ĳ��ȡ�
    *@return false��ʧ�ܣ�true���ɹ�.
    */ 
    bool packageToXmlNode(char const* szNodeName,
        CwxPackageReaderEx& package,
        char* szOut,
        CWX_UINT32& uiOutLen);
    ///���ش�����Ϣ
    char const* getErrMsg() const{ return m_szErrMsg;}
private:
    ///xmlת��Ϊpackage
    bool xmlToPackage(XmlTreeNode const * treeNode, char* szOut, CWX_UINT32& uiOutLen);
    ///packageת��Ϊxml
    bool packageToXml(CwxKeyValueItemEx const& item, char* szOut, CWX_UINT32& uiOutLen);
    ///append <key>
    bool appendXmlKeyBegin(char const* szKey, CWX_UINT16 unKeyLen, char* szOut, CWX_UINT32& uiOutLen);
    ///append </key>
    bool appendXmlKeyEnd(char const* szKey, CWX_UINT16 unKeyLen, char* szOut, CWX_UINT32& uiOutLen);
private:
    CwxEncodeXml const*  m_xmlEncode;///<xml�������ַ��滻��
    char m_szErrMsg[512];///<����buf
};

#endif 
