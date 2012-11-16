#ifndef __UNISTOR_POCO_H__
#define __UNISTOR_POCO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cwx_msg_header.h"
#include "cwx_package_reader_ex.h"
#include "cwx_package_writer_ex.h"
#include "cwx_md5.h"
#include "cwx_crc32.h"
#include "cwx_md5.h"
#include "cwx_crc32.h"

    ///定义MASTER标志位
#define  UNISTOR_MASTER_ATTR_BIT  0x01

    ///协议的消息类型定义
    ///RECV服务类型的消息类型定义
#define UNISTOR_MSG_TYPE_TIMESTAMP             0   ///<时钟消息类型
#define UNISTOR_MSG_TYPE_RECV_ADD              1   ///<ADD key/value
#define UNISTOR_MSG_TYPE_RECV_ADD_REPLY        2   ///<set key/value的回复
#define UNISTOR_MSG_TYPE_RECV_SET              3   ///<SET key
#define UNISTOR_MSG_TYPE_RECV_SET_REPLY        4   ///<SET key的回复
#define UNISTOR_MSG_TYPE_RECV_UPDATE           5   ///update key
#define UNISTOR_MSG_TYPE_RECV_UPDATE_REPLY     6   ///<update key的回复
#define UNISTOR_MSG_TYPE_RECV_INC              7   ///<inc key
#define UNISTOR_MSG_TYPE_RECV_INC_REPLY        8   ///<inc key的回复
#define UNISTOR_MSG_TYPE_RECV_DEL              9   ///<del key
#define UNISTOR_MSG_TYPE_RECV_DEL_REPLY        10  ///<del key的回复
#define UNISTOR_MSG_TYPE_RECV_EXIST            11  ///<Key是否存在
#define UNISTOR_MSG_TYPE_RECV_EXIST_REPLY      12  ///<Key是否存在的回复
#define UNISTOR_MSG_TYPE_RECV_GET              13  ///<get key
#define UNISTOR_MSG_TYPE_RECV_GET_REPLY        14  ///<get key的回复
#define UNISTOR_MSG_TYPE_RECV_GETS             15  ///<get 多个key
#define UNISTOR_MSG_TYPE_RECV_GETS_REPLY       16  ///<get 多个key的回复
#define UNISTOR_MSG_TYPE_RECV_LIST             17  ///<获取列表
#define UNISTOR_MSG_TYPE_RECV_LIST_REPLY       18  ///<列表回复
#define UNISTOR_MSG_TYPE_RECV_IMPORT           19  ///<import数据
#define UNISTOR_MSG_TYPE_RECV_IMPORT_REPLY     20  ///<import的回复
#define UNISTOR_MSG_TYPE_RECV_AUTH             21  ///<认证
#define UNISTOR_MSG_TYPE_RECV_AUTH_REPLY       22  ///<认证回复

    ///数据导出数据类型
#define UNISTOR_MSG_TYPE_EXPORT_REPORT         51  ///<数据导出export
#define UNISTOR_MSG_TYPE_EXPORT_REPORT_REPLY   52  ///<数据导出的reply
#define UNISTOR_MSG_TYPE_EXPORT_DATA           53  ///<数据导出的数据
#define UNISTOR_MSG_TYPE_EXPORT_DATA_REPLY     54  ///<数据导出的数据reply
#define UNISTOR_MSG_TYPE_EXPORT_END            55  ///<数据导出完成

    ///分发的消息类型定义
#define UNISTOR_MSG_TYPE_SYNC_REPORT           101 ///<同步SID点报告消息类型
#define UNISTOR_MSG_TYPE_SYNC_REPORT_REPLY     102 ///<report返回
#define UNISTOR_MSG_TYPE_SYNC_CONN             103 ///<连接通报
#define UNISTOR_MSG_TYPE_SYNC_CONN_REPLY       104 ///<连接通报回复
#define UNISTOR_MSG_TYPE_SYNC_DATA             105 ///<发送数据
#define UNISTOR_MSG_TYPE_SYNC_DATA_REPLY       106 ///<数据的回复
#define UNISTOR_MSG_TYPE_SYNC_DATA_CHUNK       107 ///<chunk模式发送数据
#define UNISTOR_MSG_TYPE_SYNC_DATA_CHUNK_REPLY 108 ///<chunk模式发送数据的回复
#define UNISTOR_MSG_TYPE_SYNC_ERR              109 ///<sync的错误消息


    ///通信的key定义
