#include "unistor_poco.h"
#include <zlib.h>

///pack ��Ϣ����0���ɹ���-1��ʧ��
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

///���ô�master��ȡ������λ
CWX_UINT8 unistor_set_from_master(CWX_UINT8* ucAttr){
    CWX_SET_ATTR(*ucAttr, UNISTOR_MASTER_ATTR_BIT);
    return *ucAttr;
}

///check�Ƿ������˴�master��ȡ������λ��0�����ǣ�>0����
CWX_UINT8 isFromMaster(CWX_UINT8 ucAttr){
    return CWX_CHECK_ATTR(ucAttr, UNISTOR_MASTER_ATTR_BIT);
}

///�����master��ȡ������λ
CWX_UINT8 clearFromMaster(CWX_UINT8* ucAttr){
    return CWX_CLR_ATTR(*ucAttr, UNISTOR_MASTER_ATTR_BIT);
}

///pack import key�����ݡ� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_import(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer������ͨ��writer����
                             char* buf,  ///<pack�����ݿռ�
                             CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                             CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiExpire,  ///<��ʱʱ�䣬��Ϊ0�����
                             CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                             CWX_UINT8   bCache, ///<�Ƿ�cache����Ϊtrue�����
                             char const* user,  ///<�û�����Ϊ0�����
                             char const* passwd, ///<�û������Ϊ0�����
                             char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
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

///����Add key�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_import(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<�յ���import��Ϣ��
                              CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                              CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                              CWX_KEY_VALUE_ITEM_S const** data,  ///<����data�ֶ�
                              CWX_UINT32* uiExpire,  ///<����expire����Ϊ0��ʾû��ָ��
                              CWX_UINT32* uiVersion, ///<���ذ汾
                              CWX_UINT8*       bCache,    ///<����cache
                              char const** user,     ///<�����û���0��ʾ������
                              char const** passwd,   ///<���ؿ��0��ʾ������
                              char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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

///pack Add key�����ݡ� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_add(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer������ͨ��writer����
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                          CWX_KEY_VALUE_ITEM_S const* data, ///<data
                          CWX_UINT32 uiExpire,  ///<��ʱʱ�䣬��Ϊ0�����
                          CWX_UINT32 uiSign,    ///<��ǣ���Ϊ0�����
                          CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                          CWX_UINT8   bCache, ///<�Ƿ�cache����Ϊtrue�����
                          char const* user,  ///<�û�����Ϊ0�����
                          char const* passwd, ///<�û������Ϊ0�����
                          char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
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

///����Add key�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_add(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<�յ���import��Ϣ��
                           CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** data,  ///<����data�ֶ�
                           CWX_UINT32* uiExpire,  ///<����expire����Ϊ0��ʾû��ָ��
                           CWX_UINT32* uiSign,    ///<����sign
                           CWX_UINT32* uiVersion, ///<���ذ汾
                           CWX_UINT8*  bCache,    ///<����cache
                           char const** user,     ///<�����û���0��ʾ������
                           char const** passwd,   ///<���ؿ��0��ʾ������
                           char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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


///pack set�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_set(struct CWX_PG_WRITER_EX* writer,///<����pack��writer������ͨ��writer����
                char* buf,  ///<pack�����ݿռ�
                CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                CWX_KEY_VALUE_ITEM_S const* key, ///<key
                CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                CWX_KEY_VALUE_ITEM_S const* data, ///<data
                CWX_UINT32 uiSign, ///<��ǣ���Ϊ0�����
                CWX_UINT32 uiExpire, ///<��ʱʱ�䣬��Ϊ0�����
                CWX_UINT32 uiVersion,///<�汾����Ϊ0�����
                CWX_UINT8  bCache, ///<�Ƿ�cache����Ϊtrue�����
                char const* user, ///<�û�����Ϊ0�����
                char const* passwd,///<�û������Ϊ0�����
                char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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


///parse set�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_set(struct CWX_PG_READER_EX* reader,  ///<reader
                           char const* msg, ///<�յ���import��Ϣ��
                           CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key, ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** data, ///<����data�ֶ�
                           CWX_UINT32* uiSign, ///<����sign
                           CWX_UINT32* uiExpire, ///<����expire
                           CWX_UINT32* uiVersion, ///<���ذ汾
                           CWX_UINT8*   bCache,  ///<����cache
                           char const** user, ///<�����û���0��ʾ������
                           char const** passwd, ///<���ؿ��0��ʾ������
                           char* szErr2K  ///<���ʱ�Ĵ�����Ϣ
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

