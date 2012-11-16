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

    ///����MASTER��־λ
#define  UNISTOR_MASTER_ATTR_BIT  0x01

    ///Э�����Ϣ���Ͷ���
    ///RECV�������͵���Ϣ���Ͷ���
#define UNISTOR_MSG_TYPE_TIMESTAMP             0   ///<ʱ����Ϣ����
#define UNISTOR_MSG_TYPE_RECV_ADD              1   ///<ADD key/value
#define UNISTOR_MSG_TYPE_RECV_ADD_REPLY        2   ///<set key/value�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_SET              3   ///<SET key
#define UNISTOR_MSG_TYPE_RECV_SET_REPLY        4   ///<SET key�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_UPDATE           5   ///update key
#define UNISTOR_MSG_TYPE_RECV_UPDATE_REPLY     6   ///<update key�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_INC              7   ///<inc key
#define UNISTOR_MSG_TYPE_RECV_INC_REPLY        8   ///<inc key�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_DEL              9   ///<del key
#define UNISTOR_MSG_TYPE_RECV_DEL_REPLY        10  ///<del key�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_EXIST            11  ///<Key�Ƿ����
#define UNISTOR_MSG_TYPE_RECV_EXIST_REPLY      12  ///<Key�Ƿ���ڵĻظ�
#define UNISTOR_MSG_TYPE_RECV_GET              13  ///<get key
#define UNISTOR_MSG_TYPE_RECV_GET_REPLY        14  ///<get key�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_GETS             15  ///<get ���key
#define UNISTOR_MSG_TYPE_RECV_GETS_REPLY       16  ///<get ���key�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_LIST             17  ///<��ȡ�б�
#define UNISTOR_MSG_TYPE_RECV_LIST_REPLY       18  ///<�б�ظ�
#define UNISTOR_MSG_TYPE_RECV_IMPORT           19  ///<import����
#define UNISTOR_MSG_TYPE_RECV_IMPORT_REPLY     20  ///<import�Ļظ�
#define UNISTOR_MSG_TYPE_RECV_AUTH             21  ///<��֤
#define UNISTOR_MSG_TYPE_RECV_AUTH_REPLY       22  ///<��֤�ظ�

    ///���ݵ�����������
#define UNISTOR_MSG_TYPE_EXPORT_REPORT         51  ///<���ݵ���export
#define UNISTOR_MSG_TYPE_EXPORT_REPORT_REPLY   52  ///<���ݵ�����reply
#define UNISTOR_MSG_TYPE_EXPORT_DATA           53  ///<���ݵ���������
#define UNISTOR_MSG_TYPE_EXPORT_DATA_REPLY     54  ///<���ݵ���������reply
#define UNISTOR_MSG_TYPE_EXPORT_END            55  ///<���ݵ������

    ///�ַ�����Ϣ���Ͷ���
#define UNISTOR_MSG_TYPE_SYNC_REPORT           101 ///<ͬ��SID�㱨����Ϣ����
#define UNISTOR_MSG_TYPE_SYNC_REPORT_REPLY     102 ///<report����
#define UNISTOR_MSG_TYPE_SYNC_CONN             103 ///<����ͨ��
#define UNISTOR_MSG_TYPE_SYNC_CONN_REPLY       104 ///<����ͨ���ظ�
#define UNISTOR_MSG_TYPE_SYNC_DATA             105 ///<��������
#define UNISTOR_MSG_TYPE_SYNC_DATA_REPLY       106 ///<���ݵĻظ�
#define UNISTOR_MSG_TYPE_SYNC_DATA_CHUNK       107 ///<chunkģʽ��������
#define UNISTOR_MSG_TYPE_SYNC_DATA_CHUNK_REPLY 108 ///<chunkģʽ�������ݵĻظ�
#define UNISTOR_MSG_TYPE_SYNC_ERR              109 ///<sync�Ĵ�����Ϣ


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

///ϵͳ��Ϣ��key����
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

#define UNISTOR_DEF_CHUNK_SIZE_KB     64    ///<ȱʡ��chunk��С
#define UNISTOR_ZIP_EXTRA_BUF           128

