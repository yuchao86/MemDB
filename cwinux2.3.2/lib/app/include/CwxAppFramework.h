#ifndef __CWX_APP_FRAMEWORK_H__
#define __CWX_APP_FRAMEWORK_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppFramework.h
@brief ����ܹ�����CwxAppFramework
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxDate.h"
#include "CwxAppConfig.h"
#include "CwxAppMacro.h"
#include "CwxLogger.h"
#include "CwxAppTcpAcceptor.h"
#include "CwxAppTcpConnector.h"
#include "CwxAppUnixAcceptor.h"
#include "CwxAppUnixConnector.h"
#include "CwxAppHandler4TcpConn.h"
#include "CwxAppHandler4UnixConn.h"
#include "CwxAppHandler4IoEvent.h"
#include "CwxAppHandler4Signal.h"
#include "CwxAppHandler4Time.h"
#include "CwxAppHandler4Notice.h"
#include "CwxAppHandler4Stdio.h"
#include "CwxAppHandlerCache.h"
#include "CwxAppConnInfo.h"
#include "CwxTss.h"
#include "CwxThreadPool.h"
#include "CwxCommander.h"
#include "CwxTaskBoard.h"
#include "CwxThreadPoolMgr.h"
#include "CwxAppListenMgr.h"
#include "CwxAppForkMgr.h"
#include "CwxAppReactor.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxAppFramework
@brief �ܹ�����Ļ��࣬ʵ����ͨ�š��źš�ʱ�ӡ�stdio����־�ȵĹ���
*/
class CWX_API CwxAppFramework
{
private:
    ///Service��host�����hash��KEY����
    class HostMapKey
    {
    public:
        HostMapKey(CWX_UINT32 uiSvrId, CWX_UINT32 uiHostId)
            :m_uiSvrId(uiSvrId),m_uiHostId(uiHostId)
        {
        }
        bool operator==(HostMapKey const& item) const
        {
            return (m_uiSvrId == item.m_uiSvrId) &&
                (m_uiHostId == item.m_uiHostId); 
        }
        size_t hash() const
        {
            return (m_uiSvrId<<3) + m_uiHostId;
        }
    private:
        CWX_UINT32 m_uiSvrId;
        CWX_UINT32 m_uiHostId;
    };
    ///IO handle��hash����
    typedef hash_set<CWX_HANDLE, char, CwxNumHash<CWX_UINT32> > HandleHash;

