#ifndef __CWX_MSG_BLOCK_H__
#define __CWX_MSG_BLOCK_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxMsgBlock.h
@brief ͨ�������շ�������Block������
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxMsgHead.h"
#include "CwxSingleton.h"

CWINUX_BEGIN_NAMESPACE

class CwxMsgBlockAlloc;
class CwxMsgBlock;
class CwxMsgQueue;

/**
@class CwxMsgSendCtrl
@brief ͨ�����ݷ��͵���Ϊ���ƶ���
*/
class CWX_API CwxMsgSendCtrl
{
public:
    enum{
        UNDO_CONN = 0,
        RESUME_CONN = 1,
        SUSPEND_CONN = 2
    };
    enum{
        FAIL_NOTICE = (1<<0),///<����ʧ��֪ͨ��־
        BEGIN_NOTICE = (1<<1),///<��ʼ����֪ͨ���
        FINISH_NOTICE = (1<<2),///<���ͽ���֪ͨ���
        CLOSE_NOTICE = (1<<3), ///<������Ϻ󣬹ر�����
        RECONN_NOTICE= (1<<4), ///<�������ӣ����ڱ������������
        FAIL_FINISH_NOTICE = FAIL_NOTICE|FINISH_NOTICE,///<��Ϣ���������ʧ�ܵ�ʱ��֪ͨ
        ALL_NOTICE = FAIL_FINISH_NOTICE|BEGIN_NOTICE,///<����ȫ����ǣ����˹رա���������
        NONE = 0///<û���κεı��
    };
public:
    ///����Ƿ���Ϣ����ǰ֪ͨ
    inline bool isBeginNotice() const;
    ///����Ƿ���Ϣ���֪ͨ
    inline bool isFinishNotice() const;
    ///����Ƿ���Ϣʧ��֪ͨ
    inline bool isFailNotice() const;
public:
    ///���û������ӵķ��Ϳ���
    inline void setConnCtrl(CWX_UINT32 uiSvrId,
        CWX_UINT32 uiHostId,
        CWX_UINT32 uiConnId,
        CWX_UINT32 uiMsgAttr=FAIL_FINISH_NOTICE,
        void* userData=NULL,
        CWX_UINT32 uiConnState=UNDO_CONN);
    ///��ȡSVR ID
    inline CWX_UINT32 getSvrId() const;
    ///����SVR ID
    inline void setSvrId(CWX_UINT32 uiSvrId);
    ///��ȡHOST ID
    inline CWX_UINT32 getHostId() const;
    ///����HOST ID
    inline void setHostId(CWX_UINT32 uiHostId);
    ///��ȡCONN ID
    inline CWX_UINT32 getConnId() const ;
    ///����CONN ID
    inline void setConnId(CWX_UINT32 uiConnId) ;
    ///��ȡSVR��ȱʡHost id
    inline CWX_UINT32 getDefHostId() const;
    ///���÷���SVR��ȱʡHOST ID
    inline void setDefHostId(CWX_UINT32 uiDefHostId);
    ///��ȡ���͵�����
    inline CWX_UINT32 getMsgAttr() const ;
    ///���÷��͵�����
    inline void setMsgAttr(CWX_UINT32 uiAttr) ;
    ///��ȡ���͵�user data
    inline void* getUserData() ;
    ///���÷��͵�user data
    inline void setUserData(void* userData) ;
    ///��ȡ����resume״̬
    inline CWX_UINT32 getConnResumeState() const ;
    ///����Ƿ�resume����
    inline bool isResumeConn() const ;
    ///����Ƿ�suspend����
    inline bool isSuspendConn() const ;
    ///����Ƿ�ִ��undo����
    inline bool isUndoConn() const ;
    ///��������resume״̬
    inline void setConnResumeState(CWX_UINT32 uiState) ;
    ///������������ʱ����λΪms
    inline void setReconnDelay(CWX_UINT32 uiMiliSecond);
    ///��ȡ��������ʱ����λΪms
    inline CWX_UINT32 getReconnDelay() const;
public:
    ///��տ��ƶ���
    inline void reset();
private:
    ///��ֹ��������ʵ����ֻ����CwxMsgBlock���󴴽�
    CwxMsgSendCtrl();
    ~CwxMsgSendCtrl();
    friend class CwxMsgBlock;
private:
    void*                  m_userData; ///<userdata
    CWX_UINT32             m_uiSvrId; ///<service id
    CWX_UINT32             m_uiHostId; ///<host id
    CWX_UINT32             m_uiConnId;///<connection id
    CWX_UINT32             m_uiDefHostId; ///<ȱʡhost id
    CWX_UINT32             m_uiMsgAttr; ///<msg's attribute for send
    CWX_UINT32             m_uiConnResumeState; ///<���ӵ�suspend��resume״̬
    CWX_UINT32             m_uiReconnDelay; ///<�������ӵ���ʱ����λΪ����
};