/*****************************************************************
�ӿ�API����
******************************************************************/

///���ô�master��ȡ������λ
CWX_UINT8 unistor_set_from_master(CWX_UINT8* ucAttr);

///check�Ƿ������˴�master��ȡ������λ��0�����ǣ�>0����
CWX_UINT8 isFromMaster(CWX_UINT8 ucAttr);

///�����master��ȡ������λ
CWX_UINT8 clearFromMaster(CWX_UINT8* ucAttr);

///pack import key�����ݡ� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_import(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer������ͨ��writer����
                             char* buf,  ///<pack�����ݿռ�
                             CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                             CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiExpire,  ///<��ʱʱ�䣬��Ϊ0�����
                             CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                             CWX_UINT8   bCache, ///<�Ƿ�cache����Ϊtrue�����
                             char const* user,  ///<�û�����ΪNULL�����
                             char const* passwd, ///<�û������ΪNULL�����
                             char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
                             );

///����Add key�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_import(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<�յ�����Ϣ��
                              CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                              CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                              CWX_KEY_VALUE_ITEM_S const** data,  ///<����data�ֶ�
                              CWX_UINT32* uiExpire,  ///<����expire����Ϊ0��ʾû��ָ��
                              CWX_UINT32* uiVersion, ///<���ذ汾
                              CWX_UINT8*       bCache,    ///<����cache
                              char const** user,     ///<�����û���NULL��ʾ������
                              char const** passwd,   ///<���ؿ��NULL��ʾ������
                              char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                              );

///pack Add key�����ݡ� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_add(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer������ͨ��writer����
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                          CWX_KEY_VALUE_ITEM_S const* data, ///<data
                          CWX_UINT32 uiExpire,  ///<��ʱʱ�䣬��Ϊ0�����
                          CWX_UINT32 uiSign,    ///<��ǣ���Ϊ0�����
                          CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                          CWX_UINT8       bCache, ///<�Ƿ�cache����Ϊtrue�����
                          char const* user,  ///<�û�����ΪNULL�����
                          char const* passwd, ///<�û������ΪNULL�����
                          char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
                          );

///����Add key�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_add(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<�յ�����Ϣ��
                           CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** data,  ///<����data�ֶ�
                           CWX_UINT32* uiExpire,  ///<����expire����Ϊ0��ʾû��ָ��
                           CWX_UINT32* uiSign,    ///<����sign
                           CWX_UINT32* uiVersion, ///<���ذ汾
                           CWX_UINT8*  bCache,    ///<����cache
                           char const** user,     ///<�����û���NULL��ʾ������
                           char const** passwd,   ///<���ؿ��NULL��ʾ������
                           char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                           );

///pack set�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_set(struct CWX_PG_WRITER_EX* writer,///<����pack��writer������ͨ��writer����
                char* buf,  ///<pack�����ݿռ�
                CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                CWX_KEY_VALUE_ITEM_S const* key, ///<key
                CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                CWX_KEY_VALUE_ITEM_S const* data, ///<data
                CWX_UINT32 uiSign, ///<��ǣ���Ϊ0�����
                CWX_UINT32 uiExpire, ///<��ʱʱ�䣬��Ϊ0�����
                CWX_UINT32 uiVersion,///<�汾����Ϊ0�����
                CWX_UINT8  bCache, ///<�Ƿ�cache����Ϊtrue�����
                char const* user, ///<�û�����ΪNULL�����
                char const* passwd,///<�û������ΪNULL�����
                char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                );

///parse set�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_set(struct CWX_PG_READER_EX* reader,  ///<reader
                           char const* msg, ///<�յ�����Ϣ��
                           CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key, ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** data, ///<����data�ֶ�
                           CWX_UINT32* uiSign, ///<����sign
                           CWX_UINT32* uiExpire, ///<����expire
                           CWX_UINT32* uiVersion, ///<���ذ汾
                           CWX_UINT8*   bCache,  ///<����cache
                           char const** user, ///<�����û���NULL��ʾ������
                           char const** passwd, ///<���ؿ��NULL��ʾ������
                           char* szErr2K  ///<���ʱ�Ĵ�����Ϣ
                           );

