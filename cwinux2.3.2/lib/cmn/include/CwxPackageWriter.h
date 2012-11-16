#ifndef  __CWX_PACKAGE_WRITER_H__
#define  __CWX_PACKAGE_WRITER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxPackageWriter.h
*@brief CwxPackageWrite����
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
@class CwxPackageWriter
@brief �γ�KEY/VALUE��PACKAGE��writer�ࡣ
*/

class  CWX_API CwxPackageWriter
{
public:
    ///enum for CwxPackage
    enum{
        ERR_MSG_LEN=511, ///< ERR bug size
        DEF_BUF_LEN=32*1024 ///<def buf len
    };
    ///���캯����
    CwxPackageWriter(CWX_UINT32 uiBufLen=DEF_BUF_LEN);
    ///����
    ~CwxPackageWriter();
public:
    ///����һ����package
    void beginPack();
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, char const* szData, CWX_UINT32 uiDataLen, bool bKeyValue = false);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, string const& strData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT8 ucData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_INT8 cData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_INT16 nData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT32 uiData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_INT32 iData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT64 ullData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_INT64 llData);


    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, char const* szData, CWX_UINT32 uiDataLen, bool bKeyValue = false);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, string const& strData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT8 ucData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT8 cData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT16 unData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT16 nData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT32 uiData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT32 iData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_UINT64 ullData);
    ///Add a new key.true: success; false:failure, get err-msg by GetError().
    bool addKeyValue(char const* szKey, CWX_UINT16 unKeyLen, CWX_INT64 llData);

    ///Package msg . true: success; false:failure, get err-msg by GetError().
    bool pack();
    ///��ȡ��ǰpackage��Key��������
    CWX_UINT32 getKeyNum() const ;
    ///return package's buf size
    CWX_UINT32 getBufSize() const;
    ///return msg's size
    CWX_UINT32 getMsgSize() const ;
    ///return msg
    char const* getMsg() const ;
    ///return the errmsg
    char const* getErrMsg() const;
private:
    ///���pack/unpack�ĺۼ�
    void reset();
private:
    CWX_UINT32            m_uiKeyNum; ///<package's key number
    char*                 m_szPackMsg; ///<package's dat
    CWX_UINT32            m_uiPackBufLen;///<package's buf length
    CWX_UINT32            m_uiCurPackPos;///<package's size
    char                  m_szErr[ERR_MSG_LEN + 1];///<��������
};


CWINUX_END_NAMESPACE

#include "CwxPackageWriter.inl"

#include "CwxPost.h"

#endif
