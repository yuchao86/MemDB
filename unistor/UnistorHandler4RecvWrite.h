#ifndef __UNISTOR_HANDLER_4_RECV_WRITE_H__
#define __UNISTOR_HANDLER_4_RECV_WRITE_H__

#include "CwxCommander.h"
#include "UnistorMacro.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxMsgBlock.h"
#include "UnistorTss.h"
#include "UnistorPoco.h"

///前置声明对象
class UnistorApp;

///KV消息处理handle
class UnistorHandler4RecvWrite : public CwxCmdOp{
public:
	///构造函数
	UnistorHandler4RecvWrite(UnistorApp* pApp):m_pApp(pApp){
        m_bCanWrite = false;
	}
	///析构函数
    virtual ~UnistorHandler4RecvWrite(){}
public:
    /**
    @brief 收到通信数据包事件的处理函数。
    @param [in] msg 收到通信数据包的事件对象。
    @param [in] pThrEnv 线程的TSS对象。
    @return -1：处理失败，0：不处理此事件，1：处理此事件。
    */
    virtual int onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv);

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
public:
    ///是否master可写，也就是否是master
    inline bool isCanWrite() const{
        return m_bCanWrite;
    }

private:
	///添加一个key。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
	int addKey(UnistorTss* pTss, ///<线程tss
        UnistorWriteMsgArg* pWriteArg, ///<add的参数
        CWX_UINT32& uiVersion, ///<返回key的新版本
        CWX_UINT32& uiFieldNum ///<返回key的field数量
        );

	///set一个key。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    int setKey(UnistorTss* pTss, ///<线程tss
        UnistorWriteMsgArg* pWriteArg, ///<add的参数
        CWX_UINT32& uiVersion, ///<返回key的新版本
        CWX_UINT32& uiFieldNum ///<返回key的field数量
        );

	///update一个key。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    int updateKey(UnistorTss* pTss, ///<线程tss
        UnistorWriteMsgArg* pWriteArg, ///<add的参数
        CWX_UINT32& uiVersion, ///<返回key的新版本
        CWX_UINT32& uiFieldNum ///<返回key的field数量
        );

	///inc一个key的计数器。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    int incKey(UnistorTss* pTss, ///<线程tss
        UnistorWriteMsgArg* pWriteArg, ///<add的参数
        CWX_INT64& llValue, ///<返回计数器的新值
        CWX_UINT32& uiVersion ///<返回key的field数量
        );

	///delete一个key。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    int delKey(UnistorTss* pTss, ///<线程tss
        UnistorWriteMsgArg* pWriteArg, ///<add的参数
        CWX_UINT32& uiVersion, ///<返回key的新版本
        CWX_UINT32& uiFieldNum ///<返回key的field数量
        );

    ///import一个key。返回值：UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    int importKey(UnistorTss* pTss, ///<线程tss
        UnistorWriteMsgArg* pWriteArg, ///<add的参数
        CWX_UINT32& uiVersion, ///<返回key的新版本
        CWX_UINT32& uiFieldNum ///<返回key的field数量
        );

    ///master变化处理函数
    void configChange(UnistorTss* pTss);
private:
	UnistorApp*               m_pApp;  ///<app对象
    volatile bool             m_bCanWrite; ///<是否master idc而且是master
};
#endif 