///pack update�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_update(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                             char* buf,  ///<pack�����ݿռ�
                             CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                             CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiSign, ///<��ǣ���Ϊ0�����
                             CWX_UINT32 uiExpire, ///<��ʱʱ�䣬��Ϊ0�����
                             CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                             char const* user, ///<�û�����ΪNULL�����
                             char const* passwd, ///<�û������ΪNULL�����
                             char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                             );

///parse update�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_update(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<�յ�����Ϣ��
                              CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                              CWX_KEY_VALUE_ITEM_S const** key, ///<����key�ֶ�
                              CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                              CWX_KEY_VALUE_ITEM_S const** data, ///<����data�ֶ�
                              CWX_UINT32* uiSign, ///<����sign
                              CWX_UINT32* uiExpire, ///<����expire����Ϊ0��ʾû��ָ��
                              CWX_UINT32* uiVersion, ///<���ذ汾
                              char const** user,     ///<�����û���NULL��ʾ������
                              char const** passwd,   ///<���ؿ��NULL��ʾ������
                              char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                              );

///pack inc�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_inc(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                          CWX_INT64   num, ///<inc�����֣������ɸ�
                          CWX_INT64   result, ///<����Ľ������Ϊ0�����,�˼�¼���յļ�������
                          CWX_INT64   max, ///<��incΪ��ֵ����ͨ��max�޶����ֵ
                          CWX_INT64   min, ///<��incΪ��ֵ����ͨ��min�޶���Сֵ
                          CWX_UINT32  uiExpire, ///<��ʱʱ�䣬��Ϊ0�����
                          CWX_UINT32  uiSign, ///<��ǣ���Ϊ0�����
                          CWX_UINT32  uiVersion, ///<�汾����Ϊ0�����
                          char const* user,  ///<�û�����ΪNULL�����
                          char const* passwd, ///<�û������ΪNULL�����
                          char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
                          );

///����inc�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_inc(struct CWX_PG_READER_EX* reader,///<reader
                           char const* msg, ///<�յ�����Ϣ��
                           CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key, ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                           CWX_INT64*   num, ///<����inc��num
                           CWX_INT64*   result, ///<��������ֵ
                           CWX_INT64*   max, ///<����max
                           CWX_INT64*   min, ///<����min
                           CWX_UINT32*  uiExpire, ///<����expire����Ϊ0��ʾû��ָ��
                           CWX_UINT32*  uiSign, ///<����sign
                           CWX_UINT32*  uiVersion, ///<����version
                           char const** user,  ///<����user
                           char const** passwd, ///<����password
                           char* szErr2K  ///<���ʱ�Ĵ�����Ϣ
                           );

///pack delete�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_del(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                          CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                          char const* user,  ///<�û�����ΪNULL�����
                          char const* passwd, ///<�û������ΪNULL�����
                          char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
                          );

///parse delete�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_del(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<�յ�����Ϣ��
                           CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                           CWX_UINT32* uiVersion, ///<���ذ汾
                           char const** user,     ///<�����û���NULL��ʾ������
                           char const** passwd,   ///<���ؿ��NULL��ʾ������
                           char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                           );

///pack��inc������ݸ��·�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_reply(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                            char* buf,  ///<pack�����ݿռ�
                            CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                            CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                            CWX_UINT16 unMsgType, ///<�ظ���Ϣ������Ϣ����
                            int ret,  ///<���ص�ret����
                            CWX_UINT32 uiVersion, ///<���صİ汾��
                            CWX_UINT32 uiFieldNum, ///<���ص�field����
                            char const* szErrMsg, ///<���صĴ�����Ϣ
                            char* szErr2K    ///<pack����ʱ�Ĵ�����Ϣ
                            );

///parse��inc������ݸ��·�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_reply(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<�յ�����Ϣ��
                             CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                             int* ret,  ///<���ص�retֵ
                             CWX_UINT32* uiVersion, ///<���ص�version
                             CWX_UINT32* uiFieldNum,  ///<���ص�field number
                             char const** szErrMsg,  ///<���صĴ�����Ϣ
                             char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                             );