#define UNISTOR_KEY_ASC    "asc"   ///<升序
#define UNISTOR_KEY_BEGIN      "begin" ///<开始 
#define UNISTOR_KEY_C       "c"  ///<cache
#define UNISTOR_KEY_CHUNK "chunk"  ///<数据发送的chunk的key
#define UNISTOR_KEY_CRC32  "crc32" ///<crc32签名
#define UNISTOR_KEY_END    "end"   ///<结束值的key
#define UNISTOR_KEY_ERR  "err"     ///error的key
#define UNISTOR_KEY_E "e"          ///<expire的key
#define UNISTOR_KEY_D "d"           ///<数据的data的key
#define UNISTOR_KEY_F  "f"          ///<field
#define UNISTOR_KEY_FN "fn"         ///<field number
#define UNISTOR_KEY_G   "g"         ///<group,为key的分组
#define UNISTOR_KEY_I "i"           ///<information
#define UNISTOR_KEY_IB "ib"         ///<isbegin
#define UNISTOR_KEY_K "k"           ///<数据的key的key
#define UNISTOR_KEY_M      "m"      ///<message的key
#define UNISTOR_KEY_MD5    "md5"    ///<md5签名的key
#define UNISTOR_KEY_MAX    "max"    ///<最大值
#define UNISTOR_KEY_MIN    "min"    ///<最小值
#define UNISTOR_KEY_N "n"           ///<number
#define UNISTOR_KEY_P "p"            ///<password
#define UNISTOR_KEY_R "r"            ///<inc的result值，此在inc的sync的时候存在
#define UNISTOR_KEY_RET  "ret"       ///<返回值的ret
#define UNISTOR_KEY_SEQ   "seq"      ///<数据同步的消息序列号
#define UNISTOR_KEY_SESSION "session" ///<数据同步的session
#define UNISTOR_KEY_SID "sid"        ///<数据变更的sid
#define UNISTOR_KEY_SIGN   "sign"    ///<数据更新的sign
#define UNISTOR_KEY_SUBSCRIBE  "subscribe" ///<订阅的key
#define UNISTOR_KEY_T "t"           ///<timestamp
#define UNISTOR_KEY_TYPE   "type"   ///<binlog的类型，也就是数据更新的消息类型
#define UNISTOR_KEY_U    "u"       ///<user
#define UNISTOR_KEY_ZIP    "zip"  ///<压缩标示
#define UNISTOR_KEY_V      "v"   ///<version key
#define UNISTOR_KEY_X      "x"  ///<引擎扩展key

