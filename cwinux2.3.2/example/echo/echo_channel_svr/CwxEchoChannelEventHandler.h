#ifndef __CWX_ECHO_CHANNEL_EVENT_HANDLER_H__
#define __CWX_ECHO_CHANNEL_EVENT_HANDLER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

#include "CwxAppHandler4Channel.h"
#include "CwxAppChannel.h"

CWINUX_USING_NAMESPACE

///echo����Ĵ���handle��Ϊcommand��handle
class CwxEchoChannelEventHandler : public  CwxAppHandler4Channel
{
public:
    ///���캯��
    CwxEchoChannelEventHandler(CwxAppChannel *channel):CwxAppHandler4Channel(channel)
    {
        m_ullMsgNum = 0;
        m_uiRecvHeadLen = 0; ///<recieved msg header's byte number.
        m_uiRecvDataLen = 0; ///<recieved data's byte number.
        m_recvMsgData = NULL; ///<the recieved msg data
    }
    ///��������
    virtual ~CwxEchoChannelEventHandler()
    {
        if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
    }
public:
    /**
    @brief ���ӿɶ��¼�������-1��close()�ᱻ����
    @return -1������ʧ�ܣ������close()�� 0������ɹ�
    */
    virtual int onInput();
    /**
    @brief ֪ͨ���ӹرա�
    @return �����������ӣ�1������engine���Ƴ�ע�᣻0������engine���Ƴ�ע�ᵫ��ɾ��handler��-1����engine�н�handle�Ƴ���ɾ����
    */
    virtual int onConnClosed();
private:
    void   replyMessage();

private:
    CWX_UINT64              m_ullMsgNum;
    CwxMsgHead             m_header;
    char                   m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN];
    CWX_UINT32             m_uiRecvHeadLen; ///<recieved msg header's byte number.
    CWX_UINT32             m_uiRecvDataLen; ///<recieved data's byte number.
    CwxMsgBlock*           m_recvMsgData; ///<the recieved msg data
};

#endif 
