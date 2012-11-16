#ifndef __UNISTOR_HANDLER_4_MASTER_H__
#define __UNISTOR_HANDLER_4_MASTER_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"
#include "UnistorConnector.h"

///ǰ����������
class UnistorApp;

///binlogͬ����session
class UnistorSyncSession{
public:
    ///���캯��
    UnistorSyncSession(CWX_UINT32 uiHostId){
        m_uiHostId = uiHostId;
        m_ullSessionId = 0;
        m_ullNextSeq = 0;
        m_unPort = 0;
        m_bZip = false;
        m_uiChunkSize = 0;
        m_unConnNum = 0;
        m_uiReportDatetime = 0;
    }
    ~UnistorSyncSession(){
        map<CWX_UINT64/*seq*/, CwxMsgBlock*>::iterator iter =  m_msg.begin();
        while(iter != m_msg.end()){
            CwxMsgBlockAlloc::free(iter->second);
            iter++;
        }
    }
public:
    ///��������Ϣ�������Ѿ��յ�����Ϣ�б�
    bool recv(CWX_UINT64 ullSeq, CwxMsgBlock* msg, list<CwxMsgBlock*>& finished){
        map<CWX_UINT32,  bool>::iterator iter = m_conns.find(msg->event().getConnId());
        if ( (iter == m_conns.end()) || !iter->second ) return false;
        finished.clear();
        if (ullSeq == m_ullNextSeq){
            finished.push_back(msg);
            m_ullNextSeq++;
            map<CWX_UINT64/*seq*/, CwxMsgBlock* >::iterator iter =  m_msg.begin();
            while(iter != m_msg.end()){
                if (iter->first == m_ullNextSeq){
                    finished.push_back(iter->second);
                    m_ullNextSeq++;
                    m_msg.erase(iter);
                    iter = m_msg.begin();
                    continue;
                }
                break;
            }
            return true;
        }
        m_msg[ullSeq] = msg;
        msg->event().setTimestamp((CWX_UINT32)time(NULL));
        return true;
    }

    //����Ƿ�ʱ
    bool isTimeout(CWX_UINT32 uiTimeout) const{
        if (!m_msg.size()) return false;
        CWX_UINT32 uiNow = time(NULL);
        return m_msg.begin()->second->event().getTimestamp() + uiTimeout < uiNow;
    }
public:
    CWX_UINT64              m_ullSessionId; ///<session id
    CWX_UINT64              m_ullNextSeq; ///<��һ�������յ�sid
    CWX_UINT32              m_uiHostId; ///<��ǰ��host id
    string                  m_strHost; ///<��ǰͬ��������
    CWX_UINT16              m_unPort; ///<ͬ���Ķ˿ں�
    string                  m_strUser; ///<ͬ�����û���
    string                  m_strPasswd; ///<��ǰͬ�����û�����
    bool                    m_bZip;  ///<�Ƿ�ѹ��
    string                  m_strSign; ///<ǩ��
    CWX_UINT32              m_uiChunkSize; ///<chunk��size
    CWX_UINT16              m_unConnNum;   ///<conn ������
    map<CWX_UINT64/*seq*/, CwxMsgBlock*>  m_msg;   ///<�ȴ��������Ϣ
    map<CWX_UINT32,  bool/*�Ƿ��Ѿ�report*/>  m_conns; ///<����������
    CWX_UINT32              m_uiReportDatetime; ///<�����ʱ�����������ָ����ʱ��û�лظ�����ر�
};

///slave��master����binlog�Ĵ���handle
class UnistorHandler4Master : public CwxCmdOp{
public:
    ///���캯��
    UnistorHandler4Master(UnistorApp* pApp):m_pApp(pApp){
        m_unzipBuf = NULL;
        m_uiBufLen = 0;
        m_syncSession = NULL;
        m_uiCurHostId = 0;
    }
    ///��������
    virtual ~UnistorHandler4Master(){
        if (m_unzipBuf) delete [] m_unzipBuf;
        if (m_syncSession) delete m_syncSession;
    }
public:
    /**
    @brief ���ӹر��¼��ĵĺ�����
    @param [in] msg ���ӹرյ��¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onConnClosed(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief �յ�ͨ�����ݰ��¼��Ĵ�������
    @param [in] msg �յ�ͨ�����ݰ����¼�����
    @param [in] pThrEnv �̵߳�TSS����
    @return -1������ʧ�ܣ�0����������¼���1��������¼���
    */
    virtual int onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv);