///系统信息的key定义
#define UNISTOR_SYS_KEY_PID "pid"  ///<进程ID
#define UNISTOR_SYS_KEY_PPID "ppid" ///<父进程ID
#define UNISTOR_SYS_KEY_VERSION "version"  ///<版本号
#define UNISTOR_SYS_KEY_MODIFY  "modify" ///<代码修改时间
#define UNISTOR_SYS_KEY_COMPILE "compile" ///<代码编译时间
#define UNISTOR_SYS_KEY_START   "start"   ///<服务启动时间
#define UNISTOR_SYS_KEY_ENGINE  "engine"  ///<存储引擎名字
#define UNISTOR_SYS_KEY_ENGINE_VERSION "engine_version" ///<存储引擎的版本
#define UNISTOR_SYS_KEY_ENGINE_STATE   "engine_state" ///<存储引擎的状态
#define UNISTOR_SYS_KEY_ENGINE_ERROR   "engine_error" ///<存储引擎的错误信息
#define UNISTOR_SYS_KEY_BINLOG_STATE   "binlog_state" ///<binlog的状态
#define UNISTOR_SYS_KEY_BINLOG_ERROR   "binlog_error" ///<binlog的错误信息
#define UNISTOR_SYS_KEY_BINLOG_MIN_SID       "binlog_min_sid"   ///<binlog的最小的sid值
#define UNISTOR_SYS_KEY_BINLOG_MIN_TIMESTAMP "binlog_min_timestamp" ///<binlog的最小时间戳
#define UNISTOR_SYS_KEY_BINLOG_MIN_FILE     "binlog_min_file" ///<binlog的最小文件
#define UNISTOR_SYS_KEY_BINLOG_MAX_SID  "binlog_max_sid" ///binlog的最大sid
#define UNISTOR_SYS_KEY_BINLOG_MAX_TIMESTAMP "binlog_max_timestamp" ///<binlog的最大时间戳
#define UNISTOR_SYS_KEY_BINLOG_MAX_FILE "binlog_max_file" ///<binlog的最大文件
#define UNISTOR_SYS_KEY_READ_THREAD_NUM  "read_thread_num" ///<读线程的数量
#define UNISTOR_SYS_KEY_READ_THREAD_QUEUE "read_thread_queue" ///<线程的队列的消息数量
#define UNISTOR_SYS_KEY_READ_THREAD_CONNECT "read_thread_connect" ///<读线程的链接数量
#define UNISTOR_SYS_KEY_READ_THREAD_QUEUE_PREX "read_thread_queue_" ///<线程的队列前缀
#define UNISTOR_SYS_KEY_READ_THREAD_CONNECT_PREX "read_thread_connect_" ///<读线程的链接前缀
#define UNISTOR_SYS_KEY_WRITE_THREAD_QUEUE "write_thread_queue" ///<写线程的队列
#define UNISTOR_SYS_KEY_TRANS_THREAD_QUEUE "trans_thread_queue" ///<转发线程队列
#define UNISTOR_SYS_KEY_CHECKPOINT_THREAD_QUEUE "checkpoint_thread_queue" ///<checkpoint线程的队列
#define UNISTOR_SYS_KEY_ZK_THREAD_QUEUE  "zk_thread_queue" ///<zookeeper线程的队列
#define UNISTOR_SYS_KEY_INNER_SYNC_THREAD_QUEUE "inner_sync_thread_queue" ///<内部同步的线程队列
#define UNISTOR_SYS_KEY_OUTER_SYNC_THREAD_QUEUE "outer_sync_thread_queue" ///<外部同步的线程队列
#define UNISTOR_SYS_KEY_MASTER_TRANS_MSG_NUM           "master_trans_msg_num" ///<当前转发消息的数量
#define UNISTOR_SYS_KEY_ZK_STATE "zk_state"  ///<zookeeper的连接状态
#define UNISTOR_SYS_KEY_ZK_ERROR "zk_error"  ///<zookeeper的错误信息
#define UNISTOR_SYS_KEY_CACHE_STATE "cache_state" ///<cache的状态
#define UNISTOR_SYS_KEY_CACHE_ERR   "cache_error" ///<cache的错误消息
#define UNISTOR_SYS_KEY_WRITE_CACHE_KEY "write_cache_key" ///<写cache中key的数量
#define UNISTOR_SYS_KEY_WRITE_CACHE_SPACE "write_cache_space" ///<写cache的数据大小
#define UNISTOR_SYS_KEY_READ_CACHE_MAX_SIZE "read_cache_max_size" ///<读cache的最大大小
#define UNISTOR_SYS_KEY_READ_CACHE_MAX_KEY  "read_cache_max_key" ///<读cache的最大key的数量
#define UNISTOR_SYS_KEY_READ_CACHE_USED_SIZE    "read_cache_used_size" ///<读cache占用的空间大小
#define UNISTOR_SYS_KEY_READ_CACHE_USED_CAPACITY "read_cache_used_capacity" ///<读cache占用的数据的容量
#define UNISTOR_SYS_KEY_READ_CACHE_USED_DATA_SIZE "read_cache_used_data_size" ///<读cache的实际数据大小
#define UNISTOR_SYS_KEY_READ_CACHE_FREE_SIZE      "read_cache_free_size" ///<读cache空闲的空间大小
#define UNISTOR_SYS_KEY_READ_CACHE_FREE_CAPACITY  "read_cache_free_capacity" ///<读cache空闲的容量
#define UNISTOR_SYS_KEY_READ_CACHE_KEY       "read_cache_key" ///<cache的key的数量
#define UNISTOR_SYS_KEY_GET_NUM                   "get_num" ///<get的访问数量
#define UNISTOR_SYS_KEY_GET_READ_CACHE_NUM        "get_read_cache_num" ///<read cache的数量
#define UNISTOR_SYS_KEY_GET_EXIST_NUM             "get_exist_num" ///<存在的数量
#define UNISTOR_SYS_KEY_GETS_NUM                  "gets_num" ///<gets的访问数量
#define UNISTOR_SYS_KEY_GETS_KEY_NUM              "gets_key_num" ///<gets的key的数量
#define UNISTOR_SYS_KEY_GETS_KEY_READ_CACHE_NUM   "gets_key_read_cache_num" ///<gets 的key使用cache的数量
#define UNISTOR_SYS_KEY_GETS_KEY_EXIST_NUM        "gets_key_exist_num" ///<gets的key存在的数量
#define UNISTOR_SYS_KEY_LIST_NUM                  "list_num" ///<list访问的数量
#define UNISTOR_SYS_KEY_EXIST_NUM                 "exist_num" ///<exist的访问数量
#define UNISTOR_SYS_KEY_EXIST_READ_CACHE_NUM      "exist_read_cache_num" ///<exist的read cache的数量
#define UNISTOR_SYS_KEY_EXIST_EXIST_NUM           "exist_exist_num" ///<exist的存在数量
#define UNISTOR_SYS_KEY_ADD_NUM                   "add_num" ///<add的数量
#define UNISTOR_SYS_KEY_ADD_READ_CACHE_NUM        "add_read_cache_num"//<add的read cache数量
#define UNISTOR_SYS_KEY_ADD_WRITE_CACHE_NUM       "add_write_cache_num" ///<add的write cache数量
#define UNISTOR_SYS_KEY_SET_NUM                   "set_num" ///<set的数量
#define UNISTOR_SYS_KEY_SET_READ_CACHE_NUM        "set_read_cache_num"//<set的read cache数量
#define UNISTOR_SYS_KEY_SET_WRITE_CACHE_NUM       "set_write_cache_num" ///<set的write cache数量
#define UNISTOR_SYS_KEY_UPDATE_NUM                "update_num" ///<update的数量
#define UNISTOR_SYS_KEY_UPDATE_READ_CACHE_NUM     "update_read_cache_num"//<update的read cache数量
#define UNISTOR_SYS_KEY_UPDATE_WRITE_CACHE_NUM    "update_write_cache_num" ///<update的write cache数量
#define UNISTOR_SYS_KEY_INC_NUM                   "inc_num" ///<inc的数量
#define UNISTOR_SYS_KEY_INC_READ_CACHE_NUM        "inc_read_cache_num"//<inc的read cache数量
#define UNISTOR_SYS_KEY_INC_WRITE_CACHE_NUM       "inc_write_cache_num" ///<inc的write cache数量
#define UNISTOR_SYS_KEY_DEL_NUM                   "del_num" ///<del的数量
#define UNISTOR_SYS_KEY_DEL_READ_CACHE_NUM        "del_read_cache_num"//<del的read cache数量
#define UNISTOR_SYS_KEY_DEL_WRITE_CACHE_NUM       "del_write_cache_num" ///<del的write cache数量
#define UNISTOR_SYS_KEY_IMPORT_NUM                "import_num" ///<del的数量
#define UNISTOR_SYS_KEY_IMPORT_READ_CACHE_NUM     "import_read_cache_num"//<del的read cache数量
#define UNISTOR_SYS_KEY_IMPORT_WRITE_CACHE_NUM    "import_write_cache_num" ///<del的write cache数量
#define UNISTOR_SYS_KEY_ENGINE_INFO               "engine_info"   ///<engine的说明信息
#define UNISTOR_SYS_KEY_ENGINE_SYS_KEY            "engine_sys_key" ///<engine的系统key

    ///错误代码定义，0~100是系统级的错误
