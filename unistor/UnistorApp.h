#ifndef __UNISTOR_APP_H__
#define __UNISTOR_APP_H__

#include "UnistorMacro.h"
#include "CwxAppFramework.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "UnistorHandler4Recv.h"
#include "UnistorStore.h"
#include "CwxThreadPool.h"
#include "CwxAppChannel.h"
#include "UnistorHandler4Dispatch.h"
#include "UnistorHandler4Master.h"
#include "UnistorHandler4RecvWrite.h"
#include "UnistorHandler4Checkpoint.h"
#include "UnistorDef.h"
#include "CwxBinLogMgr.h"
#include "UnistorHandler4Zk.h"
#include "UnistorHandler4Trans.h"

///Ӧ����Ϣ����
#define UNISTOR_VERSION "1.0.8"
#define UNISTOR_MODIFY_DATE "20120604080000"

///unistor�����app����
class UnistorApp : public CwxAppFramework{
public:
    enum{
		MAX_MONITOR_REPLY_SIZE = 1024 * 1024, ///<��ص�buf�Ĵ�С
        LOG_FILE_SIZE = 30, ///<ÿ����ѭ��ʹ����־�ļ���MByte
        LOG_FILE_NUM = 7, ///<��ѭ��ʹ����־�ļ�������
        SVR_TYPE_RECV = CwxAppFramework::SVR_TYPE_USER_START, ///<���ݽ���svr-id����
		SVR_TYPE_RECV_WRITE = CwxAppFramework::SVR_TYPE_USER_START + 1, ///<����д��svr-id
		SVR_TYPE_MONITOR = CwxAppFramework::SVR_TYPE_USER_START + 2, ///<�������
		SVR_TYPE_INNER_SYNC = CwxAppFramework::SVR_TYPE_USER_START + 3, ///<�ڲ��첽�ַ�����
        SVR_TYPE_OUTER_SYNC = CwxAppFramework::SVR_TYPE_USER_START + 4, ///<�ⲿ�첽�ַ�����
		SVR_TYPE_MASTER = CwxAppFramework::SVR_TYPE_USER_START + 5, ///<Master���ݷַ�
		SVR_TYPE_CHECKPOINT = CwxAppFramework::SVR_TYPE_USER_START + 6, ///<checkpoint�߳�
        SVR_TYPE_ZK = CwxAppFramework::SVR_TYPE_USER_START + 7, ///<zookeeper����Ϣ����
        SVR_TYPE_TRANSFER=CwxAppFramework::SVR_TYPE_USER_START + 8, ///<slave��д����ѯת��

		THREAD_GROUP_INNER_SYNC = CwxAppFramework::THREAD_GROUP_USER_START + 1, ///<�ڲ�����ͬ���̳߳ص�id
        THREAD_GROUP_OUTER_SYNC = CwxAppFramework::THREAD_GROUP_USER_START + 2, ///<�ⲿ����ͬ���̳߳ص�id
		THREAD_GROUP_WRITE = CwxAppFramework::THREAD_GROUP_USER_START + 3, ///���ݸ��µ��̳߳ص�id
		THREAD_GROUP_CHECKPOINT = CwxAppFramework::THREAD_GROUP_USER_START + 4, ///<checkpoint���߳�group 
        THREAD_GROUP_ZK = CwxAppFramework::THREAD_GROUP_USER_START + 5, ///<zookeeper���̳߳�
        THREAD_GROUP_TRANSFER = CwxAppFramework::THREAD_GROUP_USER_START + 6, ///<��Ϣת�����̳߳�
		THREAD_GROUP_RECV_BASE = CwxAppFramework::THREAD_GROUP_USER_START + 7 ///<���ݽ����߳������ʼsvr id
    };
    ///���캯��
	UnistorApp();
    ///��������
	virtual ~UnistorApp();
    ///���س�ʼ������������ֵ��0���ɹ���-1:ʧ�ܡ�
    virtual int init(int argc, ///<main��argc
        char** argv ///<main��argv
        );
public:
    /**
    @brief ʱ��֪ͨ��ֻҪ������ʱ�Ӽ������ᶨʱ���ô�API��
    @param [in] current ��ǰ��ʱ�䡣
    @return void
    */
    virtual void onTime(CwxTimeValue const& current);
    /**
    @brief �ź�֪ͨ�����յ���һ��û�����ε��źţ���ᶨʱ���ô�API��
    @param [in] signum �յ����źš�
    @return void
    */
    virtual void onSignal(int signum);
    /**
    @brief ֪ͨ��������һ��CWX_APP_EVENT_MODE���ӡ������������Ƿ�����ģʽ��<br>
    @param [in] uiSvrId ���ӵ�svr id��
    @param [in] uiHostId ���ӵ�host id��
    @param [in] handle ���ӵ�handle��
    @param [out] bSuspendListen ���ڱ������ӣ���Ϊtrue,��ֹͣ���������������������
    @return <0���ر����ӣ� >=0������Ч��
    */
    virtual int onConnCreated(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_HANDLE handle,
        bool& bSuspendListen
        );

