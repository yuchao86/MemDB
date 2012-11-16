#ifndef __CWX_APP_AIO_WINDOW_H__
#define __CWX_APP_AIO_WINDOW_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppAioWindow.h
@brief �첽ͨ�ŵ���Ϣ���͡����տ�����Ķ��塣����TCP�Ļ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-12
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxAppAioWindow
@brief �첽ͨ�ŵ���Ϣ���͡����տ����࣬����TCP�Ļ�����
*/
class CWX_API CwxAppAioWindow
{
public:
    ///����״̬����
    enum{
        STATE_CLOSED = 0,///<�ر�״̬
        STATE_CONNECTED = 1,///<���ӽ���״̬
        STATE_SYNCING = 2///<���ݷ���״̬
    };
    ///ͨ�Ŵ��ڿ��Ʋ���
    enum{
        DEF_WINDOW_SIZE = 512,///<ȱʡ��ͨ�Ŵ��ڵĴ�С
        MAX_WINDOW_SIZE = 8192///<����ͨ�Ŵ��ڴ�С
    };
    /**
    @brief ���캯��
    @param [in] uiSvrId ���ӵ�SVR-ID
    @param [in] uiHostId ���ӵ�HOST-ID
    @param [in] uiConnId ���ӵ�����ID
    @param [in] uiWindowSize ���ӵ��첽ͨ�Ż�����С
    */
    CwxAppAioWindow(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_UINT32 uiConnId,
        CWX_UINT32 uiWindowSize=DEF_WINDOW_SIZE)
    {
        m_pHandle = NULL;
        m_uiWindowSize = uiWindowSize;
        if (!m_uiWindowSize) m_uiWindowSize = 1;
        if (m_uiWindowSize > MAX_WINDOW_SIZE) m_uiWindowSize = MAX_WINDOW_SIZE;
        m_uiSvrId = uiSvrId;
        m_uiHostId = uiHostId;
        m_uiConnId = uiConnId;
        m_uiGroupId = 0;
        m_uiGroupNum = 0;
        m_uiState = STATE_CLOSED;
        m_ullStartSid = 0;
    };
    ///����Ŀ�������
    CwxAppAioWindow& operator=(CwxAppAioWindow const& item)
    {
        if (&item != this)
        {
            m_pHandle = item.m_pHandle;
            m_uiSvrId = item.m_uiSvrId;
            m_uiHostId = item.m_uiHostId;
            m_uiConnId = item.m_uiConnId;
            m_uiGroupId = item.m_uiGroupId;
            m_uiGroupNum = item.m_uiGroupNum;
            m_uiState = item.m_uiState;
            m_ullStartSid = item.m_ullStartSid;
            m_strHostName = item.m_strHostName;
        }
        return *this;
    }
public:
    ///��ȡ���ڵ�����ͬ���ľ��
    void*& getHandle()
    {
        return m_pHandle; 
    }
    ///���ô��ڵ�����ͬ���ľ��
    void setHandle(void* pHandle)
    {
        m_pHandle = pHandle;
    }
    ///��ȡ�������ӵ�SVR-ID
    CWX_UINT32 getSvrId() const
    {
        return m_uiSvrId;
    }
    ///��ȡ�������ӵ�HOST-ID
    CWX_UINT32 getHostId() const 
    {
        return m_uiHostId;
    }
    ///��ȡ�������ӵ�CONN-ID
    CWX_UINT32 getConnId() const 
    {
        return m_uiConnId;
    }
    ///��ȡ�������ӵķ���ID
    CWX_UINT32 getGroupId() const 
    {
        return m_uiGroupId;
    }
    ///���ô������ӵķ���ID
    void setGroupId(CWX_UINT32 uiGroup) 
    {
        m_uiGroupId = uiGroup;
    }
    ///��ȡ�������ӵķ�������
    CWX_UINT32 getGroupNum() const 
    {
        return m_uiGroupNum;
    }
    ///���ô������ӵķ�������
    void setGroupNum(CWX_UINT32 uiNum) 
    {
        m_uiGroupNum = uiNum;
    }
    ///��ȡ�������ӵ����ӵ�״̬
    CWX_UINT32 getState() const 
    {
        return m_uiState;
    }
    ///���ô������ӵ����ӵ�״̬
    void setState(CWX_UINT32 uiState) 
    {
        m_uiState = uiState;
    }
    ///���ô��ڵĳ�ʼͬ����SID
    void setStartSid(CWX_UINT64 const& sid)
    {
        setState(STATE_SYNCING);
        m_ullStartSid = sid;
        m_sendSidSet.clear();
    }
    ///��ȡ���ڵĳ�ʼͬ����SID
    CWX_UINT64 const& getStartSid() const
    {
        return m_ullStartSid;
    }
    ///���ô��ڷ��͵�SID��true����ȷ��false����Ϣ�Ѿ�����
    bool sendOneMsg(CWX_UINT64 ullSid)
    {
        return m_sendSidSet.insert(ullSid).second?true:false;
    }
    ///���ô����յ���SID��true����ȷ��false������Ϣû�з��ͻ�������
    bool recvOneMsg(CWX_UINT64 ullSid, bool bOrder=true/*�Ƿ�Ӧ�����򷵻�*/) 
    {
        if (bOrder)
        {
            set<CWX_UINT64>::iterator iter = m_sendSidSet.begin();
            if ((iter != m_sendSidSet.end()) && (*iter == ullSid))
            {
                m_sendSidSet.erase(iter);
                return true;
            }
            return false;
        }
        return m_sendSidSet.erase(ullSid)?true:false;
    }
    ///��ȡ��һ����Ҫ���ص���Ϣ��true�����ڣ�false��������
    bool getNextRecvMsg(CWX_UINT64& ullSid)
    {
        set<CWX_UINT64>::iterator iter = m_sendSidSet.begin();
        if (iter != m_sendSidSet.end())
        {
            ullSid = *iter;
            return true;
        }
        return false;
    }
    ///�жϴ������ӵ������Ƿ�ر�
    bool isClosed() const 
    {
        return STATE_CLOSED == m_uiState;
    }
    ///�жϴ������ӵ������Ƿ���
    bool isConnected() const 
    {
        return STATE_CONNECTED == m_uiState;
    }
    ///�жϴ��������Ƿ������ݷ���״̬
    bool isSyncing() const
    {
        return STATE_SYNCING == m_uiState;
    }
    ///�ж��Ƿ��п��е��첽��Ϣ���ʹ���
    bool isEnableSend() const 
    {
        return m_sendSidSet.size() < m_uiWindowSize;
    }
    ///��ȡ���ڵĴ�С
    CWX_UINT32 getWindowSize() const 
    {
        return m_uiWindowSize;
    }
    ///��ȡ���ڱ�ʹ�õĴ�С
    CWX_UINT32 getUsedSize() const 
    {
        return m_sendSidSet.size();
    }
    ///��ȡ����δ�ظ�����Ϣ��SID
    set<CWX_UINT64> const& getWaitingSid() const
    {
        return m_sendSidSet;
    }
    ///��ȡ��Ϣ���ն˵Ļ�������
    string const& getHostName() const
    {
        return m_strHostName;
    }
    ///������Ϣ���ն˵Ļ�������
    void setHostName(string const& name)
    {
        m_strHostName = name;
    }
private:
    CWX_UINT32     m_uiWindowSize;///<���ڴ�С
    void*           m_pHandle;///<���������첽���͵ľ��
    CWX_UINT32      m_uiSvrId;///<�������ӵ�SVR-ID
    CWX_UINT32      m_uiHostId;///<�������ӵ�HOST ID
    CWX_UINT32      m_uiConnId;///<�������ӵ�CONN ID
    CWX_UINT32      m_uiGroupId;///<�������ӵķ���ID
    CWX_UINT32      m_uiGroupNum;///<�������ӵķ�������
    CWX_UINT32      m_uiState;///<�������ӵ�״̬
    CWX_UINT64      m_ullStartSid;///<�������ӵ���ʼͬ��SID
    set<CWX_UINT64> m_sendSidSet;///<���ظ�������ͬ��SID
    string          m_strHostName;///<�Զ˵Ļ�����
};

///�������ӵ�hash����
typedef hash_map<CWX_UINT32/*conn_id*/, CwxAppAioWindow*, CwxNumHash<CWX_UINT32>, CwxNumHash<CWX_UINT32> > CwxAppAioWindowHash;

CWINUX_END_NAMESPACE

#endif
