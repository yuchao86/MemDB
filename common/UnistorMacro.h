#ifndef __UNISTOR_MACRO_H__
#define __UNISTOR_MACRO_H__


#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

CWINUX_USING_NAMESPACE


///ͨ�ŵ�key����
#define UNISTOR_KEY_ASC    "asc"   ///<����
#define UNISTOR_KEY_BEGIN      "begin" ///<��ʼ 
#define UNISTOR_KEY_C       "c"  ///<cache
#define UNISTOR_KEY_CHUNK "chunk"  ///<���ݷ��͵�chunk��key
#define UNISTOR_KEY_CRC32  "crc32" ///<crc32ǩ��
#define UNISTOR_KEY_END    "end"   ///<����ֵ��key
#define UNISTOR_KEY_ERR  "err"     ///error��key
#define UNISTOR_KEY_E "e"          ///<expire��key
#define UNISTOR_KEY_D "d"           ///<���ݵ�data��key
#define UNISTOR_KEY_F  "f"          ///<field
#define UNISTOR_KEY_FN "fn"         ///<field number
#define UNISTOR_KEY_G   "g"         ///<group,Ϊkey�ķ���
#define UNISTOR_KEY_I "i"           ///<information
#define UNISTOR_KEY_IB "ib"         ///<isbegin
#define UNISTOR_KEY_K "k"           ///<���ݵ�key��key
#define UNISTOR_KEY_M      "m"      ///<message��key
#define UNISTOR_KEY_MD5    "md5"    ///<md5ǩ����key
#define UNISTOR_KEY_MAX    "max"    ///<���ֵ
#define UNISTOR_KEY_MIN    "min"    ///<��Сֵ
#define UNISTOR_KEY_N "n"           ///<number
#define UNISTOR_KEY_P "p"            ///<password
#define UNISTOR_KEY_R "r"            ///<inc��resultֵ������inc��sync��ʱ�����
#define UNISTOR_KEY_RET  "ret"       ///<����ֵ��ret
#define UNISTOR_KEY_SEQ   "seq"      ///<����ͬ������Ϣ���к�
#define UNISTOR_KEY_SESSION "session" ///<����ͬ����session
#define UNISTOR_KEY_SID "sid"        ///<���ݱ����sid
#define UNISTOR_KEY_SIGN   "sign"    ///<���ݸ��µ�sign
#define UNISTOR_KEY_SUBSCRIBE  "subscribe" ///<���ĵ�key
#define UNISTOR_KEY_T "t"           ///<timestamp
#define UNISTOR_KEY_TYPE   "type"   ///<binlog�����ͣ�Ҳ�������ݸ��µ���Ϣ����
#define UNISTOR_KEY_U    "u"       ///<user
#define UNISTOR_KEY_ZIP    "zip"  ///<ѹ����ʾ
#define UNISTOR_KEY_V      "v"   ///<version key
#define UNISTOR_KEY_X      "x"  ///<������չkey