    ///��Ϣ�����������Ͷ���
    typedef void (*fnNoticeApi)(CwxAppFramework* pApp, CwxAppNotice* pNotice);

public:
    ///APPģʽ��ö�����Ͷ���
    enum{
        APP_MODE_ALONE=1,///<������ģʽ
        APP_MODE_TWIN=2,///<˫����ģʽ
        APP_MODE_PIPE=3///<�����ģʽ
    };
    ///ϵͳ������������
    enum{
        DEF_LOG_CHECK_SECOND = 600,///<ȱʡ��������־�����
        MIN_LOG_CHECK_SECOND = 10,///<��С��������־�����
        MAX_LOG_CHECK_SECOND = 1200,///<����������־�����
        DEF_KEEPALIVE_SECOND = 30,///<ȱʡ������KEEP-ALIVE�����
        MIN_KEEPALIVE_SECOND = 1,///<��С������KEEP-ALIVE�����
        MAX_KEEPALIVE_SECOND = 300,///<��������KEEP-ALIVE�����
        DEF_KEEPALIVE_REPLY_SECOND = 10///<KEEP-ALIVE�ظ�ʱ��
    };
    ///�����������߳��鶨��
    enum{
        SVR_TYPE_USER_START  = 2,///�û��������͵���Сֵ
        THREAD_GROUP_SYS = 1,///<ϵͳ�߳���ID��Ҳ����ͨ�ŵ��߳���ID
        THREAD_GROUP_USER_START = 2///<�û��߳���ID����Сֵ
    };
    /**
    @brief ���캯��
    @param [in] unAppMode APP��ģʽ��ȱʡΪ˫����ģʽ
    @param [in] uiMaxTaskNum Taskboard�й����TASK�����ֵ
    */
    CwxAppFramework(CWX_UINT16 unAppMode=CwxAppFramework::APP_MODE_TWIN,
        CWX_UINT32 uiMaxTaskNum=CWX_APP_MAX_TASK_NUM);
    ///��������
    virtual ~CwxAppFramework();
    /**
    @brief �ܹ��ĳ�ʼ����������������Ҫ���ش�APIʵ���Լ�����ĳ�ʼ����<br>
    ����Ҫ���ȵ��ø����init()��ʵ�ָ��໷���ĳ�ʼ����
    @param [in] argc main��argc��Ϊ�������������
    @param [in] argv main��argv��Ϊ����������б�
    @return -1��ʧ�ܣ� 0���ɹ��� 1��help
    */
    virtual int init(int argc, char** argv);
public:
    /**
    @brief ע����ܱ�׼�������Ϣ����ע��ɹ������׼��������뽫ͨ��OnStdinInput֪ͨ��
    @return -1��ע��ʧ�ܣ�0��ע��ɹ�
    */
    int noticeStdinListen();
    /**
    @brief ���ܹ�ע��һ��������TCP ��ַ+PORT�������ø�������
    @param [in] uiSvrId �ɴ˼��������������ӵ�SVR ID��
    @param [in] szAddr ������IP��ַ, *��ʾ�������еı��ص�ַ
    @param [in] unPort �����Ķ˿ں�
    @param [in] bRawData �˼�����ַ�����������ӵ��Ͻ��ܵ����ݣ��Ƿ���а�ͷ<br>
    true�����а�ͷ��false��û�а�ͷ
    @param [in] unMode Framework�����ӵĹ���ʽ��<br>
            CWX_APP_MSG_MODE��ʾ�мܹ�������Ϣ���շ���<br>
            CWX_APP_EVENT_MODE��ʾ�ܹ�ֻ���������϶�д�¼���֪ͨ��
    @param [in] fn listen socket����Ϥ���ú�������Ϊ�գ������á�
    @param [in] fnArg fn������arg������
    @return >0���˼�����Listen ID��-1��ʧ�ܡ�
    */
    int noticeTcpListen(CWX_UINT32 uiSvrId,
        char const* szAddr,
        CWX_UINT16 unPort,
        bool bRawData = false,
        CWX_UINT16 unMode=CWX_APP_MSG_MODE,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* fnArg = NULL);
    /**
    @brief ���ܹ�ע��һ��local ipc�����������ø�������
    @param [in] uiSvrId �ɴ˼��������������ӵ�SVR ID��
    @param [in] szPathFile local ipc��Ӧ���ļ�
    @param [in] bRawData �˼�����ַ�����������ӵ��Ͻ��ܵ����ݣ��Ƿ���а�ͷ<br>
    true�����а�ͷ��false��û�а�ͷ
    @param [in] unMode Framework�����ӵĹ���ʽ��<br>
            CWX_APP_MSG_MODE��ʾ�мܹ�������Ϣ���շ���<br>
            CWX_APP_EVENT_MODE��ʾ�ܹ�ֻ���������϶�д�¼���֪ͨ��
    @param [in] fn listen socket����Ϥ���ú�������Ϊ�գ������á�
    @param [in] fnArg fn������arg������
    @return >0���˼�����Listen ID��-1��ʧ�ܡ�
    */
    int noticeLsockListen(CWX_UINT32 uiSvrId,
        char const* szPathFile,
        bool bRawData = false,
        CWX_UINT16 unMode=CWX_APP_MSG_MODE,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* arg = NULL);
    /**
    @brief ���ܹ�ע��һ��������TCP����
    @param [in] uiSvrId �趨���ӵ�SVR ID��
    @param [in] uiHostId �趨���ӵ�Host ID��
    @param [in] szAddr ���ӵ�IP��ַ��
    @param [in] unPort ���ӵĶ˿ںš�
    @param [in] bRawData �������Ͻ��ܵ����ݣ��Ƿ���а�ͷ.true�����а�ͷ��false��û�а�ͷ
    @param [in] unMinRetryInternal ����ʧ��ʱ����С���Ӽ��.
    @param [in] unMaxRetryInternal ����ʧ��ʱ��������Ӽ��.
    @param [in] fn socket����Ϥ���ú�������Ϊ�գ������á�
    @param [in] fnArg fn������arg������
	@param [in] uiMiliTimeout ���ӳ�ʱʱ�䣬ȱʡ0��ʾ�����г�ʱ���ƣ���λΪ���롣
    @return  >0�������ӵ�CONN_ID��-1��ע��ʧ�ܡ�
    */
    int noticeTcpConnect(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        char const* szAddr,
        CWX_UINT16 unPort,
        bool bRawData = false,
        CWX_UINT16 unMinRetryInternal = 1,
        CWX_UINT16 unMaxRetryInternal = 60,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* fnArg=NULL,
		CWX_UINT32 uiMiliTimeout=0);
    /**
    @brief ���ܹ�ע��һ��������Local IPC����
    @param [in] uiSvrId �趨���ӵ�SVR ID��
    @param [in] uiHostId �趨���ӵ�Host ID��
    @param [in] szPathFile local IPC���Ӷ�Ӧ�ı����ļ���
    @param [in] bRawData �������Ͻ��ܵ����ݣ��Ƿ���а�ͷ. true�����а�ͷ��false��û�а�ͷ
    @param [in] unMinRetryInternal ����ʧ��ʱ����С���Ӽ��.
    @param [in] unMaxRetryInternal ����ʧ��ʱ��������Ӽ��.
    @param [in] fn socket����Ϥ���ú�������Ϊ�գ������á�
    @param [in] fnArg fn������arg������
    @return  >0�������ӵ�CONN_ID��-1��ע��ʧ�ܡ�
    */
    int noticeLsockConnect(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        char const* szPathFile,
        bool bRawData = false,
        CWX_UINT16 unMinRetryInternal = 1,
        CWX_UINT16 unMaxRetryInternal = 60,
        CWX_NET_SOCKET_ATTR_FUNC fn=NULL,
        void* fnArg = NULL);
    /**
    @brief ���ܹ�ע��һ��IO handle����,������һ����CWX_APP_MSG_MODEģʽ��
    @param [in] uiSvrId �趨���ӵ�SVR ID��
    @param [in] uiHostId �趨���ӵ�Host ID��
    @param [in] handle IO handle��
    @param [in] bRawData �������Ͻ��ܵ����ݣ��Ƿ���а�ͷ. true�����а�ͷ��false��û�а�ͷ.
    @param [in] userData ���������ص��û�����
    @param [in] pPai ͨ����������������
    @return  >0�������ӵ�CONN_ID��-1��ע��ʧ�ܡ�
    */
    int noticeHandle4Msg(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_HANDLE handle,
        bool bRawData = false,
        void* userData = NULL
        );
    /**
    @brief ���ܹ�ע��һ��IO handle���¼�����
    @param [in] uiSvrId  Svr id��
    @param [in] uiHostId  host id
    @param [in] handle IO handle��
    @param [in] userData ���������ص��û�����
    @param [in] unEventMask �����¼�����.
    @param [in] uiMillSecond ��ʱ��������0��ʾ�����г�ʱ��⡣
    @return 0���ɹ�;-1��ע��ʧ�ܡ�
    */
    int noticeHandle4Event(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_HANDLE handle,
        void* userData,
        CWX_UINT16 unEventMask= CwxAppHandler4Base::RW_MASK,
        CWX_UINT32 uiMillSecond=0
        );
    /**
    @brief �ر�Socket��listen����
    @param [in] uiListenId Listen��id��
    @return  0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeCloseListen(CWX_UINT32 uiListenId);
    /**
    @brief �ر�һ���мܹ������Socket ���ӣ�ΪCWX_APP_MSG_MODE
    @param [in] uiConnId socket������ID��
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeCloseConn(CWX_UINT32 uiConnId);
    /**
    @brief resume һ�������˿ڵļ���
    @param [in] uiListenId ������Listen id.
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeResumeListen(CWX_UINT32 uiListenId);
    /**
    @brief suspend һ�������˿ڵļ���
    @param [in] uiListenId ������Listen id.
    @return  0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeSuspendListen(CWX_UINT32 uiListenId);

    /**
    @brief resume һ�����ӵ����ݽ���
    @param [in] uiConnId ���ӵ�CONN ID.
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeResumeConn(CWX_UINT32 uiConnId);
    /**
    @brief suspend һ�����ӵ����ݽ���
    @param [in] uiConnId ���ӵ�CONN ID.
    @return  0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeSuspendConn(CWX_UINT32 uiConnId);

    /**
    @brief ȡ��һ��handle���¼�����
    @param [in] handle �����¼��� handle��
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeCancelHandle4Event(CWX_HANDLE handle);
    /**
    @brief ���½���ָ����socket������
    @param [in] conn_id ���ӵ�connection id��
    @param [in] uiDelay ������ʱ�ĺ�������0��ʾ�������ӡ�
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int noticeReconnect(CWX_UINT32 conn_id, CWX_UINT32 uiDelay=0);
    /**
    @brief ֪ͨ�¼�֪ͨ��pForkEnv����fork��صĻ�����Ϣ��fork���child���̵Ĳ���
    @param [in] pForkEnv fork��صĻ�����Ϣ��fork���child���̵Ĳ�����
    @return -1����ǰ��framework�Ļ�����ֱֹ��fork��0��success��
    */
    int noticeFork(CwxAppForkEnv* pForkEnv);
public:
    /**
    @brief ��CWX_APP_MSG_MODEģʽ��һ�����ӷ������ݡ�
    @param [in] msg Ҫ���͵���Ϣ������Ϊ�գ������ݰ��мܹ������ͷš�
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int sendMsgByConn(CwxMsgBlock* msg);
    /**
    @brief ��CWX_APP_MSG_MODEģʽ��SVR���鷢����Ϣ�����巢�͵�������OnSendMsgBySvr()��ѡ����
    @param [in] msg Ҫ���͵���Ϣ������Ϊ�գ������ݰ��мܹ������ͷš�
    @return 0���ɹ��� -1��ʧ�ܡ�
    */
    int sendMsgBySvr(CwxMsgBlock* msg);

public:
    /**
    @brief ����ͨ���̵߳�thread self store��
    @return ͨ���̵߳�tss����ָ��
    */
    virtual CwxTss* onTssEnv();
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
        bool& bSuspendListen);

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
    @brief ��������ʧ�ܡ�
    @param [in] conn ʧ�����ӵ���Ϣ��
    @return <0 ֹͣ���ӣ�0��Ĭ�Ϸ�ʽ�� >0���´��������Ե�ʱ��������λΪms��
    */
    virtual int onFailConnect(CwxAppHandler4Msg& conn);
    /**
    @brief ֪ͨCWX_APP_MSG_MODEģʽ�����ӹرա�
    @param [in] conn �رյ����ӡ�
    @return �����������ӣ�-1����ʾ�����ӣ�0��Ĭ�Ϸ�ʽ�� >0���´�������ʱ��������λΪms���������Ӻ��ԡ�
    */
    virtual int onConnClosed(CwxAppHandler4Msg & conn);

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
    /**
    @brief ֪ͨsendMsgByMsg()������Ϣ����Ҫ���û��Լ�ѡ���͵����Ӳ����͡�<br>
    @param [in] msg Ҫ���͵���Ϣ��
    @return -1���޷����͡� 0���ɹ�������Ϣ��
    */
    virtual int onSendMsgBySvr(CwxMsgBlock* msg);
    /**
    @brief ֪ͨCWX_APP_MSG_MODEģʽ�����ӿ�ʼ����һ����Ϣ��<br>
    ֻ����sendMsg()��ʱ��ָ��BEGIN_NOTICE��ʱ��ŵ���.
    @param [in] msg Ҫ���͵���Ϣ��
    @param [in] conn ������Ϣ�����ӡ�
    @return -1��ȡ����Ϣ�ķ��͡� 0��������Ϣ��
    */
    virtual int onStartSendMsg(CwxMsgBlock* msg,
        CwxAppHandler4Msg& conn);
    /**
    @brief ֪ͨCWX_APP_MSG_MODEģʽ���������һ����Ϣ�ķ��͡�<br>
    ֻ����sendMsg()��ʱ��ָ��FINISH_NOTICE��ʱ��ŵ���.
    @param [in,out] msg ���뷢����ϵ���Ϣ��������NULL����msg���ϲ��ͷţ�����ײ��ͷš�
    @param [in] conn ������Ϣ�����ӡ�
    @return 
            CwxMsgSendCtrl::UNDO_CONN�����޸����ӵĽ���״̬
            CwxMsgSendCtrl::RESUME_CONN�������Ӵ�suspend״̬��Ϊ���ݽ���״̬��
            CwxMsgSendCtrl::SUSPEND_CONN�������Ӵ����ݽ���״̬��Ϊsuspend״̬
    */
    virtual CWX_UINT32 onEndSendMsg(CwxMsgBlock*& msg,
        CwxAppHandler4Msg& conn);

    /**
    @brief ֪ͨCWX_APP_MSG_MODEģʽ�������ϣ�һ����Ϣ����ʧ�ܡ�<br>
    ֻ����sendMsg()��ʱ��ָ��FAIL_NOTICE��ʱ��ŵ���.
    @param [in,out] msg ����ʧ�ܵ���Ϣ��������NULL����msg���ϲ��ͷţ�����ײ��ͷš�
    @return void��
    */
    virtual void onFailSendMsg(CwxMsgBlock*& msg);
    /**
    @brief ��һ��IO HANDLE ��READ/WRITE READY��ʱ��֪ͨ��
    @param [in] uiSvrId handle��svr id��
    @param [in] uiHostId handle��host id��
    @param [in] handle �ɶ����д��IO HANDLE��
    @param [in] unRegEventMask ��ص��¼���
    @param [in] unEventMask  ready���¼�������ʱ�Ļ�������CwxAppHandler4Base::TIMEOUT_MASK��
    @param [in] userData ע��ʱ��userData
    @return void��
    */
    virtual void onEvent4Handle(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_HANDLE handle,
        CWX_UINT16 unRegEventMask,
        CWX_UINT16 unEventMask,
        void* userData);
    /**
    @brief STDIN������ʱ�䡣
    @param [in] msg �յ�����Ϣ
    @return void��
    */
    virtual void onStdinInput(char* msg);

    ///STDIN�ر��¼������ն˹رյ�ʱ�򣬾ͻ��յ�����Ϣ������Ϣֻ֪ͨһ�顣
    virtual void onStdinClosed();
    ///֪ͨ�����˳�����Ҫֹͣ�ϲ���߳�
    virtual void onStop();
    ///�ܹ��Ĺҹ�����������������Ϣ�ͼܹ�����
    virtual void onHook();
