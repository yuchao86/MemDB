#include "unistor_poco.h"
#include <zlib.h>

///pack 消息包。0：成功；-1：失败
static int unistor_pack_msg(CWX_UINT16 unMsgType,
                           CWX_UINT32 uiTaskId,
                           char* buf,
                           CWX_UINT32* buf_len,
                           char const* szData,
                           CWX_UINT32 data_len,
                           CWX_UINT8 ucAttr)
{
    CWX_MSG_HEADER_S head;
    head.m_ucVersion = 0;
    head.m_ucAttr = ucAttr;
    head.m_uiTaskId =uiTaskId;
    head.m_unMsgType = unMsgType;
    head.m_uiDataLen = data_len;

    if (*buf_len < CWX_MSG_HEAD_LEN + data_len) return -1;
    cwx_msg_pack_head(&head, buf);
    memcpy(buf + CWX_MSG_HEAD_LEN, szData, data_len);
    *buf_len = CWX_MSG_HEAD_LEN + data_len;
    return 0;
}

///设置从master获取的属性位
CWX_UINT8 unistor_set_from_master(CWX_UINT8* ucAttr){
    CWX_SET_ATTR(*ucAttr, UNISTOR_MASTER_ATTR_BIT);
    return *ucAttr;
}

///check是否设置了从master获取的属性位。0：不是；>0：是
CWX_UINT8 isFromMaster(CWX_UINT8 ucAttr){
    return CWX_CHECK_ATTR(ucAttr, UNISTOR_MASTER_ATTR_BIT);
}

///清除从master获取的属性位
CWX_UINT8 clearFromMaster(CWX_UINT8* ucAttr){
    return CWX_CLR_ATTR(*ucAttr, UNISTOR_MASTER_ATTR_BIT);
}

///pack import key的数据。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_import(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer，内容通过writer返回
                             char* buf,  ///<pack的数据空间
                             CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                             CWX_UINT32 uiTaskId, ///<消息包的task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiExpire,  ///<超时时间，若为0则不添加
                             CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                             CWX_UINT8   bCache, ///<是否cache，若为true则不添加
                             char const* user,  ///<用户，若为0则不添加
                             char const* passwd, ///<用户口令，若为0则不添加
                             char* szErr2K       ///<pack出错时的错误信息
                             )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, 0)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_D, data->m_szData, data->m_uiDataLen, data->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (uiExpire){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_E, uiExpire)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bCache){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_C, bCache)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_IMPORT,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///解析Add key的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_import(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<收到的import消息包
                              CWX_UINT32  msg_len, ///<收到的import消息包的长度
                              CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                              CWX_KEY_VALUE_ITEM_S const** data,  ///<返回data字段
                              CWX_UINT32* uiExpire,  ///<返回expire，若为0表示没有指定
                              CWX_UINT32* uiVersion, ///<返回版本
                              CWX_UINT8*       bCache,    ///<返回cache
                              char const** user,     ///<返回用户，0表示不存在
                              char const** passwd,   ///<返回口令，0表示不存在
                              char* szErr2K     ///<解包时的错误信息
                              )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get data
    *data = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_D, 0);
    if (!(*data)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    if ((*data)->m_bKeyValue){
        if (!cwx_pg_is_valid_ex((*data)->m_szData, (*data)->m_uiDataLen)){
            if (szErr2K) snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get timeout
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_E, uiExpire, 0)){
        *uiExpire = 0;
    }
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get cache
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_C, bCache, 0)){
        *bCache = 1;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack Add key的数据。 返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_add(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer，内容通过writer返回
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                          CWX_KEY_VALUE_ITEM_S const* data, ///<data
                          CWX_UINT32 uiExpire,  ///<超时时间，若为0则不添加
                          CWX_UINT32 uiSign,    ///<标记，若为0则不添加
                          CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                          CWX_UINT8   bCache, ///<是否cache，若为true则不添加
                          char const* user,  ///<用户，若为0则不添加
                          char const* passwd, ///<用户口令，若为0则不添加
                          char* szErr2K       ///<pack出错时的错误信息
                          )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_D, data->m_szData, data->m_uiDataLen, data->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (uiExpire){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_E, uiExpire)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiSign){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_SIGN, uiSign)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bCache){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_C, bCache)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_ADD,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///解析Add key的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_add(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<收到的import消息包
                           CWX_UINT32  msg_len, ///<收到的import消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** data,  ///<返回data字段
                           CWX_UINT32* uiExpire,  ///<返回expire，若为0表示没有指定
                           CWX_UINT32* uiSign,    ///<返回sign
                           CWX_UINT32* uiVersion, ///<返回版本
                           CWX_UINT8*  bCache,    ///<返回cache
                           char const** user,     ///<返回用户，0表示不存在
                           char const** passwd,   ///<返回口令，0表示不存在
                           char* szErr2K     ///<解包时的错误信息
                           )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get data
    *data = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_D, 0);
    if (!(*data)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    if ((*data)->m_bKeyValue){
        if (!cwx_pg_is_valid_ex((*data)->m_szData, (*data)->m_uiDataLen)){
            if (szErr2K) snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get timeout
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_E, uiExpire, 0)){
        *uiExpire = 0;
    }
    //get sign
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_SIGN, uiSign, 0)){
        *uiSign = 0;
    }
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get cache
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_C, bCache, 0)){
        *bCache = 1;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
	return UNISTOR_ERR_SUCCESS;
}