#define UNISTOR_SYS_KEY_PID "pid"  ///<����ID
#define UNISTOR_SYS_KEY_PPID "ppid" ///<������ID
#define UNISTOR_SYS_KEY_VERSION "version"  ///<�汾��
#define UNISTOR_SYS_KEY_MODIFY  "modify" ///<�����޸�ʱ��
#define UNISTOR_SYS_KEY_COMPILE "compile" ///<�������ʱ��
#define UNISTOR_SYS_KEY_START   "start"   ///<��������ʱ��
#define UNISTOR_SYS_KEY_ENGINE  "engine"  ///<�洢��������
#define UNISTOR_SYS_KEY_ENGINE_VERSION "engine_version" ///<�洢����İ汾
#define UNISTOR_SYS_KEY_ENGINE_STATE   "engine_state" ///<�洢�����״̬
#define UNISTOR_SYS_KEY_ENGINE_ERROR   "engine_error" ///<�洢����Ĵ�����Ϣ
#define UNISTOR_SYS_KEY_BINLOG_STATE   "binlog_state" ///<binlog��״̬
#define UNISTOR_SYS_KEY_BINLOG_ERROR   "binlog_error" ///<binlog�Ĵ�����Ϣ
#define UNISTOR_SYS_KEY_BINLOG_MIN_SID       "binlog_min_sid"   ///<binlog����С��sidֵ
#define UNISTOR_SYS_KEY_BINLOG_MIN_TIMESTAMP "binlog_min_timestamp" ///<binlog����Сʱ���
#define UNISTOR_SYS_KEY_BINLOG_MIN_FILE     "binlog_min_file" ///<binlog����С�ļ�
#define UNISTOR_SYS_KEY_BINLOG_MAX_SID  "binlog_max_sid" ///binlog�����sid
#define UNISTOR_SYS_KEY_BINLOG_MAX_TIMESTAMP "binlog_max_timestamp" ///<binlog�����ʱ���
#define UNISTOR_SYS_KEY_BINLOG_MAX_FILE "binlog_max_file" ///<binlog������ļ�
#define UNISTOR_SYS_KEY_READ_THREAD_NUM  "read_thread_num" ///<���̵߳�����
#define UNISTOR_SYS_KEY_READ_THREAD_QUEUE "read_thread_queue" ///<�̵߳Ķ��е���Ϣ����
#define UNISTOR_SYS_KEY_READ_THREAD_CONNECT "read_thread_connect" ///<���̵߳���������
#define UNISTOR_SYS_KEY_READ_THREAD_QUEUE_PREX "read_thread_queue_" ///<�̵߳Ķ���ǰ׺
#define UNISTOR_SYS_KEY_READ_THREAD_CONNECT_PREX "read_thread_connect_" ///<���̵߳�����ǰ׺
#define UNISTOR_SYS_KEY_WRITE_THREAD_QUEUE "write_thread_queue" ///<д�̵߳Ķ���
#define UNISTOR_SYS_KEY_TRANS_THREAD_QUEUE "trans_thread_queue" ///<ת���̶߳���
#define UNISTOR_SYS_KEY_CHECKPOINT_THREAD_QUEUE "checkpoint_thread_queue" ///<checkpoint�̵߳Ķ���
#define UNISTOR_SYS_KEY_ZK_THREAD_QUEUE  "zk_thread_queue" ///<zookeeper�̵߳Ķ���
#define UNISTOR_SYS_KEY_INNER_SYNC_THREAD_QUEUE "inner_sync_thread_queue" ///<�ڲ�ͬ�����̶߳���
#define UNISTOR_SYS_KEY_OUTER_SYNC_THREAD_QUEUE "outer_sync_thread_queue" ///<�ⲿͬ�����̶߳���
#define UNISTOR_SYS_KEY_MASTER_TRANS_MSG_NUM           "master_trans_msg_num" ///<��ǰת����Ϣ������
#define UNISTOR_SYS_KEY_ZK_STATE "zk_state"  ///<zookeeper������״̬
#define UNISTOR_SYS_KEY_ZK_ERROR "zk_error"  ///<zookeeper�Ĵ�����Ϣ
#define UNISTOR_SYS_KEY_CACHE_STATE "cache_state" ///<cache��״̬
#define UNISTOR_SYS_KEY_CACHE_ERR   "cache_error" ///<cache�Ĵ�����Ϣ
#define UNISTOR_SYS_KEY_WRITE_CACHE_KEY "write_cache_key" ///<дcache��key������
#define UNISTOR_SYS_KEY_WRITE_CACHE_SPACE "write_cache_space" ///<дcache�����ݴ�С
#define UNISTOR_SYS_KEY_READ_CACHE_MAX_SIZE "read_cache_max_size" ///<��cache������С
#define UNISTOR_SYS_KEY_READ_CACHE_MAX_KEY  "read_cache_max_key" ///<��cache�����key������
#define UNISTOR_SYS_KEY_READ_CACHE_USED_SIZE    "read_cache_used_size" ///<��cacheռ�õĿռ��С
#define UNISTOR_SYS_KEY_READ_CACHE_USED_CAPACITY "read_cache_used_capacity" ///<��cacheռ�õ����ݵ�����
#define UNISTOR_SYS_KEY_READ_CACHE_USED_DATA_SIZE "read_cache_used_data_size" ///<��cache��ʵ�����ݴ�С
#define UNISTOR_SYS_KEY_READ_CACHE_FREE_SIZE      "read_cache_free_size" ///<��cache���еĿռ��С
#define UNISTOR_SYS_KEY_READ_CACHE_FREE_CAPACITY  "read_cache_free_capacity" ///<��cache���е�����
#define UNISTOR_SYS_KEY_READ_CACHE_KEY       "read_cache_key" ///<cache��key������
#define UNISTOR_SYS_KEY_GET_NUM                   "get_num" ///<get�ķ�������
#define UNISTOR_SYS_KEY_GET_READ_CACHE_NUM        "get_read_cache_num" ///<read cache������
#define UNISTOR_SYS_KEY_GET_EXIST_NUM             "get_exist_num" ///<���ڵ�����
#define UNISTOR_SYS_KEY_GETS_NUM                  "gets_num" ///<gets�ķ�������
#define UNISTOR_SYS_KEY_GETS_KEY_NUM              "gets_key_num" ///<gets��key������
#define UNISTOR_SYS_KEY_GETS_KEY_READ_CACHE_NUM   "gets_key_read_cache_num" ///<gets ��keyʹ��cache������
#define UNISTOR_SYS_KEY_GETS_KEY_EXIST_NUM        "gets_key_exist_num" ///<gets��key���ڵ�����
#define UNISTOR_SYS_KEY_LIST_NUM                  "list_num" ///<list���ʵ�����
#define UNISTOR_SYS_KEY_EXIST_NUM                 "exist_num" ///<exist�ķ�������
#define UNISTOR_SYS_KEY_EXIST_READ_CACHE_NUM      "exist_read_cache_num" ///<exist��read cache������
#define UNISTOR_SYS_KEY_EXIST_EXIST_NUM           "exist_exist_num" ///<exist�Ĵ�������
#define UNISTOR_SYS_KEY_ADD_NUM                   "add_num" ///<add������
#define UNISTOR_SYS_KEY_ADD_READ_CACHE_NUM        "add_read_cache_num"//<add��read cache����
#define UNISTOR_SYS_KEY_ADD_WRITE_CACHE_NUM       "add_write_cache_num" ///<add��write cache����
#define UNISTOR_SYS_KEY_SET_NUM                   "set_num" ///<set������
#define UNISTOR_SYS_KEY_SET_READ_CACHE_NUM        "set_read_cache_num"//<set��read cache����
#define UNISTOR_SYS_KEY_SET_WRITE_CACHE_NUM       "set_write_cache_num" ///<set��write cache����
#define UNISTOR_SYS_KEY_UPDATE_NUM                "update_num" ///<update������
#define UNISTOR_SYS_KEY_UPDATE_READ_CACHE_NUM     "update_read_cache_num"//<update��read cache����
#define UNISTOR_SYS_KEY_UPDATE_WRITE_CACHE_NUM    "update_write_cache_num" ///<update��write cache����
#define UNISTOR_SYS_KEY_INC_NUM                   "inc_num" ///<inc������
#define UNISTOR_SYS_KEY_INC_READ_CACHE_NUM        "inc_read_cache_num"//<inc��read cache����
#define UNISTOR_SYS_KEY_INC_WRITE_CACHE_NUM       "inc_write_cache_num" ///<inc��write cache����
#define UNISTOR_SYS_KEY_DEL_NUM                   "del_num" ///<del������
#define UNISTOR_SYS_KEY_DEL_READ_CACHE_NUM        "del_read_cache_num"//<del��read cache����
#define UNISTOR_SYS_KEY_DEL_WRITE_CACHE_NUM       "del_write_cache_num" ///<del��write cache����
#define UNISTOR_SYS_KEY_IMPORT_NUM                "import_num" ///<del������
#define UNISTOR_SYS_KEY_IMPORT_READ_CACHE_NUM     "import_read_cache_num"//<del��read cache����
#define UNISTOR_SYS_KEY_IMPORT_WRITE_CACHE_NUM    "import_write_cache_num" ///<del��write cache����
#define UNISTOR_SYS_KEY_ENGINE_INFO               "engine_info"   ///<engine��˵����Ϣ
#define UNISTOR_SYS_KEY_ENGINE_SYS_KEY            "engine_sys_key" ///<engine��ϵͳkey