/**
@class CwxEventInfo
@brief �ܹ����¼�����󣬶���ϵͳ�ĸ����¼����͡���Ϊcommander��Ϣ�ַ��Ļ���
*/
class CWX_API CwxEventInfo
{
public:
    ///�¼����Ͷ���
    enum {
        DUMMY = 0,///<dummy�¼�����Ϊ��Ч�¼�����
        CONN_CREATED = 1,///<���ӽ����¼�
        CONN_CLOSED = 2,///<���ӹر��¼�
        RECV_MSG = 3,///<�յ�ͨ�������¼�
        END_SEND_MSG = 4,///<ͨ�����ݰ���������¼�
        FAIL_SEND_MSG = 5,///<ͨ�����ݰ�����ʧ���¼�
        TIMEOUT_CHECK = 6,///<��ʱ����¼�
        EVENT_4_HANDLE = 7,///<IO��ע����¼��������¼�
        SYS_EVENT_NUM = 8,///<ϵͳ�¼�������
    };
public:
    ///��ȡ�¼�����
    inline CWX_UINT16  getEvent() const ;
    ///�����¼�����
    inline void setEvent(CWX_UINT16 unEvent) ;
    ///��ȡ�¼���Ӧ��SVR-ID
    inline CWX_UINT32  getSvrId() const ;
    ///�����¼���Ӧ��SVR-ID
    inline void setSvrId(CWX_UINT32 uiSvrId) ;
    ///��ȡ�¼���Ӧ��HOST-ID
    inline CWX_UINT32 getHostId() const ;
    ///�����¼���Ӧ��HOST-ID
    inline void setHostId(CWX_UINT32 uiHostId) ;
    ///��ȡ�¼���Ӧ������ID
    inline CWX_UINT32 getConnId() const;
    ///�����¼���Ӧ������ID
    inline void setConnId(CWX_UINT32 uiConnId) ;
    ///��ȡ�¼���Ӧ�����ӵ��û�����
    inline void* getConnUserData() ;
    ///�����¼���Ӧ�����ӵ��û�����
    inline void setConnUserData(void* userData) ;
    ///��ȡ�¼���Ӧ��TASK ID
    inline CWX_UINT32 getTaskId() const ;
    ///�����¼���Ӧ��TASK ID
    inline void setTaskId(CWX_UINT32 uiTaskId) ;
    ///��ȡ�¼���Ӧ��ͨ�����ݰ��İ�ͷ
    inline CwxMsgHead& getMsgHeader();
    ///�����¼���Ӧ��ͨ�����ݰ��İ�ͷ
    inline void setMsgHeader(CwxMsgHead const& header) ;
    ///��ȡ�¼���Ӧ���ӵ�����handle
    inline CWX_HANDLE getIoHandle() ;
    ///�����¼���Ӧ���ӵ�����handle
    inline void setIoHandle(CWX_HANDLE handle) ;
    ///��ȡ�¼���Ӧ��IO�¼���������
    inline CWX_UINT16 getRegEventMask() const ;
    ///�����¼���Ӧ��IO�¼���������
    inline void setRegEventMask(CWX_UINT16 unMask) ;
    ///��ȡ�¼���Ӧ��IO�¼�����IO�¼�
    inline CWX_UINT16 getReadyEvent() const ;
    ///�����¼���Ӧ��IO�¼�����IO�¼�
    inline void setReadyEvent(CWX_UINT16 unEvent) ;
    ///��ȡ�¼���ʱ���
    inline CWX_UINT64 getTimestamp() const ;
    ///�����¼���ʱ���
    inline void setTimestamp(CWX_UINT64 ullTimestamp) ;
    ///����¼�����
    inline void reset();
private:
    ///��ֹ����������ΪCwxMsgBlock����ĸ�������
    CwxEventInfo();
    ///��������
    ~CwxEventInfo();
    ///��ֹ����ֵ����
    CwxEventInfo& operator=(CwxEventInfo const& );
    friend class CwxMsgBlock;
public:
    CWX_UINT32     m_uiArg;///<�û���UINT32���͵Ĳ���
    CWX_UINT64     m_ullArg;///<�û���UINT64���͵Ĳ���
    void*          m_pArg;///<�û���ָ�����͵Ĳ���
private:
    CWX_UINT16     m_unEvent;///<�¼�����
    CWX_UINT32     m_uiSvrId;///<�¼���Ӧ��SVR ID
    CWX_UINT32     m_uiHostId;///<�¼���Ӧ��HOST ID
    CWX_UINT32     m_uiConnId;///<�¼���Ӧ������ID
    void*          m_connUserData;///<�����ϵ�userData
    CWX_UINT32     m_uiTaskId;///<�¼���ӦTASK��Task-ID
    CwxMsgHead   m_msgHeader;///<�¼���Ӧͨ�����ݰ��İ�ͷ
    CWX_HANDLE     m_ioHandle;///<�¼���IO HANDLE
    CWX_UINT16     m_unRegEventMask;///<�¼���IO �¼�������
    CWX_UINT16     m_unReadyHandleEvent;///<�¼���IO �¼����¼�
    CWX_UINT64     m_ullTimeStamp;///<�¼���ʱ���
};