///pack set的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_set(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer，内容通过writer返回
                char* buf,  ///<pack的数据空间
                CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                CWX_UINT32 uiTaskId, ///<消息包的task id
                CWX_KEY_VALUE_ITEM_S const* key, ///<key
                CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                CWX_KEY_VALUE_ITEM_S const* data, ///<data
                CWX_UINT32 uiSign, ///<标记，若为0则不添加
                CWX_UINT32 uiExpire, ///<超时时间，若为0则不添加
                CWX_UINT32 uiVersion,///<版本，若为0则不添加
                CWX_UINT8  bCache, ///<是否cache，若为true则不添加
                char const* user, ///<用户，若为0则不添加
                char const* passwd,///<用户口令，若为0则不添加
                char* szErr2K ///<pack出错时的错误信息
                )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_D, data->m_szData, data->m_uiDataLen, data->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (uiSign){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_SIGN, uiSign)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiExpire){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_E, uiExpire)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bCache){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_C, bCache)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_SET,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///parse set的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_set(struct CWX_PG_READER_EX* reader,  ///<reader
                           char const* msg, ///<收到的import消息包
                           CWX_UINT32  msg_len, ///<收到的import消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key, ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** data, ///<返回data字段
                           CWX_UINT32* uiSign, ///<返回sign
                           CWX_UINT32* uiExpire, ///<返回expire
                           CWX_UINT32* uiVersion, ///<返回版本
                           CWX_UINT8*   bCache,  ///<返回cache
                           char const** user, ///<返回用户，0表示不存在
                           char const** passwd, ///<返回口令，0表示不存在
                           char* szErr2K  ///<解包时的错误信息
                           )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get data
    *data = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_D, 0);
    if (!(*data)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    if ((*data)->m_bKeyValue){
        if (!cwx_pg_is_valid_ex((*data)->m_szData, (*data)->m_uiDataLen)){
            if (szErr2K) snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get sign
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_SIGN, uiSign, 0)){
        *uiSign = 0;
    }
    //get timeout
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_E, uiExpire, 0)){
        *uiExpire = 0;
    }
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get cache
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_C, bCache, 0)){
        *bCache = 1;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack update的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_update(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                             char* buf,  ///<pack的数据空间
                             CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                             CWX_UINT32 uiTaskId, ///<消息包的task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiSign, ///<标记，若为0则不添加
                             CWX_UINT32 uiExpire, ///<超时时间，若为0则不添加
                             CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                             char const* user, ///<用户，若为0则不添加
                             char const* passwd, ///<用户口令，若为0则不添加
                             char* szErr2K ///<pack出错时的错误信息
                             )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_D, data->m_szData, data->m_uiDataLen, data->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (uiSign){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_SIGN, uiSign)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiExpire){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_E, uiExpire)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_UPDATE,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///parse update的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_update(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<收到的import消息包
                              CWX_UINT32  msg_len, ///<收到的import消息包的长度
                              CWX_KEY_VALUE_ITEM_S const** key, ///<返回key字段
                              CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                              CWX_KEY_VALUE_ITEM_S const** data, ///<返回data字段
                              CWX_UINT32* uiSign, ///<返回sign
                              CWX_UINT32* uiExpire, ///<返回expire，若为0表示没有指定
                              CWX_UINT32* uiVersion, ///<返回版本
                              char const** user,     ///<返回用户，0表示不存在
                              char const** passwd,   ///<返回口令，0表示不存在
                              char* szErr2K     ///<解包时的错误信息
                              )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get data
    *data = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_D, 0);
    if (!(*data)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    if ((*data)->m_bKeyValue){
        if (!cwx_pg_is_valid_ex((*data)->m_szData, (*data)->m_uiDataLen)){
            if (szErr2K) snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get sign
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_SIGN, uiSign, 0)){
        *uiSign = 0;
    }
    //get timeout
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_E, uiExpire, 0)){
        *uiExpire = 0;
    }
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}



