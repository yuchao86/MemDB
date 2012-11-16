#ifndef __UNISTOR_HANDLER_4_DISPATCH_H__
#define __UNISTOR_HANDLER_4_DISPATCH_H__


#include "CwxCommander.h"
#include "CwxAppAioWindow.h"
#include "UnistorMacro.h"
#include "UnistorTss.h"
#include "CwxAppHandler4Channel.h"
#include "CwxAppChannel.h"
#include "UnistorDef.h"
#include "CwxBinLogMgr.h"
#include "UnistorPoco.h"
#include "UnistorSubscribe.h"
#include "UnistorStore.h"

///ǰ����������
class UnistorApp;
class UnistorHandler4Dispatch;

///�ַ����ӵ�sync session��Ϣ����
class UnistorDispatchSyncSession{
public:
    ///���캯��
    UnistorDispatchSyncSession(){
        m_ullSeq = 0;
        m_ullSessionId = 0;
        m_bClosed = false;
        m_pCursor = NULL;
        m_uiChunk = 0;
        m_ullStartSid = 0;
        m_ullSid = 0;
        m_bNext = false;
        m_bZip = false;
    }
    ~UnistorDispatchSyncSession(){
    }
public:
    void addConn(UnistorHandler4Dispatch* conn);
    ///�����γ�session id������session id
    CWX_UINT64 reformSessionId(){
        CwxTimeValue timer;
        timer.now();
        m_ullSessionId = timer.to_usec();
        return m_ullSessionId;
    }

public:
    CWX_UINT64        m_ullSessionId; ///<session id
    CWX_UINT64        m_ullSeq; ///<��ǰ�����кţ���0��ʼ��
    bool                     m_bClosed; ///<�Ƿ���Ҫ�ر�
    map<CWX_UINT32, UnistorHandler4Dispatch*> m_conns; ///<����������
    CwxBinLogCursor*         m_pCursor; ///<binlog�Ķ�ȡcursor
    CWX_UINT32               m_uiChunk; ///<chunk��С
    CWX_UINT64               m_ullStartSid; ///<report��sid
    CWX_UINT64               m_ullSid; ///<��ǰ���͵���sid
    bool                     m_bNext; ///<�Ƿ�����һ����Ϣ
    UnistorSubscribe         m_subscribe; ///<��Ϣ���Ķ���
    string                   m_strSign; ///<ǩ������
    bool                     m_bZip; ///<�Ƿ�ѹ��
    string                   m_strHost; ///<session����Դ����
};

///export��sesion����
class UnistorDispatchExportSession{
public:

    UnistorDispatchExportSession(){
        m_ullSeq = 0;
        m_pCursor = NULL;
        m_uiChunk = 0;
    }

    ~UnistorDispatchExportSession(){
    }
public:
    CWX_UINT64                  m_ullSeq; ///<��ǰ�����кţ���0��ʼ��
    UnistorStoreCursor*         m_pCursor; ///<����ͬ����cursor����
    CWX_UINT32                  m_uiChunk; ///<chunk��С
    UnistorSubscribe            m_subscribe; ///<��Ϣ���Ķ���
    string                      m_strHost; ///<session����Դ����
};

///tss��user object����
class UnistorDispatchThreadUserObj:public UnistorTssUserObj{
public:
    UnistorDispatchThreadUserObj(){}
    ~UnistorDispatchThreadUserObj(){}
public:
    ///�ͷ���Դ
    void free(UnistorApp* pApp);
public:
    map<CWX_UINT64, UnistorDispatchSyncSession* > m_sessionMap;  ///<session��map��keyΪsession id
    list<UnistorDispatchSyncSession*>             m_freeSession; ///<��Ҫ�رյ�session
};


///�첽binlog�ַ�����Ϣ����handler
class UnistorHandler4Dispatch : public CwxAppHandler4Channel{
public:
    enum{
        DISPATCH_TYPE_INIT = 0, ///<��ʼ��״̬
        DISPATCH_TYPE_SYNC = 1, ///<����ͬ������
        DISPATCH_TYPE_EXPORT = 2 ///<export������
    };
public:
    ///���캯��
    UnistorHandler4Dispatch(UnistorApp* pApp,
        CwxAppChannel* channel,
        CWX_UINT32 uiConnId,
        bool bInner=false);
    ///��������
    virtual ~UnistorHandler4Dispatch();
public:
    /**
    @brief ���ӿɶ��¼�������-1��close()�ᱻ����
    @return -1������ʧ�ܣ������close()�� 0������ɹ�
    */
    virtual int onInput();
    /**
    @brief ֪ͨ���ӹرա�
    @return 1������engine���Ƴ�ע�᣻0����engine���Ƴ�ע�ᵫ��ɾ��handler��-1����engine�н�handle�Ƴ���ɾ����
    */
    virtual int onConnClosed();
    /**
    @brief Handler��redo�¼�����ÿ��dispatchʱִ�С�
    @return -1������ʧ�ܣ������close()�� 0������ɹ�
    */
    virtual int onRedo();

public:
    ///����binlog������ֵ��0��δ����һ��binlog��1��������һ��binlog��-1��ʧ�ܣ�
    int syncSendBinLog(UnistorTss* pTss);