///pack inc�ķ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_inc_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                char* buf,  ///<pack�����ݿռ�
                                CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                CWX_UINT16 unMsgType, ///<��Ϣ����
                                int ret,  ///<ret����
                                CWX_INT64 llNum, ///<��������ֵ
                                CWX_UINT32 uiVersion, ///<�汾��
                                char const* szErrMsg, ///<������Ϣ
                                char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                );

///parse inc���ص���Ϣ���� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_inc_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                 char const* msg, ///<�յ�����Ϣ��
                                 CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                 int* ret,  ///<���ص�retֵ
                                 CWX_UINT32* uiVersion, ///<���صİ汾
                                 CWX_INT64* llNum, ///<���صļ�������ֵ
                                 char const** szErrMsg, ///<������Ϣ
                                 char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                                 );

///pack get�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_get_key(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                         char* buf,  ///<pack�����ݿռ�
                         CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                         CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                         CWX_KEY_VALUE_ITEM_S const* key, ///<key
                         CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                         CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                         CWX_UINT8 bVersion, ///<�Ƿ��ȡ�汾
                         CWX_UINT8 bMaster, ///<�Ƿ��master��ȡ
                         char const* user,  ///<�û�����ΪNULL�����
                         char const* passwd, ///<�û������ΪNULL�����
                         CWX_UINT8 ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                         char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
                         );

///parse get�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_get_key(struct CWX_PG_READER_EX* reader, ///<reader
                          char const* msg, ///<�յ�����Ϣ��
                          CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                          CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                          CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                          CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                          CWX_UINT8*          bVersion, ///<�汾
                          char const** user,     ///<�����û���NULL��ʾ������
                          char const** passwd,   ///<���ؿ��NULL��ʾ������
                          CWX_UINT8* ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                          char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                          );

///pack exist�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_exist_key(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                           char* buf,  ///<pack�����ݿռ�
                           CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                           CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                           CWX_KEY_VALUE_ITEM_S const* key, ///<key
                           CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                           CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                           CWX_UINT8 bVersion, ///<�Ƿ��ȡ�汾
                           CWX_UINT8 bMaster, ///<�Ƿ��master��ȡ
                           char const* user,  ///<�û�����ΪNULL�����
                           char const* passwd, ///<�û������ΪNULL�����
                           char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
                           );

///parse exist�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_exist_key(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<�յ�����Ϣ��
                            CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                            CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                            CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                            CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                            CWX_UINT8*        bVersion, ///<�汾
                            char const** user,     ///<�����û���NULL��ʾ������
                            char const** passwd,   ///<���ؿ��NULL��ʾ������
                            char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                            );

///pack multi-get���ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_get_keys(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                          struct CWX_PG_WRITER_EX* writer1, ///<����pack��writer1
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          char const* const*  keys, ///<key������
                          CWX_UINT16  unKeyNum, ///<key������
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                          char const* user,  ///<�û�����ΪNULL�����
                          char const* passwd, ///<�û������ΪNULL�����
                          CWX_UINT8 ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                          CWX_UINT8 bMaster, ///<�Ƿ��master��ȡ
                          char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
                          );

///parse multi-get�����ݰ��� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_get_keys(struct CWX_PG_READER_EX* reader,///<reader
                           struct CWX_PG_READER_EX* reader1,///<reader1
                           char const* msg, ///<�յ�����Ϣ��
                           CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** keys,///<key���б�
                           CWX_UINT16* unKeyNum, ///<key������
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                           char const** user,     ///<�����û���NULL��ʾ������
                           char const** passwd,   ///<���ؿ��NULL��ʾ������
                           CWX_UINT8*   ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                           char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
                           );

///pack ��ȡkey�б�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_get_list(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* begin, ///<��ʼ��key
                          CWX_KEY_VALUE_ITEM_S const* end,  ///<������key
                          CWX_UINT16  num,  ///<���ص�����
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����ΪNULL�����
                          CWX_UINT8     bAsc, ///<�Ƿ�����
                          CWX_UINT8     bBegin, ///<�Ƿ��ȡbegin��ֵ
                          CWX_UINT8     bKeyInfo, ///<�Ƿ񷵻�key��info
                          CWX_UINT8     bMaster, ///<�Ƿ��master��ȡ
                          char const* user,  ///<�û�����ΪNULL�����
                          char const* passwd, ///<�û������ΪNULL�����
                          char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
                          );