///pack inc的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_inc(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                          CWX_INT64   num, ///<inc的数字，可正可负
                          CWX_INT64   result, ///<计算的结果，若为0则不添加,此记录最终的计算结果。
                          CWX_INT64   max, ///<若inc为正值，则通过max限定最大值
                          CWX_INT64   min, ///<若inc为负值，则通过min限定最小值
                          CWX_UINT32  uiExpire, ///<超时时间，若为0则不添加
                          CWX_UINT32  uiSign, ///<标记，若为0则不添加
                          CWX_UINT32  uiVersion, ///<版本，若为0则不添加
                          char const* user,  ///<用户，若为0则不添加
                          char const* passwd, ///<用户口令，若为0则不添加
                          char* szErr2K       ///<pack出错时的错误信息
                          )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (0 != cwx_pg_writer_add_key_int64_ex(writer, UNISTOR_KEY_N, num)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (result){
        if (0 != cwx_pg_writer_add_key_int64_ex(writer, UNISTOR_KEY_R, result)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (max){
        if (0 != cwx_pg_writer_add_key_int64_ex(writer, UNISTOR_KEY_MAX, max)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (min){
        if (0 != cwx_pg_writer_add_key_int64_ex(writer, UNISTOR_KEY_MIN, min)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiExpire){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_E, uiExpire)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiSign){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_SIGN, uiSign)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_INC,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///解析inc的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_inc(struct CWX_PG_READER_EX* reader,///<reader
                           char const* msg, ///<收到的import消息包
                           CWX_UINT32  msg_len, ///<收到的import消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key, ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
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
                           )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get num
    if (!cwx_pg_reader_get_int64_ex(reader, UNISTOR_KEY_N, num, 0)){
        *num = 0;
    }
    //get result
    if (!cwx_pg_reader_get_int64_ex(reader, UNISTOR_KEY_R, result, 0)){
        *result = 0;
    }
    //get max
    *max = 0;
    *min = 0;
    if (*num > 0){
        cwx_pg_reader_get_int64_ex(reader, UNISTOR_KEY_MAX, max, 0);
    }else if (*num < 0){
        cwx_pg_reader_get_int64_ex(reader, UNISTOR_KEY_MIN, min, 0);
    }
    //get expire
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_E, uiExpire, 0)){
        *uiExpire = 0;
    }
    //get sign
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_SIGN, uiSign, 0)){
        *uiSign = 0;
    }
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack delete的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_del(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                          CWX_UINT32 uiVersion, ///<版本，若为0则不添加
                          char const* user,  ///<用户，若为0则不添加
                          char const* passwd, ///<用户口令，若为0则不添加
                          char* szErr2K       ///<pack出错时的错误信息
                          )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_DEL,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse delete的数据包。返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_del(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<收到的import消息包
                           CWX_UINT32  msg_len, ///<收到的import消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                           CWX_UINT32* uiVersion, ///<返回版本
                           char const** user,     ///<返回用户，0表示不存在
                           char const** passwd,   ///<返回口令，0表示不存在
                           char* szErr2K     ///<解包时的错误信息
                           )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

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
                            )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_int32_ex(writer, UNISTOR_KEY_RET, ret)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (UNISTOR_ERR_SUCCESS != ret){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_ERR, szErrMsg?szErrMsg:"", szErrMsg?strlen(szErrMsg):0, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }else{
        if (uiVersion){
            if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
                return UNISTOR_ERR_ERROR;
            }
        }
        if (uiFieldNum){
            if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_FN, uiFieldNum)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
                return UNISTOR_ERR_ERROR;
            }
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(unMsgType,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse除inc外的数据更新返回消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_reply(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<收到的import消息包
                             CWX_UINT32  msg_len, ///<收到的import消息包的长度
                             int* ret,  ///<返回的ret值
                             CWX_UINT32* uiVersion, ///<返回的version
                             CWX_UINT32* uiFieldNum,  ///<返回的field number
                             char const** szErrMsg,  ///<返回的错误信息
                             char* szErr2K ///<解包时的错误信息
                             )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get ret
    if (!cwx_pg_reader_get_int32_ex(reader, UNISTOR_KEY_RET, ret, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    if (UNISTOR_ERR_SUCCESS != ret){
        item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_ERR, 0);
        if (!item){
            if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
            return UNISTOR_ERR_ERROR;
        }
        *szErrMsg = item->m_szData;
    }else{
        *szErrMsg = "";
        if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
            uiVersion = 0;
        }
        if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_FN, uiFieldNum, 0)){
            uiFieldNum = 0;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

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
                                )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_int32_ex(writer, UNISTOR_KEY_RET, ret)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (UNISTOR_ERR_SUCCESS != ret){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_ERR, szErrMsg?szErrMsg:"", szErrMsg?strlen(szErrMsg):0, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }else if (uiVersion){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, uiVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
        if (0 != cwx_pg_writer_add_key_int64_ex(writer, UNISTOR_KEY_N, llNum)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(unMsgType,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse inc返回的消息包。 返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_inc_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                 char const* msg, ///<收到的import消息包
                                 CWX_UINT32  msg_len, ///<收到的import消息包的长度
                                 int* ret,  ///<返回的ret值
                                 CWX_UINT32* uiVersion, ///<返回的版本
                                 CWX_INT64* llNum, ///<返回的计数器的值
                                 char const** szErrMsg, ///<错误信息
                                 char* szErr2K ///<解包时的错误信息
                                 )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
        *uiVersion = 0;
    }
    //get ret
    if (!cwx_pg_reader_get_int32_ex(reader, UNISTOR_KEY_RET, ret, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    if (UNISTOR_ERR_SUCCESS != ret){
        item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_ERR, 0);
        if (!item){
            if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
            return UNISTOR_ERR_ERROR;
        }
        *szErrMsg = item->m_szData;
    }else{
        *szErrMsg = "";
        if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, uiVersion, 0)){
            uiVersion = 0;
        }
        if (!cwx_pg_reader_get_int64_ex(reader, UNISTOR_KEY_N, llNum, 0)){
            llNum = 0;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///pack get的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_get_key(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                         char* buf,  ///<pack的数据空间
                         CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                         CWX_UINT32 uiTaskId, ///<消息包的task id
                         CWX_KEY_VALUE_ITEM_S const* key, ///<key
                         CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                         CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                         CWX_UINT8 bVersion, ///<是否获取版本
                         CWX_UINT8 bMaster, ///<是否从master获取
                         char const* user,  ///<用户，若为0则不添加
                         char const* passwd, ///<用户口令，若为0则不添加
                         CWX_UINT8 ucKeyInfo, ///<是否获取key的infomation
                         char* szErr2K   ///<pack出错时的错误信息
                         )
{
    CWX_UINT8 ucAttr = 0;
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bVersion){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_V, bVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (ucKeyInfo){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_I, ucKeyInfo)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bMaster){
        ucAttr = UNISTOR_MASTER_ATTR_BIT;
    }else{
        ucAttr = 0;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_GET,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        ucAttr))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse get的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_get_key(struct CWX_PG_READER_EX* reader, ///<reader
                          char const* msg, ///<收到的import消息包
                          CWX_UINT32  msg_len, ///<收到的import消息包的长度
                          CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                          CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                          CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                          CWX_UINT8*          bVersion, ///<版本
                          char const** user,     ///<返回用户，0表示不存在
                          char const** passwd,   ///<返回口令，0表示不存在
                          CWX_UINT8* ucKeyInfo, ///<是否获取key的infomation
                          char* szErr2K     ///<解包时的错误信息
                          )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get version
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_V, bVersion, 0)){
        *bVersion = 0;
    }
    //get info
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_I, ucKeyInfo, 0)){
        *ucKeyInfo = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack exist的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_exist_key(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                           char* buf,  ///<pack的数据空间
                           CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                           CWX_UINT32 uiTaskId, ///<消息包的task id
                           CWX_KEY_VALUE_ITEM_S const* key, ///<key
                           CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                           CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                           CWX_UINT8 bVersion, ///<是否获取版本
                           CWX_UINT8 bMaster, ///<是否从master获取
                           char const* user,  ///<用户，若为0则不添加
                           char const* passwd, ///<用户口令，若为0则不添加
                           char* szErr2K   ///<pack出错时的错误信息
                           )
{
    CWX_UINT8 ucAttr = 0;
    cwx_pg_writer_begin_pack_ex(writer);
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, key->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bVersion){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_V, bVersion)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bMaster){
        ucAttr = UNISTOR_MASTER_ATTR_BIT;
    }else{
        ucAttr = 0;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_EXIST,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        ucAttr))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse exist的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_exist_key(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<收到的import消息包
                            CWX_UINT32  msg_len, ///<收到的import消息包的长度
                            CWX_KEY_VALUE_ITEM_S const** key,   ///<返回key字段
                            CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                            CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                            CWX_UINT8*        bVersion, ///<版本
                            char const** user,     ///<返回用户，0表示不存在
                            char const** passwd,   ///<返回口令，0表示不存在
                            char* szErr2K     ///<解包时的错误信息
                            )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get version
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_V, bVersion, 0)){
        *bVersion = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack multi-get数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_get_keys(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                          struct CWX_PG_WRITER_EX* writer1, ///<用于pack的writer1
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          char const* const*  keys, ///<key的数组
                          CWX_UINT16  unKeyNum, ///<key的数量
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                          char const* user,  ///<用户，若为0则不添加
                          char const* passwd, ///<用户口令，若为0则不添加
                          CWX_UINT8 ucKeyInfo, ///<是否获取key的infomation
                          CWX_UINT8 bMaster, ///<是否从master获取
                          char* szErr2K   ///<pack出错时的错误信息
                          )
{
    CWX_UINT8 ucAttr = 0;
    CWX_UINT16 i=0;
    cwx_pg_writer_begin_pack_ex(writer);
    if (!unKeyNum){
        if (szErr2K) strcpy(szErr2K, "No key");
        return UNISTOR_ERR_ERROR;
    }
    if (1 == unKeyNum){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, keys[0], strlen(keys[0]), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }else{
        cwx_pg_writer_begin_pack_ex(writer1);
        for (i=0; i<unKeyNum; i++){
            if (0 != cwx_pg_writer_add_key_ex(writer1, UNISTOR_KEY_K, keys[i], strlen(keys[i]), 0)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer1));
                return UNISTOR_ERR_ERROR;
            }

        }
        cwx_pg_writer_pack_ex(writer1);
        if (0 != cwx_pg_writer_add_key_ex(writer,
            UNISTOR_KEY_K,
            cwx_pg_writer_get_msg_ex(writer1),
            cwx_pg_writer_get_msg_size_ex(writer1),
            1))
        {
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (ucKeyInfo){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_I, ucKeyInfo)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bMaster){
        ucAttr = UNISTOR_MASTER_ATTR_BIT;
    }else{
        ucAttr = 0;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_GETS,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        ucAttr))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}