///�����Ķ��󴴽�symbol name
#define UNISTOR_ENGINE_CREATE_SYMBOL_NAME  "unistor_create_engine"

///������붨�壬0~100��ϵͳ���Ĵ���
#define UNISTOR_ERR_SUCCESS          0  ///<�ɹ�
#define UNISTOR_ERR_ERROR            1 ///<�������ֵ���ͳ����
#define UNISTOR_ERR_FAIL_AUTH        2 ///<��Ȩʧ��
#define UNISTOR_ERR_LOST_SYNC        3 ///<ʧȥ��ͬ��״̬
#define UNISTOR_ERR_NO_MASTER        4 ///<û��master
#define UNISTOR_ERR_TOO_MANY_WRITE   5 ///<̫���д
#define UNISTOR_ERR_TOO_MANY_TRANS   6 ///<̫���ת����Ϣ
//100������Ӧ�ü��Ĵ���
#define UNISTOR_ERR_NEXIST            101   ///<������
#define UNISTOR_ERR_EXIST             102   ///<����
#define UNISTOR_ERR_VERSION           103   ///<�汾����
#define UNISTOR_ERR_OUTRANGE          104   ///<inc������Χ
#define UNISTOR_ERR_TIMEOUT           105   ///<��ʱ


#define UNISTOR_TRANS_TIMEOUT_SECOND   5  ///<����ת����ʱ
#define UNISTOR_CONN_TIMEOUT_SECOND    3  ///<���ӳ�ʱ