///parse get list�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_get_list(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<�յ�����Ϣ��
                           CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** begin, ///<���ؿ�ʼ
                           CWX_KEY_VALUE_ITEM_S const** end, ///<���ؼ���
                           CWX_UINT16*  num, ///<��ȡ������
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���ΪNULL��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����ΪNULL��ʾ������
                           CWX_UINT8*        bAsc, ///<����
                           CWX_UINT8*        bBegin, ///<�Ƿ��ȡ��ʼֵ
                           CWX_UINT8*        bKeyInfo, ///<�Ƿ񷵻�key��info
                           char const** user, ///<�û�
                           char const** passwd, ///<����
                           char*        szErr2K ///<����Ĵ�����Ϣ
                           );

///pack��Ȩ��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_auth(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                           char* buf,  ///<pack�����ݿռ�
                           CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                           CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                           char const* user, ///<�û�����ΪNULL�����
                           char const* passwd,///<�û������ΪNULL�����
                           char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                           );

///parse��Ȩ�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_auth(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<�յ�����Ϣ��
                            CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                            char const** user, ///<�û�
                            char const** passwd, ///<����
                            char*     szErr2K ///<���ʱ�Ĵ�����Ϣ
                            );

///pack��Ȩ�ظ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_auth_reply(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                 CWX_UINT16 unMsgType, ///<��Ϣ����
                                 int ret, ///<��Ȩ���
                                 char const* szErrMsg, ///<������Ϣ
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                 );

///parse��Ȩ�ظ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_auth_reply(struct CWX_PG_READER_EX* reader,///<reader
                                  char const* msg, ///<�յ�����Ϣ��
                                  CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                  int* ret,///<��Ȩ���
                                  char const** szErrMsg,///<������Ϣ
                                  char* szErr2K///<���ʱ�Ĵ�����Ϣ
                                  );

///pack export��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_report(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                               char* buf,  ///<pack�����ݿռ�
                               CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                               CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                               CWX_UINT32  uiChunkSize, ///<���ݷ��͵�chunk��С
                               char const* subscribe, ///<���ݶ�������
                               char const* key, ///<��ʼ��key
                               char const* extra, ///<extra��Ϣ����ΪNULL�����
                               char const* user, ///<�û���
                               char const* passwd, ///<����
                               char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                               );

///parse export��report���ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_report(struct CWX_PG_READER_EX* reader,///<reader
                                char const* msg, ///<�յ�����Ϣ��
                                CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                CWX_UINT32*  uiChunkSize,///<���ݷ��͵�chunk��С
                                char const** subscribe,///<���ݶ����������ձ�ʾȫ������
                                char const** key,///<��ʼ��key���ձ�ʾû������
                                char const** extra, ///<extra��Ϣ����ΪNULL��ʾû��ָ��
                                char const** user,///<�û���
                                char const** passwd,///<����
                                char* szErr2K///<���ʱ�Ĵ�����Ϣ
                                );

///pack export��report�ظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_report_reply(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                                     char* buf,  ///<pack�����ݿռ�
                                     CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                     CWX_UINT32 uiTaskId,///<��Ϣ����task id
                                     CWX_UINT64 ullSession, ///<session
                                     CWX_UINT64 ullSid,  ///<���ݿ�ʼ����ʱ��sid
                                     char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                     );

///parse export��report�ظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_report_reply(struct CWX_PG_READER_EX* reader,///<reader
                                      char const* msg, ///<�յ�����Ϣ��
                                      CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                      CWX_UINT64* ullSession,///<session
                                      CWX_UINT64* ullSid,///<���ݿ�ʼ����ʱ��sid
                                      char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                                      );