///parse multi-get的数据包。 返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_get_keys(struct CWX_PG_READER_EX* reader,///<reader
                           struct CWX_PG_READER_EX* reader1,///<reader1
                           char const* msg, ///<收到的import消息包
                           CWX_UINT32  msg_len, ///<收到的import消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** keys,///<key的列表
                           CWX_UINT16* unKeyNum, ///<key的数量
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                           char const** user,     ///<返回用户，0表示不存在
                           char const** passwd,   ///<返回口令，0表示不存在
                           CWX_UINT8*   ucKeyInfo, ///<是否获取key的infomation
                           char* szErr2K     ///<解包时的错误信息
                           )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!item){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    if (item->m_bKeyValue){
        if (0 != cwx_pg_reader_unpack_ex(reader1, item->m_szData, item->m_uiDataLen, 0, 1)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader1));
            return UNISTOR_ERR_ERROR;
        }
        *unKeyNum = cwx_pg_reader_get_key_num_ex(reader1);
        *keys = cwx_pg_reader_get_keys_ex(reader1);
    }else{
        *keys = item;
        *unKeyNum = 1;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get info
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_I, ucKeyInfo, 0)){
        *ucKeyInfo = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack 获取key列表的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_get_list(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          CWX_KEY_VALUE_ITEM_S const* begin, ///<开始的key
                          CWX_KEY_VALUE_ITEM_S const* end,  ///<结束的key
                          CWX_UINT16  num,  ///<返回的数量
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field字段，若为0的不添加
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra信息，若为0则不添加
                          CWX_UINT8     bAsc, ///<是否升序
                          CWX_UINT8     bBegin, ///<是否获取begin的值
                          CWX_UINT8     bKeyInfo, ///<是否返回key的info
                          CWX_UINT8     bMaster, ///<是否从master获取
                          char const* user,  ///<用户，若为0则不添加
                          char const* passwd, ///<用户口令，若为0则不添加
                          char* szErr2K )  ///<pack出错时的错误信息
{
    CWX_UINT8 ucAttr = 0;
    cwx_pg_writer_begin_pack_ex(writer);
    if (begin && begin->m_uiDataLen){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_BEGIN, begin->m_szData, begin->m_uiDataLen, begin->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (end && end->m_uiDataLen){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_END, end->m_szData, end->m_uiDataLen, end->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (num){
        if (0 != cwx_pg_writer_add_key_uint16_ex(writer, UNISTOR_KEY_N, num)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (field){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_F, field->m_szData, field->m_uiDataLen, 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bKeyInfo){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_I, bKeyInfo)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bAsc){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_ASC, bAsc)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bBegin){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_IB, bBegin)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bMaster){
        ucAttr = UNISTOR_MASTER_ATTR_BIT;
    }else{
        ucAttr = 0;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_LIST,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        ucAttr))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse get list的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_get_list(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<收到的import消息包
                           CWX_UINT32  msg_len, ///<收到的import消息包的长度
                           CWX_KEY_VALUE_ITEM_S const** begin, ///<返回开始
                           CWX_KEY_VALUE_ITEM_S const** end, ///<返回技术
                           CWX_UINT16*  num, ///<获取的数量
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field字段，若为0表示不存在
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra信息，若为0表示不存在
                           CWX_UINT8*        bAsc, ///<升序
                           CWX_UINT8*        bBegin, ///<是否获取开始值
                           CWX_UINT8*        bKeyInfo, ///<是否返回key的info
                           char const** user, ///<用户
                           char const** passwd, ///<口令
                           char*        szErr2K ///<解包的错误信息
                           )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get begin
    *begin = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_BEGIN, 0);
    //get end
    *end = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_END, 0);
    //get num
    if (!cwx_pg_reader_get_uint16_ex(reader, UNISTOR_KEY_N, num, 0)){
        *num = 0;
    }
    //get field
    *field = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_F, 0);
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    //get asc
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_ASC, bAsc, 0)){
        *bAsc = 0;
    }
    //get isbegin
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_IB, bBegin, 0)){
        *bBegin = 0;
    }
    //get info
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_I, bKeyInfo, 0)){
        *bKeyInfo = 0;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack鉴权消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_auth(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                           char* buf,  ///<pack的数据空间
                           CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                           CWX_UINT32 uiTaskId, ///<消息包的task id
                           char const* user, ///<用户，若为0则不添加
                           char const* passwd,///<用户口令，若为0则不添加
                           char* szErr2K ///<pack出错时的错误信息
                           )
{
    cwx_pg_writer_begin_pack_ex(writer);
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_RECV_AUTH,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse鉴权的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_auth(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<收到的消息包
                            CWX_UINT32  msg_len, ///<收到的消息包的长度
                            char const** user, ///<用户
                            char const** passwd, ///<口令
                            char*     szErr2K ///<解包时的错误信息
                            )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:0;
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:0;
    return UNISTOR_ERR_SUCCESS;
}

///pack鉴权回复的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_recv_auth_reply(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT32 uiTaskId, ///<消息包的task id
                                 CWX_UINT16 unMsgType, ///<消息类型
                                 int ret, ///<鉴权结果
                                 char const* szErrMsg, ///<错误消息
                                 char* szErr2K ///<pack出错时的错误信息
                                 )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //add ret
    if (0 != cwx_pg_writer_add_key_int32_ex(writer, UNISTOR_KEY_RET, ret)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //add err
    if (UNISTOR_ERR_SUCCESS != ret){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_ERR, szErrMsg, strlen(szErrMsg), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(unMsgType,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse鉴权回复的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_recv_auth_reply(struct CWX_PG_READER_EX* reader,///<reader
                                  char const* msg, ///<收到的消息包
                                  CWX_UINT32  msg_len, ///<收到的消息包的长度
                                  int* ret,///<鉴权结果
                                  char const** szErrMsg,///<错误消息
                                  char* szErr2K///<解包时的错误信息
                                  )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get ret
    if (!cwx_pg_reader_get_int32_ex(reader, UNISTOR_KEY_RET, ret, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    if (UNISTOR_ERR_SUCCESS != ret){
        item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_ERR, 0);
        *szErrMsg = item?item->m_szData:"";
    }else{
        *szErrMsg = "";
    }
    return UNISTOR_ERR_SUCCESS;
}



///pack export的report消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_report(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                               char* buf,  ///<pack的数据空间
                               CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                               CWX_UINT32 uiTaskId, ///<消息包的task id
                               CWX_UINT32  uiChunkSize, ///<数据发送的chunk大小
                               char const* subscribe, ///<数据订阅描述
                               char const* key, ///<开始的key
                               char const* extra, ///<extra信息，若为0则不添加
                               char const* user, ///<用户名
                               char const* passwd, ///<口令
                               char* szErr2K ///<pack出错时的错误信息
                               )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //add chunk
    if (uiChunkSize){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_CHUNK, uiChunkSize)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //add subscribe
    if (subscribe){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_SUBSCRIBE, subscribe, strlen(subscribe), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //add key
    if (key){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key, strlen(key), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //add extra
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra, strlen(extra), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }

    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_SYNC_REPORT,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}

///parse export的report数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_report(struct CWX_PG_READER_EX* reader,///<reader
                                char const* msg, ///<收到的消息包
                                CWX_UINT32  msg_len, ///<收到的消息包的长度
                                CWX_UINT32*  uiChunkSize,///<数据发送的chunk大小
                                char const** subscribe,///<数据订阅描述，空表示全部订阅
                                char const** key,///<开始的key，空表示没有限制
                                char const** extra, ///<extra信息，若为0表示没有指定
                                char const** user,///<用户名
                                char const** passwd,///<口令
                                char* szErr2K///<解包时的错误信息
                                )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get chunk
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_CHUNK, uiChunkSize, 0)){
        *uiChunkSize = UNISTOR_DEF_CHUNK_SIZE_KB;
    }
    //get subscribe
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_SUBSCRIBE, 0);
    *subscribe = item?item->m_szData:"";
    //get key
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    *key = item?item->m_szData:0;
    //get extra
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_X, 0);
    *extra = item?item->m_szData:0;
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:"";
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:"";
    return UNISTOR_ERR_SUCCESS;
}