public:
    ///�����ܹ�, 0:�ɹ���-1��ʧ�ܡ��Ƴ�ǰ�����Լ��ͷ��Լ���
    int run();
    ///ֹͣ�ܹ�
    void stop();
    /**
    @brief �����������в���
    @param [in] argc main��argc��Ϊ�������������
    @param [in] argv main��argv��Ϊ����������б�
    @return -1��ʧ�ܣ� 0���ɹ��� 1��help
    */
    int parse(int argc, char** argv);
    ///output help information
    void help();
    ///����ָ�����ź�
    void blockSignal(int signal);
    ///�����ָ���źŵ�����
    void unblockSignal(int signal);
    ///�¼������hook api,-1���˳���0������
    static int hook(void* arg);
public:
    ///���ؽ�������״̬�Ƿ�����
    bool isAppRunValid() const;
    ///���ý�������״̬�Ƿ�����
    void setAppRunValid(bool bValid);
    ///��ȡ�������в�������ԭ��
    string const& getAppRunFailReason() const;
    ///���ý������в�������ԭ��
    void setAppRunFailReason(string const& strReason);
    ///���ó���İ汾��
    void setAppVersion(string const& strVersion);
    ///��ȡ����İ汾��
    string const& getAppVersion() const;
    ///���ó���������޸�ʱ��
    void setLastModifyDatetime(string const& strDatetime);
    ///��ȡ����������޸�ʱ��
    string const& getLastModifyDatetime() const;
    ///���ó���ı���ʱ��
    void setLastCompileDatetime(string const& strDatetime);
    ///��ȡ����ı���ʱ��
    string const& getLastCompileDatetime() const;
    ///��������������Ƿ�Ҫֹͣ����
    bool isCmdStop() const;
    ///��������������Ƿ�Ҫ������������
    bool isCmdRestart() const;
    ///�����Ƿ������˳�
    bool isStopped() const;
    ///��ȡ����Ŀ¼
    char const*  getWordDir() const;
    ///���ù���Ŀ¼
    void setWorkDir(char const* szWorkDir);
    ///��ȡ�����ļ�������
    char const* getConfFile() const;
    ///���������ļ�
    void setConfFile(char const* szConfFile);
    ///��ȡϵͳ��ʱ�Ӽ��
    CWX_UINT32 getClick() const ;
    ///����ϵͳ��ʱ�Ӽ��,��λΪ���룬0��ʾû��ʱ�ӡ�
    void setClick(CWX_UINT32 uiInternal);
    ///��ȡѭ����־�ļ��Ĵ�С
    CWX_UINT32 getLogFileSize() const;
    ///����ѭ����־�ļ��Ĵ�С
    void setLogFileSize(CWX_UINT32 uiSize);
    ///��ȡ��־����
    CWX_UINT32 getLogLevel() const;
    ///������־����
    void setLogLevel(CWX_UINT32 unLevel);
    ///��ȡѭ����־������
    CWX_UINT16 getLogFileNum() const;
    ///����ѭ����־������
    void setLogFileNum(CWX_UINT16 unLogFileNum);
    ///����ѭ����־����־�ļ�
    void toLogFileNo(CWX_UINT16 unLogFileNo);
    ///��ȡkeep-alive��ʱ����
    CWX_UINT16 getKeepAliveSecond() const;
    ///����keep-alive��ʱ����
    void setKeepAliveSecond(CWX_UINT16 unSecond);
    ///��ȡlog��check���
    CWX_UINT16 getLogCheckSecond() const ;
    ///����Log��check���
    void setLogCheckSecond(CWX_UINT16 unSecond) ;
    ///��ȡ�Ƿ���debugģʽ
    bool isDebug() const;
    ///����debugģʽ
    void setDebug(bool bDebug);
    ///check enable onHook
    bool isEnableHook();
    ///set enable onHook
    void enableHook(bool bEnable);
    ///���ý���ID
    CWX_UINT16 getProcId() const;
    ///��ȡ����ID
    void setProcId(CWX_UINT16 uiProcId);
    ///��ȡ�ܹ���commander
    CwxCommander& getCommander();
    ///��ȡ�ܹ���taskboard����
    CwxTaskBoard& getTaskBoard();
    ///��ȡ�ܹ����̳߳ع���������
    CwxThreadPoolMgr* getThreadPoolMgr();
    ///��ȡͨ���̵߳�tss��Ϣ
    CwxTss*  getAppTss();
    ///��ȡhandle cache
    CwxAppHandlerCache*  getHandlerCache();
    ///��ȡTCP connector
    CwxAppTcpConnector* getTcpConnector();
    ///��ȡUNIX connector
    CwxAppUnixConnector* getUnixConnector();
    ///return the reactor.
    CwxAppReactor* reactor();
