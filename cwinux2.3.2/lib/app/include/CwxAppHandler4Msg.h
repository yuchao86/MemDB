#ifndef __CWX_APP_HANDLER_4_MSG_H__
#define __CWX_APP_HANDLER_4_MSG_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4Msg.h
@brief �������msg�շ���TCP��PIPE��������IOͨ�š������Handle����CwxAppHandler4Msg��
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxMsgBlock.h"
#include "CwxMsgHead.h"
#include "CwxSocket.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxLogger.h"
#include "CwxAppHandler4Base.h"
#include "CwxAppConnInfo.h"
#include "CwxAppReactor.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxAppHandler4Msg
@brief TCP��PIPE��������IOͨ�š�״̬�����Handle����
*/
class CwxAppFramework;
class CwxAppReactor;

class CWX_API CwxAppHandler4Msg:public CwxAppHandler4Base
{
public:
    ///���캯��
    CwxAppHandler4Msg(CwxAppFramework* pApp,
        CwxAppReactor *reactor);
    ///��������
    ~CwxAppHandler4Msg();
public:
    /**
    @brief ��ʼ�����������ӣ�����Reactorע������
    @param [in] arg �������ӵ�acceptor��ΪNULL
    @return -1���������������ӣ� 0�����ӽ����ɹ�
    */
    virtual int open (void * arg= 0);
    /**
    @brief ���������ϵ��¼�
    @param [in] event ���ӵ�handle�ϵ��¼�
    @param [in] handle  �������¼���handle��
    @return -1������ʧ�ܣ� 0������ɹ�
    */
    virtual int handle_event(int event, CWX_HANDLE handle=CWX_INVALID_HANDLE);
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE);
    /**
    @brief ��ȡ���ӵĶԶ˵�ַ
    @param [in,out] szBuf ���ص�ַ��buf,��ȡ�ɹ�����\\0������
    @param [in] unSize szBuf�Ĵ�С��
    @return ����szBuf
    */
    virtual char* getRemoteAddr(char* szBuf, CWX_UINT16 unSize);
    /**
    @brief ��ȡ���ӵĶԶ�port
    @return ���ӶԶ˵�port
    */
    virtual CWX_UINT16 getRemotePort();
    /**
    @brief ��ȡ���ӱ��˵ĵ�ַ
    @param [in,out] szBuf ���ص�ַ��buf,��ȡ�ɹ�����\\0������
    @param [in] unSize szBuf�Ĵ�С��
    @return ����szBuf
    */
    virtual char* getLocalAddr(char* szBuf, CWX_UINT16 unSize);
    /**
    @brief ��ȡ���ӵı���port
    @return ���ӶԶ˵�port
    */
    virtual CWX_UINT16 getLocalPort();
    ///������Ϣ
    virtual int handle_output();
    ///������Ϣ
    virtual int handle_input();
    ///��ʱ
    virtual void handle_timeout();
public:
    ///��ȡ���ӵ���Ϣ����
    CwxAppConnInfo& getConnInfo();
    ///��ȡ���ӵ���Ϣ����
    CwxAppConnInfo const& getConnInfo() const;
    ///��ն���
    void clear();
    ///��ȡ��һ�������͵���Ϣ������ֵ��0��û�д�������Ϣ��1,�����һ����������Ϣ
    inline int getNextMsg();
    ///��Ҫ���͵���Ϣ�Ŷӣ�����ֵ��true���ɹ���false��ʧ�ܣ�ʧ��ʱ��Ϊ��������
    inline bool putMsg(CwxMsgBlock* msg);
    ///����û����Ϣ���ͣ�ʹ���ӵķ��ͼ������.����ֵ�� -1: failure, 0: success
    inline int cancelWakeup();
    ///�������ӵĿ�д��أ��Է���δ������ϵ�����.����ֵ�� -1:failure�� 0:success��
    inline int wakeUp();
    ///����Ƿ�suspend���ӵĿɶ�����д���
    bool isStopListen() const;
    ///����stop listen
    void setStopListen(bool bStop);
    ///�Ƿ��пɷ��͵����ݰ�
    bool isEmpty() const;
    ///��ȡ����ʱ�Ĵ������
    int getConnErrNo() const;
    ///��ȡapp
    CwxAppFramework* getApp()
    {
        return m_pApp;
    }
private:
    ///�Է������ķ�ʽ��������Ϣ������ֵ,-1: failure; 0: not send all;1:send a msg
    inline int nonBlockSend();
protected:
    CwxAppFramework* m_pApp;
    CwxMsgHead          m_header;
    CwxAppConnInfo         m_conn;///<connection information
    CWX_UINT32             m_uiSendByte; ///the sent bytes number for current message.
    CwxMsgBlock*         m_curSndingMsg; ///<current sending msg;
    CwxMsgBlock*         m_waitSendMsgHead; ///<The header for wait to be sent msg.
    CwxMsgBlock*         m_waitSendMsgTail;   ///<The tail for wait to be sent msg.
    char                  m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN];///<the buf for header
    CWX_UINT32             m_uiRecvHeadLen; ///<recieved msg header's byte number.
    CWX_UINT32             m_uiRecvDataLen; ///<recieved data's byte number.
    CwxMsgBlock*        m_recvMsgData; ///<the recieved msg data
    bool                  m_bStopListen; ///<stop listen for passive connection
    int                   m_connErrNo;
};
CWINUX_END_NAMESPACE

#include "CwxAppHandler4Msg.inl"

#include "CwxPost.h"

#endif