    /**
    @brief ֪ͨ����CWX_APP_MSG_MODEһ�����ӡ�����������ȫ���Ƿ�����ģʽ��<br>
    @param [in] conn ���Ӷ��֣�ֻ���ڴ�API��ʱ���ϲ㲻�ܻ��档
    @param [out] bSuspendConn ��Ϊtrue,����ͣ��Ϣ���գ�false�����������ϵ���Ϣ
    @param [out] bSuspendListen ���ڱ������ӣ���Ϊtrue,��ֹͣ���������������������
    @return <0���ر����ӣ� >=0������Ч��
    */
	virtual int onConnCreated(CwxAppHandler4Msg& conn,
		bool& bSuspendConn,
		bool& bSuspendListen);
    /**
    @brief ֪ͨCWX_APP_MSG_MODEģʽ�����ӹرա�
    @param [in] conn �رյ����ӡ�
    @return �����������ӣ�-1����ʾ�����ӣ�0��Ĭ�Ϸ�ʽ�� >0���´�������ʱ��������λΪms���������Ӻ��ԡ�
    */
    virtual int onConnClosed(CwxAppHandler4Msg& conn);
    /**
    @brief ֪ͨ��CWX_APP_MSG_MODEģʽ�ġ���raw���������յ�һ����Ϣ
    @param [in] msg �յ�����Ϣ���ձ�ʾû����Ϣ�塣
    @param [in] conn �յ���Ϣ�����ӡ�
    @param [in] header �յ���Ϣ����Ϣͷ��
    @param [out] bSuspendConn ��Ϊtrue,����ͣ��Ϣ���գ�false�����������ϵ���Ϣ��
    @return -1����Ϣ��Ч���ر����ӡ� 0��������������Ϣ�� >0�������Ӵ������Ͻ�����Ϣ��
    */
    virtual int onRecvMsg(CwxMsgBlock* msg,
		CwxAppHandler4Msg& conn,
		CwxMsgHead const& header,
		bool& bSuspendConn);
    /**
    @brief ֪ͨCWX_APP_MSG_MODEģʽ�ġ�raw�������������ݵ��������Ҫ�û��Լ���ȡ
    @param [in] conn ����Ϣ�����ӡ�
    @param [out] bSuspendConn ��Ϊtrue,����ͣ��Ϣ���գ�false�����������ϵ���Ϣ��
    @return -1����Ϣ��Ч���ر����ӡ� 0���ɹ���
    */
    virtual int onRecvMsg(CwxAppHandler4Msg& conn,
		bool& bSuspendConn);
public:
    ///��ȡ������Ϣ����
    inline UnistorConfig const& getConfig() const{
        return m_config;
    }

    ///��ȡStore driver����ָ��
    inline UnistorStore* getStore(){
        return m_store;
    }

    ///��ȡmaster handler
    inline UnistorHandler4Master* getMasterHandler(){
        return m_masterHandler;
    }

    ///��ȡwrite handler
    inline UnistorHandler4RecvWrite* getRecvWriteHandler(){
        return m_recvWriteHandler;
    }

    ///��ȡ�ڲ�ͬ�����̳߳�
    inline CwxThreadPool* getInnerSyncThreadPool(){
        return m_innerSyncThreadPool;
    }

    ///��ȡ�ڲ�ͬ����channel
    inline CwxAppChannel* getInnerSyncChannel(){
        return m_innerSyncChannel;
    }

    ///��ȡ�ⲿͬ�����̳߳�
    inline CwxThreadPool* getOuterSyncThreadPool(){
        return m_outerSyncThreadPool;
    }