public:
    /*
    һ�½ӿ����ɵײ��msg handler���õģ���������������ܹ�ʹ�á�
    */ 

    ///֪ͨһ�����ӽ�������onConnCreated()��Ӧ
    int openConn(CwxAppHandler4Msg& conn,
        bool& bStopListen);
    ///֪ͨ�յ���һ����Ϣ����onRecvMsg()��Ӧ��
    int recvMessage(CwxMsgHead& header,
        CwxMsgBlock* msg,
        CwxAppHandler4Msg& conn,
        bool& bSuspendConn);
    ///֪ͨ���ӹرգ���onConnClosed()��Ӧ��
    int connClosed(CwxAppHandler4Msg& conn);
    ///accept notice event
    void noticeEvent();
protected:
    ///���üܹ����������ã�����ֵ��0:success, -1:failure.
    virtual int initRunEnv();
    ///�ͷżܹ���������Դ��ÿ��������������ش�API�ͷ��Լ�����Դ�����������û����destroy��
    ///��API�����ǿ�����ģ�Ҳ����˵�����Է�������
    ///��app�����̳߳أ����̳߳ص�stop�����ڴ�API�С�
    virtual void destroy();

private:
    ///ע��ܹ��¼����յ��¼�������
    void  regNoticeFunc();
    ///notice send msg by conn
    static void innerNoticeSendMsgByConn(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice send msg by svr
    static void innerNoticeSendMsgBySvr(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice tcp connect
    static void innerNoticeTcpConnect(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice unix connect
    static void innerNoticeUnixConnect(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice add Io listen
    static void innerNoticeAddIoHandle(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice tcp accept
    static void innerNoticeTcpListen(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice unix accept
    static void innerNoticeUnixListen(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice close listen
    static void innerNoticeCloseListen(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice close connect
    static void innerNoticeCloseConnect(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///notice noticeReconnect 
    static void innerNoticeReconnect(CwxAppFramework* pApp,
        CwxAppNotice* pNotice);
    ///֪ͨ�յ���һ��ϵͳ��Ϣ��
    int recvSysMessage(CwxMsgBlock* msg,
        CwxAppHandler4Msg& conn,
        CwxMsgHead const& header);

private:
    bool                       m_bAppRunValid;///<���������������
    string                     m_strAppRunFailReason;///<�������в�������ԭ��
    string                     m_strVersion;///<����İ汾��
    string                     m_strLastModifyDatetime;///<���������޸�ʱ��
    string                     m_strLastCompileDatetime;///<����ı���ʱ��
    CwxAppHandler4StdIo*        m_pStdIoHandler;///<���ձ�׼�����Handle
    CwxAppHandler4Time*         m_pTimeHandler; ///<ʱ���¼���handler
    CwxAppHandler4Notice*       m_pNoticeHandler; ///<����֪ͨ�¼���Handle
    CWX_UINT16                 m_unAppMode;///<APP ģʽ
    bool                       m_bCmdStop;///<���������Ƿ�Ҫֹͣ����
    bool                       m_bCmdRestart;///<���������Ƿ�Ҫ��������
    bool                       m_bStopped;///<�����Ƿ����˳�״̬
    CwxAppReactor*	            m_pReactor; ///<the reactor for event loop.
    CWX_UINT32                 m_uiTimeClick; ///<ʱ�ӿ̶ȣ���λΪms
    string                      m_strWorkDir; ///<����Ŀ¼.
    string                      m_strAppName; //<��������
    string                      m_strConfFile; ///<����������ļ�
    CWX_UINT32                  m_uiLogSize; ///<��־�ļ��Ĵ�С.
    CWX_UINT16                  m_unLogFileNum; ///<��־�ļ�������
    CWX_UINT16                  m_unDefKeepAliveSecond; ///<����KEEP-ALIVE�ļ����
    CWX_UINT16                  m_unLogCheckSecond; ///<����log�ļ��ļ����
    bool                        m_bEnableHook;///<�Ƿ񿪷��ܹ���HOOK
    CWX_UINT16                  m_uiProcId; ///<�����ģʽ�Ľ������
    CwxCommander              m_commander;///<commander����
    CwxTaskBoard              m_taskBoard;///<taskboard����
    CwxAppListenMgr*             m_pListenMgr;///<��������������
    CwxTss*                  m_pTss;///<ͨ���̵߳�TSS
    CwxAppHandlerCache*          m_pHandleCache;///<�ͷ����Ӿ��cache
    CwxAppTcpConnector*         m_pTcpConnector; ///<TCP��connector
    CwxAppUnixConnector*        m_pUnixConnector; ///<unix��connector
    //not lock for communite thread only access
    fnNoticeApi                m_arrNoticeApi[CwxAppNotice::ALL_NOTICE_NUM + 1];///<notcie����Ϣӳ��
    CwxThreadPoolMgr*        m_pThreadPoolMgr;///<�̳߳ع������
    //���Կ���
    bool                      m_bDebug; ///<�Ƿ��ڵ�����
    //following is fork's info
    CwxAppForkMgr              m_forkMgr; ///fork���¼�������
};
CWINUX_END_NAMESPACE

#include "CwxAppFramework.inl"

#include "CwxPost.h"


#endif
