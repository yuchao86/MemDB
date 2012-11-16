#ifndef __CWX_APP_CONN_INFO_H__
#define __CWX_APP_CONN_INFO_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxAppConnInfo.h
*@brief ������CwxAppConnInfo
*@author cwinux@gmail.com
*@version 0.1
*@date  2010-07-30
*@warning  ��.
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxMsgHead.h"
#include "CwxAppMacro.h"
#include "CwxAppConfig.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxLogger.h"
#include "CwxNetMacro.h"

CWINUX_BEGIN_NAMESPACE
class CwxAppHandler4Msg;
/**
@class CwxAppConnInfo
@brief ���Ӷ��󣬱������ӵ�״̬�����ͼ������շ�����Ϣ��
*/
class CWX_API CwxAppConnInfo{
public:
    ///����״̬����
    enum{
        IDLE = 0,      ///<��ʼ����״̬
        CONNECTING,    ///<�����е�״̬������connector���ã���ת��ΪESTABLISHING��FAILED
        TIMEOUT,       ///<�ȴ���ʱ״̬��Ҳ���ǵȴ���һ���������ӡ���ת��ΪESTABLISHING��FAILED
        ESTABLISHING,  ///<�ȴ���������ת��ΪESTABLISHED��FAILED��
        ESTABLISHED,   ///<�����Ѿ�������״̬����ת��ΪFAILED
        FAILED         ///<����ʧ�ܵ�״̬����ת��ΪCONNECTING��TIMEOUT
    };
public:
    /**
    @brief ���캯��
    */
    CwxAppConnInfo();
    ///��������
    ~CwxAppConnInfo();
public:
    ///��ȡ���ӵ�SVR-ID
    CWX_UINT32 getSvrId() const; 
    ///�������ӵ�SVR-ID
    void setSvrId(CWX_UINT32 uiSvrId);

    ///��ȡ���ӵ�HOST-ID
    CWX_UINT32 getHostId() const;
    ///�������ӵ�HOST-ID
    void setHostId(CWX_UINT32 uiHostId);

    ///��ȡ���ӵ�����ID
    CWX_UINT32 getConnId() const; 
    ///�������ӵ�����ID
    void setConnId(CWX_UINT32 uiConnId);

    ///��ȡ�������ӵ�Listen ID
    CWX_UINT32 getListenId() const;
    ///���ñ������ӵ�Listen ID
    void setListenId(CWX_UINT32 uiListenId);
	///��ȡ���ӳ�ʱʱ��
	CWX_UINT32 getConnectTimeout() const;
	///�������ӳ�ʱʱ��
	void setConnectTimeout(CWX_UINT32 uiTimeout);
    ///��ȡ���ӵ�״̬
    CWX_UINT16 getState() const;
    ///�������ӵ�״̬
    void setState(CWX_UINT16 unState);

