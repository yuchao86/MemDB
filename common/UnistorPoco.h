#ifndef __UNISTOR_POCO_H__
#define __UNISTOR_POCO_H__

#include "UnistorMacro.h"
#include "CwxMsgBlock.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxCrc32.h"
#include "CwxMd5.h"


//unistor的协议定义对象
class UnistorPoco
{
public:
    enum{///<消息包头的属性位
        MSG_HEAD_ATTR_MASTER = 0x01 ///<从master获取消息
    };
    
    enum ///<消息类型定义
    {
        ///RECV服务类型的消息类型定义
        MSG_TYPE_TIMESTAMP = 0, ///<时钟消息类型
        MSG_TYPE_RECV_ADD = 1, ///<ADD key/value
        MSG_TYPE_RECV_ADD_REPLY = 2, ///<set key/value的回复
        MSG_TYPE_RECV_SET = 3, ///<SET key
        MSG_TYPE_RECV_SET_REPLY = 4, ///<SET key的回复
		MSG_TYPE_RECV_UPDATE = 5, ///update key
		MSG_TYPE_RECV_UPDATE_REPLY = 6, ///<update key的回复
		MSG_TYPE_RECV_INC = 7, ///<inc key
		MSG_TYPE_RECV_INC_REPLY = 8, ///<inc key的回复
		MSG_TYPE_RECV_DEL = 9, ///<del key
		MSG_TYPE_RECV_DEL_REPLY = 10, ///<del key的回复
        MSG_TYPE_RECV_EXIST = 11, ///<Key是否存在
        MSG_TYPE_RECV_EXIST_REPLY = 12, ///<Key是否存在的回复
		MSG_TYPE_RECV_GET = 13, ///<get key
		MSG_TYPE_RECV_GET_REPLY = 14, ///<get key的回复
		MSG_TYPE_RECV_GETS = 15, ///<get 多个key
		MSG_TYPE_RECV_GETS_REPLY = 16, ///<get 多个key的回复
		MSG_TYPE_RECV_LIST = 17, ///<获取列表
		MSG_TYPE_RECV_LIST_REPLY = 18, ///<列表回复
        MSG_TYPE_RECV_IMPORT = 19, ///<获取列表
        MSG_TYPE_RECV_IMPORT_REPLY = 20, ///<列表回复
        MSG_TYPE_RECV_AUTH = 21, ///<认证
        MSG_TYPE_RECV_AUTH_REPLY = 22, ///<认证回复
        ///数据导出数据类型
        MSG_TYPE_EXPORT_REPORT = 51, ///<数据导出export
        MSG_TYPE_EXPORT_REPORT_REPLY = 52, ///<数据导出的reply
        MSG_TYPE_EXPORT_DATA = 53, ///<数据导出的数据
        MSG_TYPE_EXPORT_DATA_REPLY = 54, ///<数据导出的数据reply
        MSG_TYPE_EXPORT_END = 55, ///<数据导出完成
        ///分发的消息类型定义
        MSG_TYPE_SYNC_REPORT = 101, ///<同步SID点报告消息类型
        MSG_TYPE_SYNC_REPORT_REPLY = 102, ///<report返回
        MSG_TYPE_SYNC_CONN = 103, ///<连接通报
        MSG_TYPE_SYNC_CONN_REPLY = 104, ///<连接通报回复
        MSG_TYPE_SYNC_DATA = 105,  ///<发送数据
        MSG_TYPE_SYNC_DATA_REPLY = 106, ///<数据的回复
        MSG_TYPE_SYNC_DATA_CHUNK = 107,  ///<chunk模式发送数据
        MSG_TYPE_SYNC_DATA_CHUNK_REPLY = 108, ///<chunk模式发送数据的回复
        MSG_TYPE_SYNC_ERR = 109 ///<sync的错误消息
    };
    enum
    {
        MAX_CONTINUE_SEEK_NUM = 8192
    };
public:
    ///设置从master获取的属性位
    inline static CWX_UINT8 setFromMaster(CWX_UINT8& ucAttr){
        CWX_SET_ATTR(ucAttr, MSG_HEAD_ATTR_MASTER);
        return ucAttr;
    }
    ///check是否设置了从master获取的属性位
    inline static bool isFromMaster(CWX_UINT8 ucAttr){
        return CWX_CHECK_ATTR(ucAttr, MSG_HEAD_ATTR_MASTER);
    }
    ///清除从master获取的属性位
    inline static CWX_UINT8 clearFromMaster(CWX_UINT8& ucAttr){
        return CWX_CLR_ATTR(ucAttr, MSG_HEAD_ATTR_MASTER);
    }
    ///pack Add key的数据。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvImport(CwxPackageWriterEx* writer, ///<用于pack的writer，内容通过writer返回
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<超时时间，若为0则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        bool       bCache=true, ///<是否cache，若为true则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
        );

    ///pack Add key的消息包。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvImport(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<超时时间，若为0则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        bool       bCache=true, ///<是否cache，若为true则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
        );

    ///解析Add key的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvImport(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<返回key字段
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        CwxKeyValueItemEx const*& data,  ///<返回data字段
        CWX_UINT32& uiExpire,  ///<返回expire，若为0表示没有指定
        CWX_UINT32& uiVersion, ///<返回版本
        bool&       bCache,    ///<返回cache
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
        char* szErr2K=NULL     ///<解包时的错误信息
        );



    ///pack Add key的数据。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvAdd(CwxPackageWriterEx* writer, ///<用于pack的writer，内容通过writer返回
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<超时时间，若为0则不添加
        CWX_UINT32 uiSign=0,    ///<标记，若为0则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        bool       bCache=true, ///<是否cache，若为true则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
        );

	///pack Add key的消息包。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
	static int packRecvAdd(CwxPackageWriterEx* writer, ///<用于pack的writer
		CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
		CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<超时时间，若为0则不添加
        CWX_UINT32 uiSign=0,    ///<标记，若为0则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        bool       bCache=true, ///<是否cache，若为true则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
		);

	///解析Add key的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
	static int parseRecvAdd(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
		CwxKeyValueItemEx const*& data,  ///<返回data字段
		CWX_UINT32& uiExpire,  ///<返回expire，若为0表示没有指定
        CWX_UINT32& uiSign,    ///<返回sign
        CWX_UINT32& uiVersion, ///<返回版本
        bool&       bCache,    ///<返回cache
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
		char* szErr2K=NULL     ///<解包时的错误信息
        );

    ///pack set的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvSet(CwxPackageWriterEx* writer,///<用于pack的writer，内容通过writer返回
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<标记，若为0则不添加
        CWX_UINT32 uiExpire=0, ///<超时时间，若为0则不添加
        CWX_UINT32 uiVersion=0,///<版本，若为0则不添加
        bool   bCache=true, ///<是否cache，若为true则不添加
        char const* user=NULL, ///<用户，若为NULL则不添加
        char const* passwd=NULL,///<用户口令，若为NULL则不添加
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///pack set的消息体。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvSet(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<标记，若为0则不添加
        CWX_UINT32 uiExpire=0, ///<超时时间，若为0则不添加
        CWX_UINT32 uiVersion=0,///<版本，若为0则不添加
        bool   bCache=true, ///<是否cache，若为true则不添加
        char const* user=NULL, ///<用户，若为NULL则不添加
        char const* passwd=NULL,///<用户口令，若为NULL则不添加
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse set的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvSet(CwxPackageReaderEx* reader,  ///<reader
        CwxKeyValueItemEx const*& key, ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        CwxKeyValueItemEx const*& data, ///<返回data字段
        CWX_UINT32& uiSign, ///<返回sign
		CWX_UINT32& uiExpire, ///<返回expire
        CWX_UINT32& uiVersion, ///<返回版本
        bool&   bCache,  ///<返回cache
        char const*& user, ///<返回用户，NULL表示不存在
        char const*& passwd, ///<返回口令，NULL表示不存在
        char* szErr2K=NULL  ///<解包时的错误信息
        );

    ///pack update的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvUpdate(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<标记，若为0则不添加
        CWX_UINT32 uiExpire=0, ///<超时时间，若为0则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        char const* user=NULL, ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

	///pack update的消息包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
	static int packRecvUpdate(CwxPackageWriterEx* writer, ///<用于pack的writer
		CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
		CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<标记，若为0则不添加
        CWX_UINT32 uiExpire=0, ///<超时时间，若为0则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        char const* user=NULL, ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL ///<pack出错时的错误信息
		);

	///parse update的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
	static int parseRecvUpdate(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key, ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        CwxKeyValueItemEx const*& data, ///<返回data字段
        CWX_UINT32& uiSign, ///<返回sign
        CWX_UINT32& uiExpire, ///<返回expire，若为0表示没有指定
        CWX_UINT32& uiVersion, ///<返回版本
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
        char* szErr2K=NULL     ///<解包时的错误信息
		);

    ///pack inc的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvInc(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CWX_INT64   num, ///<inc的数字，可正可负
        CWX_INT64   result, ///<计算的结果，若为0则不添加,此记录最终的计算结果。
        CWX_INT64   max=0, ///<若inc为正值，则通过max限定最大值
        CWX_INT64   min=0, ///<若inc为负值，则通过min限定最小值
        CWX_UINT32  uiExpire=0, ///<超时时间，若为0则不添加
        CWX_UINT32  uiSign=0, ///<标记，若为0则不添加
        CWX_UINT32  uiVersion=0, ///<版本，若为0则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
        );

	///pack inc的消息包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
	static int packRecvInc(CwxPackageWriterEx* writer, ///<用于pack的writer
		CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
		CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CWX_INT64   num, ///<inc的数字，可正可负
        CWX_INT64   result, ///<计算的结果，若为0则不添加,此记录最终的计算结果。
        CWX_INT64   max=0, ///<若inc为正值，则通过max限定最大值
        CWX_INT64   min=0, ///<若inc为负值，则通过min限定最小值
        CWX_UINT32  uiExpire=0, ///<超时时间，若为0则不添加
        CWX_UINT32  uiSign=0, ///<标记，若为0则不添加
        CWX_UINT32  uiVersion=0, ///<版本，若为0则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
		);

	///解析inc的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
	static int parseRecvInc(CwxPackageReaderEx* reader,///<reader
        CwxKeyValueItemEx const*& key, ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        CWX_INT64&   num, ///<返回inc的num
        CWX_INT64&   result, ///<运算结果的值
        CWX_INT64&   max, ///<返回max
        CWX_INT64&   min, ///<返回min
        CWX_UINT32& uiExpire, ///<返回expire，若为0表示没有指定
        CWX_UINT32&  uiSign, ///<返回sign
        CWX_UINT32&  uiVersion, ///<返回version
        char const*& user,  ///<返回user
        char const*& passwd, ///<返回password
		char* szErr2K=NULL  ///<解包时的错误信息
        );

    ///pack delete的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvDel(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
        );

    ///pack delete的消息包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvDel(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        CWX_UINT32 uiVersion=0, ///<版本，若为0则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL       ///<pack出错时的错误信息
        );

    ///parse delete的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvDel(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        CWX_UINT32& uiVersion, ///<返回版本
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
        char* szErr2K=NULL     ///<解包时的错误信息
        );

    ///pack除inc外的数据更新返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvReply(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
		CWX_UINT16 unMsgType, ///<回复消息包的消息类型
        int ret,  ///<返回的ret代码
        CWX_UINT32 uiVersion, ///<返回的版本号
        CWX_UINT32 uiFieldNum, ///<返回的field数量
        char const* szErrMsg, ///<返回的错误信息
        char* szErr2K=NULL    ///<pack出错时的错误信息
        );

    ///parse除inc外的数据更新返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<返回的数据包
        int& ret,  ///<返回的ret值
        CWX_UINT32& uiVersion, ///<返回的version
        CWX_UINT32& uiFieldNum,  ///<返回的field number
        char const*& szErrMsg,  ///<返回的错误信息
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack inc的返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvIncReply(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT16 unMsgType, ///<消息类型
        int ret,  ///<ret代码
        CWX_INT64 llNum, ///<计数器的值
        CWX_UINT32 uiVersion, ///<版本号
        char const* szErrMsg, ///<错误信息
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse inc返回的消息包。 返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvIncReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<返回的数据包
        int& ret,  ///<返回的ret值
        CWX_UINT32& uiVersion, ///<返回的版本
        CWX_INT64& llNum, ///<返回的计数器的值
        char const*& szErrMsg, ///<错误信息
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack get的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packGetKey(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        bool bVersion = false, ///<是否获取版本
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        CWX_UINT8 ucKeyInfo=0, ///<是否获取key的infomation
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///pack get的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packGetKey(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        bool bVersion = false, ///<是否获取版本
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        bool bMaster = false, ///<是否从master获取
        CWX_UINT8 ucKeyInfo=0, ///<是否获取key的infomation
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///parse get的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseGetKey(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        bool&        bVersion, ///<版本
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
        CWX_UINT8& ucKeyInfo, ///<是否获取key的infomation
        char* szErr2K=NULL     ///<解包时的错误信息
        );

    ///pack exist的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExistKey(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        bool bVersion = false, ///<是否获取版本
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///pack exist的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExistKey(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        bool bVersion = false, ///<是否获取版本
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        bool bMaster = false, ///<是否从master获取
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///parse exist的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseExistKey(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<返回key字段
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        bool&        bVersion, ///<版本
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
        char* szErr2K=NULL     ///<解包时的错误信息
        );

    ///pack multi-get数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packGetKeys(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxPackageWriterEx* writer1, ///<用于pack的writer1
        list<pair<char const*, CWX_UINT16> > const& keys, ///<key的列表
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        CWX_UINT8 ucKeyInfo=0, ///<是否获取key的infomation
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///pack multi-get消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packGetKeys(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxPackageWriterEx* writer1, ///<用于pack的writer1
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        list<pair<char const*, CWX_UINT16> > const& keys, ///<key的列表
        CwxKeyValueItemEx const* field, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra, ///<extra信息，若为NULL则不添加
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        bool bMaster = false, ///<是否从master获取
        CWX_UINT8 ucKeyInfo=0, ///<是否获取key的infomation
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///parse multi-get的数据包。 返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseGetKeys(CwxPackageReaderEx* reader,///<reader
        CwxPackageReaderEx* reader1,///<reader1
        list<pair<char const*, CWX_UINT16> >& keys,///<key的列表
        CWX_UINT32& uiKeyNum, ///<key的数量
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        char const*& user,     ///<返回用户，NULL表示不存在
        char const*& passwd,   ///<返回口令，NULL表示不存在
        CWX_UINT8&   ucKeyInfo, ///<是否获取key的infomation
        char* szErr2K=NULL     ///<解包时的错误信息
        );

    ///pack 获取key列表的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packGetList(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxKeyValueItemEx const* begin=NULL, ///<开始的key
        CwxKeyValueItemEx const* end=NULL,  ///<结束的key
        CWX_UINT16  num=0,  ///<返回的数量
        CwxKeyValueItemEx const* field=NULL, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra=NULL, ///<extra信息，若为NULL则不添加
        bool        bAsc=true, ///<是否升序
        bool        bBegin=true, ///<是否获取begin的值
        bool        bKeyInfo=false, ///<是否返回key的info
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///pack 获取key列表的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packGetList(CwxPackageWriterEx* writer,
        CwxMsgBlock*& msg,
        CWX_UINT32 uiTaskId,
        CwxKeyValueItemEx const* begin=NULL, ///<开始的key
        CwxKeyValueItemEx const* end=NULL,  ///<结束的key
        CWX_UINT16  num=0,  ///<返回的数量
        CwxKeyValueItemEx const* field=NULL, ///<field字段，若为NULL的不添加
        CwxKeyValueItemEx const* extra=NULL, ///<extra信息，若为NULL则不添加
        bool        bAsc=true, ///<是否升序
        bool        bBegin=true, ///<是否获取begin的值
        bool        bKeyInfo=false, ///<是否返回key的info
        char const* user=NULL,  ///<用户，若为NULL则不添加
        char const* passwd=NULL, ///<用户口令，若为NULL则不添加
        bool bMaster = false, ///<是否从master获取
        char* szErr2K=NULL   ///<pack出错时的错误信息
        );

    ///parse get list的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseGetList(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& begin, ///<返回开始
        CwxKeyValueItemEx const*& end, ///<返回技术
        CWX_UINT16&  num, ///<获取的数量
        CwxKeyValueItemEx const*& field, ///<field字段，若为NULL表示不存在
        CwxKeyValueItemEx const*& extra, ///<extra信息，若为NULL表示不存在
        bool&        bAsc, ///<升序
        bool&        bBegin, ///<是否获取开始值
        bool&        bKeyInfo, ///<是否返回key的info
        char const*& szUser, ///<用户
        char const*& szPasswd, ///<口令
        char*        szErr2K=NULL ///<解包的错误信息
        );

    ///pack鉴权消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvAuth(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxMsgBlock*& msg,///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId,///<消息包的task id
        char const* szUser = NULL, ///<用户，若为NULL则不添加
        char const* szPasswd = NULL,///<用户口令，若为NULL则不添加
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse鉴权的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvAuth(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg,///<数据包
        char const*& szUser,///<返回用户，NULL表示不存在
        char const*& szPasswd,///<返回口令，NULL表示不存在
        char*     szErr2K=NULL ///<解包时的错误信息
        );

    ///pack鉴权回复的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packRecvAuthReply(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxMsgBlock*& msg,///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId,///<消息包的task id
        CWX_UINT16 unMsgType, ///<消息类型
        int ret, ///<鉴权结果
        char const* szErrMsg, ///<错误消息
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse鉴权回复的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseRecvAuthReply(CwxPackageReaderEx* reader,///<reader
        CwxMsgBlock const* msg,///<数据包
        int& ret,///<鉴权结果
        char const*& szErrMsg,///<错误消息
        char* szErr2K=NULL///<解包时的错误信息
        );

    ///pack export的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExportReport(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxMsgBlock*& msg,///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId,///<消息包的task id
        CWX_UINT32  uiChunkSize, ///<数据发送的chunk大小
        char const* subscribe = NULL, ///<数据订阅描述
        char const* key = NULL, ///<开始的key
        char const* extra = NULL, ///<extra信息，若为NULL则不添加
        char const* user=NULL, ///<用户名
        char const* passwd=NULL, ///<口令
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse export的report数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseExportReport(CwxPackageReaderEx* reader,///<reader
        CwxMsgBlock const* msg,///<数据包
        CWX_UINT32&  uiChunkSize,///<数据发送的chunk大小
        char const*& subscribe,///<数据订阅描述，空表示全部订阅
        char const*& key,///<开始的key，空表示没有限制
        char const*& extra, ///<extra信息，若为NULL表示没有指定
        char const*& user,///<用户名
        char const*& passwd,///<口令
        char* szErr2K=NULL///<解包时的错误信息
        );

    ///pack export的report回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExportReportReply(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxMsgBlock*& msg,///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId,///<消息包的task id
        CWX_UINT64 ullSession, ///<session
        CWX_UINT64 ullSid,  ///<数据开始发送时的sid
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse export的report回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseExportReportReply(CwxPackageReaderEx* reader,///<reader
        CwxMsgBlock const* msg,///<数据包
        CWX_UINT64& ullSession,///<session
        CWX_UINT64& ullSid,///<数据开始发送时的sid
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack一条export的key/value的数据。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExportDataItem(CwxPackageWriterEx* writer,///<用于pack的writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const& data, ///<data
        CwxKeyValueItemEx const* extra, ///<extra
        CWX_UINT32 version, ///<版本号
        CWX_UINT32 expire, ///<超时时间
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse一条export的key/value的数据返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseExportDataItem(CwxPackageReaderEx* reader,///<reader
        char const* szData, ///<key/value数据
        CWX_UINT32  uiDataLen, ///<key/value数据的长度
        CwxKeyValueItemEx const*& key, ///<数据的key
        CwxKeyValueItemEx const*& data, ///<数据的data
        CwxKeyValueItemEx const*& extra, ///<extra
        CWX_UINT32& version, ///<数据的版本
        CWX_UINT32& expire, ///<数据的超时
        char* szErr2K=NULL///<解包时的错误信息
        );

    ///pack以chunk组织的多条export的key/value的消息。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packMultiExportData(
        CWX_UINT32 uiTaskId, ///<消息包的task id
        char const* szData,  ///<多条key/value组成的数据package
        CWX_UINT32 uiDataLen, ///<数据的长度
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT64 ullSeq, ///<序列号
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse以chunk组织的多条export的key/value的数据。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseMultiExportData(
        CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg,///<数据包
        CWX_UINT64& ullSeq, ///<序列号
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack export数据的reply消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExportDataReply(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSeq, ///<序列号
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse export数据的reply消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseExportDataReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSeq, ///<序列号
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack export完成的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packExportEnd(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSid, ///<完成时的sid
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse export完成的消息包返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseExportEnd(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSid,///<完成时的sid
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack binlog sync的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packReportData(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSid, ///<开始的sid
        bool      bNewly,  ///<是否从最新binlog开始同步
        CWX_UINT32  uiChunkSize, ///<同步的chunk大小
        char const* subscribe = NULL, ///<binlog订阅规则，空表示全部订阅
        char const* user=NULL, ///<用户名
        char const* passwd=NULL, ///<用户口令
        char const* sign=NULL, ///<签名方式，空表示不签名
        bool        zip = false, ///<是否压缩
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse binlog sync的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseReportData(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSid, ///<开始的sid
        bool&       bNewly, ///<是否从最新binlog开始同步
        CWX_UINT32&  uiChunkSize, ///<同步的chunk大小
        char const*& subscribe, ///<binlog订阅规则，空表示全部订阅
        char const*& user, ///<用户名
        char const*& passwd, ///<用户口令
        char const*& sign, ///<签名方式，空表示不签名
        bool&        zip, ///<是否压缩
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack report的回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packReportDataReply(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSession, ///<session id
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse report的回复数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseReportDataReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSession, ///<session id
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack sync的session连接报告消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packReportNewConn(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSession, ///<连接所属的session
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse sync的session连接报告数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseReportNewConn(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSession, ///<连接所属的session
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack report或sync的出错消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packSyncErr(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        int ret, ///<错误代码
        char const* szErrMsg, ///<错误消息
        char* szErr2K=NULL///<pack出错时的错误信息
        );

    ///parse report或sync的出错数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseSyncErr(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        int& ret,  ///<错误代码
        char const*& szErrMsg,  ///<错误消息
        char* szErr2K=NULL ///<解包时的错误信息
        );


    ///pack sync的一条binlog的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packSyncData(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSid, ///<binlog的sid
        CWX_UINT32 uiTimeStamp, ///<binlog的时间戳
        CwxKeyValueItemEx const& data, ///<binlog的data
        CWX_UINT32 group,  ///<binlog所属的分组
        CWX_UINT32 type,   ///<binlog的类型，也就是消息类型
        CWX_UINT32 version,  ///<对应的key的版本
        CWX_UINT64 ullSeq,  ///<消息的序列号
        char const* sign=NULL, ///<签名方式
        bool       zip = false, ///<是否压缩
        char* szErr2K=NULL///<pack出错时的错误信息
        );

    ///pack 一条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packSyncDataItem(CwxPackageWriterEx* writer, ///<用于pack的writer
        CWX_UINT64 ullSid, ///<binlog的sid
        CWX_UINT32 uiTimeStamp, ///<binlog的时间戳
        CwxKeyValueItemEx const& data, ///<binlog的data
        CWX_UINT32 group,  ///<binlog所属的分组
        CWX_UINT32 type,   ///<binlog的类型，也就是消息类型
        CWX_UINT32 version,  ///<对应的key的版本
        char const* sign=NULL, ///<签名方式
        char* szErr2K=NULL///<pack出错时的错误信息
        );

    ///pack 多条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packMultiSyncData(
        CWX_UINT32 uiTaskId, ///<任务id
        char const* szData, ///<多条消息的数据buf
        CWX_UINT32 uiDataLen, ///<多条数据的数据buf长度
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT64 ullSeq, ///<消息包的消息序列号
        bool  zip = false, ///<是否压缩
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse一条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseSyncData(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSid, ///<binlog的sid
        CWX_UINT32& uiTimeStamp, ///<binlog的时间戳
        CwxKeyValueItemEx const*& data, ///<binlog的数据
        CWX_UINT32& group, ///<binlog所属的group
        CWX_UINT32& type, ///<binlog对应的数据变更消息类型
        CWX_UINT32& version, ///<binlog对应的数据变更的key的版本
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///parse一条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseSyncData(CwxPackageReaderEx* reader,
        char const* szData,
        CWX_UINT32 uiDataLen,
        CWX_UINT64& ullSid, ///<binlog的sid
        CWX_UINT32& uiTimeStamp, ///<binlog的时间戳
        CwxKeyValueItemEx const*& data, ///<binlog的数据
        CWX_UINT32& group, ///<binlog所属的group
        CWX_UINT32& type, ///<binlog对应的数据变更消息类型
        CWX_UINT32& version, ///<binlog对应的数据变更的key的版本
        char* szErr2K=NULL ///<解包时的错误信息
        );

    ///pack sync binlog的回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int packSyncDataReply(CwxPackageWriterEx* writer, ///<用于pack的writer
        CwxMsgBlock*& msg, ///<返回的消息包，对象由内部分配
        CWX_UINT32 uiTaskId, ///<消息包的task id
        CWX_UINT64 ullSeq, ///<消息的序列号
        CWX_UINT16 unMsgType, ///<消息类型
        char* szErr2K=NULL ///<pack出错时的错误信息
        );

    ///parse sync binlog的回复数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
    static int parseSyncDataReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<数据包
        CWX_UINT64& ullSeq, ///<消息的序列号
        char* szErr2K=NULL  ///<解包时的错误信息
        );

    ///设置数据同步包的seq号
    inline static void setSeq(char* szBuf, CWX_UINT64 ullSeq){
        CWX_UINT32 byte4 = (CWX_UINT32)(ullSeq>>32);
        byte4 = CWX_HTONL(byte4);
        memcpy(szBuf, &byte4, 4);
        byte4 = (CWX_UINT32)(ullSeq&0xFFFFFFFF);
        byte4 = CWX_HTONL(byte4);
        memcpy(szBuf + 4, &byte4, 4);

    }    
    ///获取数据同步包的seq号
    inline static CWX_UINT64 getSeq(char const* szBuf) {
        CWX_UINT64 ullSeq = 0;
        CWX_UINT32 byte4;
        memcpy(&byte4, szBuf, 4);
        ullSeq = CWX_NTOHL(byte4);
        memcpy(&byte4, szBuf+4, 4);
        ullSeq <<=32;
        ullSeq += CWX_NTOHL(byte4);
        return ullSeq;
    }

    ///是否继续查找订阅的消息类型
    inline static bool isContinueSeek(CWX_UINT32 uiSeekedNum){
        return MAX_CONTINUE_SEEK_NUM>uiSeekedNum;
    }
private:
    ///禁止创建对象实例
    UnistorPoco()
    {
    }
    ///析构函数
    ~UnistorPoco();
};





#endif