/**
@class CwxMsgBlock
@brief ͨ�������շ�������Block���󡣶�������������ɣ�
��һ���֣����ջ��͵�����ͨ�����ݰ�����Ϊ����block��
�ڶ����֣��������ݵ�Event�������������յ����ݰ���������Ϣ����Ϣ������Ϣ����Command����Event�¼��ַ���
�������֣��������ݵ�MsgInfo��������������Ϣ���͵�svr-id��host-id��conn-id������������Ϊ�Ŀ�����Ϣ��
���⣬Ϊ�˷�ֹ�ڴ���Ƭ��CwxMsgBlockֻ����CwxMsgBlockAlloc������䡢�ͷš�
*/
class CWX_API CwxMsgBlock
{
public:
    ///��ȡBlock�ڴ����ݿ�ĵ�ǰ��λ��
    inline char *rd_ptr (void) const;
    ///��Block�ڴ����ݿ�Ķ�λ��ǰ��n���ֽ�
    inline void rd_ptr (size_t n) ;
    ///��ȡBlock�ڴ����ݿ�ĵ�ǰдλ��
    inline char *wr_ptr (void) const ;
    ///��Block�ڴ����ݿ��дλ��ǰ��n���ֽ�
    inline void wr_ptr(size_t n) ;
    ///��ȡBlock���ݿ������ݵĴ�С��Ϊдָ�����ָ����ڴ�λ�ò
    inline size_t length(void) const ;
    ///��ȡBlock���ݿ�Ŀռ��С
    inline size_t capacity(void) const ;
    ///��Block�Ķ�дָ���Ƶ�Block�Ŀ�ͷλ��
    inline void reset() ;
    ///��ȡ��Block�������Event��Ϣ���û������������е����ݡ��ڷ�����Ϣ��ʱ�򣬲����޸Ĵ����ݡ�
    inline CwxEventInfo& event() ;
    ///��ȡ��Block��صķ���Msginfo��Ϣ���������мܹ������û�ֻ������Ϣ����ʧ�ܻ���ɵ�ʱ�򣬲鿴����Ϣ
    inline CwxMsgSendCtrl& send_ctrl();
public:
    CwxMsgBlock*          m_next;///<��һ��Block����
private:
    friend class CwxMsgBlockAlloc;
    friend class CwxMsgQueue;
    ///������������ֹdelete
    ~CwxMsgBlock();
    ///ָ�����ݿ��С�Ĺ��캯������ֹnew
    CwxMsgBlock(size_t size);
    ///������Block���ݿ�Ĺ��캯��
    CwxMsgBlock();
private:
    char*                   m_buf; ///<�ڴ��buf
    size_t                  m_rdPos; ///<�ڴ�Ķ�λ��
    size_t                  m_wrPos; ///<�ڴ��дλ��
    size_t                  m_len;   ///<�ڴ�Ĵ�С
    CwxEventInfo          m_event;///<���յ����ݰ���ص����Ӽ�������Ϣ����
    CwxMsgSendCtrl        m_sendCtrl;///<���ݰ�����ʱ�����Ӽ����Ϳ��ƶ���
    bool                    m_bFree; ///<�Ƿ�CwxMsgBlockAlloc�ͷ�

};

