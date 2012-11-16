#ifndef __CWX_APP_HANDLER_4_BASE_H__
#define __CWX_APP_HANDLER_4_BASE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppHandler4Base.h
@brief Event handler�Ļ���
@author cwinux@gmail.com
@version 0.1
@date 2010-08-01
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxMsgBlock.h"
#include "CwxAppMacro.h"
#include "CwxAppConfig.h"

CWINUX_BEGIN_NAMESPACE

class CwxAppReactor;
class CwxAppEpoll;
class CWX_API CwxAppHandler4Base{
public:
    ///�����¼�����
    enum{
        TIMEOUT_MASK = 0x01,
        READ_MASK = 0x02,
        WRITE_MASK = 0x04,
        SIGNAL_MASK = 0x08,
        PERSIST_MASK = 0x10,
        PREAD_MASK = READ_MASK|PERSIST_MASK,
        RW_MASK = READ_MASK|WRITE_MASK,
        IO_MASK = RW_MASK|TIMEOUT_MASK|PERSIST_MASK,
        ALL_EVENTS_MASK =  TIMEOUT_MASK|
                          READ_MASK|
                          WRITE_MASK|
                          SIGNAL_MASK|
                          PERSIST_MASK
    };
public :
    ///��������.
    virtual ~CwxAppHandler4Base (void);
    /**
    @brief handler open����handler�ĳ�ʼ����reactor��ע�������
    @param [in] arg  Handler�ĳ�ʼ�����������������Լ�����
    @return -1��ʧ�ܣ� 0���ɹ�
    */
    virtual int open (void * arg= 0)=0;
    /**
    @brief Handler���¼�֪ͨ�ص���
    @param [in] event  �������¼����ͣ�ΪTIMEOUT_MASK��READ_MASK��WRITE_MASK��<br>
            SIGNAL_MASK����ϡ�
    @param [in] handle  �������¼���handle��
    @return -1��ʧ�ܣ�reactor����������close��
            0���ɹ���
    */
    virtual int handle_event(int event, CWX_HANDLE handle=CWX_INVALID_HANDLE)=0;
    
    ///handle close
    virtual int close(CWX_HANDLE handle=CWX_INVALID_HANDLE)=0;
public:
    ///����handle��reactor
    void reactor (CwxAppReactor *reactor);
    ///��ȡhandle��reactor.
    CwxAppReactor *reactor(void);
    ///��ȡIo handle
    CWX_HANDLE getHandle(void) const;
    ///����IO handle
    void setHandle (CWX_HANDLE);
    ///����handle type
    void setType(int type);
    ///��ȡhandle type
    int getType() const;
    ///��ȡע������
    int getRegType() const;
    ///����ע������
    void setRegType(int type);
    ///���ó�ʱʱ��
    void setTimeout(CWX_UINT64 ullTimeout);
    ///��ȡ��ʱʱ��
    CWX_UINT64 getTimeout() const;
    ///��ȡheap�е�index
    int index() const;
    ///����heap�е�index
    void index(int index);
    ///��ʱ�ȽϺ���
    bool operator>(CwxAppHandler4Base const& base) const;
protected:
    /// Force CwxAppHandler4Base to be an abstract base class.
    CwxAppHandler4Base (CwxAppReactor *reactor);
    friend class CwxAppReactor;
    friend class CwxAppEpoll;
private:
    CwxAppReactor *        m_reactor; ///<reactor�����ָ��
    int                    m_regType; ///<handler��reactorע������
    CWX_HANDLE             m_handler; ///<�¼���io handle
    int                    m_type; ///<event handle type;
    CWX_UINT64             m_ullTimeout; ///<��ʱ��ʱ��
    int                    m_index;
};

CWINUX_END_NAMESPACE

#include "CwxAppHandler4Base.inl"
#include "CwxPost.h"
#endif
