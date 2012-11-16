#ifndef __UNISTOR_HANDLER_4_ZK_H__
#define __UNISTOR_HANDLER_4_ZK_H__

#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"
#include "ZkLocker.h"
#include "UnistorDef.h"
#include "CwxThreadPool.h"

///ǰ������
class UnistorApp;

///zk�¼������handle
class UnistorHandler4Zk {
public:
    enum{
        INIT_RETRY_INTERNAL = 10, ///<��ʼ�����Լ����Ϊ10s
        REFETCH_INTERNAL = 60, ///<���»�ȡʱ����
        MAX_ZK_DATA_SIZE = 1024 * 1024 ///<����zk�ռ��С
    };
public:
    ///���캯��
    UnistorHandler4Zk(UnistorApp* pApp);
    ///��������
    virtual ~UnistorHandler4Zk();
public:
    ///��ʼ����0:�ɹ���-1��ʧ��
    int init();
    ///��ʼzookeeper�¼�
    void doEvent(CwxTss* tss, CwxMsgBlock*& msg, CWX_UINT32 uiLeftEvent);
    ///ֹͣzookeeper���
    void stop();
public:
    ///�Ƿ��Ѿ���ʼ��
    bool isInit() const{
        return m_bInit;
    }

    ///�Ƿ��Ѿ�����
    bool isConnected() const {
        return m_bConnected;
    }
    
    ///�Ƿ��Ѿ���֤
    bool isAuth() const {
        return m_bAuth;
    }

    ///�������
    bool isValid() const {
        return m_bValid;
    }

    ///�Ƿ���master
    bool isMaster(){
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        return _isMaster();
    }
    
    ///��ȡ������Ϣ
    void getConf(UnistorZkConf& conf) {
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        conf = m_conf;
    };
    
    ///��ȡ������Ϣ
    void getLockInfo(UnistorZkLock& zkLock) {
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        zkLock = m_lock;
    }; ///<������Ϣ

    ///��ȡ������Ϣ
    void getErrMsg(string& strErrMsg){
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        strErrMsg = m_szErr2K;
    }
private:
    ///����zk������ֵ��0���ɹ���-1��ʧ��
    int _connect();

    ///zk��֤������ֵ��1����֤�ɹ���0���ȴ���֤�����-1����֤ʧ��
    int _auth();

    ///��ʼ��������Ϣ
    void _reset();

    ///����������Ϣ������ֵ��0���ɹ���-1��ʧ��
    int  _loadZkConf(CwxTss* tss);

    ///����������ֵ��0���ɹ���-1��ʧ��
    int _lock();

    ///��ȡ������Ϣ������ֵ��0���ɹ���-1��ʧ��
    int _loadLockInfo(CwxTss* tss);

    ///�Ƿ���master������ֵ��true���ǣ�false�����ǡ�
    inline bool _isMaster() const{
        if (m_zkLocker) return m_zkLocker->isLocked();
        return false;
    }

    ///��ʼ��zk������Ϣ������ֵ��0���ɹ���-1��ʧ��
    int _init();

    ///����host����Ϣ������ֵ��0���ɹ���-1��ʧ��
    int _setHostInfo();

    ///����master����Ϣ������ֵ��0���ɹ���-1��ʧ��
    int _setMasterInfo();

    ///��ȡhost������ͬ����Ϣ������ֵ��0���ɹ���-1��ʧ��
    int _getHostInfo(char const* szHost, ///<��ȡ�����ı�־
        CWX_UINT64& ullSid, ///<��ȡ������ǰ��sidֵ
        CWX_UINT32& uiTimeStamp ///<��ȡ����Сsid��ʱ�����
        );

    ///��ȡmaster����Ϣ������ֵ��0���ɹ���-1��ʧ��
    int _getMasterInfo(string& strMaster, ///<master�ı�־
        CWX_UINT64& ullMasterSid, ///<Ҫ��Ϊmaster����Сsid
        CWX_UINT64& ullSid, ///<master�ĵ�ǰsid
        string& strTimeStamp ///<master��ǰsid�ļ�¼��ʱ���
        );

    ///֪ͨ���ñ仯
    void _noticeConfChange();

    ///֪ͨ���仯
    void _noticeLockChange();

    ///���������ļ�������ֵ��0���ɹ���-1��ʧ��
    int  _parseConf(UnistorZkConf& conf);

    ///timeout��������ֵ��0���ɹ���-1��ʧ��
    int _dealTimeoutEvent(CwxTss* tss, ///<tss
        CwxMsgBlock*& msg, ///<��Ϣ
        CWX_UINT32 uiLeftEvent ///<��Ϣ��������������Ϣ����
        )
        ;
    ///timeout��������ֵ��0���ɹ���-1��ʧ��
    int _dealConnectedEvent(CwxTss* tss, ///<tss
        CwxMsgBlock*& msg, ///<��Ϣ
        CWX_UINT32 uiLeftEvent ///<��Ϣ��������������Ϣ����
        );
private:
    ///�ڲ���node���wacher�ص�function
    static void watcher(zhandle_t *zzh, ///<zk handle
        int type, ///<�������
        int state, ///<״̬
        const char *path, ///<����Ľڵ�
        void* context  ///<������Ϣ������UnistorHandler4Zk�����ָ�롣
        );

    ///zk����֤�Ļص�����
    static void zk_auth_callback(int rc, ///<��֤�ķ��ش���
        const void *data ///<UnistorHandler4Zk�����ָ�롣
        );
    
    ///zk�ڵ�����watch�ص�����
    static void nodeDataWatcher(zhandle_t *zzh, ///<zk handle
        int type, ///<�������
        int state, ///<״̬
        const char *path, ///<����Ľڵ�
        void* context  ///<������Ϣ������UnistorHandler4Zk�����ָ�롣
        );

    ///���Ļص�����
    static void lock_complete(bool bLock, ///<�Ƿ�lock
        void* context ///<UnistorHandler4Zk�����ָ�롣
        );
private:
    UnistorApp*               m_pApp;  ///<app����
    ZkAdaptor*                m_zk;    ///<zk����
    ZkLocker*                 m_zkLocker; ///<Locker����
    volatile bool             m_bInit;   ///<�Ƿ��Ѿ���ʼ��
    volatile bool             m_bConnected; ///<�Ƿ��Ѿ���������
    volatile bool             m_bAuth; ///<�Ƿ��Ѿ���֤
    volatile bool             m_bValid;   ///<�Ƿ�������״̬
    clientid_t*               m_clientId; ///<client id
    UnistorZkConf             m_conf; ///<�����ļ�
    UnistorZkLock             m_lock; ///<������Ϣ
    string                    m_strZkLockNode; ///<zk��lock�ڵ���
    string                    m_strZkConfNode; ///<zk��conf�ڵ�����
    string                    m_strMasterNode; ///<zk��master�ڵ�����
    string                    m_strHostNode;   ///<zk��host�ڵ�����
    char                      m_szZkDataBuf[MAX_ZK_DATA_SIZE];
    char                      m_szErr2K[2048]; ///<������Ϣ;
    CwxMutexLock              m_mutex;  ///���ݱ�����
    CWX_UINT64                m_ullVersion; ///<���û�lock�ı���汾��
};

#endif 