///pack update�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_update(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                             char* buf,  ///<pack�����ݿռ�
                             CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                             CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                             CWX_KEY_VALUE_ITEM_S const* key, ///<key
                             CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                             CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                             CWX_KEY_VALUE_ITEM_S const* data, ///<data
                             CWX_UINT32 uiSign, ///<��ǣ���Ϊ0�����
                             CWX_UINT32 uiExpire, ///<��ʱʱ�䣬��Ϊ0�����
                             CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                             char const* user, ///<�û�����Ϊ0�����
                             char const* passwd, ///<�û������Ϊ0�����
                             char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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


///parse update�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_update(struct CWX_PG_READER_EX* reader, ///<reader
                              char const* msg, ///<�յ���import��Ϣ��
                              CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                              CWX_KEY_VALUE_ITEM_S const** key, ///<����key�ֶ�
                              CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                              CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                              CWX_KEY_VALUE_ITEM_S const** data, ///<����data�ֶ�
                              CWX_UINT32* uiSign, ///<����sign
                              CWX_UINT32* uiExpire, ///<����expire����Ϊ0��ʾû��ָ��
                              CWX_UINT32* uiVersion, ///<���ذ汾
                              char const** user,     ///<�����û���0��ʾ������
                              char const** passwd,   ///<���ؿ��0��ʾ������
                              char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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



///pack inc�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_inc(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                          CWX_INT64   num, ///<inc�����֣������ɸ�
                          CWX_INT64   result, ///<����Ľ������Ϊ0�����,�˼�¼���յļ�������
                          CWX_INT64   max, ///<��incΪ��ֵ����ͨ��max�޶����ֵ
                          CWX_INT64   min, ///<��incΪ��ֵ����ͨ��min�޶���Сֵ
                          CWX_UINT32  uiExpire, ///<��ʱʱ�䣬��Ϊ0�����
                          CWX_UINT32  uiSign, ///<��ǣ���Ϊ0�����
                          CWX_UINT32  uiVersion, ///<�汾����Ϊ0�����
                          char const* user,  ///<�û�����Ϊ0�����
                          char const* passwd, ///<�û������Ϊ0�����
                          char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
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

///����inc�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_inc(struct CWX_PG_READER_EX* reader,///<reader
                           char const* msg, ///<�յ���import��Ϣ��
                           CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key, ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
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

///pack delete�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_del(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* key, ///<key
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                          CWX_UINT32 uiVersion, ///<�汾����Ϊ0�����
                          char const* user,  ///<�û�����Ϊ0�����
                          char const* passwd, ///<�û������Ϊ0�����
                          char* szErr2K       ///<pack����ʱ�Ĵ�����Ϣ
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

///parse delete�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_del(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<�յ���import��Ϣ��
                           CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                           CWX_UINT32* uiVersion, ///<���ذ汾
                           char const** user,     ///<�����û���0��ʾ������
                           char const** passwd,   ///<���ؿ��0��ʾ������
                           char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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

///parse��inc������ݸ��·�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_reply(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<�յ���import��Ϣ��
                             CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                             int* ret,  ///<���ص�retֵ
                             CWX_UINT32* uiVersion, ///<���ص�version
                             CWX_UINT32* uiFieldNum,  ///<���ص�field number
                             char const** szErrMsg,  ///<���صĴ�����Ϣ
                             char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///parse inc���ص���Ϣ���� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_inc_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                 char const* msg, ///<�յ���import��Ϣ��
                                 CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                                 int* ret,  ///<���ص�retֵ
                                 CWX_UINT32* uiVersion, ///<���صİ汾
                                 CWX_INT64* llNum, ///<���صļ�������ֵ
                                 char const** szErrMsg, ///<������Ϣ
                                 char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///pack get�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_get_key(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                         char* buf,  ///<pack�����ݿռ�
                         CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                         CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                         CWX_KEY_VALUE_ITEM_S const* key, ///<key
                         CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                         CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                         CWX_UINT8 bVersion, ///<�Ƿ��ȡ�汾
                         CWX_UINT8 bMaster, ///<�Ƿ��master��ȡ
                         char const* user,  ///<�û�����Ϊ0�����
                         char const* passwd, ///<�û������Ϊ0�����
                         CWX_UINT8 ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                         char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
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