///pack export的report回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_report_reply(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                                     char* buf,  ///<pack的数据空间
                                     CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                     CWX_UINT32 uiTaskId,///<消息包的task id
                                     CWX_UINT64 ullSession, ///<session
                                     CWX_UINT64 ullSid,  ///<数据开始发送时的sid
                                     char* szErr2K ///<pack出错时的错误信息
                                     )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //add session
    if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SESSION, ullSession)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //add sid
    if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SID, ullSid)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }

    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_EXPORT_REPORT_REPLY,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse export的report回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_report_reply(struct CWX_PG_READER_EX* reader,///<reader
                                      char const* msg, ///<收到的消息包
                                      CWX_UINT32  msg_len, ///<收到的消息包的长度
                                      CWX_UINT64* ullSession,///<session
                                      CWX_UINT64* ullSid,///<数据开始发送时的sid
                                      char* szErr2K ///<解包时的错误信息
                                      )
{
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SESSION, ullSession, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SESSION);
        return UNISTOR_ERR_ERROR;
    }
    //get sid
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SID, ullSid, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SID);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///pack一条export的key/value的数据。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_data_item(struct CWX_PG_WRITER_EX* writer,///<用于pack的writer
                                  CWX_KEY_VALUE_ITEM_S const* key, ///<key
                                  CWX_KEY_VALUE_ITEM_S const* data, ///<data
                                  CWX_KEY_VALUE_ITEM_S const* extra, ///<extra
                                  CWX_UINT32 version, ///<版本号
                                  CWX_UINT32 expire, ///<超时时间
                                  char* szErr2K ///<pack出错时的错误信息
                                  )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //add key
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_K, key->m_szData, key->m_uiDataLen, 0)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //add data
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_D, data->m_szData, data->m_uiDataLen, data->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //add extra
    if (extra){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_X, extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //version
    if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, version)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //expire
    if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_E, expire)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    cwx_pg_writer_pack_ex(writer);
    return UNISTOR_ERR_SUCCESS;
}

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
                                   )
{
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get key
    *key = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_K, 0);
    if (!(*key)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get data
    *data = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_D, 0);
    if (!(*data)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    //get extra
    *extra = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_E, 0);
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, version, 0)){
        version = 0;
    }
    //get expire
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_E, expire, 0)){
        expire = 0;
    }
    return UNISTOR_ERR_SUCCESS;
}

///pack以chunk组织的多条export的key/value的消息。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_multi_export_data(CWX_UINT32 uiTaskId, ///<消息包的task id
                                   char const* szData,  ///<多条key/value组成的数据package
                                   CWX_UINT32 uiDataLen, ///<数据的长度
                                   char* buf,  ///<pack的数据空间
                                   CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                   CWX_UINT64 ullSeq, ///<序列号
                                   char* szErr2K ///<pack出错时的错误信息
                                   )
{
    CWX_MSG_HEADER_S head;
    head.m_ucAttr = 0;
    head.m_ucVersion = 0;
    head.m_uiDataLen = uiDataLen+sizeof(ullSeq);
    head.m_uiTaskId = uiTaskId;
    head.m_unMsgType = UNISTOR_MSG_TYPE_EXPORT_DATA;
    if (*buf_len < CWX_MSG_HEAD_LEN + uiDataLen + sizeof(ullSeq)){
        if (szErr2K) snprintf(szErr2K, 2047, "memory[%u] is too small, [%u] is needed.", *buf_len,  (CWX_UINT32)(CWX_MSG_HEAD_LEN + uiDataLen + sizeof(ullSeq)));
        return UNISTOR_ERR_ERROR;
    }
    cwx_msg_pack_head(&head, buf);
    //seq seq
    unistor_set_seq(buf + CWX_MSG_HEAD_LEN, ullSeq);
    //copy data
    memcpy(buf + CWX_MSG_HEAD_LEN + sizeof(ullSeq), szData, uiDataLen);
    *buf_len = CWX_MSG_HEAD_LEN + uiDataLen + sizeof(ullSeq);
    return UNISTOR_ERR_SUCCESS;
}