///packһ��export��key/value�����ݡ�����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_data_item(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                                  CWX_KEY_VALUE_ITEM_S const* key, ///<key
                                  CWX_KEY_VALUE_ITEM_S const* data, ///<data
                                  CWX_KEY_VALUE_ITEM_S const* extra, ///<extra
                                  CWX_UINT32 version, ///<�汾��
                                  CWX_UINT32 expire, ///<��ʱʱ��
                                  char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                  );

///parseһ��export��key/value�����ݷ���ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_data_item(struct CWX_PG_READER_EX* reader,///<reader
                                   char const* msg, ///<key/value����
                                   CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                   CWX_KEY_VALUE_ITEM_S const** key, ///<���ݵ�key
                                   CWX_KEY_VALUE_ITEM_S const** data, ///<���ݵ�data
                                   CWX_KEY_VALUE_ITEM_S const** extra, ///<extra
                                   CWX_UINT32* version, ///<���ݵİ汾
                                   CWX_UINT32* expire, ///<���ݵĳ�ʱ
                                   char* szErr2K///<���ʱ�Ĵ�����Ϣ
                                   );

///pack��chunk��֯�Ķ���export��key/value����Ϣ������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_multi_export_data(CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                   char const* szData,  ///<����key/value��ɵ�����package
                                   CWX_UINT32 uiDataLen, ///<���ݵĳ���
                                   char* buf,  ///<pack�����ݿռ�
                                   CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                   CWX_UINT64 ullSeq, ///<���к�
                                   char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                   );

///parse��chunk��֯�Ķ���export��key/value�����ݡ�����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_multi_export_data(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value����
                                    CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                    CWX_UINT64* ullSeq, ///<���к�
                                    char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                                    );

///pack export���ݵ�reply��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_data_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                   char* buf,  ///<pack�����ݿռ�
                                   CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                   CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                   CWX_UINT64 ullSeq, ///<���к�
                                   char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                   );

///parse export���ݵ�reply��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value����
                                    CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                    CWX_UINT64* ullSeq, ///<���к�
                                    char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                                    );


///pack export��ɵ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_end(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                            char* buf,  ///<pack�����ݿռ�
                            CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                            CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                            CWX_UINT64 ullSid, ///<���ʱ��sid
                            char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                            );

///parse export��ɵ���Ϣ������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_end(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<key/value����
                             CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                             CWX_UINT64* ullSid,///<���ʱ��sid
                             char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                             );

///pack binlog sync��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_report_data(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                             char* buf,  ///<pack�����ݿռ�
                             CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                             CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                             CWX_UINT64 ullSid, ///<��ʼ��sid
                             CWX_UINT8  bNewly,  ///<�Ƿ������binlog��ʼͬ��
                             CWX_UINT32  uiChunkSize, ///<ͬ����chunk��С
                             char const* subscribe, ///<binlog���Ĺ��򣬿ձ�ʾȫ������
                             char const* user, ///<�û���
                             char const* passwd, ///<�û�����
                             char const* sign, ///<ǩ����ʽ���ձ�ʾ��ǩ��
                             CWX_UINT8  zip, ///<�Ƿ�ѹ��
                             char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                             );

///parse binlog sync��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_report_data(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<key/value����
                              CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                              CWX_UINT64* ullSid, ///<��ʼ��sid
                              CWX_UINT8*  bNewly, ///<�Ƿ������binlog��ʼͬ��
                              CWX_UINT32*  uiChunkSize, ///<ͬ����chunk��С
                              char const** subscribe, ///<binlog���Ĺ��򣬿ձ�ʾȫ������
                              char const** user, ///<�û���
                              char const** passwd, ///<�û�����
                              char const** sign, ///<ǩ����ʽ���ձ�ʾ��ǩ��
                              CWX_UINT8*   zip, ///<�Ƿ�ѹ��
                              char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                              );

///pack report�Ļظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_report_data_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                   char* buf,  ///<pack�����ݿռ�
                                   CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                   CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                   CWX_UINT64 ullSession, ///<session id
                                   char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                   );

///parse report�Ļظ����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_report_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value����
                                    CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                    CWX_UINT64* ullSession, ///<session id
                                    char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                                    );