    ///��ȡ�ⲿͬ����channel
    inline CwxAppChannel* getOuterSyncChannel(){
        return m_outerSyncChannel;
    }
    
    ///��ȡwrite���̳߳�
    inline 	CwxThreadPool* getWriteTheadPool(){
        return m_writeThreadPool;
    }

    ///��ȡrecv ͨ��channel����
	inline  CwxAppChannel** getRecvChannels(){
        return m_recvChannel;
    }

    ///��ȡrecv�̳߳ص�����
	inline  CwxThreadPool** getRecvThreadPools(){
        return m_recvThreadPool;
    }

    ///��ȡת��ͨ��channel
    inline CwxAppChannel* getTransChannel(){
        return m_transChannel;
    }

    ///��ȡת���̳߳�
    inline CwxThreadPool* getTransThreadPool() {
        return m_transThreadPool;
    }

    ///��ȡzookeeper���̳߳�
    inline CwxThreadPool*  getZkThreadPool(){
        return m_zkThreadPool;
    }

    ///��ȡzookeeper����Ϣ����handler
    inline UnistorHandler4Zk* getZkHandler() const{
        return m_zkHandler;
    }

    ///��ȡrecv����������
    inline UnistorConnAttr* getRecvSockAttr(){
        return &m_recvCliSockAttr;
    }

    ///��ȡ�ַ����յ���������
    inline UnistorConnAttr* getSyncCliSockAttr(){
        return &m_syncCliSockAttr;
    }

    ///����kv���recv���ӵ�����
    static int setConnSockAttr(CWX_HANDLE handle, void* arg);

    ///�������ʱ���Ƿ�ص�
    static bool isClockBack(CWX_UINT32& uiLastTime, CWX_UINT32 uiNow){
        if (uiLastTime > uiNow + 1){
            uiLastTime = uiNow;
            return true;
        }
        uiLastTime = uiNow;
        return false;
    }

protected:
    ///�������л�������API
    virtual int initRunEnv();
    ///�ͷ���Դ
    virtual void destroy();
private:
	///stats���-1����Ϊ����ر����ӣ�0�����ر�����
	int monitorStats(char const* buf, ///<�յ�������
        CWX_UINT32 uiDataLen, ///<���ݳ���
        CwxAppHandler4Msg& conn ///<�յ����ݵ�����
        );

	///�γɼ�����ݣ����ؼ�����ݵĳ���
	CWX_UINT32 packMonitorInfo();
    
    ///�ַ�channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
    static int dealRecvThreadQueue(UnistorTss* tss, ///<�߳�tss
        CwxMsgQueue* queue, ///<recv�̳߳ص�queue
        CWX_UINT32 uiQueueIndex, ///<recv�̵߳����
        UnistorApp* app, ///<app����
        CwxAppChannel* channel ///<�̶߳�Ӧ��ͨ��channel
        );

    ///receive channel���̺߳�����argΪpair<app,int>����
    static void* recvThreadMain(CwxTss* tss, ///<�̵߳ĵ�tss
        CwxMsgQueue* queue, ///<receive�̵߳Ķ���
        void* arg ///<��Ϊpair����һ������Ϊapp���ڶ�������Ϊreceive�̵߳����
        );
    
    ///�ڲ�ͬ��channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
    static int dealInnerSyncThreadQueue(UnistorTss* tss, ///<�߳�tss
        CwxMsgQueue* queue, ///<inner binlog�̳߳ص�queue
        UnistorApp* app, ///<app����
        CwxAppChannel* channel ///<�̵߳�ͨ��channel
        );

    ///���ַ�channel���̺߳�����argΪapp����
	static void* innerSyncThreadMain(CwxTss* tss, ///<�߳�tss
        CwxMsgQueue* queue, ///<�̳߳ص�queue
        void* arg ///<app����
        );
    
    ///�ⲿͬ��channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
    static int dealOuterSyncThreadQueue(UnistorTss* tss,  ///<�߳�tss
        CwxMsgQueue* queue, ///<outer binlog�̳߳ص�queue
        UnistorApp* app, ///<app����
        CwxAppChannel* channel ///<�̵߳�ͨ��channel
        );