///parse get�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_get_key(struct CWX_PG_READER_EX* reader, ///<reader
                          char const* msg, ///<�յ���import��Ϣ��
                          CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                          CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                          CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                          CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                          CWX_UINT8*          bVersion, ///<�汾
                          char const** user,     ///<�����û���0��ʾ������
                          char const** passwd,   ///<���ؿ��0��ʾ������
                          CWX_UINT8* ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                          char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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

///pack exist�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_exist_key(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                           char* buf,  ///<pack�����ݿռ�
                           CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                           CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                           CWX_KEY_VALUE_ITEM_S const* key, ///<key
                           CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                           CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                           CWX_UINT8 bVersion, ///<�Ƿ��ȡ�汾
                           CWX_UINT8 bMaster, ///<�Ƿ��master��ȡ
                           char const* user,  ///<�û�����Ϊ0�����
                           char const* passwd, ///<�û������Ϊ0�����
                           char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
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

///parse exist�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_exist_key(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<�յ���import��Ϣ��
                            CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                            CWX_KEY_VALUE_ITEM_S const** key,   ///<����key�ֶ�
                            CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                            CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                            CWX_UINT8*        bVersion, ///<�汾
                            char const** user,     ///<�����û���0��ʾ������
                            char const** passwd,   ///<���ؿ��0��ʾ������
                            char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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

///pack multi-get���ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_get_keys(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                          struct CWX_PG_WRITER_EX* writer1, ///<����pack��writer1
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          char const* const*  keys, ///<key������
                          CWX_UINT16  unKeyNum, ///<key������
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                          char const* user,  ///<�û�����Ϊ0�����
                          char const* passwd, ///<�û������Ϊ0�����
                          CWX_UINT8 ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                          CWX_UINT8 bMaster, ///<�Ƿ��master��ȡ
                          char* szErr2K   ///<pack����ʱ�Ĵ�����Ϣ
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

///parse multi-get�����ݰ��� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_get_keys(struct CWX_PG_READER_EX* reader,///<reader
                           struct CWX_PG_READER_EX* reader1,///<reader1
                           char const* msg, ///<�յ���import��Ϣ��
                           CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** keys,///<key���б�
                           CWX_UINT16* unKeyNum, ///<key������
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                           char const** user,     ///<�����û���0��ʾ������
                           char const** passwd,   ///<���ؿ��0��ʾ������
                           CWX_UINT8*   ucKeyInfo, ///<�Ƿ��ȡkey��infomation
                           char* szErr2K     ///<���ʱ�Ĵ�����Ϣ
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

///pack ��ȡkey�б�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_get_list(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          CWX_KEY_VALUE_ITEM_S const* begin, ///<��ʼ��key
                          CWX_KEY_VALUE_ITEM_S const* end,  ///<������key
                          CWX_UINT16  num,  ///<���ص�����
                          CWX_KEY_VALUE_ITEM_S const* field, ///<field�ֶΣ���Ϊ0�Ĳ����
                          CWX_KEY_VALUE_ITEM_S const* extra, ///<extra��Ϣ����Ϊ0�����
                          CWX_UINT8     bAsc, ///<�Ƿ�����
                          CWX_UINT8     bBegin, ///<�Ƿ��ȡbegin��ֵ
                          CWX_UINT8     bKeyInfo, ///<�Ƿ񷵻�key��info
                          CWX_UINT8     bMaster, ///<�Ƿ��master��ȡ
                          char const* user,  ///<�û�����Ϊ0�����
                          char const* passwd, ///<�û������Ϊ0�����
                          char* szErr2K )  ///<pack����ʱ�Ĵ�����Ϣ
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

///parse get list�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_get_list(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<�յ���import��Ϣ��
                           CWX_UINT32  msg_len, ///<�յ���import��Ϣ���ĳ���
                           CWX_KEY_VALUE_ITEM_S const** begin, ///<���ؿ�ʼ
                           CWX_KEY_VALUE_ITEM_S const** end, ///<���ؼ���
                           CWX_UINT16*  num, ///<��ȡ������
                           CWX_KEY_VALUE_ITEM_S const** field, ///<field�ֶΣ���Ϊ0��ʾ������
                           CWX_KEY_VALUE_ITEM_S const** extra, ///<extra��Ϣ����Ϊ0��ʾ������
                           CWX_UINT8*        bAsc, ///<����
                           CWX_UINT8*        bBegin, ///<�Ƿ��ȡ��ʼֵ
                           CWX_UINT8*        bKeyInfo, ///<�Ƿ񷵻�key��info
                           char const** user, ///<�û�
                           char const** passwd, ///<����
                           char*        szErr2K ///<����Ĵ�����Ϣ
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

