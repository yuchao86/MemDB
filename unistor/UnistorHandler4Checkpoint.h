#ifndef __UNISTOR_HANDLER_4_CHECKPOINT_H__
#define __UNISTOR_HANDLER_4_CHECKPOINT_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"

///声明app对象
class UnistorApp;

///checkpoint的处理handle
class UnistorHandler4Checkpoint : public CwxCmdOp{
public:
    ///构造函数
    UnistorHandler4Checkpoint(UnistorApp* pApp):m_pApp(pApp){
		m_bCheckOuting = false;
		m_uiLastCheckTime = time(NULL);
    }
    ///析构函数
    virtual ~UnistorHandler4Checkpoint(){
    }
public:
    /**
    @brief 超时检查事件的处理函数。
    @param [in] msg 超时检查的事件对象。
    @param [in] pThrEnv 线程的TSS对象。
    @return -1：处理失败，0：不处理此事件，1：处理此事件。
    */
    virtual int onTimeoutCheck(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    /**
    @brief 用户自定义事件的处理函数。
    @param [in] msg 用户自定义事件的事件对象。
    @param [in] pThrEnv 线程的TSS对象。
    @return -1：处理失败，0：不处理此事件，1：处理此事件。
    */
    virtual int onUserEvent(CwxMsgBlock*& msg, CwxTss* pThrEnv);
    ///检测是否需要checkpoint
	bool isNeedCheckOut(time_t now) const;
private:
	bool				      m_bCheckOuting; ///<是否正在checkpoint
	CWX_UINT32				  m_uiLastCheckTime; ///<上次checkpoint的时间
    UnistorApp*               m_pApp;  ///<app对象
};

#endif 
