#ifndef __CWX_MSG_HEAD_H__
#define __CWX_MSG_HEAD_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxMsgHead.h
*@brief CWINUXͨ�Žṹ��Э��ͷ�����ļ�
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-05-30
*@warning  ��.
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"

CWINUX_BEGIN_NAMESPACE

class CwxMsgBlock;
/**
*@class  CwxMsgHead
*@brief  ��Ϣͷ����������
*/
class CWX_API CwxMsgHead
{
public:
    enum{
        ATTR_SYS_MSG = (1<<0), ///<ϵͳ��Ϣ, bit0
        ATTR_COMPRESS = (1<<1) ///<������ѹ������
    };
    enum{
        MSG_TYPE_KEEPALIVE = 1, ///<keep alive  msg 
        MSG_TYPE_KEEPALIVE_REPLY = 2 ///<keep alive reply msg 
    };
public:
    enum{
        MSG_HEAD_LEN = 14 ///<��Ϣͷ�ĳ���
    };    
    ///���캯��
    CwxMsgHead()
    {
        memset(this, 0x00, sizeof(CwxMsgHead));
    }
    CwxMsgHead(CWX_UINT8 ucVersion,
        CWX_UINT8 ucAttr,
        CWX_UINT16 unMsgType,
        CWX_UINT32 uiTaskId,
        CWX_UINT32 uiDateLen)
    {
        m_ucVersion = ucVersion;
        m_ucAttr = ucAttr;
        m_unMsgType = unMsgType;
        m_uiTaskId = uiTaskId;
        m_uiDataLen = uiDateLen;
    }

    ///��������
    CwxMsgHead(CwxMsgHead const& item)
    {
        memcpy(this, &item, sizeof(item));
    }
    ///��ֵ����
    CwxMsgHead& operator=(CwxMsgHead const& item)
    {
        memcpy(this, &item, sizeof(item));
        return *this;
    }
    ///����Ϣͷ����������ֽ�������ݰ�����, 
    inline char const* toNet();
    ///���յ�����Ϣͷ�������Ϣͷ����
    /**
    *@param [in] szHead ��Ϣ��ͷ
    *@return false:У�������true:�����ȷ
    */
    inline bool fromNet(char const* szHead);
    ///�Ƿ�keepalive��Ϣ
    inline bool isKeepAlive(bool bReply) const
    {
        if (bReply) return isAttr(ATTR_SYS_MSG) && (MSG_TYPE_KEEPALIVE_REPLY==m_unMsgType);
        return isAttr(ATTR_SYS_MSG) && (MSG_TYPE_KEEPALIVE==m_unMsgType);
    }
    ///�Ƿ���ϵͳ��Ϣ
    inline bool isSysMsg() const
    {
        return isAttr(ATTR_SYS_MSG);
    }
    ///�γ�KeepAlive��Ϣ����
    /**
    *@param [in] bReply false��keepalive��ѯ����true��keepalive�ظ�����
    *@return keepalive����Ϣ����
    */
    CwxMsgBlock* packKeepalive(bool bReply);
    ///�γ�һ��ϵͳ��Ϣ�ķ�����Ϣ����Ϊ�գ��򲻻ظ�
    /**
    *@param [in] header �յ���ϵͳ��Ϣ����Ϣͷ
    *@param [in] msg �յ���ϵͳ��Ϣ�����ݡ�
    *@return keepalive����Ϣ����
    */
    CwxMsgBlock* packSysMsgReply(CwxMsgHead const& header, CwxMsgBlock* msg);
    ///���
    inline void reset()
    {
        memset(this, 0x00, sizeof(CwxMsgHead));
    }
public:
    ///����Ƿ�������ָ��������
    inline bool isAttr(CWX_UINT8 ucAttr) const
    {
        return 0 != (m_ucAttr&ucAttr);
    }
    ///����ָ��������
    inline void addAttr(CWX_UINT8 ucAttr)
    {
        m_ucAttr |= ucAttr;
    }
    ///���ָ��������
    inline void clrAttr(CWX_UINT8 ucAttr)
    {
        m_ucAttr &= ~ucAttr;
    }
    ///set attr
    inline void setAttr(CWX_UINT8 ucAttr)
    {
        m_ucAttr = ucAttr;
    }
    ///get attr
    inline CWX_UINT8 getAttr() const
    {
        return m_ucAttr;
    }
    ///set verion
    inline void setVersion(CWX_UINT8 ucVersion)
    {
        m_ucVersion = ucVersion;
    }
    ///get version
    inline CWX_UINT8 getVersion() const
    {
        return m_ucVersion;
    }
    ///set msg type
    inline void setMsgType(CWX_UINT16 unMsgType)
    {
        m_unMsgType = unMsgType;
    }
    ///get msg type
    inline CWX_UINT16 getMsgType() const
    {
        return m_unMsgType;
    }
    ///set task id
    inline void setTaskId(CWX_UINT32 uiTaskId)
    {
        m_uiTaskId = uiTaskId;
    }
    ///get task id
    inline CWX_UINT32 getTaskId() const
    {
        return m_uiTaskId;
    }
    ///set data len����������ͷ
    inline void setDataLen(CWX_UINT32 uiDataLen)
    {
        m_uiDataLen = uiDataLen;
    }
    ///get data len����������ͷ
    inline CWX_UINT32 getDataLen() const
    {
        return m_uiDataLen;
    }
    ///����package������С
    static void setMaxMsgSize(CWX_UINT32 uiSize);
    ///��ȡpackage������С
    static CWX_UINT32 getMaxMsgSize();
private:
    CWX_UINT8      m_ucVersion;  ///<��Ϣ�汾��
    CWX_UINT8      m_ucAttr;     ///<��Ϣ����
    CWX_UINT16     m_unMsgType;  ///<��Ϣ����
    CWX_UINT32     m_uiTaskId;   ///<�����ID
    CWX_UINT32     m_uiDataLen;  ///<���͵����ݳ��ȣ���������ͷ
    char           m_szHead[MSG_HEAD_LEN]; ///< msg's data
    static CWX_UINT32  m_uiMaxMsgSize;
};

CWINUX_END_NAMESPACE
#include "CwxMsgHead.inl"
#include "CwxPost.h"
#endif