    ///�ⲿ�ַ�channel���̺߳�����argΪapp����
    static void* outerSyncThreadMain(CwxTss* tss, ///<�߳�tss
        CwxMsgQueue* queue, ///<�̳߳ص�queue
        void* arg ///<app����
        );

    ///�ⲿͬ��channel�Ķ�����Ϣ����������ֵ��0��������-1������ֹͣ
    static int dealTransThreadQueue(UnistorTss* tss, ///<�߳�tss
        CwxMsgQueue* queue,///<�̳߳ص�queue
        UnistorApp* app, ///<app����
        CwxAppChannel* channel ///<�̵߳�ͨ��channel
        );

    ///�ⲿ�ַ�channel���̺߳�����argΪapp����
    static void* transThreadMain(CwxTss* tss, ///<�߳�tss
        CwxMsgQueue* queue, ///<�̳߳ص�queue
        void* arg ///<app����
        );

    ///�ⲿ�ַ�channel���̺߳�����argΪapp����
    static void* zkThreadMain(CwxTss* tss, ///<�߳�tss
        CwxMsgQueue* queue, ///<�̳߳ص�queue
        void* arg ///<app����
        );

    ///�洢��������Ϣͨ����0���ɹ���-1��ʧ��
    static int storeMsgPipe(void* app, ///<app����
        CwxMsgBlock* msg, ///<���Դ洢�������Ϣ
        bool bWriteThread, ///<�Ƿ��͸�write�̣߳������checkpoint�߳�
        char* szErr2K ///<����ʱ�Ĵ�����Ϣ
        ); 

    //��ȡϵͳkey��1���ɹ���0�������ڣ�-1��ʧ��;
    static int getSysKey(void* pApp, ///<app����
        char const* key, ///<Ҫ��ȡ��key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
        CWX_UINT32& uiLen  ///<szData���ݵ��ֽ���
        );


    ///�������硣0���ɹ���-1��ʧ��
	int startNetwork();
private:
    UnistorConfig                 m_config;            ///<�����ļ�
    UnistorStore*                 m_store;             ///<unistor�洢driver����
    CwxAppChannel**               m_recvChannel;       ///<unistor��ѯ��channel����
    CwxThreadPool**               m_recvThreadPool;    ///<unistor��ѯ���̳߳ض���
    pair<UnistorApp*, CWX_UINT32>* m_recvArgs;         ///<�߳�������������
    CwxThreadPool*                m_writeThreadPool;    ///<����slave����ͬ��master���ݣ�����master���ڴ���д����
	UnistorHandler4Master*         m_masterHandler;    ///<��master������Ϣ��handle
	UnistorHandler4RecvWrite*      m_recvWriteHandler; ///<�����޸ĵ�handle
	CwxThreadPool*                m_innerSyncThreadPool; ///<�ڲ���Ϣ�ַ����̳߳ض���
	CwxAppChannel*                m_innerSyncChannel;     ///<�ڲ���Ϣ�ַ���channel
    CwxThreadPool*                m_outerSyncThreadPool; ///<�ⲿ����Ϣ�ַ����̳߳ض���
    CwxAppChannel*                m_outerSyncChannel;     ///<�ⲿ��Ϣ�ַ���channel
	UnistorHandler4Checkpoint*     m_checkpointHandler; ///<checkpoint��handle
	CwxThreadPool*                m_checkpointThreadPool;///<checkpoint���̳߳ض���
    CwxThreadPool*                m_transThreadPool;///<��Ϣת�����̳߳ض���
    CwxAppChannel*                m_transChannel;     ///<��Ϣת����channel
    CwxThreadPool*                m_zkThreadPool;///<zk���̳߳ض���
    UnistorHandler4Zk*             m_zkHandler; ///<zk�¼������handler
    UnistorConnAttr               m_recvSvrSockAttr; ///<���ݽ���svr�˵�socket����
    UnistorConnAttr               m_recvCliSockAttr; ///<���ݽ���client�˵�socket����
    UnistorConnAttr               m_syncSvrSockAttr; ///<ͬ��svr�˵�socket����
    UnistorConnAttr               m_syncCliSockAttr; ///<ͬ��cli�˵�socket����
	char                        m_szBuf[MAX_MONITOR_REPLY_SIZE];///<�����Ϣ�Ļظ�buf
	string						m_strStartTime;       ///<unistor������ʱ���
};
#endif