    ///��ȡ���ӵĴ���ʱ��
    time_t getCreateTime() const;
    ///�������ӵĴ���ʱ��
    void setCreateTime(time_t ttTime);
    ///��ȡ����ʧ�����ӵĴ���
    CWX_UINT32 getFailConnNum() const;
    ///��������ʧ�����Ӵ���
    void setFailConnNum(CWX_UINT32 uiNum);
    ///��������ʧ�����Ӵ���
    CWX_UINT32 incFailConnNum();
    ///��ȡʧЧ����������С������ʱ����
    CWX_UINT16 getMinRetryInternal() const;
    ///����ʧЧ����������С������ʱ����
    void setMinRetryInternal(CWX_UINT16 unInternal);
    ///��ȡʧЧ�����������������ʱ����
    CWX_UINT16 getMaxRetryInternal() const;
    ///����ʧЧ�����������������ʱ����
    void setMaxRetryInternal(CWX_UINT16 unInternal);
    ///��ȡ�����Ƿ�Ϊ��������
    bool isActiveConn() const;
    ///����Ϊ��������
    void setActiveConn(bool bActive);
    ///��ȡ�����Ƿ������ر�
    bool isActiveClose() const;
    ///��������ʱ�����ر�
    void setActiveClose(bool bActive);
    ///��ȡ���ӵ����ݰ��Ƿ��а�ͷ
    bool isRawData() const ;
    ///�������ӵ����ݰ���raw��ʽ
    void setRawData(bool bRaw);
    ///��ȡ���������յ���Ϣ��ʱ��
    time_t  getLastRecvMsgTime() const;
    ///�������������յ���Ϣ��ʱ��
    void setLastRecvMsgTime(time_t ttTime);
    ///��ȡ�������·�����Ϣ��ʱ��
    time_t  getLastSendMsgTime() const;
    ///�����������·�����Ϣ��ʱ��
    void setLastSendMsgTime(time_t ttTime);
    ///��ȡ���ӵ��û�����
    void*  getUserData() const;
    ///�������ӵ��û�����
    void setUserData(void* userData);
    ///��ȡ���ӵȴ����͵������Ϣ��������0��ʾû������
    CWX_UINT32 getMaxWaitingMsgNum() const;
    ///�����������ĵȴ����͵���Ϣ������Ĭ��0��ʾû������
    void setMaxWaitingMsgNum(CWX_UINT32 uiNum=0);
    ///�ж��Ƿ����Ӵ����Ͷ�������
    bool isWaitingMsgQueueFull() const;
    ///��ȡ���ӵȴ����͵���Ϣ������
    CWX_UINT32 getWaitingMsgNum() const;
    ///�������ӵȴ����͵���Ϣ������
    void setWaitingMsgNum(CWX_UINT32 uiNum);
    ///�������ӵȴ����͵���Ϣ������
    CWX_UINT32 incWaitingMsgNum();
    ///�������ӵȴ����͵���Ϣ������
    CWX_UINT32 decWaitingMsgNum();
    ///��ȡ�����Ѿ��������յ�����Ϣ��������
    CWX_UINT32 getContinueRecvNum() const;
    ///���������Ѿ������յ�����Ϣ��������
    void setContinueRecvNum(CWX_UINT32 uiNum);
    ///��ȡ�������͵���Ϣ����
    CWX_UINT32 getContinueSendNum() const;
    ///�����������͵���Ϣ����
    void setContinueSendNum(CWX_UINT32 uiNum);
    ///�ж϶Ͽ��������Ƿ���Ҫ����
    bool isNeedReconnect() const;
    ///�Ƿ����CwxAppFramework::onCreate
    bool isInvokeCreate() const;
    ///�����Ƿ����CwxAppFramework::onCreate
    void setInvokeCreate(bool bInvoke);
    ///�Ƿ���������
    bool isReconn() const;
    ///�����Ƿ�����
    void setReconn(bool bReconnect);
    ///��ȡ����������ʱ�ĺ�����
    CWX_UINT32 getReconnDelay() const;
    ///��������������ʱ�ĺ�����
    void setReconnDelay(CWX_UINT32 uiDelay);
    ///��ȡsocket���õ�function
    CWX_NET_SOCKET_ATTR_FUNC getSockFunc() const;
    ///����socket���õ�function
    void setSockFunc(CWX_NET_SOCKET_ATTR_FUNC fn);
    ///��ȡsocket����function��arg
    void* getSockFuncArg() const;
    ///����socket����function��arg
    void setSockFuncArg(void* arg);
    ///��ȡ���Ӷ�Ӧ��handler
    CwxAppHandler4Msg* getHandler();
    ///�������Ӷ�Ӧ��handler
    void setHandler(CwxAppHandler4Msg*  pHandler);
public:
    ///�����Ӷ����״̬��Ϣ��λ
    void reset();
private:
    CWX_UINT32         m_uiSvrId;  ///<svr id
    CWX_UINT32         m_uiHostId; ///<host id
    CWX_UINT32         m_uiConnId;  ///<connection id
	CWX_UINT32         m_uiConnectTimeout; ///<���ӳ�ʱʱ�䵥λΪms
    CWX_UINT32         m_uiListenId; ///<accept connection's  acceptor ID. for passive conn.
    CWX_UINT16         m_unState; ///<connection state.
    time_t             m_ttCreateTime;///<connection's create time
    CWX_UINT16          m_unMinRetryInternal; ///<min re-connect internal
    CWX_UINT16         m_unMaxRetryInternal; ///<max re-connect internal
    CWX_UINT32          m_uiFailConnNum; ///<count for failure connect
    bool               m_bActiveConn; ///< sign for active connection.
    bool               m_bActiveClose; ///< sign for close connection by user.
    bool               m_bRawData; ///< sign for raw data connection
    time_t             m_ttLastRecvMsgTime;///<last recv msg time
    time_t             m_ttLastSendMsgTime;///<last send msg time
    void*              m_pUserData; ///<user dat for connection
    CWX_UINT32         m_uiContinueRecvNum; ///<conintue recv msg num
    CWX_UINT32         m_uiContinueSendNum; ///<�������͵��������
    CWX_UINT32         m_uiMaxWaitingMsgNum; ///<���ȴ���Ϣ������
    CWX_UINT32         m_uiWaitingMsgNum;///<waiting msg num
    bool               m_bInvokeCreate; ///<�Ƿ���open��ʱ�򣬵���CwxAppFramework::onCreate��Ĭ�ϵ���
    bool               m_bReconn; ///<�Ƿ�������
    CWX_UINT32         m_uiReconnDelay; ///<������ʱ�ĺ�����
    CWX_NET_SOCKET_ATTR_FUNC m_fn; ///<socket ���õ�function
    void*              m_fnArg; ////<socket ���õ�function��arg ����
    CwxAppHandler4Msg*  m_pHandler; ///<���Ӷ�Ӧ��Handler
};


CWINUX_END_NAMESPACE
#include "CwxAppConnInfo.inl"
#include "CwxPost.h"
#endif