    ///packһ��binlog������ֵ��-1��ʧ�ܣ�1���ɹ�
    int syncPackOneBinLog(CwxPackageWriterEx* writer, ///<writer����
        CwxMsgBlock*& block, ///<pack���γɵ����ݰ�
        CWX_UINT64 ullSeq, ///<��Ϣ���к�
        CWX_UINT32 uiVersion, ///<binlog�İ汾��
        CWX_UINT32 uiType, ///<binlog�ı����Ϣ����
        CwxKeyValueItemEx const* pData, ///<���������
        char* szErr2K ///<��ʧ�ܷ��ش�����Ϣ
        );

    ///pack����binlog������ֵ��-1��ʧ�ܣ�1���ɹ�
    int syncPackMultiBinLog(CwxPackageWriterEx* writer, ///<writer����
        CwxPackageWriterEx* writer_item, ///<writer����
        CWX_UINT32 uiVersion, ///<binlog�İ汾��
        CWX_UINT32 uiType, ///<binlog�ı����Ϣ����
        CwxKeyValueItemEx const* pData, ///<���������
        CWX_UINT32&  uiLen, ///<����pack�굱ǰbinlog���������ݰ��Ĵ�С
        char* szErr2K ///<��ʧ�ܷ��ش�����Ϣ
        );

    ///��λ����Ҫ��binlog��������ֵ��1�����ּ�¼��0��û�з��֣�-1������
    int syncSeekToBinlog(UnistorTss* tss, ///<�߳�tss
        CWX_UINT32& uiSkipNum ///<�����Ա�����binlog����������ʣ��ֵ
        );

    ///��binlog��λ��report��sid������ֵ��1���ɹ���0��̫��-1������
    int syncSeekToReportSid(UnistorTss* tss);

    ///����export�����ݡ�����ֵ��0��δ����һ�����ݣ�1��������һ�����ݣ�-1��ʧ�ܣ�
    int exportSendData(UnistorTss* pTss);
    
    ///��ȡ����id
    inline CWX_UINT32 getConnId() const{
        return m_uiConnId;
    }

public:
    ///�ַ��̵߳��¼����ȴ�����
    static void doEvent(UnistorApp* app, ///<app����
        UnistorTss* tss, ///<�߳�tss
        CwxMsgBlock*& msg ///<�¼���Ϣ
        );

    ///����رյ�session
    static void dealClosedSession(UnistorApp* app, ///<app����
        UnistorTss* tss  ///<�߳�tss
        );
private:
    ///�յ�һ����Ϣ����������ֵ��0���ɹ���-1��ʧ��
    int recvMessage();

    ///�յ�sync report����Ϣ������ֵ��0���ɹ���-1��ʧ��
    int recvSyncReport(UnistorTss* pTss);
    
    ///�յ�sync new conn��report��Ϣ������ֵ��0���ɹ���-1��ʧ��
    int recvSyncNewConnection(UnistorTss* pTss);
    
    ///�յ�binlog sync��reply��Ϣ������ֵ��0���ɹ���-1��ʧ��
    int recvSyncReply(UnistorTss* pTss);

    ///�յ�chunkģʽ�µ�binlog sync reply��Ϣ������ֵ��0���ɹ���-1��ʧ��
    int recvSyncChunkReply(UnistorTss* pTss);

    ///�յ�export��report��Ϣ������ֵ��0���ɹ���-1��ʧ��
    int recvExportReport(UnistorTss* pTss);

    ///�յ�export���ݵ�reply��Ϣ������ֵ��0���ɹ���-1��ʧ��
    int recvExportReply(UnistorTss* pTss);
private:
    bool                     m_bInner; ///<�Ƿ��ڲ��ַ�
    bool                     m_bReport; ///<�Ƿ��Ѿ�����
    CWX_UINT8                m_ucDispatchType; ///<�ַ�����
    UnistorDispatchSyncSession*  m_syncSession; ///<���Ӷ�Ӧ��session
    UnistorDispatchExportSession* m_exportSession; ///<����export��session
    CWX_UINT64               m_ullSessionId; ///<session��id
    CWX_UINT64               m_ullSentSeq; ///<���͵����к�
    CWX_UINT32               m_uiConnId; ///<����id
    UnistorApp*              m_pApp;  ///<app����
    CwxMsgHead               m_header; ///<��Ϣͷ
    char                     m_szHeadBuf[CwxMsgHead::MSG_HEAD_LEN+1]; ///<��Ϣͷ��buf
    CWX_UINT32               m_uiRecvHeadLen; ///<recieved msg header's byte number.
    CWX_UINT32               m_uiRecvDataLen; ///<recieved data's byte number.
    CwxMsgBlock*             m_recvMsgData; ///<the recieved msg data
    string                   m_strPeerHost; ///<�Զ�host
    CWX_UINT16               m_unPeerPort; ///<�Զ�port
    UnistorTss*              m_tss;        ///<�����Ӧ��tss����


};

#endif 