///pack��Ȩ��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_auth(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                           char* buf,  ///<pack�����ݿռ�
                           CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                           CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                           char const* user, ///<�û�����Ϊ0�����
                           char const* passwd,///<�û������Ϊ0�����
                           char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse��Ȩ�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_auth(struct CWX_PG_READER_EX* reader, ///<reader
                            char const* msg, ///<�յ�����Ϣ��
                            CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                            char const** user, ///<�û�
                            char const** passwd, ///<����
                            char*     szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///pack��Ȩ�ظ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_recv_auth_reply(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                 CWX_UINT16 unMsgType, ///<��Ϣ����
                                 int ret, ///<��Ȩ���
                                 char const* szErrMsg, ///<������Ϣ
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse��Ȩ�ظ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_recv_auth_reply(struct CWX_PG_READER_EX* reader,///<reader
                                  char const* msg, ///<�յ�����Ϣ��
                                  CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                  int* ret,///<��Ȩ���
                                  char const** szErrMsg,///<������Ϣ
                                  char* szErr2K///<���ʱ�Ĵ�����Ϣ
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



///pack export��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_report(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                               char* buf,  ///<pack�����ݿռ�
                               CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                               CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                               CWX_UINT32  uiChunkSize, ///<���ݷ��͵�chunk��С
                               char const* subscribe, ///<���ݶ�������
                               char const* key, ///<��ʼ��key
                               char const* extra, ///<extra��Ϣ����Ϊ0�����
                               char const* user, ///<�û���
                               char const* passwd, ///<����
                               char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse export��report���ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_report(struct CWX_PG_READER_EX* reader,///<reader
                                char const* msg, ///<�յ�����Ϣ��
                                CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                CWX_UINT32*  uiChunkSize,///<���ݷ��͵�chunk��С
                                char const** subscribe,///<���ݶ����������ձ�ʾȫ������
                                char const** key,///<��ʼ��key���ձ�ʾû������
                                char const** extra, ///<extra��Ϣ����Ϊ0��ʾû��ָ��
                                char const** user,///<�û���
                                char const** passwd,///<����
                                char* szErr2K///<���ʱ�Ĵ�����Ϣ
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

///pack export��report�ظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_report_reply(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                                     char* buf,  ///<pack�����ݿռ�
                                     CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                     CWX_UINT32 uiTaskId,///<��Ϣ����task id
                                     CWX_UINT64 ullSession, ///<session
                                     CWX_UINT64 ullSid,  ///<���ݿ�ʼ����ʱ��sid
                                     char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse export��report�ظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_report_reply(struct CWX_PG_READER_EX* reader,///<reader
                                      char const* msg, ///<�յ�����Ϣ��
                                      CWX_UINT32  msg_len, ///<�յ�����Ϣ���ĳ���
                                      CWX_UINT64* ullSession,///<session
                                      CWX_UINT64* ullSid,///<���ݿ�ʼ����ʱ��sid
                                      char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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


///packһ��export��key/value�����ݡ�����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_data_item(struct CWX_PG_WRITER_EX* writer,///<����pack��writer
                                  CWX_KEY_VALUE_ITEM_S const* key, ///<key
                                  CWX_KEY_VALUE_ITEM_S const* data, ///<data
                                  CWX_KEY_VALUE_ITEM_S const* extra, ///<extra
                                  CWX_UINT32 version, ///<�汾��
                                  CWX_UINT32 expire, ///<��ʱʱ��
                                  char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///pack��chunk��֯�Ķ���export��key/value����Ϣ������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_multi_export_data(CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                   char const* szData,  ///<����key/value��ɵ�����package
                                   CWX_UINT32 uiDataLen, ///<���ݵĳ���
                                   char* buf,  ///<pack�����ݿռ�
                                   CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                   CWX_UINT64 ullSeq, ///<���к�
                                   char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse��chunk��֯�Ķ���export��key/value�����ݡ�����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_multi_export_data(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value����
                                    CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                    CWX_UINT64* ullSeq, ///<���к�
                                    char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///pack export���ݵ�reply��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_data_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                   char* buf,  ///<pack�����ݿռ�
                                   CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                   CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                   CWX_UINT64 ullSeq, ///<���к�
                                   char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse export���ݵ�reply��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value����
                                    CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                    CWX_UINT64*  ullSeq, ///<���к�
                                    char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///pack export��ɵ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_export_end(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                            char* buf,  ///<pack�����ݿռ�
                            CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                            CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                            CWX_UINT64 ullSid, ///<���ʱ��sid
                            char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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


///parse export��ɵ���Ϣ������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_export_end(struct CWX_PG_READER_EX* reader, ///<reader
                             char const* msg, ///<key/value����
                             CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                             CWX_UINT64* ullSid,///<���ʱ��sid
                             char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///pack report�Ļظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_report_data_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                   char* buf,  ///<pack�����ݿռ�
                                   CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                   CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                   CWX_UINT64 ullSession, ///<session id
                                   char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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


///����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
///parse report�Ļظ����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_report_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                    char const* msg, ///<key/value����
                                    CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                    CWX_UINT64* ullSession, ///<session id
                                    char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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


///pack sync��session���ӱ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_report_new_conn(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                 CWX_UINT64 ullSession, ///<����������session
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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

///parse sync��session���ӱ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_report_new_conn(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value����
                                  CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                  CWX_UINT64* ullSession, ///<����������session
                                  char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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

///pack report��sync�ĳ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_sync_err(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                          char* buf,  ///<pack�����ݿռ�
                          CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                          CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                          int ret, ///<�������
                          char const* szErrMsg, ///<������Ϣ
                          char* szErr2K///<pack����ʱ�Ĵ�����Ϣ
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

///parse report��sync�ĳ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_sync_err(struct CWX_PG_READER_EX* reader, ///<reader
                           char const* msg, ///<key/value����
                           CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                           int* ret,  ///<�������
                           char const** szErrMsg,  ///<������Ϣ
                           char* szErr2K ///<���ʱ�Ĵ�����Ϣ
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
        if (strcmp(sign, UNISTOR_KEY_CRC32) == 0){//CRC32ǩ��
            CWX_UINT32 uiCrc32 = cwx_crc32_value(cwx_pg_writer_get_msg_ex(writer), cwx_pg_writer_get_msg_size_ex(writer));
            if (0 != cwx_pg_writer_add_key_ex(writer, UNISTOR_KEY_CRC32, (char*)&uiCrc32, sizeof(uiCrc32), 0)){
                if (szErr2K) strcpy(szErr2K, cwx_pg_writer_get_error_ex(writer));
                return UNISTOR_ERR_ERROR;
            }
        }else if (strcmp(sign, UNISTOR_KEY_MD5) == 0){//md5ǩ��
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

///pack ����binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_multi_sync_data(CWX_UINT32 uiTaskId, ///<����id
                                 char const* szData, ///<������Ϣ������buf
                                 CWX_UINT32 uiDataLen, ///<�������ݵ�����buf����
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT64 ullSeq, ///<��Ϣ������Ϣ���к�
                                 CWX_UINT8  zip, ///<�Ƿ�ѹ��
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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


///pack sync binlog�Ļظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_pack_sync_data_reply(struct CWX_PG_WRITER_EX* writer, ///<����pack��writer
                                 char* buf,  ///<pack�����ݿռ�
                                 CWX_UINT32* buf_len, ///<����ռ�Ĵ�С������pack������ݴ�С
                                 CWX_UINT32 uiTaskId, ///<��Ϣ����task id
                                 CWX_UINT64 ullSeq, ///<��Ϣ�����к�
                                 CWX_UINT16 unMsgType, ///<��Ϣ����
                                 char* szErr2K ///<pack����ʱ�Ĵ�����Ϣ
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


///parse sync binlog�Ļظ����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
int unistor_parse_sync_data_reply(struct CWX_PG_READER_EX* reader, ///<reader
                                  char const* msg, ///<key/value����
                                  CWX_UINT32  msg_len, ///<key/value���ݵĳ���
                                  CWX_UINT64* ullSeq, ///<��Ϣ�����к�
                                  char* szErr2K  ///<���ʱ�Ĵ�����Ϣ
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

///��������ͬ������seq��
void unistor_set_seq(char* szBuf, CWX_UINT64 ullSeq){
    CWX_UINT32 byte4 = (CWX_UINT32)(ullSeq>>32);
    byte4 = htonl(byte4);
    memcpy(szBuf, &byte4, 4);
    byte4 = (CWX_UINT32)(ullSeq&0xFFFFFFFF);
    byte4 = htonl(byte4);
    memcpy(szBuf + 4, &byte4, 4);

}    
///��ȡ����ͬ������seq��
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