///parse以chunk组织的多条export的key/value的数据。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_multi_export_data(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value数据
                                    CWX_UINT32  msg_len, ///<key/value数据的长度
                                    CWX_UINT64* ullSeq, ///<序列号
                                    char* szErr2K ///<解包时的错误信息
                                    )
{
    if (msg_len < sizeof(ullSeq)){
        if (szErr2K) snprintf(szErr2K, 2047, "Data Length[%u] is too less, no seq id", msg_len);
        return UNISTOR_ERR_ERROR;
    }
    *ullSeq = unistor_get_seq(msg);
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///pack export数据的reply消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_data_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                   char* buf,  ///<pack的数据空间
                                   CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                   CWX_UINT32 uiTaskId, ///<消息包的task id
                                   CWX_UINT64 ullSeq, ///<序列号
                                   char* szErr2K ///<pack出错时的错误信息
                                   )
{
    CWX_MSG_HEADER_S head;
    head.m_ucAttr = 0;
    head.m_ucVersion = 0;
    head.m_uiDataLen = sizeof(ullSeq);
    head.m_uiTaskId = uiTaskId;
    head.m_unMsgType = UNISTOR_MSG_TYPE_EXPORT_DATA_REPLY;
    writer = NULL;
    if (*buf_len < CWX_MSG_HEAD_LEN + sizeof(ullSeq)){
        if (szErr2K) snprintf(szErr2K, 2047, "memory[%u] is too small, [%u] is needed.", *buf_len,  (CWX_UINT32)(CWX_MSG_HEAD_LEN + sizeof(ullSeq)));
        return UNISTOR_ERR_ERROR;
    }
    cwx_msg_pack_head(&head, buf);
    //seq seq
    unistor_set_seq(buf + CWX_MSG_HEAD_LEN, ullSeq);
    *buf_len = CWX_MSG_HEAD_LEN + sizeof(ullSeq);
    return UNISTOR_ERR_SUCCESS;
}

///parse export数据的reply消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value数据
                                    CWX_UINT32  msg_len, ///<key/value数据的长度
                                    CWX_UINT64*  ullSeq, ///<序列号
                                    char* szErr2K ///<解包时的错误信息
                                    )
{
    reader = NULL;
    if (msg_len < sizeof(ullSeq)){
        if (szErr2K) snprintf(szErr2K, 2047, "Data Length[%u] is too less, no seq id", msg_len);
        return UNISTOR_ERR_ERROR;
    }
    *ullSeq = unistor_get_seq(msg);
    return UNISTOR_ERR_SUCCESS;
}

///pack export完成的消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_export_end(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                            char* buf,  ///<pack的数据空间
                            CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                            CWX_UINT32 uiTaskId, ///<消息包的task id
                            CWX_UINT64 ullSid, ///<完成时的sid
                            char* szErr2K ///<pack出错时的错误信息
                            )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //sid
    if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SID, ullSid)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_EXPORT_END,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///parse export完成的消息包返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_export_end(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<key/value数据
                             CWX_UINT32  msg_len, ///<key/value数据的长度
                             CWX_UINT64* ullSid,///<完成时的sid
                             char* szErr2K ///<解包时的错误信息
                             )
{
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get sid
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SID, ullSid, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SID);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

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
                             )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //sid
    if (!bNewly){
        if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SID, ullSid)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //chunk
    if (uiChunkSize){
        if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_CHUNK, uiChunkSize)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //subscribe
    if (subscribe){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_SUBSCRIBE, subscribe, strlen(subscribe), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //user
    if (user){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_U, user, strlen(user), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    //passwd
    if (passwd){
        if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_P, passwd, strlen(passwd), 0)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    if (sign){
        if ((strcmp(sign, UNISTOR_KEY_CRC32) == 0) || (strcmp(sign, UNISTOR_KEY_MD5)==0)){
            if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_SIGN, sign, strlen(sign), 0)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
                return UNISTOR_ERR_ERROR;
            }
        }
    }
    if (zip){
        if (0 != cwx_pg_writer_add_key_uint8_ex(writer, UNISTOR_KEY_ZIP, zip)){
            if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
            return UNISTOR_ERR_ERROR;
        }
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_SYNC_REPORT,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

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
                              )
{
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get sid
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SID, ullSid, 0)){
        *bNewly = 1;
    }else{
        *bNewly = 0;
    }
    //get chunk
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_CHUNK, uiChunkSize, 0)){
        *uiChunkSize = 0;
    }
    //get subscribe
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_SUBSCRIBE, 0);
    *subscribe = item?item->m_szData:"";
    //get user
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_U, 0);
    *user = item?item->m_szData:"";
    //get passwd
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_P, 0);
    *passwd = item?item->m_szData:"";
    //get sign
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_SIGN, 0);
    *sign = item?item->m_szData:"";
    if ((strcmp(*sign, UNISTOR_KEY_CRC32)!=0) && (strcmp(*sign, UNISTOR_KEY_MD5)!=0)){
        *sign = "";
    }
    //get zip
    if (!cwx_pg_reader_get_uint8_ex(reader, UNISTOR_KEY_ZIP, zip, 0)){
        *zip = 0;
    }
    return UNISTOR_ERR_SUCCESS;
}

///pack report的回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_report_data_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                   char* buf,  ///<pack的数据空间
                                   CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                   CWX_UINT32 uiTaskId, ///<消息包的task id
                                   CWX_UINT64 ullSession, ///<session id
                                   char* szErr2K ///<pack出错时的错误信息
                                   )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //session
    if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SESSION, ullSession)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_SYNC_REPORT_REPLY,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
