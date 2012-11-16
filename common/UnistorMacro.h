#ifndef __UNISTOR_MACRO_H__
#define __UNISTOR_MACRO_H__


#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

CWINUX_USING_NAMESPACE


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

///驱动的对象创建symbol name
#define UNISTOR_ENGINE_CREATE_SYMBOL_NAME  "unistor_create_engine"

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


#define UNISTOR_TRANS_TIMEOUT_SECOND   5  ///<数据转发超时
#define UNISTOR_CONN_TIMEOUT_SECOND    3  ///<连接超时

#define UNISTOR_CLOCK_INTERANL_SECOND  1  ///<时钟间隔
#define UNISTOR_CHECK_ZK_LOCK_EXPIRE   10  ///<检测zk锁状况超时

#define UNISTOR_MAX_DATA_SIZE              2 * 1024 * 1024 ///<最大的DATA大小
#define UNISTOR_MAX_KEY_SIZE               1024            ///<最大的key的大小
#define UNISTOR_MAX_KV_SIZE				  (2 * 1024 * 1024 +  8 * 1024) ///<最大的data size
#define UNISTOR_DEF_KV_SIZE                (64 * 1024) ///<缺省的存储大小
#define UNISTOR_MAX_KVS_SIZE              (20 * 1024 * 1024) ///<最大返回数据包大小为20M
#define UNISTOR_MAX_CHUNK_KSIZE         (20 * 1024) ///<最大的chunk size
#define UNISTOR_ZIP_EXTRA_BUF           128
#define UNISTOR_DEF_LIST_NUM             50    ///<list的缺省数量
#define UNISTOR_MAX_LIST_NUM             1000   ///<list的最大数量
#define UNISTOR_MAX_BINLOG_FLUSH_COUNT  10000 ///<服务启动时，最大的skip sid数量
#define UNISTOR_KEY_START_VERION        1 ///<key的起始版本号
#define UNISTOR_MAX_GETS_KEY_NUM        1024 ///<最大的mget的key的数量

#define  UNISTOR_WRITE_CACHE_MBYTE          128  ///<128M的写cache
#define  UNISTOR_WRITE_CACHE_KEY_NUM        10000 ///<cache的最大记录数

#define  UNISTOR_DEF_SOCK_BUF_KB  64   ///<缺省的socket buf，单位为KB
#define  UNISTOR_MIN_SOCK_BUF_KB  4    ///<最小的socket buf，单位为KB
#define  UNISTOR_MAX_SOCK_BUF_KB  (8 * 1024) ///<最大的socket buf，单位为KB
#define  UNISTOR_DEF_CHUNK_SIZE_KB 64   ///<缺省的chunk大小，单位为KB
#define  UNISTOR_MIN_CHUNK_SIZE_KB  4   ///<最小的chunk大小，单位为KB
#define  UNISTOR_MAX_CHUNK_SIZE_KB  UNISTOR_MAX_CHUNK_KSIZE ///<最大的chunk大小，单位为KB
#define  UNISTOR_DEF_WRITE_CACHE_FLUSH_NUM  1000  ///<unistor存储触发flush的缺省数据变更数量
#define  UNISTOR_MIN_WRITE_CACHE_FLUSH_NUM  1     ///<unistor存储触发flush的最小数据变更数量
#define  UNISTOR_MAX_WRITE_CACHE_FLUSH_NUM  500000 ///<unistor存储触发flush的最大数据变更数量
#define  UNISTOR_DEF_WRITE_CACHE_FLUSH_SECOND  60  ///<unistor存储触发flush的缺省时间
#define  UNISTOR_MIN_WRITE_CACHE_FLUSH_SECOND  1     ///<unistor存储触发flush的最小时间
#define  UNISTOR_MAX_WRITE_CACHE_FLUSH_SECOND  1800  ///<unistor存储触发flush的最大时间

