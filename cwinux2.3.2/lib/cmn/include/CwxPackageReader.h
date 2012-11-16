#ifndef  __CWX_PACKAGE_READER_H__
#define  __CWX_PACKAGE_READER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxPackageReader.h
*@brief CwxPackageReader����
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-06-05
*@warning  nothing
*@bug    
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxCommon.h"
#include "CwxPackage.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxPackageReader
@brief Key/Value��ʽ��Package�Ķ�ȡ��
*/

class CWX_API CwxPackageReader
{
    typedef map<char const*, CwxKeyValueItem*, CwxCharCaseLess2 > CWX_PACKAGE_KEY_MAP;
    typedef map<char const*, CwxKeyValueItem*, CwxCharCaseLess2 >::const_iterator CWX_PACKAGE_KEY_MAP_ITER;
public:
    ///enum for CwxPackageReader
    enum{
        KEY_VALUE_ALIGN = 16,
        ERR_MSG_LEN=511 ///< ERR bug size
    };
    ///���캯����bCaseSensive��ʾkey�Ƿ�Ϊ��Сд���С�ȱʡΪ����
    CwxPackageReader(bool bCaseSensive=true);
    ///����
    ~CwxPackageReader();
public:
    ///�Գ���ΪuiMsgLen��package�����
    /**
    * �佫����szMsg�Ƿ���һ����Ч��package����bIndex=true������package��Key����������<br>
    * szMsg����Ϊһ���հ����հ���һ����Ч��package��
    *@param [in] szMsg ��Ҫ�����package��
    *@param [in] uiMsgLen package�ĳ��ȡ�
    *@param [in] bIndex �Ƿ��package��Key����������
    *@param [in] bCaseSensive key�������Ƿ��Сд���С�true���ǣ�false�����ǡ�
    *@return false:����true:�����ȷ��ͨ��GetError()��ȡʧ�ܵ�ԭ��
    */
    bool  unpack(char const* szMsg, CWX_UINT32 uiMsgLen, bool bIndex = true, bool bCaseSensive=true);
    ///Get key for name [szKey]. null if not exists.
    CwxKeyValueItem const*  getKey(char const* szKey, bool bSubKey=false) const;
    ///NULL: not exist, otherwise, return the key/value, index from 0.
    CwxKeyValueItem const* getKey(CWX_UINT32 index) const;
    ///Get data by key for string. false:not exist, true:get one. 
    bool getKey(char const* szKey, string& value, bool bSubKey=false) const ;
    ///Get data by key for UINT32. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_UINT32& value, bool bSubKey=false) const ;
    ///Get data by key for INT32. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_INT32& value, bool bSubKey=false) const;
    ///Get data by key for UINT16. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_UINT16& value, bool bSubKey=false) const;
    ///Get data by key for INT16. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_INT16& value, bool bSubKey=false) const;
    ///Get data by key for UINT8. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_UINT8& value, bool bSubKey=false) const;
    ///Get data by key for char. false:not exist, true:get one. 
    bool getKey(char const* szKey, char& value, bool bSubKey=false) const;
    ///Get data by key for INT64. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_INT64& value, bool bSubKey=false) const ;
    ///Get data by key for UINT64. false:not exist, true:get one. 
    bool getKey(char const* szKey, CWX_UINT64& value, bool bSubKey=false) const ;
    ///��ȡ��ǰpackage��Key��������
    CWX_UINT32 getKeyNum() const ;
    ///return msg's size
    CWX_UINT32 getMsgSize() const ;
    ///return msg
    char const* getMsg() const ;
    ///return the errmsg
    char const* getErrMsg() const;
    ///return case-sensive sign
    bool isCaseSensive() const;
private:
    ///���pack/unpack�ĺۼ�
    void reset();
private:
    CwxKeyValueItem*        m_pKeyValue; ///<key/value's vector
    CWX_PACKAGE_KEY_MAP*    m_pKeyIndex; ///<key's index
    CWX_UINT32             m_uiTotalKeyNum;///m_pKeyValue's array size
    CWX_UINT32             m_uiKeyNum; ///<package's key number
    char const*            m_szPackMsg; ///<package's dat
    CWX_UINT32             m_uiPackBufLen;///<package's buf length
    bool                  m_bIndex;///<�Ƿ���package������
    bool                  m_bCaseSensive;
    mutable CwxKeyValueItem       m_tmpKey; ///<��ʱkey;
    char                  m_szErr[ERR_MSG_LEN + 1];///<��������
};


CWINUX_END_NAMESPACE

#include "CwxPackageReader.inl"

#include "CwxPost.h"

#endif