#define UNISTOR_ERR_SUCCESS          0  ///<成功
#define UNISTOR_ERR_ERROR            1 ///<无需区分的笼统错误
#define UNISTOR_ERR_FAIL_AUTH        2 ///<鉴权失败
#define UNISTOR_ERR_LOST_SYNC        3 ///<失去了同步状态
#define UNISTOR_ERR_NO_MASTER        4 ///<没有master
#define UNISTOR_ERR_TOO_MANY_WRITE   5 ///<太多的写
#define UNISTOR_ERR_TOO_MANY_TRANS   6 ///<太多的转发消息
    //100以上是应用及的错误
#define UNISTOR_ERR_NEXIST            101   ///<不存在
#define UNISTOR_ERR_EXIST             102   ///<存在
#define UNISTOR_ERR_VERSION           103   ///<版本错误
#define UNISTOR_ERR_OUTRANGE          104   ///<inc超出范围
#define UNISTOR_ERR_TIMEOUT           105   ///<超时

#define UNISTOR_DEF_CHUNK_SIZE_KB     64    ///<缺省的chunk大小
#define UNISTOR_ZIP_EXTRA_BUF           128

/*****************************************************************
接口API定义
******************************************************************/

///设置从master获取的属性位
CWX_UINT8 unistor_set_from_master(CWX_UINT8* ucAttr);

///check是否设置了从master获取的属性位。0：不是；>0：是
CWX_UINT8 isFromMaster(CWX_UINT8 ucAttr);

///清除从master获取的属性位
CWX_UINT8 clearFromMaster(CWX_UINT8* ucAttr);

///pack import key的数据。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_import(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer，内容通过writer返回
                             char* buf,  ///<pack的数据空间
                             CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                             CWX_UINT32 uiTaskId, ///<消息包的task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiExpire,  ///<超时时间，若为0则不添加
                             CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                             CWX_UINT8   bCache, ///<是否cache，若为true则不添加
                             char const* user,  ///<用户，若为NULL则不添加
                             char const* passwd, ///<用户口令，若为NULL则不添加
                             char* szErr2K       ///<pack出错时的错误信息
                             );

///解析Add key的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_import(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<收到的消息包
                              CWX_UINT32  msg_len, ///<收到的消息包的长度
                              CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                              CWX_KEY_VALUE_ITEM_S const** data,  ///<返回data字段
                              CWX_UINT32* uiExpire,  ///<返回expire，若为0表示没有指定
                              CWX_UINT32* uiVersion, ///<返回版本
                              CWX_UINT8*       bCache,    ///<返回cache
                              char const** user,     ///<返回用户，NULL表示不存在
                              char const** passwd,   ///<返回口令，NULL表示不存在
                              char* szErr2K     ///<解包时的错误信息
                              );

///pack Add key的数据。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_add(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer，内容通过writer返回
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                          CWX_KEY_VALUE_ITEM_S const* data, ///<data
                          CWX_UINT32 uiExpire,  ///<超时时间，若为0则不添加
                          CWX_UINT32 uiSign,    ///<标记，若为0则不添加
                          CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                          CWX_UINT8       bCache, ///<是否cache，若为true则不添加
                          char const* user,  ///<用户，若为NULL则不添加
                          char const* passwd, ///<用户口令，若为NULL则不添加
                          char* szErr2K       ///<pack出错时的错误信息
                          );

///解析Add key的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_add(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<收到的消息包
                           CWX_UINT32  msg_len, ///<收到的消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** data,  ///<返回data字段
                           CWX_UINT32* uiExpire,  ///<返回expire，若为0表示没有指定
                           CWX_UINT32* uiSign,    ///<返回sign
                           CWX_UINT32* uiVersion, ///<返回版本
                           CWX_UINT8*  bCache,    ///<返回cache
                           char const** user,     ///<返回用户，NULL表示不存在
                           char const** passwd,   ///<返回口令，NULL表示不存在
                           char* szErr2K     ///<解包时的错误信息
                           );

///pack set的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_set(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer，内容通过writer返回
                char* buf,  ///<pack的数据空间
                CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                CWX_UINT32 uiTaskId, ///<消息包的task id
                CWX_KEY_VALUE_ITEM_S const* key, ///<key
                CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                CWX_KEY_VALUE_ITEM_S const* data, ///<data
                CWX_UINT32 uiSign, ///<标记，若为0则不添加
                CWX_UINT32 uiExpire, ///<超时时间，若为0则不添加
                CWX_UINT32 uiVersion,///<版本，若为0则不添加
                CWX_UINT8  bCache, ///<是否cache，若为true则不添加
                char const* user, ///<用户，若为NULL则不添加
                char const* passwd,///<用户口令，若为NULL则不添加
                char* szErr2K ///<pack出错时的错误信息
                );

///parse set的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_set(struct CWX_PG_READER_EX* reader,  ///<reader
                           char const* msg, ///<收到的消息包
                           CWX_UINT32  msg_len, ///<收到的消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key, ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** data, ///<返回data字段
                           CWX_UINT32* uiSign, ///<返回sign
                           CWX_UINT32* uiExpire, ///<返回expire
                           CWX_UINT32* uiVersion, ///<返回版本
                           CWX_UINT8*   bCache,  ///<返回cache
                           char const** user, ///<返回用户，NULL表示不存在
                           char const** passwd, ///<返回口令，NULL表示不存在
                           char* szErr2K  ///<解包时的错误信息
                           );

///pack update的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_update(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                             char* buf,  ///<pack的数据空间
                             CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                             CWX_UINT32 uiTaskId, ///<消息包的task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiSign, ///<标记，若为0则不添加
                             CWX_UINT32 uiExpire, ///<超时时间，若为0则不添加
                             CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                             char const* user, ///<用户，若为NULL则不添加
                             char const* passwd, ///<用户口令，若为NULL则不添加
                             char* szErr2K ///<pack出错时的错误信息
                             );