#define UNISTOR_CLOCK_INTERANL_SECOND  1  ///<ʱ�Ӽ��
#define UNISTOR_CHECK_ZK_LOCK_EXPIRE   10  ///<���zk��״����ʱ

#define UNISTOR_MAX_DATA_SIZE              2 * 1024 * 1024 ///<����DATA��С
#define UNISTOR_MAX_KEY_SIZE               1024            ///<����key�Ĵ�С
#define UNISTOR_MAX_KV_SIZE				  (2 * 1024 * 1024 +  8 * 1024) ///<����data size
#define UNISTOR_DEF_KV_SIZE                (64 * 1024) ///<ȱʡ�Ĵ洢��С
#define UNISTOR_MAX_KVS_SIZE              (20 * 1024 * 1024) ///<��󷵻����ݰ���СΪ20M
#define UNISTOR_MAX_CHUNK_KSIZE         (20 * 1024) ///<����chunk size
#define UNISTOR_ZIP_EXTRA_BUF           128
#define UNISTOR_DEF_LIST_NUM             50    ///<list��ȱʡ����
#define UNISTOR_MAX_LIST_NUM             1000   ///<list���������
#define UNISTOR_MAX_BINLOG_FLUSH_COUNT  10000 ///<��������ʱ������skip sid����
#define UNISTOR_KEY_START_VERION        1 ///<key����ʼ�汾��
#define UNISTOR_MAX_GETS_KEY_NUM        1024 ///<����mget��key������

#define  UNISTOR_WRITE_CACHE_MBYTE          128  ///<128M��дcache
#define  UNISTOR_WRITE_CACHE_KEY_NUM        10000 ///<cache������¼��

#define  UNISTOR_DEF_SOCK_BUF_KB  64   ///<ȱʡ��socket buf����λΪKB
#define  UNISTOR_MIN_SOCK_BUF_KB  4    ///<��С��socket buf����λΪKB
#define  UNISTOR_MAX_SOCK_BUF_KB  (8 * 1024) ///<����socket buf����λΪKB
#define  UNISTOR_DEF_CHUNK_SIZE_KB 64   ///<ȱʡ��chunk��С����λΪKB
#define  UNISTOR_MIN_CHUNK_SIZE_KB  4   ///<��С��chunk��С����λΪKB
#define  UNISTOR_MAX_CHUNK_SIZE_KB  UNISTOR_MAX_CHUNK_KSIZE ///<����chunk��С����λΪKB
#define  UNISTOR_DEF_WRITE_CACHE_FLUSH_NUM  1000  ///<unistor�洢����flush��ȱʡ���ݱ������
#define  UNISTOR_MIN_WRITE_CACHE_FLUSH_NUM  1     ///<unistor�洢����flush����С���ݱ������
#define  UNISTOR_MAX_WRITE_CACHE_FLUSH_NUM  500000 ///<unistor�洢����flush��������ݱ������
#define  UNISTOR_DEF_WRITE_CACHE_FLUSH_SECOND  60  ///<unistor�洢����flush��ȱʡʱ��
#define  UNISTOR_MIN_WRITE_CACHE_FLUSH_SECOND  1     ///<unistor�洢����flush����Сʱ��
#define  UNISTOR_MAX_WRITE_CACHE_FLUSH_SECOND  1800  ///<unistor�洢����flush�����ʱ��

#define  UNISTOR_DEF_CONN_NUM  10            ///<����ͬ��ȱʡ�Ĳ�����������
#define  UNISTOR_MIN_CONN_NUM  1            ///<����ͬ����С�Ĳ�����������
#define  UNISTOR_MAX_CONN_NUM  256          ///<����ͬ�����Ĳ����������� 
#define  UNISTOR_DEF_EXPIRE_CONNCURRENT 32  ///<��ʱ����ȱʡ������Ϣ����
#define  UNISTOR_MIN_EXPIRE_CONNCURRENT 1   ///<��ʱ������С������Ϣ����
#define  UNISTOR_MAX_EXPIRE_CONNCURRENT 256 ///<��ʱ������󲢷���Ϣ����
#define  UNISTOR_CHECKOUT_INTERNAL   5      ///<�洢�����checkpoint�ļ��
#define  UNISTOR_PER_FETCH_EXPIRE_KEY_NUM 1024  ///<��ʱ����һ�λ�ȡkey������
#define  UNISTOR_EXPORT_CONTINUE_SEEK_NUM  4096  ///<���ݵ���һ�α�����key������

#define  UNISTOR_DEF_MAX_WRITE_QUEUE_MSG_NUM  50000 ///<д����ȱʡ�����Ϣ������
#define  UNISTOR_MIN_MAX_WRITE_QUEUE_MSG_NUM  5000  ///<д������С�����Ϣ������
#define  UNISTOR_MAX_MAX_WRITE_QUEUE_MSG_NUM  500000 ///<д�������ȱʡ��Ϣ������

#define  UNISTOR_DEF_MAX_MASTER_TRAN_MSG_NUM  25000 ///<д����ȱʡ�����Ϣ������
#define  UNISTOR_MIN_MAX_MASTER_TRAN_MSG_NUM  25000  ///<д������С�����Ϣ������
#define  UNISTOR_MAX_MAX_MASTER_TRAN_MSG_NUM  250000 ///<д�������ȱʡ��Ϣ������




#define  UNISTOR_REPORT_TIMEOUT  30  ///<report�ĳ�ʱʱ��
#define  UNISTOR_TRAN_AUTH_TIMEOUT 30  ///<ת����֤�ĳ�ʱʱ��

#define  UNISTOR_MASTER_SWITCH_SID_INC  1000000  ///<unistor����master�л�����master��Ծ��sid��ֵ



///��ȡϵͳ��Ϣ�ĺ�����-1��ʧ�ܣ�0�������ڣ�1����ȡ�ɹ�
typedef int (*UNISTOR_GET_SYS_INFO_FN)(void* pApp, ///<app����
                                      char const* key, ///<Ҫ��ȡ��key
                                      CWX_UINT16 unKeyLen, ///<key�ĳ���
                                      char* szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
                                      CWX_UINT32& uiLen  ///<szData���ݵ��ֽ���
                                      );
///��ʼд�ĺ���������ֵ��0���ɹ���-1��ʧ��
typedef int (*UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN)(void* context, char* szErr2K);
///д���ݣ�����ֵ��0���ɹ���-1��ʧ��
typedef int (*UNISTOR_WRITE_CACHE_WRITE_WRITE_FN)(void* context, char const* szKey, CWX_UINT16 unKeyLen, char const* szData, CWX_UINT32 uiDataLen, bool bDel, CWX_UINT32 ttOldExpire, char* szStoreKeyBuf, CWX_UINT16 unKeyBufLen, char* szErr2K);
///�ύ���ݣ�����ֵ��0���ɹ���-1��ʧ��
typedef int (*UNISTOR_WRITE_CACHE_WRITE_END_FN)(void* context, CWX_UINT64 ullSid, void* userData, char* szErr2K);
///key����ȱȽϺ�����true����ȣ�false�������
typedef bool (*UNISTOR_KEY_CMP_EQUAL_FN)(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len);
///key��less�ȽϺ�����0����ȣ�-1��С�ڣ�1������
typedef int (*UNISTOR_KEY_CMP_LESS_FN)(char const* key1, CWX_UINT16 unKey1Len, char const* key2, CWX_UINT16 unKey2Len);
///key��hash������
typedef size_t (*UNISTOR_KEY_HASH_FN)(char const* key1, CWX_UINT16 unKey1Len);
///key��group������
typedef CWX_UINT32 (*UNISTOR_KEY_GROUP_FN)(char const* key1, CWX_UINT16 unKey1Len);

#endif
