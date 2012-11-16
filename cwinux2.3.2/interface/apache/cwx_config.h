#ifndef __CWX_CONFIG_H__
#define __CWX_CONFIG_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/


/**
@file cwx_config.h
@brief apache module�����ݶ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-12
@warning
@bug
*/

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_protocol.h"
#include "apr_lib.h"
#include "apr_hash.h"
#include "ap_config.h"
#include "apr_strings.h"
#include "apr_thread_mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

///CWX_UINT64 �������Ͷ���
#define CWX_UINT64   uint64_t

///CWX_UINT32 �������Ͷ���
#define CWX_UINT32   uint32_t

///CWX_UINT16 �������Ͷ���
#define CWX_UINT16   uint16_t
///CWX_UINT8 �������Ͷ���
#define CWX_UINT8   uint8_t
///����bool����
#ifndef bool
#define bool unsigned char
#endif
///����true
#ifndef true
#define true 1
#endif
///����false
#ifndef false
#define false 0
#endif

///ȱʡ���respone
#define CWX_DEF_SHOW 0
///ȱʡ���delay
#define CWX_DEF_DELAY 1
///ȱʡΪ�־�����
#define CWX_DEF_PERSISTANT 1
///ȱʡ�Ĳ�ѯ��ʱΪ10s
#define CWX_DEF_QUERY_TIMEOUT 10000
///ȱʡ�����ӳ�ʱΪ1s
#define CWX_DEF_CONN_TIMEOUT  1000
///ȱʡ�Ļظ���ʱΪ9s
#define CWX_DEF_REPLY_TIMEOUT 9000
///ȱʡ��ʵЧ�������Լ��Ϊ2s
#define CWX_DEF_RESTORE_TIMEOUT 2000
///ȱʡ�����ٿ��г־�������Ϊ2
#define CWX_DEF_MIN_IDLE_CONN 2
///ȱʡ�������г־�������Ϊ4
#define CWX_DEF_MAX_IDLE_CONN 4

///������Ƕ��̻߳������򲻽����ٽ�������
#if ! APR_HAS_THREADS
///������������
#define apr_thread_mutex_t int
///������
#define apr_thread_mutex_create(a,b,c)
///��ȡ��
#define apr_thread_mutex_lock(a)
///�ͷ���
#define apr_thread_mutex_unlock(a)
#endif

#define CWX_VERSION "1.0"

//��־���
#ifdef _DEBUG
#define CWX_LOG(x) cwx_log x
#define CWX_ERR(x) cwx_log x
#else
#define CWX_LOG(x)
#define CWX_ERR(x) cwx_log x
#endif

struct CWX_HOST;
struct CWX_CONFIG;
struct CWX_SERVICE;

///key/value item
typedef struct CWX_KEY_VALUE{
    char*          m_key; ///<key������
    char*          m_lowerkey; ///<key��Сд
    char*          m_value; ///<key������
    struct CWX_KEY_VALUE*  m_next; ///<��һ��key
}CWX_KEY_VALUE;

///���ؾ�����response key�����ݽṹ
typedef struct CWX_KEY_ITEM{
    apr_hash_t*         m_bkey_hash; ///<���븺�ؾ����key��hash����Ϊ�գ������m_ex_bkey_hash
    apr_hash_t*         m_ex_bkey_hash; ///<�����븺�ؾ����key��hash������m_bkey_hash��Ϊ�գ���ʾȫ��������븺�ؾ��⡣
    CWX_KEY_VALUE*      m_header; ///<http response��key�б���Ϊ�գ����ʾû�������key
}CWX_KEY_ITEM;

///�������ͼ���Ϣ���͵�ͷ
typedef struct CWX_SVR_MSG_KEY{
    char*               m_svr_name; ///<������
    char*               m_msg_type; ///<��Ϣ����
    bool                m_exclude;
    CWX_KEY_VALUE*       m_key_value; ///<��Ӧ��key/valueֵ
    struct CWX_SVR_MSG_KEY*     m_next; ///<��һ��
}CWX_SVR_MSG_KEY;


