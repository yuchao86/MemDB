#ifndef  __CWX_MSG_QUEUE_H__
#define  __CWX_MSG_QUEUE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxMsgQueue.h
*@brief CwxMsgQueue����
*@author cwinux@gmail.com
*@version 1.0
*@date  2010-07-05
*@warning  nothing
*@bug    
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxCommon.h"
#include "CwxCondition.h"
#include "CwxTimeValue.h"
#include "CwxMsgBlock.h"

CWINUX_BEGIN_NAMESPACE

/**
*@class CwxMsgQueue
*@brief �����ߡ������߶��С�
*/
class CWX_API CwxMsgQueue
{
public:
    enum
    {
        ///ȱʡ�Ķ��пռ�����ޣ���������push��Ϣʱ����.
        DEFAULT_HWM = 1024 * 1024,
        ///ȱʡ�Ķ��пռ�����ޣ��������޺�ֻ�е������޲���push��Ϣ.
        DEFAULT_LWM = 1024 * 1024,
        ///�״̬
        ACTIVATED = 1,
        ///deactivated״̬.
        DEACTIVATED = 2
    };
public:
    CwxMsgQueue(size_t hwm=DEFAULT_HWM, size_t lwm=DEFAULT_LWM);
    ~CwxMsgQueue();
public:
    ///�رն��У�ͬʱ�ͷ�������Ϣ��-1��ʧ�ܣ�>=0�������е���Ϣ����
    int close();
    ///ֻ�ͷŶ����е�������Ϣ��-1��ʧ�ܣ�>=0�������е���Ϣ����
    int flush (void);
    ///��һ����Ϣ�ŵ����е�ͷ����-1��ʧ�ܣ�>=0�������е���Ϣ����
    int enqueue (CwxMsgBlock *msg,
        CwxTimeValue *timeout = 0);
    ///��һ����Ϣ�ŵ����е�β����-1��ʧ�ܣ�>=0�������е���Ϣ����
    int enqueue_tail (CwxMsgBlock * msg,
        CwxTimeValue *timeout = 0);
    ///�Ӷ��е�ͷ����ȡһ����Ϣ��-1��ʧ�ܣ�>=0�������е���Ϣ����
    int dequeue (CwxMsgBlock *&msg,
        CwxTimeValue *timeout = 0);
    ///�������Ƿ�full
    bool isFull (void);
    ///�������Ƿ��.
    bool isEmpty(void);
    ///��ȡ������Ϣ�Ŀռ��С.
    size_t getMsgBytes(void);
    ///��ȡ����Msg��length.
    size_t getMsgLength(void);
    ///��ȡ��Ϣ������.
    size_t getMsgCount(void);
    ///deactive ��Ϣ���У�-1��ʧ�ܣ����򷵻���ǰ��״̬
    int deactivate (void);
    ///active��Ϣ���У�-1��ʧ�ܣ����򷵻���ǰ��״̬
    int activate (void);
    ///��ȡ��Ϣ���е�״̬.
    int getState (void);
    ///�������Ƿ�deactive״̬
    bool isDeactived(void);
    ///�������Ƿ�active״̬
    bool isActivate(void);
    ///��ȡhigh water mark
    size_t getHwm(void) const;
    ///����high water mark
    void setHwm(size_t hwm);
    ///��ȡlow water mark
    size_t getLwm(void) const;
    ///����low water mark
    void setLwm(size_t lwm);
    /// ��ȡ��.
    CwxMutexLock& lock();

private:
    ///deactive ��Ϣ���У�-1��ʧ�ܣ����򷵻���ǰ��״̬
    int _deactivate(void);
    ///active��Ϣ���У�-1��ʧ�ܣ����򷵻���ǰ��״̬
    int _activate(void);
    ///ֻ�ͷŶ����е�������Ϣ��-1��ʧ�ܣ�>=0�������е���Ϣ����
    int _flush(void);
    ///�������Ƿ�full
    bool _isFull (void);
    ///�������Ƿ��.
    bool _isEmpty(void);
    ///�ȴ������п�λ��
    int _waitNotFullCond(CwxTimeValue *timeout = 0);
    ///�ȴ�������Ϣ
    int _waitNotEmptyCond(CwxTimeValue *timeout = 0);

    // = Disallow copying and assignment.
    CwxMsgQueue (const CwxMsgQueue &);
    CwxMsgQueue& operator= (const CwxMsgQueue &);

protected:
    /// ���е�״̬.
    int m_state;
    /// ��Ϣ���������ͷ.
    CwxMsgBlock* m_head;
    /// ��Ϣ���������β.
    CwxMsgBlock* m_tail;
    /// low water mark.
    size_t m_lowWaterMark;
    /// High water mask.
    size_t m_highWaterMark;
    /// ��ǰ�����е�msg block�Ŀռ��ֽ��ܺ�.
    size_t m_curMsgBytes;
    /// ��ǰ�����е�msg�ĳ����ܺ�.
    size_t m_curLength;
    /// ������msg������.
    size_t m_curCount;
    /// ͬ����������.
    CwxMutexLock m_lock;
    /// Used to make threads sleep until the queue is no longer empty.
    CwxCondition m_notEmptyCond;
    /// Used to make threads sleep until the queue is no longer full.
    CwxCondition m_notFullCond;
};


CWINUX_END_NAMESPACE

#include "CwxMsgQueue.inl"

#include "CwxPost.h"

#endif