///parse report的回复数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_report_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value数据
                                    CWX_UINT32  msg_len, ///<key/value数据的长度
                                    CWX_UINT64* ullSession, ///<session id
                                    char* szErr2K ///<解包时的错误信息
                                    )
{
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SESSION, ullSession, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s]", UNISTOR_KEY_SESSION);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///pack sync的session连接报告消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_report_new_conn(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT32 uiTaskId, ///<消息包的task id
                                 CWX_UINT64 ullSession, ///<连接所属的session
                                 char* szErr2K ///<pack出错时的错误信息
                                 )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //session
    if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SESSION, ullSession)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_SYNC_CONN,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse sync的session连接报告数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_report_new_conn(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value数据
                                  CWX_UINT32  msg_len, ///<key/value数据的长度
                                  CWX_UINT64* ullSession, ///<连接所属的session
                                  char* szErr2K ///<解包时的错误信息
                                  )
{
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SESSION, ullSession, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s]", UNISTOR_KEY_SESSION);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///pack report或sync的出错消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_sync_err(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                          char* buf,  ///<pack的数据空间
                          CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                          CWX_UINT32 uiTaskId, ///<消息包的task id
                          int ret, ///<错误代码
                          char const* szErrMsg, ///<错误消息
                          char* szErr2K///<pack出错时的错误信息
                          )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //ret
    if (0 != cwx_pg_writer_add_key_int32_ex(writer, UNISTOR_KEY_RET, ret)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //err
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_ERR, szErrMsg, strlen(szErrMsg), 0)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    cwx_pg_writer_pack_ex(writer);
    if (0 != unistor_pack_msg(UNISTOR_MSG_TYPE_SYNC_ERR,
        uiTaskId,
        buf,
        buf_len,
        cwx_pg_writer_get_msg_ex(writer),
        cwx_pg_writer_get_msg_size_ex(writer),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            (CWX_UINT32)(CWX_MSG_HEAD_LEN + cwx_pg_writer_get_msg_size_ex(writer)));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///parse report或sync的出错数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_sync_err(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<key/value数据
                           CWX_UINT32  msg_len, ///<key/value数据的长度
                           int* ret,  ///<错误代码
                           char const** szErrMsg,  ///<错误消息
                           char* szErr2K ///<解包时的错误信息
                           )
{
    //get err
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get ret
    if (!cwx_pg_reader_get_int32_ex(reader, UNISTOR_KEY_RET, ret, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s]", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    item = cwx_pg_reader_get_key_ex(reader, UNISTOR_KEY_ERR, 0);
    if (!item){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
        return UNISTOR_ERR_ERROR;
    }
    *szErrMsg = item->m_szData;
    return UNISTOR_ERR_SUCCESS;
}

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
                           )
{
    int ret = 0;
    CWX_MSG_HEADER_S head;
    unsigned long ulDestLen = 0;
    cwx_pg_writer_begin_pack_ex(writer);
    ret = unistor_pack_sync_data_item(writer,
        ullSid,
        uiTimeStamp,
        data,
        group,
        type,
        version,
        sign,
        szErr2K);
    if (UNISTOR_ERR_SUCCESS != ret) return ret;
    cwx_pg_writer_pack_ex(writer);

    head.m_ucAttr = 0;
    head.m_ucVersion = 0;
    head.m_uiDataLen = 0;
    head.m_uiTaskId = uiTaskId;
    head.m_unMsgType = UNISTOR_MSG_TYPE_SYNC_DATA;

    if (*buf_len < CWX_MSG_HEAD_LEN +  sizeof(ullSeq) + head.m_uiDataLen + UNISTOR_ZIP_EXTRA_BUF){
        if (szErr2K) snprintf(szErr2K, 2047, "Buf[%u] is too small, [%u] is needed.", *buf_len, (CWX_UINT32)(CWX_MSG_HEAD_LEN + head.m_uiDataLen + UNISTOR_ZIP_EXTRA_BUF));
        return UNISTOR_ERR_ERROR;
    }
    ulDestLen = cwx_pg_writer_get_msg_size_ex(writer) + UNISTOR_ZIP_EXTRA_BUF;
    if (zip){
        if (Z_OK != compress2((unsigned char*)(buf + CWX_MSG_HEAD_LEN + sizeof(ullSeq)),
            &ulDestLen,
            (unsigned char const*)cwx_pg_writer_get_msg_ex(writer),
            cwx_pg_writer_get_msg_size_ex(writer),
            Z_DEFAULT_COMPRESSION))
        {
            zip = 0;
        }
    }
    if (zip){
        head.m_ucAttr = CWX_MSG_ATTR_SYS_MSG;
        head.m_uiDataLen = ulDestLen + sizeof(ullSeq);
    }else{
        head.m_ucAttr = 0;
        head.m_uiDataLen = cwx_pg_writer_get_msg_size_ex(writer) + sizeof(ullSeq);
        memcpy(buf + CWX_MSG_HEAD_LEN + sizeof(ullSeq), cwx_pg_writer_get_msg_ex(writer),cwx_pg_writer_get_msg_size_ex(writer)); 
    }
    cwx_msg_pack_head(&head, buf);
    //seq seq
    unistor_set_seq(buf+ CWX_MSG_HEAD_LEN, ullSeq);
    *buf_len = CWX_MSG_HEAD_LEN + head.m_uiDataLen;
    return UNISTOR_ERR_SUCCESS;
}

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
                                )
{
    cwx_pg_writer_begin_pack_ex(writer);
    //sid
    if (0 != cwx_pg_writer_add_key_uint64_ex(writer, UNISTOR_KEY_SID, ullSid)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //timestamp
    if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_T, uiTimeStamp)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //data
    if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_D, data->m_szData, data->m_uiDataLen, data->m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //group
    if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_G, group)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //type
    if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_TYPE, type)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    //version
    if (0 != cwx_pg_writer_add_key_uint32_ex(writer, UNISTOR_KEY_V, version)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
        return UNISTOR_ERR_ERROR;
    }
    if (sign){
        if (strcmp(sign, UNISTOR_KEY_CRC32) == 0){//CRC32签名
            CWX_UINT32 uiCrc32 = cwx_crc32_value(cwx_pg_writer_get_msg_ex(writer), cwx_pg_writer_get_msg_size_ex(writer));
            if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_CRC32, (char*)&uiCrc32, sizeof(uiCrc32), 0)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
                return UNISTOR_ERR_ERROR;
            }
        }else if (strcmp(sign, UNISTOR_KEY_MD5) == 0){//md5签名
            unsigned char szMd5[16];
            cwx_md5_context md5;
            cwx_md5_start(&md5);
            cwx_md5_update(&md5, (unsigned char*)cwx_pg_writer_get_msg_ex(writer), cwx_pg_writer_get_msg_size_ex(writer));
            cwx_md5_finish(&md5, szMd5);
            if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_MD5, (char*)szMd5, 16, 0)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
                return UNISTOR_ERR_ERROR;
            }
        }
    }
    cwx_pg_writer_pack_ex(writer);
    return UNISTOR_ERR_SUCCESS;
}