///parse update的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_update(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<收到的消息包
                              CWX_UINT32  msg_len, ///<收到的消息包的长度
                              CWX_KEY_VALUE_ITEM_S const** key, ///<返回key字段
                              CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                              CWX_KEY_VALUE_ITEM_S const** data, ///<返回data字段
                              CWX_UINT32* uiSign, ///<返回sign
                              CWX_UINT32* uiExpire, ///<返回expire，若为0表示没有指定
                              CWX_UINT32* uiVersion, ///<返回版本
                              char const** user,     ///<返回用户，NULL表示不存在
                              char const** passwd,   ///<返回口令，NULL表示不存在
                              char* szErr2K     ///<解包时的错误信息
                              );

///pack inc的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_inc(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                          CWX_INT64   num, ///<inc的数字，可正可负
                          CWX_INT64   result, ///<计算的结果，若为0则不添加,此记录最终的计算结果。
                          CWX_INT64   max, ///<若inc为正值，则通过max限定最大值
                          CWX_INT64   min, ///<若inc为负值，则通过min限定最小值
                          CWX_UINT32  uiExpire, ///<超时时间，若为0则不添加
                          CWX_UINT32  uiSign, ///<标记，若为0则不添加
                          CWX_UINT32  uiVersion, ///<版本，若为0则不添加
                          char const* user,  ///<用户，若为NULL则不添加
                          char const* passwd, ///<用户口令，若为NULL则不添加
                          char* szErr2K       ///<pack出错时的错误信息
                          );

///解析inc的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_inc(struct CWX_PG_READER_EX* reader,///<reader
                           char const* msg, ///<收到的消息包
                           CWX_UINT32  msg_len, ///<收到的消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key, ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                           CWX_INT64*   num, ///<返回inc的num
                           CWX_INT64*   result, ///<运算结果的值
                           CWX_INT64*   max, ///<返回max
                           CWX_INT64*   min, ///<返回min
                           CWX_UINT32*  uiExpire, ///<返回expire，若为0表示没有指定
                           CWX_UINT32*  uiSign, ///<返回sign
                           CWX_UINT32*  uiVersion, ///<返回version
                           char const** user,  ///<返回user
                           char const** passwd, ///<返回password
                           char* szErr2K  ///<解包时的错误信息
                           );

///pack delete的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_del(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                          CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                          char const* user,  ///<用户，若为NULL则不添加
                          char const* passwd, ///<用户口令，若为NULL则不添加
                          char* szErr2K       ///<pack出错时的错误信息
                          );

///parse delete的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_del(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<收到的消息包
                           CWX_UINT32  msg_len, ///<收到的消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                           CWX_UINT32* uiVersion, ///<返回版本
                           char const** user,     ///<返回用户，NULL表示不存在
                           char const** passwd,   ///<返回口令，NULL表示不存在
                           char* szErr2K     ///<解包时的错误信息
                           );

///pack除inc外的数据更新返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_reply(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                            char* buf,  ///<pack的数据空间
                            CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                            CWX_UINT32 uiTaskId, ///<消息包的task id
                            CWX_UINT16 unMsgType, ///<回复消息包的消息类型
                            int ret,  ///<返回的ret代码
                            CWX_UINT32 uiVersion, ///<返回的版本号
                            CWX_UINT32 uiFieldNum, ///<返回的field数量
                            char const* szErrMsg, ///<返回的错误信息
                            char* szErr2K    ///<pack出错时的错误信息
                            );

///parse除inc外的数据更新返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_reply(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<收到的消息包
                             CWX_UINT32  msg_len, ///<收到的消息包的长度
                             int* ret,  ///<返回的ret值
                             CWX_UINT32* uiVersion, ///<返回的version
                             CWX_UINT32* uiFieldNum,  ///<返回的field number
                             char const** szErrMsg,  ///<返回的错误信息
                             char* szErr2K ///<解包时的错误信息
                             );

///pack inc的返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_inc_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                char* buf,  ///<pack的数据空间
                                CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                CWX_UINT32 uiTaskId, ///<消息包的task id
                                CWX_UINT16 unMsgType, ///<消息类型
                                int ret,  ///<ret代码
                                CWX_INT64 llNum, ///<计数器的值
                                CWX_UINT32 uiVersion, ///<版本号
                                char const* szErrMsg, ///<错误信息
                                char* szErr2K ///<pack出错时的错误信息
                                );

///parse inc返回的消息包。 返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_inc_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                 char const* msg, ///<收到的消息包
                                 CWX_UINT32  msg_len, ///<收到的消息包的长度
                                 int* ret,  ///<返回的ret值
                                 CWX_UINT32* uiVersion, ///<返回的版本
                                 CWX_INT64* llNum, ///<返回的计数器的值
                                 char const** szErrMsg, ///<错误信息
                                 char* szErr2K ///<解包时的错误信息
                                 );

///pack get的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_get_key(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                         char* buf,  ///<pack的数据空间
                         CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                         CWX_UINT32 uiTaskId, ///<消息包的task id
                         CWX_KEY_VALUE_ITEM_S const* key, ///<key
                         CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                         CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                         CWX_UINT8 bVersion, ///<是否获取版本
                         CWX_UINT8 bMaster, ///<是否从master获取
                         char const* user,  ///<用户，若为NULL则不添加
                         char const* passwd, ///<用户口令，若为NULL则不添加
                         CWX_UINT8 ucKeyInfo, ///<是否获取key的infomation
                         char* szErr2K   ///<pack出错时的错误信息
                         );

