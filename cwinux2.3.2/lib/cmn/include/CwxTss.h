#ifndef __CWX_TSS_H__
#define __CWX_TSS_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file CwxTss.h
@brief �߳�TSS����
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include <pthread.h>


CWINUX_BEGIN_NAMESPACE
/**
@class CwxTssInfo
@brief �̵߳Ļ�����Ϣ������Ҫ�������Ϣ������Ҫ����������󣬴˶���ֻӦ�����������ݳ�Ա��
*/
class CWX_API CwxTssInfo
{
public:
    ///���캯��
    CwxTssInfo()
    {
        m_unThreadGroup = 0;
        m_unThreadNo = 0;
        m_threadId = (pthread_t)0;
        m_bStopped = false;
        m_bBlocked = false;
        m_uiQueuedMsg = 0;
        m_ttStartTime = 0;
        m_ttUpdateTime = 0;
        m_llRecvMsgNum = 0;
    }
public:
    ///�����̵߳�group id
    inline void setThreadGroup(CWX_UINT16 unGroup)
    {
        m_unThreadGroup = unGroup;
    }
    ///��ȡ�̵߳�group id
    inline CWX_UINT16 getThreadGroup() const 
    {
        return m_unThreadGroup;
    }
    ///�����̵߳�thread id
    inline void setThreadNo(CWX_UINT16 unThreadNo)
    {
        m_unThreadNo = unThreadNo;
    }
    ///��ȡ�̵߳�thread id
    inline CWX_UINT16 getThreadNo() const 
    {
        return m_unThreadNo;
    }
    ///�����̵߳�OS thread id
    inline void setThreadId(pthread_t const& id)
    {
        m_threadId = id;
    }
    ///��ȡ�̵߳�OS thread id
    inline pthread_t const& getThreadId() const 
    {
        return m_threadId;
    }
    ///����߳��Ƿ�ֹͣ
    inline bool isStopped() const 
    {
        return m_bStopped;
    }
    ///�����߳�ֹͣ
    inline void setStopped(bool bStopped) 
    {
        m_bStopped = bStopped;
    }
    ///��ȡ�߳��Ƿ��ڵȴ���
    inline bool isBlocked() const 
    {
        return m_bBlocked;
    }
    ///�����߳��ڵȴ���
    inline void setBlocked(bool bBlocked) 
    {
        m_bBlocked = bBlocked;
    }
    ///��ȡ��������Ϣ����
    inline CWX_UINT32 getQueuedMsgNum() const
    {
        return m_uiQueuedMsg;
    }
    ///���ö�������Ϣ����
    inline void setQueuedMsgNum(CWX_UINT32 uiNum)
    {
        m_uiQueuedMsg = uiNum;
    }
    ///�����̵߳�����ʱ��
    inline void setStartTime(time_t ttTime)
    {
        m_ttStartTime = ttTime;
    }
    ///��ȡ�̵߳�����ʱ��
    inline time_t getStartTime() const
    {
        return m_ttStartTime;
    }
    ///�����̵߳�����ʱ��
    inline void setUpdateTime(time_t ttTime)
    {
        m_ttUpdateTime = ttTime;
    }
    ///��ȡ�̵߳����¸���ʱ��
    inline time_t getUpdateTime() const
    {
        return m_ttUpdateTime;
    }
    ///�����̴߳������Ϣ���������
    inline void setRecvMsgNum(CWX_UINT64 ullMsgNum)
    {
        m_llRecvMsgNum = ullMsgNum;
    }
    ///��ȡ�̴߳������Ϣ���������
    inline CWX_UINT64 getRecvMsgNum() const
    {
        return m_llRecvMsgNum;
    }
    ///�����̴߳������Ϣ���������
    inline void incRecvMsgNum()
    {
        m_llRecvMsgNum++;
    }
private:
    CWX_UINT16      m_unThreadGroup;///<�̵߳�group
    CWX_UINT16      m_unThreadNo;///<�̵߳�id
    pthread_t       m_threadId;///<�̵߳�OS id
    bool           m_bStopped;///<�߳��Ƿ�ֹͣ
    bool           m_bBlocked;///<�߳��Ƿ��ڵȴ���
    CWX_UINT32      m_uiQueuedMsg; ///<��������Ϣ����
    time_t          m_ttStartTime;///<�̵߳�����ʱ��
    time_t          m_ttUpdateTime;///<�̵߳����¸���ʱ��
    CWX_UINT64      m_llRecvMsgNum;///<�̴߳������Ϣ������
};

class CwxLogger;

/**
@class CwxTss
@brief �̵߳�TSS�����û�����������߳�����������һֱ��Ч�Ķ��󡣡�
*/
class CWX_API CwxTss
{
public:
    enum{
        TSS_2K_BUF = 2048,///<error-buffer�Ĵ�С
        TSS_ERR_BUF = 1024 * 16  ///<16K
    };
public:
    ///���캯��
    CwxTss()
    {
        m_fileNo = 0;
        m_fileName = NULL;
    }
    ///��������
    virtual ~CwxTss()
    {
    }
public:
    ///��ȡTss��thread info��Ϣ
    CwxTssInfo& getThreadInfo()
    {
        return m_threadInfo;
    }
public:
    ///��ʼ������ľ�̬����
    static int initTss();
    ///��pThrEnvע��Ϊ���ô�API��tss
    static int regTss(CwxTss* pThrEnv);
    ///unreg�̵߳�tss
    static int unRegTss();
    ///��ʵ����ģʽ����ȡ�����ʵ��
    static CwxTss* instance();
public:
    char                     m_szBuf2K[TSS_2K_BUF + 1];///������Ϣ����
    int                      m_fileNo;
    char const*              m_fileName;
private:
    friend class CwxLogger;
    char                     m_szErrMsg[TSS_ERR_BUF + 1];///������Ϣ����
    CwxTssInfo               m_threadInfo;///<��ǰTSS���߳�info
    static bool              m_bInit;///<�����Ƿ��Ѿ�ִ����init����
    static pthread_key_t     m_tssKey;///<�̵߳�tss����
};

#define  CWX_TSS(type) (type*)CwxTss::instance()
#define  CWX_TSS_2K_BUF  CwxTss::instance()->m_szBuf2K
#define  CWX_TSS_FILE_NO  CwxTss::instance()->m_fileNo
#define  CWX_TSS_FILE_NAME  CwxTss::instance()->m_fileName


CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