///pack 多条binlog的数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_multi_sync_data(CWX_UINT32 uiTaskId, ///<任务id
                                 char const* szData, ///<多条消息的数据buf
                                 CWX_UINT32 uiDataLen, ///<多条数据的数据buf长度
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT64 ullSeq, ///<消息包的消息序列号
                                 CWX_UINT8  zip, ///<是否压缩
                                 char* szErr2K ///<pack出错时的错误信息
                                 )
{
    CWX_MSG_HEADER_S head;
    unsigned long ulDestLen = uiDataLen + UNISTOR_ZIP_EXTRA_BUF;
    head.m_ucAttr = 0;
    head.m_ucVersion = 0;
    head.m_uiDataLen = uiDataLen+sizeof(ullSeq);
    head.m_uiTaskId = uiTaskId;
    head.m_unMsgType = UNISTOR_MSG_TYPE_SYNC_DATA_CHUNK;
    if (head.m_uiDataLen + CWX_MSG_HEAD_LEN + UNISTOR_ZIP_EXTRA_BUF > *buf_len){
        if (szErr2K) snprintf(szErr2K, 2047, "buf[%u] is too small, [%u] is needed.", *buf_len, (CWX_UINT32)(head.m_uiDataLen + CWX_MSG_HEAD_LEN + UNISTOR_ZIP_EXTRA_BUF));
        return UNISTOR_ERR_ERROR;
    }
    if (zip){
        if (Z_OK != compress2((unsigned char*)(buf + CWX_MSG_HEAD_LEN + sizeof(ullSeq)),
            &ulDestLen,
            (unsigned char const*)szData,
            uiDataLen,
            Z_DEFAULT_COMPRESSION))
        {
            zip = 0;
        }
    }
    if (zip){
        head.m_ucAttr = CWX_MSG_ATTR_COMPRESS;
        head.m_uiDataLen = ulDestLen + sizeof(ullSeq);
    }else{
        head.m_ucAttr = 0;
        head.m_uiDataLen = uiDataLen + sizeof(ullSeq);
        memcpy(buf + CWX_MSG_HEAD_LEN + sizeof(ullSeq), szData, uiDataLen); 
    }
    cwx_msg_pack_head(&head, buf);
    unistor_set_seq(buf + CWX_MSG_HEAD_LEN, ullSeq);
    *buf_len = head.m_uiDataLen + CWX_MSG_HEAD_LEN;
    return UNISTOR_ERR_SUCCESS;
}


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
                            )
{
    //get err
    CWX_KEY_VALUE_ITEM_S const* item = 0;
    if (0 != cwx_pg_reader_unpack_ex(reader, msg, msg_len, 0, 1)){
        if (szErr2K) strcpy(szErr2K, cwx_pg_reader_get_error_ex(reader));
        return UNISTOR_ERR_ERROR;
    }
    //get sid
    if (!cwx_pg_reader_get_uint64_ex(reader, UNISTOR_KEY_SID, ullSid, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s]", UNISTOR_KEY_SID);
        return UNISTOR_ERR_ERROR;
    }
    //get timestamp
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_T, uiTimeStamp, 0)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s]", UNISTOR_KEY_T);
        return UNISTOR_ERR_ERROR;
    }
    //get data
    *data = cwx_pg_reader_get_key_ex(reader ,UNISTOR_KEY_D ,0);
    if (!(*data)){
        if (szErr2K) snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    //get group
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_G, group, 0)){
        group = 0;
    }
    //get type
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_TYPE, type, 0)){
        type = 0;
    }
    //get version
    if (!cwx_pg_reader_get_uint32_ex(reader, UNISTOR_KEY_V, version, 0)){
        version = 0;
    }
    //get crc32
    item = cwx_pg_reader_get_key_ex(reader ,UNISTOR_KEY_CRC32 ,0);
    if (item){
        CWX_UINT32 uiOrgCrc32 = 0;
        memcpy(&uiOrgCrc32, item->m_szData, sizeof(uiOrgCrc32));
        CWX_UINT32 uiCrc32 = cwx_crc32_value(msg, item->m_szKey - msg - cwx_pg_get_key_offset_ex(item->m_unKeyLen, item->m_uiDataLen));
        if (uiCrc32 != uiOrgCrc32){
            if (szErr2K) snprintf(szErr2K, 2047, "CRC32 signture error. recv signture:%x, local signture:%x", uiOrgCrc32, uiCrc32);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get md5
    item = cwx_pg_reader_get_key_ex(reader ,UNISTOR_KEY_CRC32 ,0);
    if (item){
        unsigned char szMd5[16];
        cwx_md5_context md5;
        cwx_md5_start(&md5);
        cwx_md5_update(&md5, (unsigned char*)msg, item->m_szKey - msg - cwx_pg_get_key_offset_ex(item->m_unKeyLen, item->m_uiDataLen));
        cwx_md5_finish(&md5, szMd5);
        if (memcmp(szMd5, item->m_szData, 16) != 0){
            if (szErr2K){
                char szTmp1[33];
                char szTmp2[33];
                CWX_UINT32 i=0;
                for (i=0; i<16; i++){
                    sprintf(szTmp1 + i*2, "%2.2x", (unsigned char)item->m_szData[i]);
                    sprintf(szTmp2 + i*2, "%2.2x", szMd5[i]);
                }
                snprintf(szErr2K, 2047, "MD5 signture error. recv signture:%s, local signture:%s", szTmp1, szTmp2);
            }
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}


///pack sync binlog的回复消息包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_pack_sync_data_reply(struct CWX_PG_WRITER_EX* writer, ///<用于pack的writer
                                 char* buf,  ///<pack的数据空间
                                 CWX_UINT32* buf_len, ///<传入空间的大小，返回pack后的内容大小
                                 CWX_UINT32 uiTaskId, ///<消息包的task id
                                 CWX_UINT64 ullSeq, ///<消息的序列号
                                 CWX_UINT16 unMsgType, ///<消息类型
                                 char* szErr2K ///<pack出错时的错误信息
                                 )
{
    char szBuf[9];
    writer = NULL;
    unistor_set_seq(szBuf, ullSeq);
    if (0 != unistor_pack_msg(unMsgType,
        uiTaskId,
        buf,
        buf_len,
        szBuf,
        sizeof(ullSeq),
        0))
    {
        if (szErr2K) snprintf(szErr2K, 2047, "msg buf is too small[%u], size[%u] is needed.",
            *buf_len, 
            (CWX_UINT32)(CWX_MSG_HEAD_LEN + sizeof(ullSeq)));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///parse sync binlog的回复数据包。返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int unistor_parse_sync_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value数据
                                  CWX_UINT32  msg_len, ///<key/value数据的长度
                                  CWX_UINT64* ullSeq, ///<消息的序列号
                                  char* szErr2K  ///<解包时的错误信息
                                  )
{
    reader = NULL;
    if (msg_len < sizeof(*ullSeq)){
        if (szErr2K) snprintf(szErr2K, 2047, "Data Length[%u] is too less, no seq id", msg_len);
        return UNISTOR_ERR_ERROR;
    }
    *ullSeq = unistor_get_seq(msg);
    return UNISTOR_ERR_SUCCESS;
}

///设置数据同步包的seq号
void unistor_set_seq(char* szBuf, CWX_UINT64 ullSeq){
    CWX_UINT32 byte4 = (CWX_UINT32)(ullSeq>>32);
    byte4 = htonl(byte4);
    memcpy(szBuf, &byte4, 4);
    byte4 = (CWX_UINT32)(ullSeq&0xFFFFFFFF);
    byte4 = htonl(byte4);
    memcpy(szBuf + 4, &byte4, 4);

}    
///获取数据同步包的seq号
CWX_UINT64 unistor_get_seq(char const* szBuf){
    CWX_UINT64 ullSeq = 0;
    CWX_UINT32 byte4;
    memcpy(&byte4, szBuf, 4);
    ullSeq = ntohl(byte4);
    memcpy(&byte4, szBuf+4, 4);
    ullSeq <<=32;
    ullSeq += ntohl(byte4);
    return ullSeq;
}
