#ifndef  __CWX_PACKAGE_H__
#define  __CWX_PACKAGE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxPackage.h
*@brief CwxPackage����
*@author cwinux@gmail.com
*@version 1.0
*@date  2009-06-05
*@warning  nothing
*@bug    
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxCommon.h"
#include "CwxEscape.h"


CWINUX_BEGIN_NAMESPACE
///Key/value��Item��
class CWX_API CwxKeyValueItem
{
public:
    enum{
        BIT_PACKAGE = 31, ///<package's sign
        MAX_KV_LEN=0X7FFFFFFF///<MAX dat length
    };
public:
    ///���캯��
    CwxKeyValueItem()
    {
        memset(this, 0x00, sizeof(CwxKeyValueItem));
    }
    ///��������
    ~CwxKeyValueItem()
    {

    }
    ///��������
    CwxKeyValueItem(CwxKeyValueItem const& obj)
    {
        memcpy(this, &obj, sizeof(CwxKeyValueItem));
    }
    ///��ֵ����
    CwxKeyValueItem& operator=(CwxKeyValueItem const& obj)
    {
        memcpy(this, &obj, sizeof(CwxKeyValueItem));
        return *this;
    }
public:
    ///������ղ���
    void reset()
    {
        memset(this, 0x00, sizeof(CwxKeyValueItem));
    }
public:
    char const*          m_szKey; ///<key������
    char const*          m_szData; ///<key������
    CWX_UINT32      m_uiDataLen; ///<���ݵĳ���
    CWX_UINT16      m_unKeyLen; ///<key�ĳ���
    bool            m_bKeyValue; ///<true��value�ı���Ҳ��key/value��ʽ;false��value����key/value��ʽ
};

