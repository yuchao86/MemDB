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

///前置声明
class UnistorApp;

///zk事件处理的handle
class UnistorHandler4Zk {
public:
    enum{
        INIT_RETRY_INTERNAL = 10, ///<初始化重试间隔，为10s
        REFETCH_INTERNAL = 60, ///<重新获取时间间隔
        MAX_ZK_DATA_SIZE = 1024 * 1024 ///<最大的zk空间大小
    };
public:
    ///构造函数
    UnistorHandler4Zk(UnistorApp* pApp);
    ///析构函数
    virtual ~UnistorHandler4Zk();
public:
    ///初始化；0:成功；-1：失败
    int init();
    ///初始zookeeper事件
    void doEvent(CwxTss* tss, CwxMsgBlock*& msg, CWX_UINT32 uiLeftEvent);
    ///停止zookeeper监控
    void stop();
public:
    ///是否已经初始化
    bool isInit() const{
        return m_bInit;
    }

    ///是否已经连接
    bool isConnected() const {
        return m_bConnected;
    }
    
    ///是否已经认证
    bool isAuth() const {
        return m_bAuth;
    }

    ///正常检测
    bool isValid() const {
        return m_bValid;
    }

    ///是否是master
    bool isMaster(){
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        return _isMaster();
    }
    
    ///获取配置信息
    void getConf(UnistorZkConf& conf) {
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        conf = m_conf;
    };
    
    ///获取锁的信息
    void getLockInfo(UnistorZkLock& zkLock) {
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        zkLock = m_lock;
    }; ///<锁的信息

    ///获取错误信息
    void getErrMsg(string& strErrMsg){
        CwxMutexGuard<CwxMutexLock> lock(&m_mutex);
        strErrMsg = m_szErr2K;
    }
private:
    ///连接zk。返回值：0：成功；-1：失败
    int _connect();

    ///zk认证。返回值：1：认证成功；0：等待认证结果；-1：认证失败
    int _auth();

    ///初始化连接信息
    void _reset();

    ///加载配置信息。返回值：0：成功；-1：失败
    int  _loadZkConf(CwxTss* tss);

    ///加锁。返回值：0：成功；-1：失败
    int _lock();

    ///获取锁的信息。返回值：0：成功；-1：失败
    int _loadLockInfo(CwxTss* tss);

    ///是否是master。返回值：true：是；false：不是。
    inline bool _isMaster() const{
        if (m_zkLocker) return m_zkLocker->isLocked();
        return false;
    }

    ///初始化zk连接信息。返回值：0：成功；-1：失败
    int _init();

    ///设置host的信息。返回值：0：成功；-1：失败
    int _setHostInfo();

    ///设置master的信息。返回值：0：成功；-1：失败
    int _setMasterInfo();

    ///获取host的数据同步信息。返回值：0：成功；-1：失败
    int _getHostInfo(char const* szHost, ///<获取主机的标志
        CWX_UINT64& ullSid, ///<获取主机当前的sid值
        CWX_UINT32& uiTimeStamp ///<获取的最小sid的时间戳。
        );

    ///获取master的信息。返回值：0：成功；-1：失败
    int _getMasterInfo(string& strMaster, ///<master的标志
        CWX_UINT64& ullMasterSid, ///<要变为master的最小sid
        CWX_UINT64& ullSid, ///<master的当前sid
        string& strTimeStamp ///<master当前sid的记录的时间戳
        );

    ///通知配置变化
    void _noticeConfChange();

    ///通知锁变化
    void _noticeLockChange();

    ///解析配置文件。返回值：0：成功；-1：失败
    int  _parseConf(UnistorZkConf& conf);

    ///timeout处理。返回值：0：成功；-1：失败
    int _dealTimeoutEvent(CwxTss* tss, ///<tss
        CwxMsgBlock*& msg, ///<消息
        CWX_UINT32 uiLeftEvent ///<消息队列中滞留的消息数量
        )
        ;
    ///timeout处理。返回值：0：成功；-1：失败
    int _dealConnectedEvent(CwxTss* tss, ///<tss
        CwxMsgBlock*& msg, ///<消息
        CWX_UINT32 uiLeftEvent ///<消息队列中滞留的消息数量
        );
private:
    ///内部的node变更wacher回调function
    static void watcher(zhandle_t *zzh, ///<zk handle
        int type, ///<变更类型
        int state, ///<状态
        const char *path, ///<变更的节点
        void* context  ///<环境信息，就是UnistorHandler4Zk对象的指针。
        );

    ///zk的认证的回调函数
    static void zk_auth_callback(int rc, ///<认证的返回代码
        const void *data ///<UnistorHandler4Zk对象的指针。
        );
    
    ///zk节点变更的watch回调函数
    static void nodeDataWatcher(zhandle_t *zzh, ///<zk handle
        int type, ///<变更类型
        int state, ///<状态
        const char *path, ///<变更的节点
        void* context  ///<环境信息，就是UnistorHandler4Zk对象的指针。
        );

    ///锁的回调函数
    static void lock_complete(bool bLock, ///<是否lock
        void* context ///<UnistorHandler4Zk对象的指针。
        );
private:
    UnistorApp*               m_pApp;  ///<app对象
    ZkAdaptor*                m_zk;    ///<zk对象
    ZkLocker*                 m_zkLocker; ///<Locker对象
    volatile bool             m_bInit;   ///<是否已经初始化
    volatile bool             m_bConnected; ///<是否已经建立连接
    volatile bool             m_bAuth; ///<是否已经认证
    volatile bool             m_bValid;   ///<是否处于正常状态
    clientid_t*               m_clientId; ///<client id
    UnistorZkConf             m_conf; ///<配置文件
    UnistorZkLock             m_lock; ///<锁的信息
    string                    m_strZkLockNode; ///<zk的lock节点名
    string                    m_strZkConfNode; ///<zk的conf节点名字
    string                    m_strMasterNode; ///<zk的master节点名字
    string                    m_strHostNode;   ///<zk的host节点名字
    char                      m_szZkDataBuf[MAX_ZK_DATA_SIZE];
    char                      m_szErr2K[2048]; ///<错误消息;
    CwxMutexLock              m_mutex;  ///数据保护锁
    CWX_UINT64                m_ullVersion; ///<配置或lock的变更版本号
};

#endif 