public:
    ///zk���øı䴦����
    void configChange(UnistorTss* pTss);
    ///ʱ�Ӷ�ʱ��麯��
    void timecheck(UnistorTss* pTss);
private:
    ///�յ�һ����Ϣ�Ĵ�����������ֵ��0:�ɹ���-1��ʧ��
    int recvMsg(CwxMsgBlock*& msg, ///<�յ�����Ϣ
        list<CwxMsgBlock*>& msgs ///<���ճ��з��صĿɴ������Ϣ����list�����Ⱥ��������
        );

    ///����Sync report��reply��Ϣ������ֵ��0���ɹ���-1��ʧ��
    int dealSyncReportReply(CwxMsgBlock*& msg, ///<�յ�����Ϣ
        UnistorTss* pTss ///<tss����
        );

    ///�����յ���sync data������ֵ��0���ɹ���-1��ʧ��
    int dealSyncData(CwxMsgBlock*& msg, ///<�յ�����Ϣ
        UnistorTss* pTss ///<tss����
        );

    //�����յ���chunkģʽ�µ�sync data������ֵ��0���ɹ���-1��ʧ��
    int dealSyncChunkData(CwxMsgBlock*& msg, ///<�յ�����Ϣ
        UnistorTss* pTss ///<tss����
        );

    //���������Ϣ������ֵ��0���ɹ���-1��ʧ��
    int dealErrMsg(CwxMsgBlock*& msg,  ///<�յ�����Ϣ
        UnistorTss* pTss ///<tss����
        );

    //����binlog������ֵ��0���ɹ���-1��ʧ��
    int saveBinlog(UnistorTss* pTss, ///<tss����
        char const* szBinLog, ///<�յ���һ��binlog����
        CWX_UINT32 uiLen  ///<binlog���ݵĳ���
        );

    ///������ݵ�ǩ��������ֵ��true���ɹ���false��ʧ��
    bool checkSign(char const* data, ///<binlog����
        CWX_UINT32 uiDateLen, ///<binlog���ݵĳ���
        char const* szSign,   ///<ǩ�����ͣ�Ϊmd5��crc32
        char const* sign  ///<ǩ��ֵ
        );

    //��ȡunzip��buf������ֵ��true���ɹ���false��ʧ��
    bool prepareUnzipBuf();

    //�ر���������
    void closeSession();

    ///������masterͬ�������ӡ�����ֵ��0���ɹ���-1��ʧ��
    int createSession(UnistorTss* pTss, ///<tss����
        string const& strHost, ///<Ҫ���ӵ�����
        CWX_UINT16 unPort, ///<���ӵĶ˿ں�
        CWX_UINT16 unConnNum, ///<�������ӵ�����
        char const* user, ///<���ӵ��û�
        char const* passwd, ///<���ӵ��û�����
        CWX_UINT32 uiChunk, ///<binlogͬ����chunk��С
        char const* subscribe, ///<binlog�Ķ��Ĺ���
        char const* sign, ///<��Ϣ��ǩ����ʽ��Ϊmd5��crc32
        bool  bZip  ///<�Ƿ�ѹ��
        );

    ///����Ƿ���Ҫ�ؽ�����
    bool isNeedReconnect(UnistorTransInfo const& trans);

private:
    UnistorApp*             m_pApp;  ///<app����
    CwxPackageReaderEx        m_reader; ///<�����reader
    unsigned char*          m_unzipBuf; ///<��ѹ��buffer
    CWX_UINT32              m_uiBufLen; ///<��ѹbuffer�Ĵ�С����Ϊtrunk��20������СΪ20M��
    UnistorSyncSession*     m_syncSession; ///<����ͬ����session
    CWX_UINT32              m_uiCurHostId; ///<��ǰ��host id
};

#endif 