/**
@class CwxMsgBlockAlloc
@brief CwxMsgBlock�ķ��䡢�ͷż�����block��cache����
���������ݿ��ʱ����block��СС��1M����CwxMsgBlockAlloc��
256��512��....1M����ɢ��С���з��䣻������1M���������ڴ水��4K�ı߽���롣
���ͷ����ݿ��ʱ����Block��СС��1M����Cache��blockû�г���ָ�������ߣ�����256��512��....1M��
cache�ͷŵĶ����Թ��ٴ�ѭ��ʹ�á�������1M����ֱ���Ƿ�
*/
class CWX_API CwxMsgBlockAlloc:public CwxSingleton
{
public:
    ///BLOCK ���䡢cache���Ʋ�������
    enum{
        CACHE_MIN_BLOCK_BITS = 8,///<��С��block�Ĵ�С��Ϊ2^CACHE_MIN_BLOCK_BITS 
        CACHE_MIN_BLOCK_SIZE = 1<<CACHE_MIN_BLOCK_BITS,///<��С��block�Ĵ�С
        CACHE_MAX_BLOCK_BITS = 20,///<CACHE������block�Ĵ�С��Ϊ2^CACHE_MIN_BLOCK_BITS
        CACHE_MAX_BLOCK_SIZE = 1<<CACHE_MAX_BLOCK_BITS,///<CACHE������block�Ĵ�С
        CACHE_BLOCK_SIZE_SCALE = CACHE_MAX_BLOCK_BITS - CACHE_MIN_BLOCK_BITS + 1, ///<cache��block��С��ͬ������
        MAX_SCALE_FREE_MEM_MSIZE = 4,///<ÿ�ִ�С���͵�block�����cache�����ڴ���
        MAX_SCALE_FREE_BLOCK_NUM = 1024, ///<ÿ�ִ�С���͵�block�����cache�ĸ���
        BLOCK_ALIGN_BIT = 12 ///<��С����1M��block����4K���б߽���롣
    };
public:
    ///����signleton CwxEscapes ����
    static CwxMsgBlockAlloc* instance();
    ///<�����СΪsize��block
    static CwxMsgBlock* malloc(size_t size);
    ///<��¡msg��copy����
    static CwxMsgBlock* clone(CwxMsgBlock* block);
    ///<�ͷ�block
    static void free(CwxMsgBlock* block);
    static CwxMsgBlock* pack(CwxMsgHead& header,
        char const* szData,
        CWX_UINT32 uiDateLen);
    ///�ͷſռ�
    static void destroy();
private:
    ///��ֹ����ʵ��
    CwxMsgBlockAlloc();
    ///��ֹ�ͷ�ʵ��
    ~CwxMsgBlockAlloc();
private:
    static CwxMsgBlockAlloc*   m_pInstance;
    static CwxMutexLock    m_lock;///<thread lock for sync.
    CWX_UINT16       m_arrCacheNum[CACHE_BLOCK_SIZE_SCALE];///<cache�ĸ��ֳߴ��С��block������
    CwxMsgBlock*   m_arrCacheBlock[CACHE_BLOCK_SIZE_SCALE];///<cache�ĸ��ֳߴ��С��block������
};
CWINUX_END_NAMESPACE

#include "CwxMsgBlock.inl"
#include "CwxPost.h"
#endif