#define  UNISTOR_DEF_CONN_NUM  10            ///<数据同步缺省的并发连接数量
#define  UNISTOR_MIN_CONN_NUM  1            ///<数据同步最小的并发连接数量
#define  UNISTOR_MAX_CONN_NUM  256          ///<数据同步最大的并发连接数量 
#define  UNISTOR_DEF_EXPIRE_CONNCURRENT 32  ///<超时检查的缺省并发消息数量
#define  UNISTOR_MIN_EXPIRE_CONNCURRENT 1   ///<超时检查的最小并发消息数量
#define  UNISTOR_MAX_EXPIRE_CONNCURRENT 256 ///<超时检查的最大并发消息数量
#define  UNISTOR_CHECKOUT_INTERNAL   5      ///<存储引擎的checkpoint的间隔
#define  UNISTOR_PER_FETCH_EXPIRE_KEY_NUM 1024  ///<超时检查的一次获取key的数量
#define  UNISTOR_EXPORT_CONTINUE_SEEK_NUM  4096  ///<数据导出一次遍历的key的数量

#define  UNISTOR_DEF_MAX_WRITE_QUEUE_MSG_NUM  50000 ///<写队列缺省最大消息的数量
#define  UNISTOR_MIN_MAX_WRITE_QUEUE_MSG_NUM  5000  ///<写队列最小最大消息的数量
#define  UNISTOR_MAX_MAX_WRITE_QUEUE_MSG_NUM  500000 ///<写队列最大缺省消息的数量

#define  UNISTOR_DEF_MAX_MASTER_TRAN_MSG_NUM  25000 ///<写队列缺省最大消息的数量
#define  UNISTOR_MIN_MAX_MASTER_TRAN_MSG_NUM  25000  ///<写队列最小最大消息的数量
#define  UNISTOR_MAX_MAX_MASTER_TRAN_MSG_NUM  250000 ///<写队列最大缺省消息的数量




#define  UNISTOR_REPORT_TIMEOUT  30  ///<report的超时时间
#define  UNISTOR_TRAN_AUTH_TIMEOUT 30  ///<转发认证的超时时间

#define  UNISTOR_MASTER_SWITCH_SID_INC  1000000  ///<unistor发送master切换，新master跳跃的sid的值



///获取系统信息的函数。-1：失败；0：不存在；1：获取成功
typedef int (*UNISTOR_GET_SYS_INFO_FN)(void* pApp, ///<app对象
                                      char const* key, ///<要获取的key
                                      CWX_UINT16 unKeyLen, ///<key的长度
                                      char* szData, ///<若存在，则返回数据。内存有存储引擎分配
                                      CWX_UINT32& uiLen  ///<szData数据的字节数
                                      );
///开始写的函数，返回值：0，成功；-1：失败
typedef int (*UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN)(void* context, char* szErr2K);
///写数据，返回值：0，成功；-1：失败
typedef int (*UNISTOR_WRITE_CACHE_WRITE_WRITE_FN)(void* context, char const* szKey, CWX_UINT16 unKeyLen, char const* szData, CWX_UINT32 uiDataLen, bool bDel, CWX_UINT32 ttOldExpire, char* szStoreKeyBuf, CWX_UINT16 unKeyBufLen, char* szErr2K);
///提交数据，返回值：0，成功；-1：失败
typedef int (*UNISTOR_WRITE_CACHE_WRITE_END_FN)(void* context, CWX_UINT64 ullSid, void* userData, char* szErr2K);
///key的相等比较函数。true：相等；false：不相等
typedef bool (*UNISTOR_KEY_CMP_EQUAL_FN)(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len);
///key的less比较函数。0：相等；-1：小于；1：大于
typedef int (*UNISTOR_KEY_CMP_LESS_FN)(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len);
///key的hash函数。
typedef size_t (*UNISTOR_KEY_HASH_FN)(char const* key1, CWX_UINT16 unKey1Len);
///key的group函数。
typedef CWX_UINT32 (*UNISTOR_KEY_GROUP_FN)(char const* key1, CWX_UINT16 unKey1Len);

#endif