///parse get的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_get_key(struct CWX_PG_READER_EX* reader, ///<reader
                          char const* msg, ///<收到的消息包
                          CWX_UINT32  msg_len, ///<收到的消息包的长度
                          CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                          CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                          CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                          CWX_UINT8*          bVersion, ///<版本
                          char const** user,     ///<返回用户，NULL表示不存在
                          char const** passwd,   ///<返回口令，NULL表示不存在
                          CWX_UINT8* ucKeyInfo, ///<是否获取key的infomation
                          char* szErr2K     ///<解包时的错误信息
                          );

///pack exist的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_exist_key(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                           char* buf,  ///<pack的数据空间
                           CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                           CWX_UINT32 uiTaskId, ///<消息包的task id
                           CWX_KEY_VALUE_ITEM_S const* key, ///<key
                           CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                           CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                           CWX_UINT8 bVersion, ///<是否获取版本
                           CWX_UINT8 bMaster, ///<是否从master获取
                           char const* user,  ///<用户，若为NULL则不添加
                           char const* passwd, ///<用户口令，若为NULL则不添加
                           char* szErr2K   ///<pack出错时的错误信息
                           );

///parse exist的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_exist_key(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<收到的消息包
                            CWX_UINT32  msg_len, ///<收到的消息包的长度
                            CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                            CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                            CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                            CWX_UINT8*        bVersion, ///<版本
                            char const** user,     ///<返回用户，NULL表示不存在
                            char const** passwd,   ///<返回口令，NULL表示不存在
                            char* szErr2K     ///<解包时的错误信息
                            );

///pack multi-get数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_get_keys(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                          struct CWX_PG_WRITER_EX* writer1, ///<用于pack的writer1
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          char const* const*  keys, ///<key的数组
                          CWX_UINT16  unKeyNum, ///<key的数量
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                          char const* user,  ///<用户，若为NULL则不添加
                          char const* passwd, ///<用户口令，若为NULL则不添加
                          CWX_UINT8 ucKeyInfo, ///<是否获取key的infomation
                          CWX_UINT8 bMaster, ///<是否从master获取
                          char* szErr2K   ///<pack出错时的错误信息
                          );

///parse multi-get的数据包。 返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_get_keys(struct CWX_PG_READER_EX* reader,///<reader
                           struct CWX_PG_READER_EX* reader1,///<reader1
                           char const* msg, ///<收到的消息包
                           CWX_UINT32  msg_len, ///<收到的消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** keys,///<key的列表
                           CWX_UINT16* unKeyNum, ///<key的数量
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                           char const** user,     ///<返回用户，NULL表示不存在
                           char const** passwd,   ///<返回口令，NULL表示不存在
                           CWX_UINT8*   ucKeyInfo, ///<是否获取key的infomation
                           char* szErr2K     ///<解包时的错误信息
                           );

///pack 获取key列表的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_get_list(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* begin, ///<开始的key
                          CWX_KEY_VALUE_ITEM_S const* end,  ///<结束的key
                          CWX_UINT16  num,  ///<返回的数量
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为NULL的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为NULL则不添加
                          CWX_UINT8     bAsc, ///<是否升序
                          CWX_UINT8     bBegin, ///<是否获取begin的值
                          CWX_UINT8     bKeyInfo, ///<是否返回key的info
                          CWX_UINT8     bMaster, ///<是否从master获取
                          char const* user,  ///<用户，若为NULL则不添加
                          char const* passwd, ///<用户口令，若为NULL则不添加
                          char* szErr2K   ///<pack出错时的错误信息
                          );

///parse get list的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_get_list(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<收到的消息包
                           CWX_UINT32  msg_len, ///<收到的消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** begin, ///<返回开始
                           CWX_KEY_VALUE_ITEM_S const** end, ///<返回技术
                           CWX_UINT16*  num, ///<获取的数量
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为NULL表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为NULL表示不存在
                           CWX_UINT8*        bAsc, ///<升序
                           CWX_UINT8*        bBegin, ///<是否获取开始值
                           CWX_UINT8*        bKeyInfo, ///<是否返回key的info
                           char const** user, ///<用户
                           char const** passwd, ///<口令
                           char*        szErr2K ///<解包的错误信息
                           );

///pack鉴权消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_auth(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                           char* buf,  ///<pack的数据空间
                           CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                           CWX_UINT32 uiTaskId, ///<消息包的task id
                           char const* user, ///<用户，若为NULL则不添加
                           char const* passwd,///<用户口令，若为NULL则不添加
                           char* szErr2K ///<pack出错时的错误信息
                           );

///parse鉴权的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_auth(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<收到的消息包
                            CWX_UINT32  msg_len, ///<收到的消息包的长度
                            char const** user, ///<用户
                            char const** passwd, ///<口令
                            char*     szErr2K ///<解包时的错误信息
                            );

///pack鉴权回复的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_auth_reply(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT32 uiTaskId, ///<消息包的task id
                                 CWX_UINT16 unMsgType, ///<消息类型
                                 int ret, ///<鉴权结果
                                 char const* szErrMsg, ///<错误消息
                                 char* szErr2K ///<pack出错时的错误信息
                                 );

///parse鉴权回复的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_auth_reply(struct CWX_PG_READER_EX* reader,///<reader
                                  char const* msg, ///<收到的消息包
                                  CWX_UINT32  msg_len, ///<收到的消息包的长度
                                  int* ret,///<鉴权结果
                                  char const** szErrMsg,///<错误消息
                                  char* szErr2K///<解包时的错误信息
                                  );

