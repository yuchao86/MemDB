#ifndef __UNISTOR_HANDLER_4_RECV_H__
#define __UNISTOR_HANDLER_4_RECV_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "UnistorTss.h"
#include "CwxAppHandler4Channel.h"
#include "UnistorStoreBase.h"

///ǰ����������
class UnistorApp;
class UnistorHandler4Recv;

///�߳�tss���û�����
class UnistorRecvThreadUserObj:public UnistorTssUserObj{
public:
    UnistorRecvThreadUserObj(){
        m_uiConnNum = 0;
    }
    ~UnistorRecvThreadUserObj(){}
public:
    ///��ȡ���Ӷ���
    UnistorHandler4Recv* getConn(CWX_UINT32 uiConnId){
        hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
        if (iter != m_connMap.end()) return iter->second;
        return NULL;
    }
    ///ɾ������
    void removeConn(CWX_UINT32 uiConnId){
        hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
        if (iter != m_connMap.end()){
            m_uiConnNum --;
            m_connMap.erase(iter);
        }
    }
    ///�������
    void addConn(CWX_UINT32 uiConnId, UnistorHandler4Recv* conn){
        hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*���Ӷ���*/>::iterator iter = m_connMap.find(uiConnId);
        CWX_ASSERT(iter == m_connMap.end());
        m_connMap[uiConnId] = conn;
        m_uiConnNum++;
    }
    ///��ȡ���ӵ�����
    CWX_UINT32 getConnNum() const {
        return m_uiConnNum;
    }
private:
    CWX_UINT32          m_uiConnNum;  ///<��Ϊ�˷�ֹ��
    hash_map<CWX_UINT32/*conn id*/, UnistorHandler4Recv*/*���Ӷ���*/> m_connMap; ///�߳������������map
};

///unistor��Ϣ����handle
class UnistorHandler4Recv : public CwxAppHandler4Channel{
public:
    ///���캯��
    UnistorHandler4Recv(UnistorApp* pApp,
        CWX_UINT32 uiConnid,
        CWX_UINT32 uiPoolIndex,
        CwxAppChannel* channel):CwxAppHandler4Channel(channel),m_uiConnId(uiConnid), m_uiThreadPosIndex(uiPoolIndex), m_pApp(pApp)
    {
        m_uiRecvHeadLen = 0;
        m_uiRecvDataLen = 0;
        m_recvMsgData = 0;
        m_bAuth = false;
        m_tss = NULL;
    }
    ///��������
    virtual ~UnistorHandler4Recv(){
        if (m_recvMsgData) CwxMsgBlockAlloc::free(m_recvMsgData);
        m_recvMsgData = NULL;
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
public:
    ///��Ϣ�Ĵ�����
    static void doEvent(UnistorApp* pApp, ///<app
        UnistorTss* tss, ///<�߳�tss
        CwxMsgBlock*& msg, ///<��Ϣmsg
        CWX_UINT32 uiPoolIndex ///<�̵߳����к�
        );

    ///�γ�add��set��update��delete�����Ļظ����ݰ�������ֵ���ǿգ��ɹ���NULL��ʧ�ܡ�
    static CwxMsgBlock* packReplyMsg(UnistorTss* tss, ///<�߳�tss
        CWX_UINT32 uiTaskId, ///<��Ϣ��task id
        CWX_UINT16 unMsgType, ///<��Ϣ����
        int ret,  ///<����ֵ��ret����
        CWX_UINT32 uiVersion, ///<���ݵİ汾��
        CWX_UINT32 uiFieldNum, ///<key��field����
        char const* szErrMsg  ///<���ݲ����Ĵ�����Ϣ
        );

private:
	///�յ�һ��������ɾ�Ĳ���Ϣ������ֵ��0���ɹ���-1��ʧ��
	int recvMessage();

    ///��֤�� ����ֵ��true���ɹ���false������
    bool checkAuth(UnistorTss* pTss);

    ///Key�Ƿ���ڵ���Ϣ������������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
    int existKv(UnistorTss* pTss);

	///get һ��key����Ϣ������������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
	int getKv(UnistorTss* pTss);

    ///get ���key����Ϣ������������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
	int getKvs(UnistorTss* pTss);

	///��ȡkey�б����Ϣ������������ֵ��UNISTOR_ERR_SUCCESS���ɹ����������������
	int getList(UnistorTss* pTss);

    ///�ظ��յ�����Ϣ������ֵ��0���ɹ���-1��ʧ��
	int reply(CwxMsgBlock* msg, ///<�ظ�����Ϣ
        bool bCloseConn=false ///<�Ƿ���Ϣ������Ϻ�ر�����
        );

    ///����Ϣת����write�̡߳�����ֵUNISTOR_ERR_SUCCESS��ʾ�ɹ�������ʧ��
    int relayWriteThread();

    ///����Ϣת����transfer�߳�
    void relayTransThread(CwxMsgBlock* msg);
private:
	CWX_UINT32			   m_uiConnId; ///<����id
	CWX_UINT32			   m_uiThreadPosIndex; ///<������thread pool����
	UnistorApp*            m_pApp;  ///<app����
	CwxMsgHead             m_header; ///<�յ�����Ϣ���İ�ͷ
	char                   m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN + 1];
	CWX_UINT32             m_uiRecvHeadLen; ///<recieved msg header's byte number.
	CWX_UINT32             m_uiRecvDataLen; ///<recieved data's byte number.
	CwxMsgBlock*           m_recvMsgData; ///<the recieved msg data
    bool                   m_bAuth; ///<�����Ƿ��Ѿ���֤
    string                   m_strPeerHost; ///<�Զ�host
    CWX_UINT16               m_unPeerPort; ///<�Զ�port
    UnistorTss*              m_tss;        ///<�����Ӧ��tss����
};
#endif 