///�û���������Ϣ���ݽṹ
typedef struct CWX_USER_CONFIG{
    int           m_show; ///<�Ƿ񽫺�̨�����key/value���������-1:û�����ã�0��false��1��true
    int           m_delay; ///<�Ƿ����cwinux-delay��header��-1:û�����ã�0��false��1��true
    int           m_persistant; ///<�Ƿ�Ϊ�־����ӡ�-1:û�����ã�0��false��1��true
    int           m_query_timeout; ///<��ѯ�ĳ�ʱms����-1��û�����ã�����Ϊ���õ���ֵ��
    int           m_conn_timeout; ///<���ӵĳ�ʱms����-1��û�����ã�����Ϊ���õ���ֵ��
    int           m_reply_timeout; ///<�ظ��ĳ�ʱms����-1��û�����ã�����Ϊ���õ���ֵ��
    int           m_restore_timeout; ///<�ָ���ʱ��������λΪms��-1��û�����ã�����Ϊ���õ���ֵ��
    int           m_min_idle_conn; ///<���ٵĿ��г־�����������-1��û�����ã�����Ϊ���õ���ֵ��
    int           m_max_idle_conn; ///<���Ŀ��г־�����������-1��û�����ã�����Ϊ���õ���ֵ��
}CWX_USER_CONFIG;

typedef struct CWX_SOCKET{
    int                m_fd;   ///<���ӵ�socket
    struct CWX_SOCKET*  m_next; ///<��һ������
}CWX_SOCKET;

///�������Ӷ�������ݽṹ
typedef struct CWX_CONNECT{
    struct CWX_CONFIG*  m_config; ///<module��config
    apr_pool_t*         m_pool; ///<request's pool
    struct CWX_KEY_ITEM  m_key_item; ///<key������
    struct CWX_HOST*    m_host; ///<���ӵ�HOST����
    CWX_SOCKET*         m_socket;   ///<���ӵ�socket
    char*              m_svr_name; ///<��������
    char*              m_msg_type; ///<��Ϣ����
    request_rec*        m_request; ///<http request
    char*              m_args; ///<http request's args
    CWX_KEY_VALUE*      m_args_head; ///<url  args's head
    CWX_KEY_VALUE*      m_args_tail; ///<url's args's tail
    struct CWX_SERVICE*        m_service; ///<connect's service object
    int                m_def_host_index; ///<connect's init host
    int                m_cur_host_index; ///<current connect's host index
}CWX_CONNECT;

///������Ϣ
typedef struct CWX_HOST{
  char*              m_svr_name;///<host�ķ�����
  int                m_index; ///<����������
  char*              m_ip;///<host��ip
  CWX_UINT16         m_port;///<host�ķ����port
  struct CWX_USER_CONFIG     m_user_config;///<host���û����ã���merge service��defaultֵ��
  bool               m_is_valid; ///<�Ƿ���Ч
  CWX_UINT64          m_invalid_time; ///<ʧЧ��ʱ�䡣
  struct in_addr      m_host; ///<������addr struct
  apr_thread_mutex_t*  m_lock;  ///<��������
  CWX_SOCKET*         m_idle_socket_head; ///<�������ӵ�ͷ;
  int                m_idle_socket_num; ///<�������ӵ�������
  int                m_used_socket_num; ///<��ʹ�õ����ӵ�����
  struct CWX_SERVICE*  m_service; ///<����������service;
  struct CWX_CONFIG*   m_config; ///< apache module��config
  struct CWX_HOST*    m_next; ///<��һ������;
} CWX_HOST;

///�������ݶ���
typedef struct CWX_SERVICE{
   char*                    m_svr_name;///<������
   struct CWX_USER_CONFIG    m_user_config;///<service���û����ã���merge defaultֵ��
   struct CWX_KEY_ITEM       m_key_item; ///<key������
   struct CWX_HOST**         m_hosts; ///<host array
   int                      m_host_num; ///<host num;
   apr_hash_t*               m_uri_key_hash; ///<url's key config hash
   struct CWX_CONFIG*        m_config; ///< apache module��config
} CWX_SERVICE;

///apache module��config
typedef struct CWX_CONFIG {
  apr_pool_t*         m_pool; ///<process mempool
  apr_thread_mutex_t*  m_lock;  ///<ȫ�ֵ���
  apr_hash_t*          m_svr_hash; ///<svr��hash
  int                 m_svr_num; ///���������
  struct CWX_USER_CONFIG   m_user_config;///<service���û����ã���merge defaultֵ��
  struct CWX_KEY_ITEM     m_key_item; ///<key������
  struct CWX_HOST*     m_hosts; ///<host chain's head
  int                 m_host_num; ///<host num;
  struct CWX_SVR_MSG_KEY*   m_balance; ///<balance's config
  struct CWX_SVR_MSG_KEY*   m_header; ///<header's config
  bool                    m_init; ///<ϵͳ�Ƿ��ʼ��
}CWX_CONFIG;
#endif