///pack sync��session���ӱ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_report_new_conn(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                 CWX_UINT64 ullSession, ///<����������session
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                 );

///parse sync��session���ӱ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_report_new_conn(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value����
                                  CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                  CWX_UINT64* ullSession, ///<����������session
                                  char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                                  );

///pack report��sync�ĳ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_sync_err(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          int ret, ///<�������
                          char const* szErrMsg, ///<������Ϣ
                          char* szErr2K///<pack����ʱ�Ĵ�����Ϣ
                          );

///parse report��sync�ĳ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_sync_err(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<key/value����
                           CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                           int* ret,  ///<�������
                           char const** szErrMsg,  ///<������Ϣ
                           char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                           );


///pack sync��һ��binlog����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_sync_data(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                           char* buf,  ///<pack�����ݿռ�
                           CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                           CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                           CWX_UINT64 ullSid, ///<binlog��sid
                           CWX_UINT32 uiTimeStamp, ///<binlog��ʱ���
                           CWX_KEY_VALUE_ITEM_S const* data, ///<binlog��data
                           CWX_UINT32 group,  ///<binlog�����ķ���
                           CWX_UINT32 type,   ///<binlog�����ͣ�Ҳ������Ϣ����
                           CWX_UINT32 version,  ///<��Ӧ��key�İ汾
                           CWX_UINT64 ullSeq,  ///<��Ϣ�����к�
                           char const* sign, ///<ǩ����ʽ
                           CWX_UINT8   zip, ///<�Ƿ�ѹ��
                           char* szErr2K///<pack����ʱ�Ĵ�����Ϣ
                           );

///pack һ��binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_sync_data_item(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                CWX_UINT64 ullSid, ///<binlog��sid
                                CWX_UINT32 uiTimeStamp, ///<binlog��ʱ���
                                CWX_KEY_VALUE_ITEM_S const* data, ///<binlog��data
                                CWX_UINT32 group,  ///<binlog�����ķ���
                                CWX_UINT32 type,   ///<binlog�����ͣ�Ҳ������Ϣ����
                                CWX_UINT32 version,  ///<��Ӧ��key�İ汾
                                char const* sign, ///<ǩ����ʽ
                                char* szErr2K///<pack����ʱ�Ĵ�����Ϣ
                                );

///pack ����binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_multi_sync_data(CWX_UINT32 uiTaskId, ///<����id
                                 char const* szData, ///<������Ϣ������buf
                                 CWX_UINT32 uiDataLen, ///<�������ݵ�����buf����
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT64 ullSeq, ///<��Ϣ������Ϣ���к�
                                 CWX_UINT8  zip, ///<�Ƿ�ѹ��
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                 );

///parseһ��binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_sync_data(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<key/value����
                            CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                            CWX_UINT64* ullSid, ///<binlog��sid
                            CWX_UINT32* uiTimeStamp, ///<binlog��ʱ���
                            CWX_KEY_VALUE_ITEM_S const** data, ///<binlog������
                            CWX_UINT32* group, ///<binlog������group
                            CWX_UINT32* type, ///<binlog��Ӧ�����ݱ����Ϣ����
                            CWX_UINT32* version, ///<binlog��Ӧ�����ݱ����key�İ汾
                            char* szErr2K ///<���ʱ�Ĵ�����Ϣ
                            );

///pack sync binlog�Ļظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_sync_data_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                 CWX_UINT64 ullSeq, ///<��Ϣ�����к�
                                 CWX_UINT16 unMsgType, ///<��Ϣ����
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
                                 );

///parse sync binlog�Ļظ����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_sync_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value����
                                  CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                  CWX_UINT64* ullSeq, ///<��Ϣ�����к�
                                  char* szErr2K  ///<���ʱ�Ĵ�����Ϣ
                                  );

///��������ͬ������seq��
void unistor_set_seq(char* szBuf, CWX_UINT64 ullSeq);

///��ȡ����ͬ������seq��
CWX_UINT64 unistor_get_seq(char const* szBuf);

#ifdef __cplusplus
}
#endif

#endif
