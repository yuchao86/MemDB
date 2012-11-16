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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>


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

#define CWX_VERSION "1.0"


typedef struct {
    ngx_http_upstream_conf_t   upstream;
    bool                     show;
    bool                     delay;
    ngx_str_t                content_type;
} ngx_http_cwinux_loc_conf_t;



///key/value item
typedef struct CWX_KEY_VALUE{
    char*          m_key; ///<key������
    char*          m_lowerkey; ///<key��Сд
    char*          m_value; ///<key������
    struct CWX_KEY_VALUE*  m_next; ///<��һ��key
}CWX_KEY_VALUE;


#endif