///pack export的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_report(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                               char* buf,  ///<pack的数据空间
                               CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                               CWX_UINT32 uiTaskId, ///<消息包的task id
                               CWX_UINT32  uiChunkSize, ///<数据发送的chunk大小
                               char const* subscribe, ///<数据订阅描述
                               char const* key, ///<开始的key
                               char const* extra, ///<extra信息，若为NULL则不添加
                               char const* user, ///<用户名
                               char const* passwd, ///<口令
                               char* szErr2K ///<pack出错时的错误信息
                               );

///parse export的report数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_report(struct CWX_PG_READER_EX* reader,///<reader
                                char const* msg, ///<收到的消息包
                                CWX_UINT32  msg_len, ///<收到的消息包的长度
                                CWX_UINT32*  uiChunkSize,///<数据发送的chunk大小
                                char const** subscribe,///<数据订阅描述，空表示全部订阅
                                char const** key,///<开始的key，空表示没有限制
                                char const** extra, ///<extra信息，若为NULL表示没有指定
                                char const** user,///<用户名
                                char const** passwd,///<口令
                                char* szErr2K///<解包时的错误信息
                                );

///pack export的report回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_report_reply(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                                     char* buf,  ///<pack的数据空间
                                     CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                     CWX_UINT32 uiTaskId,///<消息包的task id
                                     CWX_UINT64 ullSession, ///<session
                                     CWX_UINT64 ullSid,  ///<数据开始发送时的sid
                                     char* szErr2K ///<pack出错时的错误信息
                                     );

///parse export的report回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_report_reply(struct CWX_PG_READER_EX* reader,///<reader
                                      char const* msg, ///<收到的消息包
                                      CWX_UINT32  msg_len, ///<收到的消息包的长度
                                      CWX_UINT64* ullSession,///<session
                                      CWX_UINT64* ullSid,///<数据开始发送时的sid
                                      char* szErr2K ///<解包时的错误信息
                                      );

///pack一条export的key/value的数据。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_data_item(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                                  CWX_KEY_VALUE_ITEM_S const* key, ///<key
                                  CWX_KEY_VALUE_ITEM_S const* data, ///<data
                                  CWX_KEY_VALUE_ITEM_S const* extra, ///<extra
                                  CWX_UINT32 version, ///<版本号
                                  CWX_UINT32 expire, ///<超时时间
                                  char* szErr2K ///<pack出错时的错误信息
                                  );

///parse一条export的key/value的数据返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_data_item(struct CWX_PG_READER_EX* reader,///<reader
                                   char const* msg, ///<key/value数据
                                   CWX_UINT32  msg_len, ///<key/value数据的长度
                                   CWX_KEY_VALUE_ITEM_S const** key, ///<数据的key
                                   CWX_KEY_VALUE_ITEM_S const** data, ///<数据的data
                                   CWX_KEY_VALUE_ITEM_S const** extra, ///<extra
                                   CWX_UINT32* version, ///<数据的版本
                                   CWX_UINT32* expire, ///<数据的超时
                                   char* szErr2K///<解包时的错误信息
                                   );

///pack以chunk组织的多条export的key/value的消息。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_multi_export_data(CWX_UINT32 uiTaskId, ///<消息包的task id
                                   char const* szData,  ///<多条key/value组成的数据package
                                   CWX_UINT32 uiDataLen, ///<数据的长度
                                   char* buf,  ///<pack的数据空间
                                   CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                   CWX_UINT64 ullSeq, ///<序列号
                                   char* szErr2K ///<pack出错时的错误信息
                                   );

///parse以chunk组织的多条export的key/value的数据。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_multi_export_data(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value数据
                                    CWX_UINT32  msg_len, ///<key/value数据的长度
                                    CWX_UINT64* ullSeq, ///<序列号
                                    char* szErr2K ///<解包时的错误信息
                                    );

///pack export数据的reply消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_data_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                   char* buf,  ///<pack的数据空间
                                   CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                   CWX_UINT32 uiTaskId, ///<消息包的task id
                                   CWX_UINT64 ullSeq, ///<序列号
                                   char* szErr2K ///<pack出错时的错误信息
                                   );

///parse export数据的reply消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value数据
                                    CWX_UINT32  msg_len, ///<key/value数据的长度
                                    CWX_UINT64* ullSeq, ///<序列号
                                    char* szErr2K ///<解包时的错误信息
                                    );


///pack export完成的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_end(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                            char* buf,  ///<pack的数据空间
                            CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                            CWX_UINT32 uiTaskId, ///<消息包的task id
                            CWX_UINT64 ullSid, ///<完成时的sid
                            char* szErr2K ///<pack出错时的错误信息
                            );

///parse export完成的消息包返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_end(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<key/value数据
                             CWX_UINT32  msg_len, ///<key/value数据的长度
                             CWX_UINT64* ullSid,///<完成时的sid
                             char* szErr2K ///<解包时的错误信息
                             );

///pack binlog sync的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_report_data(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                             char* buf,  ///<pack的数据空间
                             CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                             CWX_UINT32 uiTaskId, ///<消息包的task id
                             CWX_UINT64 ullSid, ///<开始的sid
                             CWX_UINT8  bNewly,  ///<是否从最新binlog开始同步
                             CWX_UINT32  uiChunkSize, ///<同步的chunk大小
                             char const* subscribe, ///<binlog订阅规则，空表示全部订阅
                             char const* user, ///<用户名
                             char const* passwd, ///<用户口令
                             char const* sign, ///<签名方式，空表示不签名
                             CWX_UINT8  zip, ///<是否压缩
                             char* szErr2K ///<pack出错时的错误信息
                             );

