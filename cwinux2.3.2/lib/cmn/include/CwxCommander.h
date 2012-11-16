#ifndef __CWX_COMMANDER_H__
#define __CWX_COMMANDER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxCommander.h
@brief �ܹ�Commandģʽ����Ķ��壬Command�����¼���Event-type�������¼��ķַ�
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxMsgBlock.h"
#include "CwxTss.h"

CWINUX_BEGIN_NAMESPACE

class CWX_API CwxCommander;
/**
@class CwxCmdOp
@brief ����SVR-ID���¼��Ĵ���HANDLE�Ľӿڶ��塣
*/
class CWX_API CwxCmdOp
{
public:
    ///���캯��
    CwxCmdOp()
    {
    }
    ///��������
    virtual ~CwxCmdOp()
    {

    }
public:
    /**
    @brief ���ӽ����¼��Ĵ�������
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onConnCreated(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief ���ӹر��¼��ĵĺ�����
    @param [in] msg ���ӹرյ��¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onConnClosed(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief �յ�ͨ�����ݰ��¼��Ĵ�������
    @param [in] msg �յ�ͨ�����ݰ����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief ��Ϣ��������¼��Ĵ�������
    @param [in] msg ��Ϣ������ϵ��¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onEndSendMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief ��Ϣ����ʧ���¼��Ĵ�������
    @param [in] msg ��Ϣ����ʧ�ܵ��¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onFailSendMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief ��ʱ����¼��Ĵ�������
    @param [in] msg ��ʱ�����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onTimeoutCheck(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief HANDLE��READY���¼��Ĵ�������
    @param [in] msg HANDLE��READY���¼����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onEvent4Handle(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
    /**
    @brief �û��Զ����¼��Ĵ�������
    @param [in] msg �û��Զ����¼����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onUserEvent(CwxMsgBlock*& msg, CwxTss* pThrEnv)
    {
        CWX_UNUSED_ARG(msg);
        CWX_UNUSED_ARG(pThrEnv);
        return 0;
    }
};

/**
@class CwxCommander
@brief Command�࣬�����¼���SVR-ID��ʵ���¼����䴦��Handle��ӳ�䡣
*/
class  CWX_API CwxCommander
{
    ///��Ϣӳ�亯�����Ͷ���
    typedef int (*fnEventApi)(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    ///SVR-ID���䴦��Handle��ӳ��Hash
    typedef hash_map<CWX_UINT32, CwxCmdOp*, CwxNumHash<CWX_UINT32> > CwxEventCommandHash;
public:
    ///���캯��
    CwxCommander()
        :m_msgHash(1024)
    {
        m_arrEvent[CwxEventInfo::DUMMY] = NULL;
        m_arrEvent[CwxEventInfo::CONN_CREATED] = &CwxCommander::onConnCreated;
        m_arrEvent[CwxEventInfo::CONN_CLOSED] = &CwxCommander::onConnClosed;
        m_arrEvent[CwxEventInfo::RECV_MSG] = &CwxCommander::onRecvMsg;
        m_arrEvent[CwxEventInfo::END_SEND_MSG] = &CwxCommander::onEndSendMsg;
        m_arrEvent[CwxEventInfo::FAIL_SEND_MSG] = &CwxCommander::onFailSendMsg;
        m_arrEvent[CwxEventInfo::TIMEOUT_CHECK] = &CwxCommander::onTimeoutCheck;
        m_arrEvent[CwxEventInfo::EVENT_4_HANDLE] = &CwxCommander::onEvent4Handle;
        m_arrEvent[CwxEventInfo::SYS_EVENT_NUM] = &CwxCommander::onUserEvent;
    }
    ///��������
    ~CwxCommander()
    {
        m_msgHash.clear();
    }
public:
    ///ע��SVR-IDΪuiSvrID���¼��Ĵ�����������ֵ��0:success, -1: ��SVR-ID�Ѿ�����
    int regHandle(CWX_UINT32 uiSvrID, CwxCmdOp* pHandle);
    /**
    @brief ����Ϣ�ַ����䴦��Handle
    @param [in] msg Ҫ�ַ����¼�
    @param [in] pThrEnv �̵߳�TSS����
    @param [in] iRet Handle����Ϣ�Ĵ�������-1������ʧ�ܣ�0��ָ����Handle��������¼���1������ɹ���
    @return true������Ϣ�ַ�����ָ���Ĵ���Handle��false��û��handle�������Ϣ
    */
    bool dispatch(CwxMsgBlock*& msg, CwxTss* pThrEnv, int& iRet);
    ///���Commandע���SVR-ID
    void reset()
    {
        m_msgHash.clear(); 
    }
private:
    /**
    @brief �����ӽ����¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onConnCreated(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief �����ӹر��¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onConnClosed(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ���յ�ͨ�����ݰ��¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onRecvMsg(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ��ͨ�����ݰ���������¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onEndSendMsg(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ��ͨ�����ݰ�����ʧ���¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onFailSendMsg(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ����ʱ����¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onTimeoutCheck(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ��IO Handle Ready�¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onEvent4Handle(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief ���û��¼��ַ����¼�����Handle��
    @param [in] pEventOp �¼��Ĵ���Handle��
    @param [in] msg ���ӽ������¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1��Handle����ʧ�ܣ�0��Handle��������¼���1��Handle�ɹ�������¼���
    */
    static int onUserEvent(CwxCmdOp* pEventOp, CwxMsgBlock*& msg, CwxTss* pThrEnv);
private:
    ///��ȡSVR-ID�Ĵ���Handle
    CwxCmdOp* getEventOp(CWX_UINT32 uiSvrID);
private:
    fnEventApi          m_arrEvent[CwxEventInfo::SYS_EVENT_NUM + 1];///�¼������봦��API��ӳ��
    CwxEventCommandHash   m_msgHash;///<�¼�SVR-ID���¼�����Hanlde��ӳ��
};

CWINUX_END_NAMESPACE

#include "CwxCommander.inl"
#include "CwxPost.h"
#endif