/**
*@class CwxPackage
*@brief pack/unpackһ�����ݰ�,���ڲ���package buffer���ⲿ��buf���ڲ��������䡣
*
*���ݰ�����һ�����key/value����ɣ�ÿ��key/value���Ĳ�����ɣ���ʽ���£�<br>
*�������ֽ����KEY/VALUE����볤�ȡ���key���ȡ�����\\0������key������\\0������value����ÿ���ֵĺ������£�<br>
*�������ֽ����KEY/VALUE����볤�ȡ�����Ϊһ�������ֽ����32λ���֣���Ϊ�����ֽ����BIT31Ϊ���λ��<br>
*                        ��31λΪ�����������ֽ����KEY/VALUE����볤�ȡ����ڵ�KEY/VALUE�ĳ��ȡ�<br>
*                        ���ڱ��λ��1��ʾkey��value��һ��key/value�ṹ�������ǡ�<br>
*��key���ȡ���Ϊ�����ֽ����16���֣���Ϊ�����ֽ�����ʾkey�ĳ��ȣ���Ϊ�˼ӿ����ٶ�,key�����ִ�Сд����<br>
*��key����key�����֣���package����\\0������key�����ֿ����ظ������ظ�������£�ֻ��ͨ��index��ȡ<br>
*��value����key����ֵ����package����\\0������<br>
* m_bIndex����ֵ����unpack()��beginPack()��ʱ�����ָ������makeIndex()��ʱ��Ҳ���趨ΪTRUE��
*/
class CWX_API CwxPackage
{
public:
    /**
    *@brief ��ȡpackage�е���һ��Key��
    *@param [in] szMsg ��Ҫ�����package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
    *@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
    */
    static int  getNextKey(char const* szMsg, CWX_UINT32 uiMsgLen, CwxKeyValueItem& item);
    /**
    *@brief ��ȡpackage�еĵ�uiIndex Key/Value�����unIndexΪ0�����൱��GetNextKey()��
    *@param [in] szMsg ��Ҫ�����package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [in] uiIndex Ҫ��ȡ��key��������
    *@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
    *@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
    */
    static int  getKeyByIndex(char const *szMsg, CWX_UINT32 uiMsgLen, CWX_UINT32 uiIndex, CwxKeyValueItem& item);
    /**
    *@brief ��ȡpackage�еĵ�һ��key������ΪszKey��Key/Value��
    *@param [in] szMsg ��Ҫ�����package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [in] szKey Ҫ��ȡ��key�����֣���key�������ظ�����ֻ��ȡ��һ����
    *@param [out] item ��key/value���ڣ���ͨ��item����Key/value����Ϣ��
    *@param [in] bCaseSensive key�������Ƿ��Сд���С�true����Сд���У�����Ϊ�����С�ȱʡ���С�
    *@return -1�����ĸ�ʽ�Ƿ���0:�����ڣ�>0��Key/Value�ĳ��ȡ�
    */
    static int  getKeyByName(char const *szMsg, CWX_UINT32 uiMsgLen, char const* szKey, CwxKeyValueItem& item, bool bCaseSensive=true);
    /**
    *@brief ��package�����һ����key/value��
    *@param [in,out] szMsg ��Ҫ�����package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [in] szKey Ҫ��ӵ�key�����֡�
    *@param [in] szValue key��data��
    *@param [in] uiDatalen data�ĳ���
    *@param [in] bKeyValue data�Ƿ�Ϊkey/value
    *@return -1�����Ŀռ�̫С��>=0 ����İ��ĳ��ȡ�
    */
    static int  appendKey(char *szMsg, CWX_UINT32 uiMsgLen, char const* szKey, CWX_UINT16 unKeyLen, char const* szValue, CWX_UINT32 uiDatalen, bool bKeyValue = false);
    /**
    *@brief ��package��ɾ��key����ΪszKey��Key/value��
    *@param [in, out] szMsg package��
    *@param [in, out] uiMsgLen package�ĳ��ȡ�
    *@param [in] szKey Ҫɾ����key���֡�
    *@param [in] bAll �Ƿ�Ҫɾ������key������ΪszKey��key/value
    *@param [in] bCaseSensive key�������Ƿ��Сд���С�true����Сд���У�����Ϊ�����С�ȱʡ���С�
    *@return -1����Ч��package��0��û�з��֣�>0��ɾ����������
    */
    static int  removeKey(char *szMsg, CWX_UINT32& uiMsgLen, char const* szKey, bool bAll=false, bool bCaseSensive=true);
    /**
    *@brief ��package��ɾ����unIndex��Key��
    *@param [in,out] szMsg package��
    *@param [in,out] uiMsgLen package�ĳ��ȡ�
    *@param [in] unIndex Ҫɾ��key��Index��
    *@return -1����Ч��package��0��û�з��֣�1��ɾ����һ��KEY��
    */
    static int  removeKey(char *szMsg, CWX_UINT32& uiMsgLen, CWX_UINT16 unIndex);
    /**
    *@brief ��package�е�һ��Key������ΪszKey�����ݣ��޸�ΪszDataָ�������ݡ�
    *@param [in,out] szMsg package��
    *@param [in,out] uiMsgLen package�ĳ��ȡ�
    *@param [in] uiMaxMsgLen Package�����������
    *@param [in] szKey Ҫ�޸ĵ�key��
    *@param [in] szData Ҫ�ı�ɵ���data��
    *@param [in] uiDataLen Ҫ�ı�ɵ���data�ĳ��ȡ�
    *@param [in] bKeyValue �������Ƿ�ΪKey/value��ʽ��
    *@param [in] bCaseSensive key�������Ƿ��Сд���С�true����Сд���У�����Ϊ�����С�ȱʡ���С�
    *@return -2�ռ䲻����-1����Ч��package��0��û�з��֣�1���޸���һ��KEY��
    */
    static int  modifyKey(char *szMsg, CWX_UINT32& uiMsgLen, CWX_UINT32 uiMaxMsgLen, char const* szKey, CWX_UINT16 unKeyLen, char const* szData, CWX_UINT32 uiDataLen, bool bKeyValue=false, bool bCaseSensive=true);
    /**
    *@brief ��package�е�unIndex��Key�����ݣ��޸�ΪszDataָ�������ݡ�
    *@param [in,out] szMsg package��
    *@param [in,out] uiMsgLen package�ĳ��ȡ�
    *@param [in] uiMaxMsgLen Package�����������
    *@param [in] unIndex Ҫ�޸ĵ�key��������
    *@param [in] szData Ҫ�ı�ɵ���data��
    *@param [in] uiDataLen Ҫ�ı�ɵ���data�ĳ��ȡ�
    *@param [in] bKeyValue �������Ƿ�ΪKey/value��ʽ��
    *@return -2�ռ䲻����-1����Ч��package��0��û�з��֣�1���޸���һ��KEY��
    */
    static int  modifyKey(char *szMsg, CWX_UINT32& uiMsgLen, CWX_UINT32 uiMaxMsgLen, CWX_UINT16 unIndex,char const* szData, CWX_UINT32 uiDataLen, bool bKeyValue=false);
    /**
    *@brief ��package�����ݣ�������ı�������Ƕ�׵�key����Ƕ�������
    *@param [in] szMsg package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [out] szOutBuf dump����key/value������
    *@param [in,out] uiOutBufLen ����szOutBuf�ĳ��ȣ������γɵ����ݵĳ���
    *@param [in] szTab ����ÿ��ε�Ƕ��key���������һ��key�������ַ�����NULL��ʾ��������Ĭ��Ϊ\\t��
    *@param [in] szKeyBegin һ��key/value�Ŀ�ʼ�ַ���Ĭ��ΪNULL��
    *@param [in] szKeyEnd һ��key/value�Ľ����ַ���Ĭ��Ϊ"\\n"��
    *@param [in] pEscape ��key������data��escape����NULL��ʾ�������ַ����룬ʹ��escape��encode������
    *@return -2�ռ䲻����-1����Ч��package�����򷵻�dump�����ַ����ĳ��ȡ�
    */
    static int  dump(char const* szMsg, CWX_UINT32 uiMsgLen, char* szOutBuf, CWX_UINT32& uiOutBufLen, char const* szTab="\t", char const* szKeyBegin=NULL, char const* szKeyEnd="\n", CwxEscape const* pEscape=NULL);
    /**
    *@brief ���szMsg�Ƿ���һ����Ч��Package.uiMsgLenΪ0��ʱ�򣬱�ʾΪ�հ����հ���һ����Ч�İ���
    *@param [in] szMsg Ҫ���İ�
    *@param [in] uiMsgLen ���ĳ���
    *@return true:��Ч�İ���false����Ч�İ�.
    */
    static bool isValidPackage(char const *szMsg, CWX_UINT32 uiMsgLen);
    ///��ȡpackage��key������, -1: invalid package
    static int getKeyValueNum(char const* szMsg, CWX_UINT32 uiMsgLen);
    ///ͨ��Key�ĳ��ȼ�data�ĳ��ȣ���ȡ������Key/value���ȡ�
    static CWX_UINT32 getKvLen(CWX_UINT16 unKeyLen, CWX_UINT32 uiDataLen);
    ///ͨ��key/value�ĳ��ȼ�key�ĳ��ȣ���ȡdata�ĳ���
    static CWX_UINT32 getDataLen(CWX_UINT32 uiKeyValueLen, CWX_UINT16 unKeyLen);
    ///����key��key/value�е�ƫ��
    static CWX_UINT16 getKeyOffset();
private:
    /**
    *@brief ��package�����ݣ�������ı�������Ƕ�׵�key����Ƕ�������
    *@param [in] szMsg package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [out] szOutBuf dump����key/value������
    *@param [in,out] uiOutBufLen ����szOutBuf�ĳ��ȣ������γɵ����ݵĳ���
    *@param [in] szTab ����ÿ��ε�Ƕ��key���������һ��key�������ַ�����NULL��ʾ��������Ĭ��Ϊ\\t��
    *@param [in] uiTabNum ��ǰkey��Ƕ�ײ�Ρ�
    *@param [in] szKeyBegin һ��key/value�Ŀ�ʼ�ַ���Ĭ��ΪNULL��
    *@param [in] szKeyEnd һ��key/value�Ľ����ַ���Ĭ��Ϊ"\\n"��
    *@param [in] pEscape ��key������data��escape����NULL��ʾ�������ַ����룬ʹ��escape��encode������
    *@return -2�ռ䲻����-1����Ч��package�����򷵻�dump�����ַ����ĳ��ȡ�
    */
    static int dumpEx(char const* szMsg, CWX_UINT32 uiMsgLen, char* szOutBuf, CWX_UINT32& uiOutBufLen, char const* szTab="\t", CWX_UINT32 uiTabNum = 1, char const* szKeyBegin=NULL, char const* szKeyEnd="\n", CwxEscape const* pEscape=NULL);

private:
    ///���pack/unpack�ĺۼ�
    void reset();
    ///���캯����
    CwxPackage(){}
    ///����
    ~CwxPackage(){}
};


CWINUX_END_NAMESPACE

#include "CwxPackage.inl"

#include "CwxPost.h"

#endif