///parse binlog sync的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_report_data(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<key/value数据
                              CWX_UINT32  msg_len, ///<key/value数据的长度
                              CWX_UINT64* ullSid, ///<开始的sid
                              CWX_UINT8*  bNewly, ///<是否从最新binlog开始同步
                              CWX_UINT32*  uiChunkSize, ///<同步的chunk大小
                              char const** subscribe, ///<binlog订阅规则，空表示全部订阅
                              char const** user, ///<用户名
                              char const** passwd, ///<用户口令
                              char const** sign, ///<签名方式，空表示不签名
                              CWX_UINT8*   zip, ///<是否压缩
                              char* szErr2K ///<解包时的错误信息
                              );

///pack report的回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_report_data_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                   char* buf,  ///<pack的数据空间
                                   CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                   CWX_UINT32 uiTaskId, ///<消息包的task id
                                   CWX_UINT64 ullSession, ///<session id
                                   char* szErr2K ///<pack出错时的错误信息
                                   );

///parse report的回复数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_report_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value数据
                                    CWX_UINT32  msg_len, ///<key/value数据的长度
                                    CWX_UINT64* ullSession, ///<session id
                                    char* szErr2K ///<解包时的错误信息
                                    );

///pack sync的session连接报告消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_report_new_conn(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT32 uiTaskId, ///<消息包的task id
                                 CWX_UINT64 ullSession, ///<连接所属的session
                                 char* szErr2K ///<pack出错时的错误信息
                                 );

///parse sync的session连接报告数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_report_new_conn(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value数据
                                  CWX_UINT32  msg_len, ///<key/value数据的长度
                                  CWX_UINT64* ullSession, ///<连接所属的session
                                  char* szErr2K ///<解包时的错误信息
                                  );

///pack report或sync的出错消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_sync_err(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          int ret, ///<错误代码
                          char const* szErrMsg, ///<错误消息
                          char* szErr2K///<pack出错时的错误信息
                          );

///parse report或sync的出错数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_sync_err(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<key/value数据
                           CWX_UINT32  msg_len, ///<key/value数据的长度
                           int* ret,  ///<错误代码
                           char const** szErrMsg,  ///<错误消息
                           char* szErr2K ///<解包时的错误信息
                           );


///pack sync的一条binlog的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_sync_data(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                           char* buf,  ///<pack的数据空间
                           CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                           CWX_UINT32 uiTaskId, ///<消息包的task id
                           CWX_UINT64 ullSid, ///<binlog的sid
                           CWX_UINT32 uiTimeStamp, ///<binlog的时间戳
                           CWX_KEY_VALUE_ITEM_S const* data, ///<binlog的data
                           CWX_UINT32 group,  ///<binlog所属的分组
                           CWX_UINT32 type,   ///<binlog的类型，也就是消息类型
                           CWX_UINT32 version,  ///<对应的key的版本
                           CWX_UINT64 ullSeq,  ///<消息的序列号
                           char const* sign, ///<签名方式
                           CWX_UINT8   zip, ///<是否压缩
                           char* szErr2K///<pack出错时的错误信息
                           );

///pack 一条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_sync_data_item(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                CWX_UINT64 ullSid, ///<binlog的sid
                                CWX_UINT32 uiTimeStamp, ///<binlog的时间戳
                                CWX_KEY_VALUE_ITEM_S const* data, ///<binlog的data
                                CWX_UINT32 group,  ///<binlog所属的分组
                                CWX_UINT32 type,   ///<binlog的类型，也就是消息类型
                                CWX_UINT32 version,  ///<对应的key的版本
                                char const* sign, ///<签名方式
                                char* szErr2K///<pack出错时的错误信息
                                );

///pack 多条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_multi_sync_data(CWX_UINT32 uiTaskId, ///<任务id
                                 char const* szData, ///<多条消息的数据buf
                                 CWX_UINT32 uiDataLen, ///<多条数据的数据buf长度
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT64 ullSeq, ///<消息包的消息序列号
                                 CWX_UINT8  zip, ///<是否压缩
                                 char* szErr2K ///<pack出错时的错误信息
                                 );

///parse一条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_sync_data(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<key/value数据
                            CWX_UINT32  msg_len, ///<key/value数据的长度
                            CWX_UINT64* ullSid, ///<binlog的sid
                            CWX_UINT32* uiTimeStamp, ///<binlog的时间戳
                            CWX_KEY_VALUE_ITEM_S const** data, ///<binlog的数据
                            CWX_UINT32* group, ///<binlog所属的group
                            CWX_UINT32* type, ///<binlog对应的数据变更消息类型
                            CWX_UINT32* version, ///<binlog对应的数据变更的key的版本
                            char* szErr2K ///<解包时的错误信息
                            );

///pack sync binlog的回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_sync_data_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT32 uiTaskId, ///<消息包的task id
                                 CWX_UINT64 ullSeq, ///<消息的序列号
                                 CWX_UINT16 unMsgType, ///<消息类型
                                 char* szErr2K ///<pack出错时的错误信息
                                 );

///parse sync binlog的回复数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_sync_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value数据
                                  CWX_UINT32  msg_len, ///<key/value数据的长度
                                  CWX_UINT64* ullSeq, ///<消息的序列号
                                  char* szErr2K  ///<解包时的错误信息
                                  );

///设置数据同步包的seq号
void unistor_set_seq(char* szBuf, CWX_UINT64 ullSeq);

///获取数据同步包的seq号
CWX_UINT64 unistor_get_seq(char const* szBuf);

#ifdef __cplusplus
}
#endif

#endif
