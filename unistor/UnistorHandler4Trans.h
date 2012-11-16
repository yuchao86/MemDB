#ifndef __UNISTOR_HANDLER_4_TRANS_H__
#define __UNISTOR_HANDLER_4_TRANS_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "UnistorTss.h"
#include "CwxAppHandler4Channel.h"
#include "UnistorStoreBase.h"

///ǰ������
class UnistorApp;

///��Ϣת����handle
class UnistorHandler4Trans : public CwxAppHandler4Channel{
public:
    ///���캯��
    UnistorHandler4Trans(UnistorApp* pApp, CWX_UINT32 uiConnid, CwxAppChannel *channel):
      CwxAppHandler4Channel(channel),m_uiConnId(uiConnid), m_pApp(pApp)
      {
          m_uiRecvHeadLen = 0;
          m_uiRecvDataLen = 0;
          m_recvMsgData = 0;
          m_bAuth = false;
          m_tss = NULL;
      }
      ///��������
      virtual ~UnistorHandler4Trans(){
          if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
      }
public:
    /**
    @brief ��ʼ�����������ӣ�����Reactorע������
    @param [in] arg �������ӵ�acceptor��ΪNULL
    @return -1���������������ӣ� 0�����ӽ����ɹ�
    */
    virtual int open (void * arg= 0);
    /**
    @brief ֪ͨ���ӹرա�
    @return 1������engine���Ƴ�ע�᣻0����engine���Ƴ�ע�ᵫ��ɾ��handler��-1����engine�н�handle�Ƴ���ɾ����
    */
    virtual int onConnClosed();
    /**
    @brief ���ӿɶ��¼�������-1��close()�ᱻ����
    @return -1������ʧ�ܣ������close()�� 0������ɹ�
    */
    virtual int onInput();
    /**
    @brief ֪ͨ�������һ����Ϣ�ķ��͡�<br>
    ֻ����Msgָ��FINISH_NOTICE��ʱ��ŵ���.
    @param [in,out] msg ���뷢����ϵ���Ϣ��������NULL����msg���ϲ��ͷţ�����ײ��ͷš�
    @return 
    CwxMsgSendCtrl::UNDO_CONN�����޸����ӵĽ���״̬
    CwxMsgSendCtrl::RESUME_CONN�������Ӵ�suspend״̬��Ϊ���ݽ���״̬��
    CwxMsgSendCtrl::SUSPEND_CONN�������Ӵ����ݽ���״̬��Ϊsuspend״̬
    */
    virtual CWX_UINT32 onEndSendMsg(CwxMsgBlock*& msg);

    /**
    @brief ֪ͨ�����ϣ�һ����Ϣ����ʧ�ܡ�<br>
    ֻ����Msgָ��FAIL_NOTICE��ʱ��ŵ���.
    @param [in,out] msg ����ʧ�ܵ���Ϣ��������NULL����msg���ϲ��ͷţ�����ײ��ͷš�
    @return void��
    */
    virtual void onFailSendMsg(CwxMsgBlock*& msg);

public:

    ///��Ϣ�ַ�������
    static void doEvent(UnistorApp* pApp, ///<app����
        UnistorTss* tss, ///<�߳�tss
        CwxMsgBlock*& msg ///<�ַ�����Ϣ
        );

    ///��masterת����Ϣ������ֵ��true���ɹ���false��ʧ�ܡ�
    static bool transMsg(UnistorTss* tss, ///<�߳�tss
        CWX_UINT32 uiTaskId, ///<��Ϣ��taskid
        CwxMsgBlock* msg ///<ת������Ϣ
        );

    ///��ʼ��Handler������Ϣ������ֵ��0���ɹ���-1��ʧ��
    static int init(CWX_UINT32 uiConnNum/*���ӵ�����*/){
        m_bCanTrans = false;
        m_strMasterHost = "";
        m_uiMaxConnNum = uiConnNum;
        m_uiAuthConnNum = 0; ///<�Ѿ���֤��host������
        m_authConn = new UnistorHandler4Trans*[uiConnNum]; ///<��֤������
        m_handlers = new map<CWX_UINT32, UnistorHandler4Trans*>;
        m_bRebuildConn = false;
        m_ttLastRebuildConn = 0;
        return 0;
    }

    ///�ͷ�ת��handle�Ļ�����Ϣ������ֵ��0���ɹ���-1��ʧ��
    static int destroy(){
        m_bCanTrans = false;
        m_strMasterHost = "";
        m_uiMaxConnNum = 0;
        m_uiAuthConnNum = 0; ///<�Ѿ���֤��host������
        if (m_authConn) delete [] m_authConn;
        m_authConn = NULL;
        if (m_handlers) delete m_handlers;
        m_handlers = NULL;
        m_bRebuildConn = false;
        m_ttLastRebuildConn = 0;
        return 0;
    }

    ///�ؽ�ת�������ӡ�����ֵ��0���ɹ���-1��ʧ��
    static int rebuildConn(UnistorApp* app);
    
    ///��Ⲣ����ת������
    static void checkTrans(UnistorApp* app, ///<app����
        UnistorTss* tss ///<�߳�tss
        );
private:
    CWX_UINT32				m_uiConnId; ///<����id
    UnistorApp*             m_pApp;  ///<app����
    CwxMsgHead              m_header; ///<��Ϣ����Ϣͷ
    char                    m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN + 1];
    CWX_UINT32              m_uiRecvHeadLen; ///<recieved msg header's byte number.
    CWX_UINT32              m_uiRecvDataLen; ///<recieved data's byte number.
    CwxMsgBlock*            m_recvMsgData; ///<the recieved msg data
    bool                    m_bAuth;       ///<��ǰ�����Ƿ���֤���
    UnistorTss*              m_tss;        ///<�����Ӧ��tss����
public:
    static volatile bool                m_bCanTrans; ///<�Ƿ��ܹ�ת��
private:
    static string                       m_strMasterHost; ///<��ǰ������
    static CWX_UINT32                   m_uiMaxConnNum; ///<������������
    static CWX_UINT32                   m_uiAuthConnNum; ///<�Ѿ���֤��host������
    static UnistorHandler4Trans**       m_authConn; ///<��֤������
    static map<CWX_UINT32, UnistorHandler4Trans*>* m_handlers; ///<�������ӵ�map
    static bool                         m_bRebuildConn; ///<�Ƿ���Ҫ���½�������
    static CWX_UINT32                   m_ttLastRebuildConn; ///<�ϴ��ؽ����ӵ�ʱ��

};
#endif 
