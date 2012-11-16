#include "UnistorPoco.h"
#include "CwxZlib.h"

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvImport(CwxPackageWriterEx* writer,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* extra,
                             CwxKeyValueItemEx const& data,
                             CWX_UINT32 uiExpire,
                             CWX_UINT32 uiVersion,
                             bool       bCache,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (uiExpire){
        if (!writer->addKeyValue(UNISTOR_KEY_E, strlen(UNISTOR_KEY_E), uiExpire)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bCache){
        if (!writer->addKeyValue(UNISTOR_KEY_C, strlen(UNISTOR_KEY_C), (CWX_UINT32)0)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P),  passwd)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvImport(CwxPackageWriterEx* writer,
                             CwxMsgBlock*& msg,
                             CWX_UINT32 uiTaskId,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* extra,
                             CwxKeyValueItemEx const& data,
                             CWX_UINT32 uiExpire,
                             CWX_UINT32 uiVersion,
                             bool       bCache,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
    int ret = UNISTOR_ERR_SUCCESS;
    writer->beginPack();
    if (UNISTOR_ERR_SUCCESS != (ret = packRecvImport(writer,
        key,
        extra,
        data,
        uiExpire,
        uiVersion,
        bCache,
        user,
        passwd,
        szErr2K)))
    {
        return ret;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }

    CwxMsgHead head(0, 0, MSG_TYPE_RECV_IMPORT, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
    memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, writer->getMsg(), writer->getMsgSize());
    msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());        
    return UNISTOR_ERR_SUCCESS;
}


int UnistorPoco::parseRecvImport(CwxPackageReaderEx* reader,
                              CwxKeyValueItemEx const*& key,
                              CwxKeyValueItemEx const*& extra,
                              CwxKeyValueItemEx const*& data,
                              CWX_UINT32& uiExpire,
                              CWX_UINT32& uiVersion,
                              bool&       bCache,
                              char const*& user,
                              char const*& passwd,
                              char* szErr2K)
{
    //get key
    key = reader->getKey(UNISTOR_KEY_K);
    if (!key){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
    //get data
    data = reader->getKey(UNISTOR_KEY_D);
    if (!data){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    if (data->m_bKeyValue){
        if (!CwxPackageEx::isValidPackage(data->m_szData, data->m_uiDataLen)){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get timeout
    if (!reader->getKey(UNISTOR_KEY_E, uiExpire)){
        uiExpire = 0;
    }
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
        uiVersion = 0;
    }
    //get cache
    CWX_UINT32 uiValue=0;
    if (!reader->getKey(UNISTOR_KEY_C, uiValue)){
        bCache = true;
    }else{
        bCache = uiValue?true:false;
    }
    CwxKeyValueItemEx const* item;
    //get user
    item = reader->getKey(UNISTOR_KEY_U);
    if (!item){
        user = NULL;
    }else{
        user=item->m_szData;
    }
    //get passwd
    item = reader->getKey(UNISTOR_KEY_P);
    if (!item){
        passwd = NULL;
    }else{
        passwd = item->m_szData;
    }
    return UNISTOR_ERR_SUCCESS;
}


///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvAdd(CwxPackageWriterEx* writer,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             CwxKeyValueItemEx const& data,
                             CWX_UINT32 uiExpire,
                             CWX_UINT32 uiSign,
                             CWX_UINT32 uiVersion,
                             bool       bCache,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (uiExpire){
        if (!writer->addKeyValue(UNISTOR_KEY_E, strlen(UNISTOR_KEY_E), uiExpire)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiSign){
        if (!writer->addKeyValue(UNISTOR_KEY_SIGN, strlen(UNISTOR_KEY_SIGN), uiSign)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bCache){
        if (!writer->addKeyValue(UNISTOR_KEY_C, strlen(UNISTOR_KEY_C), (CWX_UINT32)0)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P),  passwd)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvAdd(CwxPackageWriterEx* writer,
						   CwxMsgBlock*& msg,
						   CWX_UINT32 uiTaskId,
                           CwxKeyValueItemEx const& key,
                           CwxKeyValueItemEx const* field,
                           CwxKeyValueItemEx const* extra,
                           CwxKeyValueItemEx const& data,
                           CWX_UINT32 uiExpire,
                           CWX_UINT32 uiSign,
                           CWX_UINT32 uiVersion,
                           bool       bCache,
                           char const* user,
                           char const* passwd,
						   char* szErr2K)
{
    int ret = UNISTOR_ERR_SUCCESS;
    writer->beginPack();
    if (UNISTOR_ERR_SUCCESS != (ret = packRecvAdd(writer,
        key,
        field,
        extra,
        data,
        uiExpire,
        uiSign,
        uiVersion,
        bCache,
        user,
        passwd,
        szErr2K)))
    {
        return ret;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }

	CwxMsgHead head(0, 0, MSG_TYPE_RECV_ADD, uiTaskId, writer->getMsgSize());
	msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());
	if (!msg){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
		return UNISTOR_ERR_ERROR;
	}
	memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
	memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, writer->getMsg(), writer->getMsgSize());
	msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());        
	return UNISTOR_ERR_SUCCESS;
}


int UnistorPoco::parseRecvAdd(CwxPackageReaderEx* reader,
                              CwxKeyValueItemEx const*& key,
                              CwxKeyValueItemEx const*& field,
                              CwxKeyValueItemEx const*& extra,
                              CwxKeyValueItemEx const*& data,
                              CWX_UINT32& uiExpire,
                              CWX_UINT32& uiSign,
                              CWX_UINT32& uiVersion,
                              bool&       bCache,
                              char const*& user,
                              char const*& passwd,
                              char* szErr2K)
{
	//get key
	key = reader->getKey(UNISTOR_KEY_K);
	if (!key){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
		return UNISTOR_ERR_ERROR;
	}
    //get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
	//get data
	data = reader->getKey(UNISTOR_KEY_D);
	if (!data){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
		return UNISTOR_ERR_ERROR;
	}
	if (data->m_bKeyValue){
		if (!CwxPackageEx::isValidPackage(data->m_szData, data->m_uiDataLen)){
			if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
			return UNISTOR_ERR_ERROR;
		}
	}
	//get timeout
	if (!reader->getKey(UNISTOR_KEY_E, uiExpire)){
		uiExpire = 0;
	}
    //get isfield
    CWX_UINT32 uiValue=0;
    if (!reader->getKey(UNISTOR_KEY_SIGN, uiSign)){
        uiSign = 0;
    }
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
        uiVersion = 0;
    }
    //get cache
    if (!reader->getKey(UNISTOR_KEY_C, uiValue)){
        bCache = true;
    }else{
        bCache = uiValue?true:false;
    }
    CwxKeyValueItemEx const* item;
    //get user
    item = reader->getKey(UNISTOR_KEY_U);
    if (!item){
        user = NULL;
    }else{
        user=item->m_szData;
    }
    //get passwd
    item = reader->getKey(UNISTOR_KEY_P);
    if (!item){
        passwd = NULL;
    }else{
        passwd = item->m_szData;
    }
	return UNISTOR_ERR_SUCCESS;
}


///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvSet(CwxPackageWriterEx* writer,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             CwxKeyValueItemEx const& data,
                             CWX_UINT32 uiSign,
                             CWX_UINT32 uiExpire,
                             CWX_UINT32 uiVersion,
                             bool   bCache,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
	if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
		if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
		return UNISTOR_ERR_ERROR;
	}
    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
	if (uiSign){
		if (!writer->addKeyValue(UNISTOR_KEY_SIGN, strlen(UNISTOR_KEY_SIGN),  uiSign)){
			if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
			return UNISTOR_ERR_ERROR;
		}
	}
    if (uiExpire){
		if (!writer->addKeyValue(UNISTOR_KEY_E, strlen(UNISTOR_KEY_E), uiExpire)){
			if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
			return UNISTOR_ERR_ERROR;
		}
	}
    if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bCache){
        if (!writer->addKeyValue(UNISTOR_KEY_C, strlen(UNISTOR_KEY_C), (CWX_UINT32)0)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user, strlen(user))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), passwd, strlen(passwd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

int UnistorPoco::packRecvSet(CwxPackageWriterEx* writer,
                             CwxMsgBlock*& msg,
                             CWX_UINT32 uiTaskId,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             CwxKeyValueItemEx const& data,
                             CWX_UINT32 uiSign,
                             CWX_UINT32 uiExpire,
                             CWX_UINT32 uiVersion,
                             bool bCache,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
    writer->beginPack();
    int ret = packRecvSet(writer,
        key,
        field,
        extra,
        data,
        uiSign,
        uiExpire,
        uiVersion,
        bCache,
        user,
        passwd,
        szErr2K);
    if (UNISTOR_ERR_SUCCESS != ret) return ret;
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_RECV_SET, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
	memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
	memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, writer->getMsg(), writer->getMsgSize());
	msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());        
    return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvSet(CwxPackageReaderEx* reader,
                              CwxKeyValueItemEx const*& key,
                              CwxKeyValueItemEx const*& field,
                              CwxKeyValueItemEx const*& extra,
                              CwxKeyValueItemEx const*& data,
                              CWX_UINT32& uiSign,
                              CWX_UINT32& uiExpire,
                              CWX_UINT32& uiVersion,
                              bool& bCache,
                              char const*& user,
                              char const*& passwd,
                              char* szErr2K)
{
	//get key
	key = reader->getKey(UNISTOR_KEY_K);
	if (!key){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
		return UNISTOR_ERR_ERROR;
	}
    //get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
	//get data
	data = reader->getKey(UNISTOR_KEY_D);
	if (!data){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
		return UNISTOR_ERR_ERROR;
	}
	if (data->m_bKeyValue){
		if (!CwxPackageEx::isValidPackage(data->m_szData, data->m_uiDataLen)){
			if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
			return UNISTOR_ERR_ERROR;
		}
	}
	//get field
	CWX_UINT32 uiValue=0;
	if (!reader->getKey(UNISTOR_KEY_SIGN, uiSign)){
        uiSign = 0;
	}
    //get cache
    bCache = true;
    if (reader->getKey(UNISTOR_KEY_C, uiValue)){
        bCache = uiValue?true:false;
    }
	//get timeout
	if (!reader->getKey(UNISTOR_KEY_E, uiExpire)){
		uiExpire = 0;
	}
    //get v
    if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
        uiVersion = 0;
    }
    CwxKeyValueItemEx const* item;
    //get user
    item = reader->getKey(UNISTOR_KEY_U);
    if (!item){
        user = NULL;
    }else{
        user=item->m_szData;
    }
    //get passwd
    item = reader->getKey(UNISTOR_KEY_P);
    if (!item){
        passwd = NULL;
    }else{
        passwd = item->m_szData;
    }
	return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvUpdate(CwxPackageWriterEx* writer,
                                CwxKeyValueItemEx const& key,
                                CwxKeyValueItemEx const* field,
                                CwxKeyValueItemEx const* extra,
                                CwxKeyValueItemEx const& data,
                                CWX_UINT32 uiSign,
                                CWX_UINT32 uiExpire,
                                CWX_UINT32 uiVersion,
                                char const* user,
                                char const* passwd,
                                char* szErr2K
                                )
{
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (uiSign){
        if (!writer->addKeyValue(UNISTOR_KEY_SIGN, strlen(UNISTOR_KEY_SIGN), uiSign)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiExpire){
        if (!writer->addKeyValue(UNISTOR_KEY_E, strlen(UNISTOR_KEY_E), uiExpire)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user, strlen(user))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), passwd, strlen(passwd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvUpdate(CwxPackageWriterEx* writer,
						   CwxMsgBlock*& msg,
						   CWX_UINT32 uiTaskId,
                           CwxKeyValueItemEx const& key,
                           CwxKeyValueItemEx const* field,
                           CwxKeyValueItemEx const* extra,
						   CwxKeyValueItemEx const& data,
                           CWX_UINT32 uiSign,
                           CWX_UINT32 uiExpire,
                           CWX_UINT32 uiVersion,
                           char const* user,
                           char const* passwd,
						   char* szErr2K)
{
	writer->beginPack();
    int ret = packRecvUpdate(writer,
        key,
        field,
        extra,
        data,
        uiSign,
        uiExpire,
        uiVersion,
        user,
        passwd,
        szErr2K);
    if (UNISTOR_ERR_SUCCESS != ret) return ret;
	if (!writer->pack()){
		if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
		return UNISTOR_ERR_ERROR;
	}
	CwxMsgHead head(0, 0, MSG_TYPE_RECV_UPDATE, uiTaskId, writer->getMsgSize());
	msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());
	if (!msg){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
		return UNISTOR_ERR_ERROR;
	}
	memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
	memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, writer->getMsg(), writer->getMsgSize());
	msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());        
	return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvUpdate(CwxPackageReaderEx* reader,
                                 CwxKeyValueItemEx const*& key,
                                 CwxKeyValueItemEx const*& field,
                                 CwxKeyValueItemEx const*& extra,
                                 CwxKeyValueItemEx const*& data,
                                 CWX_UINT32& uiSign,
                                 CWX_UINT32& uiExpire,
                                 CWX_UINT32& uiVersion,
                                 char const*& user,
                                 char const*& passwd,
                                 char* szErr2K)
{
    //get key
    key = reader->getKey(UNISTOR_KEY_K);
    if (!key){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
    //get data
    data = reader->getKey(UNISTOR_KEY_D);
    if (!data){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    if (data->m_bKeyValue){
        if (!CwxPackageEx::isValidPackage(data->m_szData, data->m_uiDataLen)){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "key[%s] is key/value, but it's format is not valid..", UNISTOR_KEY_D);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get sign
    if (!reader->getKey(UNISTOR_KEY_SIGN, uiSign)){
        uiSign = 0;
    }
    //get timeout
    if (!reader->getKey(UNISTOR_KEY_E, uiExpire)){
        uiExpire = 0;
    }
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
        uiVersion = 0;
    }
    CwxKeyValueItemEx const* item;
    //get user
    item = reader->getKey(UNISTOR_KEY_U);
    if (!item){
        user = NULL;
    }else{
        user=item->m_szData;
    }
    //get passwd
    item = reader->getKey(UNISTOR_KEY_P);
    if (!item){
        passwd = NULL;
    }else{
        passwd = item->m_szData;
    }
    return UNISTOR_ERR_SUCCESS;
}


int UnistorPoco::packRecvInc(CwxPackageWriterEx* writer,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             CWX_INT64   num,
                             CWX_INT64   result,
                             CWX_INT64   max,
                             CWX_INT64   min,
                             CWX_UINT32  uiExpire,
                             CWX_UINT32  uiSign,
                             CWX_UINT32  uiVersion,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->addKeyValue(UNISTOR_KEY_N, strlen(UNISTOR_KEY_N), num)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (result){
        if (!writer->addKeyValue(UNISTOR_KEY_R, strlen(UNISTOR_KEY_R), result)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (max){
        if (!writer->addKeyValue(UNISTOR_KEY_MAX, strlen(UNISTOR_KEY_MAX), max)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (min){
        if (!writer->addKeyValue(UNISTOR_KEY_MIN, strlen(UNISTOR_KEY_MIN), min)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiExpire){
        if (!writer->addKeyValue(UNISTOR_KEY_E, strlen(UNISTOR_KEY_E), uiExpire)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiSign){
        if (!writer->addKeyValue(UNISTOR_KEY_SIGN, strlen(UNISTOR_KEY_SIGN), uiSign)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user, strlen(user))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), passwd, strlen(passwd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvInc(CwxPackageWriterEx* writer,
							  CwxMsgBlock*& msg,
							  CWX_UINT32 uiTaskId,
                              CwxKeyValueItemEx const& key,
                              CwxKeyValueItemEx const* field,
                              CwxKeyValueItemEx const* extra,
                              CWX_INT64   num,
                              CWX_INT64   result,
                              CWX_INT64   max,
                              CWX_INT64   min,
                              CWX_UINT32  uiExpire,
                              CWX_UINT32  uiSign,
                              CWX_UINT32  uiVersion,
                              char const* user,
                              char const* passwd,
							  char* szErr2K){
	writer->beginPack();
    int ret = packRecvInc(writer,
        key,
        field,
        extra,
        num,
        result,
        max,
        min,
        uiExpire,
        uiSign,
        uiVersion,
        user,
        passwd,
        szErr2K);
    if (UNISTOR_ERR_SUCCESS != ret) return ret;
	if (!writer->pack()){
		if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
		return UNISTOR_ERR_ERROR;
	}
	CwxMsgHead head(0, 0, MSG_TYPE_RECV_INC, uiTaskId, writer->getMsgSize());
	msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());
	if (!msg){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
		return UNISTOR_ERR_ERROR;
	}
	memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
	memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, writer->getMsg(), writer->getMsgSize());
	msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());        
	return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvInc(CwxPackageReaderEx* reader,
                              CwxKeyValueItemEx const*& key,
                              CwxKeyValueItemEx const*& field,
                              CwxKeyValueItemEx const*& extra,
                              CWX_INT64&   num,
                              CWX_INT64&   result,
                              CWX_INT64&   max,
                              CWX_INT64&   min,
                              CWX_UINT32& uiExpire,
                              CWX_UINT32&  uiSign,
                              CWX_UINT32&  uiVersion,
                              char const*& user,
                              char const*& passwd,
							   char* szErr2K){
	CwxKeyValueItemEx const* item;
	//get key
	key = reader->getKey(UNISTOR_KEY_K);
	if (!key){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
		return UNISTOR_ERR_ERROR;
	}
    //get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
	//get num
	if (!reader->getKey(UNISTOR_KEY_N, num)){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_N);
		return UNISTOR_ERR_ERROR;
	}
    //get result
    result = 0;
    reader->getKey(UNISTOR_KEY_R, result);
    //get max
    max = 0;
    min = 0;
    if (num > 0){
        reader->getKey(UNISTOR_KEY_MAX, max);
    }else if (num < 0){
        reader->getKey(UNISTOR_KEY_MIN, min);
    }
    //get expire
    if (!reader->getKey(UNISTOR_KEY_E, uiExpire)){
        uiExpire = 0;
    }
    //get sign
    if (!reader->getKey(UNISTOR_KEY_SIGN, uiSign)){
        uiSign = 0;
    }
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
        uiVersion = 0;
    }
    //get user
    item = reader->getKey(UNISTOR_KEY_U);
    if (!item){
        user = NULL;
    }else{
        user=item->m_szData;
    }
    //get passwd
    item = reader->getKey(UNISTOR_KEY_P);
    if (!item){
        passwd = NULL;
    }else{
        passwd = item->m_szData;
    }
	return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvDel(CwxPackageWriterEx* writer,
                             CwxKeyValueItemEx const& key,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             CWX_UINT32 uiVersion,
                             char const* user,
                             char const* passwd,
                             char* szErr2K)
{
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if(!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if(!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (user && user[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user, strlen(user))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (passwd && passwd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), passwd, strlen(passwd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvDel(CwxPackageWriterEx* writer,
						   CwxMsgBlock*& msg,
						   CWX_UINT32 uiTaskId,
                           CwxKeyValueItemEx const& key,
                           CwxKeyValueItemEx const* field,
                           CwxKeyValueItemEx const* extra,
                           CWX_UINT32  uiVersion,
                           char const* user,
                           char const* passwd,
						   char* szErr2K)
{
	writer->beginPack();
    int ret = packRecvDel(writer,
        key,
        field,
        extra,
        uiVersion,
        user,
        passwd,
        szErr2K);
    if (UNISTOR_ERR_SUCCESS != ret) return ret;
	if (!writer->pack()){
		if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
		return UNISTOR_ERR_ERROR;
	}
	CwxMsgHead head(0, 0, MSG_TYPE_RECV_DEL, uiTaskId, writer->getMsgSize());
	msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());
	if (!msg){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
		return UNISTOR_ERR_ERROR;
	}
	memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
	memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN, writer->getMsg(), writer->getMsgSize());
	msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize());        
	return UNISTOR_ERR_SUCCESS;
}

///返回值，UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvDel(CwxPackageReaderEx* reader,
                              CwxKeyValueItemEx const*& key,
                              CwxKeyValueItemEx const*& field,
                              CwxKeyValueItemEx const*& extra,
                              CWX_UINT32& uiVersion,
                              char const*& user,
                              char const*& passwd,
                              char* szErr2K)
{
	//get key
	key = reader->getKey(UNISTOR_KEY_K);
	if (!key){
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
		return UNISTOR_ERR_ERROR;
	}
	//get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
        uiVersion = 0;
    }
    CwxKeyValueItemEx const* item;
    //get user
    item = reader->getKey(UNISTOR_KEY_U);
    if (!item){
        user = NULL;
    }else{
        user=item->m_szData;
    }
    //get passwd
    item = reader->getKey(UNISTOR_KEY_P);
    if (!item){
        passwd = NULL;
    }else{
        passwd = item->m_szData;
    }
	return UNISTOR_ERR_SUCCESS;
}



///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvReply(CwxPackageWriterEx* writer,
                             CwxMsgBlock*& msg,
                             CWX_UINT32 uiTaskId,
							 CWX_UINT16 unMsgType,
                             int ret,
                             CWX_UINT32 uiVersion,
                             CWX_UINT32 uiFieldNum,
                             char const* szErrMsg,
                             char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_RET, strlen(UNISTOR_KEY_RET),  ret)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (UNISTOR_ERR_SUCCESS != ret){
        if (!writer->addKeyValue(UNISTOR_KEY_ERR,
            strlen(UNISTOR_KEY_ERR),
            szErrMsg?szErrMsg:"",
            szErrMsg?strlen(szErrMsg):0)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }else{
        if (uiVersion){
            if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
                if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
                return UNISTOR_ERR_ERROR;
            }
        }
        if (uiFieldNum){
            if (!writer->addKeyValue(UNISTOR_KEY_FN, strlen(UNISTOR_KEY_FN), uiFieldNum)){
                if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
                return UNISTOR_ERR_ERROR;
            }
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, unMsgType, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvReply(CwxPackageReaderEx* reader,
                                  CwxMsgBlock const* msg,
                                  int& ret,
                                  CWX_UINT32& uiVersion,
                                  CWX_UINT32& uiFieldNum,
                                  char const*& szErrMsg,
                                  char* szErr2K){
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get ret
    if (!reader->getKey(UNISTOR_KEY_RET, ret)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    if (UNISTOR_ERR_SUCCESS != ret){
        CwxKeyValueItemEx const* pItem = NULL;
        if (!(pItem = reader->getKey(UNISTOR_KEY_ERR))){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
            return UNISTOR_ERR_ERROR;
        }
        szErrMsg = pItem->m_szData;
    }else{
        szErrMsg = "";
        if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
            uiVersion = 0;
        }
        if (!reader->getKey(UNISTOR_KEY_FN, uiFieldNum)){
            uiFieldNum = 0;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvIncReply(CwxPackageWriterEx* writer,
                            CwxMsgBlock*& msg,
                            CWX_UINT32 uiTaskId,
                            CWX_UINT16 unMsgType,
                            int ret,
                            CWX_INT64 llNum,
                            CWX_UINT32 uiVersion,
                            char const* szErrMsg,
                            char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_RET, strlen(UNISTOR_KEY_RET), ret)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (UNISTOR_ERR_SUCCESS != ret){
        if (!writer->addKeyValue(UNISTOR_KEY_ERR,
            strlen(UNISTOR_KEY_ERR),
            szErrMsg?szErrMsg:"",
            szErrMsg?strlen(szErrMsg):0)){
                if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
                return UNISTOR_ERR_ERROR;
        }
    }else if (uiVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), uiVersion)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
        if (!writer->addKeyValue(UNISTOR_KEY_N, strlen(UNISTOR_KEY_N), llNum)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, unMsgType, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvIncReply(CwxPackageReaderEx* reader,
                             CwxMsgBlock const* msg,
                             int& ret,
                             CWX_UINT32& uiVersion,
                             CWX_INT64& llNum,
                             char const*& szErrMsg,
                             char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get ret
    if (!reader->getKey(UNISTOR_KEY_RET, ret)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    if (UNISTOR_ERR_SUCCESS != ret){
        CwxKeyValueItemEx const* pItem = NULL;
        if (!(pItem = reader->getKey(UNISTOR_KEY_ERR))){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
            return UNISTOR_ERR_ERROR;
        }
        szErrMsg = pItem->m_szData;
    }else{
        szErrMsg = "";
        if (!reader->getKey(UNISTOR_KEY_V, uiVersion)){
            uiVersion = 0;
        }
        if (!reader->getKey(UNISTOR_KEY_N, llNum)){
            llNum = 0;
        }
    }
    return UNISTOR_ERR_SUCCESS;

}


int UnistorPoco::packGetKey(CwxPackageWriterEx* writer,
                            CwxKeyValueItemEx const& key,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* extra,
                            bool bVersion,
                            char const* szUser,
                            char const* szPasswd,
                            CWX_UINT8 ucKeyInfo,
                            char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, key.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bVersion){
        if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), (CWX_UINT32)1)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szUser && szUser[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), szUser, strlen(szUser))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szPasswd && szPasswd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), szPasswd, strlen(szPasswd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (ucKeyInfo){
        if (!writer->addKeyValue(UNISTOR_KEY_I, strlen(UNISTOR_KEY_I), ucKeyInfo)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packGetKey(CwxPackageWriterEx* writer,
                      CwxMsgBlock*& msg,
                      CWX_UINT32 uiTaskId,
                      CwxKeyValueItemEx const& key,
                      CwxKeyValueItemEx const* field,
                      CwxKeyValueItemEx const* extra,
                      bool bVersion,
                      char const* szUser,
                      char const* szPasswd,
                      bool bMaster,
                      CWX_UINT8 ucKeyInfo,
                      char* szErr2K)
{
    int ret = 0;
    if (UNISTOR_ERR_SUCCESS != (ret = packGetKey(writer,
        key,
        field,
        extra,
        bVersion,
        szUser,
        szPasswd,
        ucKeyInfo,
        szErr2K)))
    {
        return ret;
    }
    CWX_UINT8 ucAttr=0;
    if (bMaster) setFromMaster(ucAttr);
    CwxMsgHead head(0, ucAttr, MSG_TYPE_RECV_GET, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
int UnistorPoco::parseGetKey(CwxPackageReaderEx* reader,
                       CwxKeyValueItemEx const*& key,
                       CwxKeyValueItemEx const*& field,
                       CwxKeyValueItemEx const*& extra,
                       bool&        bVersion,
                       char const*& szUser,
                       char const*& szPasswd,
                       CWX_UINT8&   ucKeyInfo,
                       char*        szErr2K)
{
    CwxKeyValueItemEx const* pItem = NULL;
    CWX_UINT32 uiValue=0;
    //get key
    if (!(key=reader->getKey(UNISTOR_KEY_K))){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
    //get version
    bVersion = false;
    if (reader->getKey(UNISTOR_KEY_V, uiValue)){
        bVersion=uiValue?true:false;
    }
    //get user
    szUser = NULL;
    pItem = reader->getKey(UNISTOR_KEY_U);
    if (pItem) szUser = pItem->m_szData;
    //get passwd
    szPasswd = NULL;
    pItem = reader->getKey(UNISTOR_KEY_P);
    if (pItem) szPasswd = pItem->m_szData;
    //get keyinfo
    if (!reader->getKey(UNISTOR_KEY_I, ucKeyInfo)){
        ucKeyInfo = 0; 
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packExistKey(CwxPackageWriterEx* writer,
                              CwxKeyValueItemEx const& key,
                              CwxKeyValueItemEx const* field,
                              CwxKeyValueItemEx const* extra,
                              bool bVersion,
                              char const* szUser,
                              char const* szPasswd,
                              char* szErr2K)
{
    return packGetKey(writer,
        key,
        field,
        extra,
        bVersion,
        szUser,
        szPasswd,
        false,
        szErr2K);
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packExistKey(CwxPackageWriterEx* writer,
                        CwxMsgBlock*& msg,
                        CWX_UINT32 uiTaskId,
                        CwxKeyValueItemEx const& key,
                        CwxKeyValueItemEx const* field,
                        CwxKeyValueItemEx const* extra,
                        bool bVersion,
                        char const* szUser,
                        char const* szPasswd,
                        bool bMaster,
                        char* szErr2K)
{
    int ret = 0;
    if (UNISTOR_ERR_SUCCESS != (ret = packExistKey(writer,
        key,
        field,
        extra,
        bVersion,
        szUser,
        szPasswd,
        szErr2K)))
    {
        return ret;
    }
    CWX_UINT8 ucAttr=0;
    if (bMaster) setFromMaster(ucAttr);
    CwxMsgHead head(0, ucAttr, MSG_TYPE_RECV_EXIST, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseExistKey(CwxPackageReaderEx* reader,
                         CwxKeyValueItemEx const*& key,
                         CwxKeyValueItemEx const*& field,
                         CwxKeyValueItemEx const*& extra,
                         bool&        bVersion,
                         char const*& szUser,
                         char const*& szPasswd,
                         char*        szErr2K)
{
    CWX_UINT8 ucKeyInfo=0;
    return parseGetKey(reader,
        key,
        field,
        extra,
        bVersion,
        szUser,
        szPasswd,
        ucKeyInfo,
        szErr2K);
}

int UnistorPoco::packGetKeys(CwxPackageWriterEx* writer,
                             CwxPackageWriterEx* writer1,
                             list<pair<char const*, CWX_UINT16> > const& keys,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             char const* szUser,
                             char const* szPasswd,
                             CWX_UINT8 ucKeyInfo,
                             char* szErr2K)
{
    writer->beginPack();

    list<pair<char const*, CWX_UINT16> >::const_iterator iter = keys.begin();
    if (iter == keys.end()){
        if (szErr2K) strcpy(szErr2K, "No key");
        return UNISTOR_ERR_ERROR;
    }
    iter++;
    if (iter == keys.end()){
        iter = keys.begin();
        if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), iter->first, iter->second, false)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }else{
        writer1->beginPack();
        iter = keys.begin();
        while(iter != keys.end()){
            if (!writer1->addKeyValue("", 0, iter->first, iter->second)){
                if (szErr2K) strcpy(szErr2K, writer1->getErrMsg());
                return UNISTOR_ERR_ERROR;
            }
            iter++;
        }
        writer1->pack();
        if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), writer1->getMsg(), writer1->getMsgSize(), true)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }

    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szUser && szUser[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), szUser, strlen(szUser))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szPasswd && szPasswd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), szPasswd, strlen(szPasswd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (ucKeyInfo){
        if (!writer->addKeyValue(UNISTOR_KEY_I, strlen(UNISTOR_KEY_I), ucKeyInfo)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packGetKeys(CwxPackageWriterEx* writer,
                             CwxPackageWriterEx* writer1,
                            CwxMsgBlock*& msg,
                            CWX_UINT32 uiTaskId,
                            list<pair<char const*, CWX_UINT16> > const& keys,
                            CwxKeyValueItemEx const* field,
                            CwxKeyValueItemEx const* extra,
                            char const* szUser,
                            char const* szPasswd,
                            bool bMaster,
                            CWX_UINT8 ucKeyInfo,
                            char* szErr2K)
{
    int ret = 0;
    if (UNISTOR_ERR_SUCCESS != ( ret = packGetKeys(writer,
        writer1,
        keys,
        field,
        extra,
        szUser,
        szPasswd,
        ucKeyInfo,
        szErr2K)))
    {
        return ret;
    }
    CWX_UINT8 ucAttr=0;
    if (bMaster) setFromMaster(ucAttr);
    CwxMsgHead head(0, ucAttr, MSG_TYPE_RECV_GETS, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
int UnistorPoco::parseGetKeys(CwxPackageReaderEx* reader,
                              CwxPackageReaderEx* reader1,
                              list<pair<char const*, CWX_UINT16> >& keys,
                              CWX_UINT32& uiKeyNum,
                              CwxKeyValueItemEx const*& field,
                              CwxKeyValueItemEx const*& extra,
                              char const*& szUser,
                              char const*& szPasswd,
                              CWX_UINT8&   ucKeyInfo,
                              char*        szErr2K)
{
    CwxKeyValueItemEx const* pItem = NULL;
    //get key
    if (!(pItem=reader->getKey(UNISTOR_KEY_K))){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    if (pItem->m_bKeyValue){
        if (!reader1->unpack(pItem->m_szData, pItem->m_uiDataLen, false, true)){
            if (szErr2K) strcpy(szErr2K, reader1->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
        keys.clear();
        for (CWX_UINT32 i=0; i<reader1->getKeyNum(); i++){
            keys.push_back(pair<char const*, CWX_UINT16>(reader1->getKey(i)->m_szData, reader1->getKey(i)->m_uiDataLen));
        }
        uiKeyNum = reader1->getKeyNum();
    }else{
        keys.push_back(pair<char const*, CWX_UINT16>(pItem->m_szData, pItem->m_uiDataLen));
        uiKeyNum = 1;
    }
    //get field
    field = reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra = reader->getKey(UNISTOR_KEY_X);
    //get user
    szUser = NULL;
    pItem = reader->getKey(UNISTOR_KEY_U);
    if (pItem) szUser = pItem->m_szData;
    //get passwd
    szPasswd = NULL;
    pItem = reader->getKey(UNISTOR_KEY_P);
    if (pItem) szPasswd = pItem->m_szData;
    //get info
    if (!reader->getKey(UNISTOR_KEY_I, ucKeyInfo)){
        ucKeyInfo = 0;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packGetList(CwxPackageWriterEx* writer,
                             CwxKeyValueItemEx const* begin,
                             CwxKeyValueItemEx const* end,
                             CWX_UINT16  num,
                             CwxKeyValueItemEx const* field,
                             CwxKeyValueItemEx const* extra,
                             bool        bAsc,
                             bool        bBegin,
                             bool        bKeyInfo,
                             char const* szUser,
                             char const* szPasswd,
                             char* szErr2K)
{
    writer->beginPack();
    if(begin && begin->m_uiDataLen){
        if (!writer->addKeyValue(UNISTOR_KEY_BEGIN, strlen(UNISTOR_KEY_BEGIN), begin->m_szData, begin->m_uiDataLen, begin->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if(end && end->m_uiDataLen){
        if (!writer->addKeyValue(UNISTOR_KEY_END, strlen(UNISTOR_KEY_END), end->m_szData, end->m_uiDataLen, end->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (num){
        if (!writer->addKeyValue(UNISTOR_KEY_N, strlen(UNISTOR_KEY_N), num)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (field){
        if (!writer->addKeyValue(UNISTOR_KEY_F, strlen(UNISTOR_KEY_F), field->m_szData, field->m_uiDataLen)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bAsc){
        if (!writer->addKeyValue(UNISTOR_KEY_ASC, strlen(UNISTOR_KEY_ASC), (CWX_UINT32)0)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!bBegin){
        if (!writer->addKeyValue(UNISTOR_KEY_IB, strlen(UNISTOR_KEY_IB), (CWX_UINT32)0)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (bKeyInfo){
        if (!writer->addKeyValue(UNISTOR_KEY_I, strlen(UNISTOR_KEY_I), (CWX_UINT32)1)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szUser && szUser[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), szUser, strlen(szUser))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szPasswd && szPasswd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), szPasswd, strlen(szPasswd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packGetList(CwxPackageWriterEx* writer,
                       CwxMsgBlock*& msg,
                       CWX_UINT32 uiTaskId,
                       CwxKeyValueItemEx const* begin,
                       CwxKeyValueItemEx const* end,
                       CWX_UINT16  num,
                       CwxKeyValueItemEx const* field,
                       CwxKeyValueItemEx const* extra,
                       bool        bAsc,
                       bool        bBegin,
                       bool        bKeyInfo,
                       char const* szUser,
                       char const* szPasswd,
                       bool bMaster,
                       char* szErr2K)
{
    int ret = 0;
    if (UNISTOR_ERR_SUCCESS != ( ret = packGetList(writer,
        begin,
        end,
        num,
        field,
        extra,
        bAsc,
        bBegin,
        bKeyInfo,
        szUser,
        szPasswd,
        szErr2K)))
    {
        return ret;
    }
    CWX_UINT8 ucAttr=0;
    if (bMaster) setFromMaster(ucAttr);
    CwxMsgHead head(0, ucAttr, MSG_TYPE_RECV_LIST, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseGetList(CwxPackageReaderEx* reader,
                        CwxKeyValueItemEx const*& begin,
                        CwxKeyValueItemEx const*& end,
                        CWX_UINT16&  num,
                        CwxKeyValueItemEx const*& field,
                        CwxKeyValueItemEx const*& extra,
                        bool&        bAsc,
                        bool&        bBegin,
                        bool&        bKeyInfo,
                        char const*& szUser,
                        char const*& szPasswd,
                        char*        )
{
    CwxKeyValueItemEx const* pItem = NULL;
    CWX_UINT32 uiValue=0;
    //get begin
    begin=reader->getKey(UNISTOR_KEY_BEGIN);
    //get end
    end = reader->getKey(UNISTOR_KEY_END);
    //get num
    num = 0;
    reader->getKey(UNISTOR_KEY_N, num);
    //get field
    field=reader->getKey(UNISTOR_KEY_F);
    //get extra
    extra=reader->getKey(UNISTOR_KEY_X);
    //get asc
    if (reader->getKey(UNISTOR_KEY_ASC, uiValue)){
        bAsc = uiValue?true:false;
    }else{
        bAsc = true;
    }
    //get isbegin
    if (reader->getKey(UNISTOR_KEY_IB, uiValue)){
        bBegin = uiValue?true:false;
    }else{
        bBegin = true;
    }
    //get ik
    if (reader->getKey(UNISTOR_KEY_I, uiValue)){
        bKeyInfo = uiValue?true:false;
    }else{
        bKeyInfo = false;
    }
    szUser = NULL;
    pItem = reader->getKey(UNISTOR_KEY_U);
    if (pItem) szUser = pItem->m_szData;
    //get passwd
    szPasswd = NULL;
    pItem = reader->getKey(UNISTOR_KEY_P);
    if (pItem) szPasswd = pItem->m_szData;
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvAuth(CwxPackageWriterEx* writer,
                        CwxMsgBlock*& msg,
                        CWX_UINT32 uiTaskId,
                        char const* szUser,
                        char const* szPasswd,
                        char* szErr2K)
{
    writer->beginPack();
    if (szUser && szUser[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), szUser, strlen(szUser))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (szPasswd && szPasswd[0]){
        if (!writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), szPasswd, strlen(szPasswd))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_RECV_AUTH, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvAuth(CwxPackageReaderEx* reader,
                         CwxMsgBlock const* msg,
                         char const*& szUser,
                         char const*& szPasswd,
                         char*     szErr2K)
{
    CwxKeyValueItemEx const* pItem = NULL;
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }

    szUser = NULL;
    pItem = reader->getKey(UNISTOR_KEY_U);
    if (pItem) szUser = pItem->m_szData;
    //get passwd
    szPasswd = NULL;
    pItem = reader->getKey(UNISTOR_KEY_P);
    if (pItem) szPasswd = pItem->m_szData;
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packRecvAuthReply(CwxPackageWriterEx* writer,
                             CwxMsgBlock*& msg,
                             CWX_UINT32 uiTaskId,
                             CWX_UINT16 unMsgType,
                             int ret,
                             char const* szErrMsg,
                             char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_RET, strlen(UNISTOR_KEY_RET), ret)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (UNISTOR_ERR_SUCCESS != ret){
        if (!writer->addKeyValue(UNISTOR_KEY_ERR, strlen(UNISTOR_KEY_ERR), szErrMsg, strlen(szErrMsg))){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, unMsgType, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseRecvAuthReply(CwxPackageReaderEx* reader,
                              CwxMsgBlock const* msg,
                              int& ret,
                              char const*& szErrMsg,
                              char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get ret
    if (!reader->getKey(UNISTOR_KEY_RET, ret)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    if (UNISTOR_ERR_SUCCESS != ret){
        CwxKeyValueItemEx const* pItem = NULL;
        if (!(pItem = reader->getKey(UNISTOR_KEY_ERR))){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
            return UNISTOR_ERR_ERROR;
        }
        szErrMsg = pItem->m_szData;
    }else{
        szErrMsg = "";
    }
    return UNISTOR_ERR_SUCCESS;
}



///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packExportReport(CwxPackageWriterEx* writer,
                            CwxMsgBlock*& msg,
                            CWX_UINT32 uiTaskId,
                            CWX_UINT32  uiChunkSize,
                            char const* subscribe,
                            char const* key,
                            char const* extra,
                            char const* user,
                            char const* passwd,
                            char* szErr2K)
{
    writer->beginPack();
    if (uiChunkSize && !writer->addKeyValue(UNISTOR_KEY_CHUNK, strlen(UNISTOR_KEY_CHUNK), uiChunkSize)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (subscribe && !writer->addKeyValue(UNISTOR_KEY_SUBSCRIBE, strlen(UNISTOR_KEY_SUBSCRIBE), subscribe, strlen(subscribe))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (key && !writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key, strlen(key))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (extra && !writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra, strlen(extra))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (user && !writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user, strlen(user))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (passwd && !writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), passwd, strlen(passwd))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_EXPORT_REPORT, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseExportReport(CwxPackageReaderEx* reader,
                           CwxMsgBlock const* msg,
                           CWX_UINT32&  uiChunkSize,
                           char const*& subscribe,
                           char const*& key,
                           char const*& extra,
                           char const*& user,
                           char const*& passwd,
                           char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!reader->getKey(UNISTOR_KEY_CHUNK, uiChunkSize)){
        uiChunkSize = UNISTOR_DEF_CHUNK_SIZE_KB;
    }
    CwxKeyValueItemEx const* pItem = NULL;
    //get subscribe
    if (!(pItem = reader->getKey(UNISTOR_KEY_SUBSCRIBE))){
        subscribe = "";
    }else{
        subscribe = pItem->m_szData;
    }
    //get key
    if (!(pItem = reader->getKey(UNISTOR_KEY_K))){
        key = NULL;
    }else{
        key = pItem->m_szData;
    }
    //get extra
    if (!(pItem = reader->getKey(UNISTOR_KEY_X))){
        extra = NULL;
    }else{
        extra = pItem->m_szData;
    }
    //get user
    if (!(pItem = reader->getKey(UNISTOR_KEY_U))){
        user = "";
    }else{
        user = pItem->m_szData;
    }
    //get passwd
    if (!(pItem = reader->getKey(UNISTOR_KEY_P))){
        passwd = "";
    }else{
        passwd = pItem->m_szData;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packExportReportReply(CwxPackageWriterEx* writer,
                                 CwxMsgBlock*& msg,
                                 CWX_UINT32 uiTaskId,
                                 CWX_UINT64 ullSession,
                                 CWX_UINT64 ullSid,
                                 char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_SESSION, strlen(UNISTOR_KEY_SESSION), ullSession)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_SID, strlen(UNISTOR_KEY_SID), ullSid)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_EXPORT_REPORT_REPLY, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseExportReportReply(CwxPackageReaderEx* reader,
                                  CwxMsgBlock const* msg,
                                  CWX_UINT64& ullSession,
                                  CWX_UINT64& ullSid,
                                  char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!reader->getKey(UNISTOR_KEY_SESSION, ullSession)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SESSION);
        return UNISTOR_ERR_ERROR;
    }
    //get sid
    if (!reader->getKey(UNISTOR_KEY_SID, ullSid)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SID);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packExportDataItem(CwxPackageWriterEx* writer,
                                    CwxKeyValueItemEx const& key,
                                    CwxKeyValueItemEx const& data,
                                    CwxKeyValueItemEx const* extra,
                                    CWX_UINT32 version,
                                    CWX_UINT32 expire,
                                    char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_K, strlen(UNISTOR_KEY_K), key.m_szData, key.m_uiDataLen, false)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (extra){
        if (!writer->addKeyValue(UNISTOR_KEY_X, strlen(UNISTOR_KEY_X), extra->m_szData, extra->m_uiDataLen, extra->m_bKeyValue)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), version)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_E, strlen(UNISTOR_KEY_E), expire)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseExportDataItem(CwxPackageReaderEx* reader,
                                     char const* szData,
                                     CWX_UINT32  uiDataLen,
                                     CwxKeyValueItemEx const*& key,
                                     CwxKeyValueItemEx const*& data,
                                     CwxKeyValueItemEx const*& extra, ///<extra
                                     CWX_UINT32& version,
                                     CWX_UINT32& expire,
                                     char* szErr2K)
{
    if (!reader->unpack(szData, uiDataLen, false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get key
    if (!(key=reader->getKey(UNISTOR_KEY_K))){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_K);
        return UNISTOR_ERR_ERROR;
    }
    //get data
    if (!(data=reader->getKey(UNISTOR_KEY_D))){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    extra = reader->getKey(UNISTOR_KEY_E);
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, version)){
        version = 0;
    }
    //get expire
    if (!reader->getKey(UNISTOR_KEY_E, expire)){
        expire = 0;
    }
    return UNISTOR_ERR_SUCCESS;
}

int UnistorPoco::packMultiExportData(
                               CWX_UINT32 uiTaskId,
                               char const* szData,
                               CWX_UINT32 uiDataLen,
                               CwxMsgBlock*& msg,
                               CWX_UINT64 ullSeq,
                               char* szErr2K)
{
    CwxMsgHead head(0, 0, MSG_TYPE_EXPORT_DATA, uiTaskId, uiDataLen+sizeof(ullSeq));
    msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + uiDataLen + UNISTOR_ZIP_EXTRA_BUF + sizeof(ullSeq));
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", uiDataLen);
        return UNISTOR_ERR_ERROR;
    }
    memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
    memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN + sizeof(ullSeq), szData, uiDataLen);
    msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + uiDataLen + sizeof(ullSeq));
    //seq seq
    setSeq(msg->rd_ptr() + CwxMsgHead::MSG_HEAD_LEN, ullSeq);
    return UNISTOR_ERR_SUCCESS;

}
int UnistorPoco::parseMultiExportData(
                                CwxPackageReaderEx* reader,
                                CwxMsgBlock const* msg,
                                CWX_UINT64& ullSeq,
                                char* szErr2K)
{
    if (msg->length() < sizeof(ullSeq)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Data Length[%u] is too less, no seq id", msg->length());
        return UNISTOR_ERR_ERROR;
    }
    ullSeq = getSeq(msg->rd_ptr());
    if (!reader->unpack(msg->rd_ptr() + sizeof(ullSeq), msg->length()-sizeof(ullSeq), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packExportDataReply(CwxPackageWriterEx* ,
                               CwxMsgBlock*& msg,
                               CWX_UINT32 uiTaskId,
                               CWX_UINT64 ullSeq,
                               char* szErr2K)
{
    char szBuf[9];
    setSeq(szBuf, ullSeq);
    CwxMsgHead head(0, 0, MSG_TYPE_EXPORT_DATA_REPLY, uiTaskId, sizeof(ullSeq));
    msg = CwxMsgBlockAlloc::pack(head, szBuf, sizeof(ullSeq));
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", sizeof(ullSeq));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseExportDataReply(CwxPackageReaderEx* ,
                                CwxMsgBlock const* msg,
                                CWX_UINT64& ullSeq,
                                char* szErr2K)
{
    if (msg->length() < sizeof(ullSeq)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Data Length[%u] is too less, no seq id", msg->length());
        return UNISTOR_ERR_ERROR;
    }
    ullSeq = getSeq(msg->rd_ptr());
    return UNISTOR_ERR_SUCCESS;
}

int UnistorPoco::packExportEnd(CwxPackageWriterEx* writer,
                         CwxMsgBlock*& msg,
                         CWX_UINT32 uiTaskId,
                         CWX_UINT64 ullSid,
                         char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_SID, strlen(UNISTOR_KEY_SID), ullSid)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }

    CwxMsgHead head(0, 0, MSG_TYPE_EXPORT_END, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", CwxMsgHead::MSG_HEAD_LEN);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


int UnistorPoco::parseExportEnd(CwxPackageReaderEx* reader,
                                CwxMsgBlock const* msg,
                                CWX_UINT64& ullSid,
                                char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!reader->getKey(UNISTOR_KEY_SID, ullSid)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv package.", UNISTOR_KEY_SID);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}



int UnistorPoco::packReportData(CwxPackageWriterEx* writer,
                          CwxMsgBlock*& msg,
                          CWX_UINT32 uiTaskId,
                          CWX_UINT64 ullSid,
                          bool      bNewly,
                          CWX_UINT32  uiChunkSize,
                          char const* subscribe,
                          char const* user,
                          char const* passwd,
                          char const* sign,
                          bool  zip,
                          char* szErr2K)
{
    writer->beginPack();
    if (!bNewly){
        if (!writer->addKeyValue(UNISTOR_KEY_SID, strlen(UNISTOR_KEY_SID), ullSid)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }
    if (uiChunkSize && !writer->addKeyValue(UNISTOR_KEY_CHUNK, strlen(UNISTOR_KEY_CHUNK), uiChunkSize)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (subscribe && !writer->addKeyValue(UNISTOR_KEY_SUBSCRIBE, strlen(UNISTOR_KEY_SUBSCRIBE), subscribe, strlen(subscribe))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (user && !writer->addKeyValue(UNISTOR_KEY_U, strlen(UNISTOR_KEY_U), user, strlen(user))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (passwd && !writer->addKeyValue(UNISTOR_KEY_P, strlen(UNISTOR_KEY_P), passwd, strlen(passwd))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (sign){
        if ((strcmp(sign, UNISTOR_KEY_CRC32) == 0) || (strcmp(sign, UNISTOR_KEY_MD5)==0)){
            if (!writer->addKeyValue(UNISTOR_KEY_SIGN, strlen(UNISTOR_KEY_SIGN), sign, strlen(sign))){
                if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
                return UNISTOR_ERR_ERROR;
            }
        }
    }
    if (zip){
        if (!writer->addKeyValue(UNISTOR_KEY_ZIP, strlen(UNISTOR_KEY_ZIP), zip)){
            if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
            return UNISTOR_ERR_ERROR;
        }
    }

    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_SYNC_REPORT, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseReportData(CwxPackageReaderEx* reader,
                           CwxMsgBlock const* msg,
                           CWX_UINT64& ullSid,
                           bool& bNewly,
                           CWX_UINT32&  uiChunkSize,
                           char const*& subscribe,
                           char const*& user,
                           char const*& passwd,
                           char const*& sign,
                           bool&        zip,
                           char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get sid
    if (!reader->getKey(UNISTOR_KEY_SID, ullSid)){
        bNewly = true;
    }else{
        bNewly = false;
    }
    if (!reader->getKey(UNISTOR_KEY_CHUNK, uiChunkSize)){
        uiChunkSize = 0;
    }
    CwxKeyValueItemEx const* pItem = NULL;
    //get subscribe
    if (!(pItem = reader->getKey(UNISTOR_KEY_SUBSCRIBE))){
        subscribe = "";
    }else{
        subscribe = pItem->m_szData;
    }
    //get user
    if (!(pItem = reader->getKey(UNISTOR_KEY_U))){
        user = "";
    }else{
        user = pItem->m_szData;
    }
    //get passwd
    if (!(pItem = reader->getKey(UNISTOR_KEY_P))){
        passwd = "";
    }else{
        passwd = pItem->m_szData;
    }
    //get sign
    if (!(pItem = reader->getKey(UNISTOR_KEY_SIGN))){
        sign = "";
    }else{
        if (strcmp(pItem->m_szData, UNISTOR_KEY_CRC32)==0){
            sign = UNISTOR_KEY_CRC32;
        }else if (strcmp(pItem->m_szData, UNISTOR_KEY_MD5)==0){
            sign = UNISTOR_KEY_MD5;
        }else{
            sign = "";
        }
    }
    CWX_UINT32 uiValue=0;
    if (!reader->getKey(UNISTOR_KEY_ZIP, uiValue)){
        zip = false;
    }else{
        zip = uiValue?true:false;
    }
    return UNISTOR_ERR_SUCCESS;
}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packReportDataReply(CwxPackageWriterEx* writer,
                               CwxMsgBlock*& msg,
                               CWX_UINT32 uiTaskId,
                               CWX_UINT64 ullSession,
                               char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_SESSION, strlen(UNISTOR_KEY_SESSION), ullSession)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_SYNC_REPORT_REPLY, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseReportDataReply(CwxPackageReaderEx* reader,
                                CwxMsgBlock const* msg,
                                CWX_UINT64& ullSession,
                                char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!reader->getKey(UNISTOR_KEY_SESSION, ullSession)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SESSION);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packReportNewConn(CwxPackageWriterEx* writer,
                             CwxMsgBlock*& msg,
                             CWX_UINT32 uiTaskId,
                             CWX_UINT64 ullSession,
                             char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_SESSION, strlen(UNISTOR_KEY_SESSION), ullSession)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_SYNC_CONN, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseReportNewConn(CwxPackageReaderEx* reader,
                              CwxMsgBlock const* msg,
                              CWX_UINT64& ullSession,
                              char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get session
    if (!reader->getKey(UNISTOR_KEY_SESSION, ullSession)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SESSION);
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packSyncErr(CwxPackageWriterEx* writer,
                                  CwxMsgBlock*& msg,
                                  CWX_UINT32 uiTaskId,
                                  int ret,
                                  char const* szErrMsg,
                                  char* szErr2K)
{
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_RET, strlen(UNISTOR_KEY_RET), ret)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_ERR, strlen(UNISTOR_KEY_ERR), szErrMsg, strlen(szErrMsg))){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_SYNC_ERR, uiTaskId, writer->getMsgSize());
    msg = CwxMsgBlockAlloc::pack(head, writer->getMsg(), writer->getMsgSize());
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;

}
///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseSyncErr(CwxPackageReaderEx* reader,
                                   CwxMsgBlock const* msg,
                                   int& ret,
                                   char const*& szErrMsg,
                                   char* szErr2K)
{
    if (!reader->unpack(msg->rd_ptr(), msg->length(), false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get ret
    if (!reader->getKey(UNISTOR_KEY_RET, ret)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_RET);
        return UNISTOR_ERR_ERROR;
    }
    //get err
    CwxKeyValueItemEx const* pItem = NULL;
    if (!(pItem = reader->getKey(UNISTOR_KEY_ERR))){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_ERR);
        return UNISTOR_ERR_ERROR;
    }
    szErrMsg = pItem->m_szData;
    return UNISTOR_ERR_SUCCESS;
}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packSyncData(CwxPackageWriterEx* writer,
                        CwxMsgBlock*& msg,
                        CWX_UINT32 uiTaskId,
                        CWX_UINT64 ullSid,
                        CWX_UINT32 uiTimeStamp,
                        CwxKeyValueItemEx const& data,
                        CWX_UINT32 group,
                        CWX_UINT32 type,
                        CWX_UINT32 version,
                        CWX_UINT64 ullSeq,
                        char const* sign,
                        bool       zip,
                        char* szErr2K){
    writer->beginPack();
    int ret = packSyncDataItem(writer,
        ullSid,
        uiTimeStamp,
        data,
        group,
        type,
        version,
        sign,
        szErr2K);
    if (UNISTOR_ERR_SUCCESS != ret) return ret;

    if (!writer->pack()){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    CwxMsgHead head(0, 0, MSG_TYPE_SYNC_DATA, uiTaskId, writer->getMsgSize() + sizeof(ullSeq));
    msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize() + UNISTOR_ZIP_EXTRA_BUF + sizeof(ullSeq));
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", writer->getMsgSize());
        return UNISTOR_ERR_ERROR;
    }
    unsigned long ulDestLen = writer->getMsgSize() + UNISTOR_ZIP_EXTRA_BUF;
    if (zip){
        if (!CwxZlib::zip((unsigned char*)msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN + sizeof(ullSeq), 
            ulDestLen,
            (unsigned char const*)writer->getMsg(),
            writer->getMsgSize()))
        {
            zip = false;
        }
    }
    if (zip){
        head.addAttr(CwxMsgHead::ATTR_COMPRESS);
        head.setDataLen(ulDestLen + sizeof(ullSeq));
        memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
        msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + ulDestLen + sizeof(ullSeq));
    }else{
        memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
        memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN + sizeof(ullSeq), writer->getMsg(), writer->getMsgSize());
        msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + writer->getMsgSize() + sizeof(ullSeq));        
    }
    //seq seq
    setSeq(msg->rd_ptr() + CwxMsgHead::MSG_HEAD_LEN, ullSeq);
    return UNISTOR_ERR_SUCCESS;
}

int UnistorPoco::packSyncDataItem(CwxPackageWriterEx* writer,
                            CWX_UINT64 ullSid,
                            CWX_UINT32 uiTimeStamp,
                            CwxKeyValueItemEx const& data,
                            CWX_UINT32 group,
                            CWX_UINT32 type,
                            CWX_UINT32 version,
                            char const* sign,
                            char* szErr2K){
    writer->beginPack();
    if (!writer->addKeyValue(UNISTOR_KEY_SID, strlen(UNISTOR_KEY_SID), ullSid)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_T, strlen(UNISTOR_KEY_T), uiTimeStamp)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_D, strlen(UNISTOR_KEY_D), data.m_szData, data.m_uiDataLen, data.m_bKeyValue)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_G, strlen(UNISTOR_KEY_G), group)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_TYPE, strlen(UNISTOR_KEY_TYPE), type)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (!writer->addKeyValue(UNISTOR_KEY_V, strlen(UNISTOR_KEY_V), version)){
        if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    if (sign){
		if (strcmp(sign, UNISTOR_KEY_CRC32) == 0){//CRC32签名
            CWX_UINT32 uiCrc32 = CwxCrc32::value(writer->getMsg(), writer->getMsgSize());
            if (!writer->addKeyValue(UNISTOR_KEY_CRC32, strlen(UNISTOR_KEY_CRC32), (char*)&uiCrc32, sizeof(uiCrc32))){
                if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
                return UNISTOR_ERR_ERROR;
            }
		}else if (strcmp(sign, UNISTOR_KEY_MD5) == 0){//md5签名
            CwxMd5 md5;
            unsigned char szMd5[16];
            md5.update((char unsigned*)writer->getMsg(), writer->getMsgSize());
            md5.final(szMd5);
            if (!writer->addKeyValue(UNISTOR_KEY_MD5, strlen(UNISTOR_KEY_MD5), (char*)szMd5, 16)){
                if (szErr2K) strcpy(szErr2K, writer->getErrMsg());
                return UNISTOR_ERR_ERROR;
            }
        }
    }
    writer->pack();
    return UNISTOR_ERR_SUCCESS;
}

int UnistorPoco::packMultiSyncData(CWX_UINT32 uiTaskId,
                                 char const* szData,
                                 CWX_UINT32 uiDataLen,
                                 CwxMsgBlock*& msg,
                                 CWX_UINT64 ullSeq,
                                 bool  zip,
                                 char* szErr2K
                                 )
{
    CwxMsgHead head(0, 0, MSG_TYPE_SYNC_DATA_CHUNK, uiTaskId, uiDataLen+sizeof(ullSeq));
    msg = CwxMsgBlockAlloc::malloc(CwxMsgHead::MSG_HEAD_LEN + uiDataLen + UNISTOR_ZIP_EXTRA_BUF + sizeof(ullSeq));
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", uiDataLen);
        return UNISTOR_ERR_ERROR;
    }
    unsigned long ulDestLen = uiDataLen + UNISTOR_ZIP_EXTRA_BUF;
    if (zip){
        if (!CwxZlib::zip((unsigned char*)(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN) + sizeof(ullSeq),
            ulDestLen,
            (unsigned char const*)szData,
            uiDataLen))
        {
            zip = false;
        }
    }
    if (zip){
        head.addAttr(CwxMsgHead::ATTR_COMPRESS);
        head.setDataLen(ulDestLen + sizeof(ullSeq));
        memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
        msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + ulDestLen + sizeof(ullSeq));
    }else{
        memcpy(msg->wr_ptr(), head.toNet(), CwxMsgHead::MSG_HEAD_LEN);
        memcpy(msg->wr_ptr() + CwxMsgHead::MSG_HEAD_LEN + sizeof(ullSeq), szData, uiDataLen);
        msg->wr_ptr(CwxMsgHead::MSG_HEAD_LEN + uiDataLen + sizeof(ullSeq));
    }
    //seq seq
    setSeq(msg->rd_ptr() + CwxMsgHead::MSG_HEAD_LEN, ullSeq);
    return UNISTOR_ERR_SUCCESS;
}


int UnistorPoco::parseSyncData(CwxPackageReaderEx* reader,
                         CwxMsgBlock const* msg,
                         CWX_UINT64& ullSid,
                         CWX_UINT32& uiTimeStamp,
                         CwxKeyValueItemEx const*& data,
                         CWX_UINT32& group,
                         CWX_UINT32& type,
                         CWX_UINT32& version,
                         char* szErr2K){
    return parseSyncData(reader,
        msg->rd_ptr(),
        msg->length(),
        ullSid,
        uiTimeStamp,
        data,
        group,
        type,
        version,
        szErr2K);
}

///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::parseSyncData(CwxPackageReaderEx* reader,
                         char const* szData,
                         CWX_UINT32 uiDataLen,
                         CWX_UINT64& ullSid,
                         CWX_UINT32& uiTimeStamp,
                         CwxKeyValueItemEx const*& data,
                         CWX_UINT32& group,
                         CWX_UINT32& type,
                         CWX_UINT32& version,
                         char* szErr2K){
    if (!reader->unpack(szData, uiDataLen, false, true)){
        if (szErr2K) strcpy(szErr2K, reader->getErrMsg());
        return UNISTOR_ERR_ERROR;
    }
    //get SID
    if (!reader->getKey(UNISTOR_KEY_SID, ullSid)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_SID);
        return UNISTOR_ERR_ERROR;
    }
    //get timestamp
    if (!reader->getKey(UNISTOR_KEY_T, uiTimeStamp)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_T);
        return UNISTOR_ERR_ERROR;
    }
    //get data
    if (!(data=reader->getKey(UNISTOR_KEY_D))){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No key[%s] in recv page.", UNISTOR_KEY_D);
        return UNISTOR_ERR_ERROR;
    }
    //get group
    if (!reader->getKey(UNISTOR_KEY_G, group)){
        group = 0;
    }
    //get type
    if (!reader->getKey(UNISTOR_KEY_TYPE, type)){
        type=0;
    }
    //get version
    if (!reader->getKey(UNISTOR_KEY_V, version)){
        version = 0;
    }
    CwxKeyValueItemEx const* pItem = NULL;
    //get crc32
    if ((pItem = reader->getKey(UNISTOR_KEY_CRC32))){
        CWX_UINT32 uiOrgCrc32 = 0;
        memcpy(&uiOrgCrc32, pItem->m_szData, sizeof(uiOrgCrc32));
        CWX_UINT32 uiCrc32 = CwxCrc32::value(szData, pItem->m_szKey - szData - CwxPackageEx::getKeyOffset(pItem->m_unKeyLen, pItem->m_uiDataLen));
        if (uiCrc32 != uiOrgCrc32){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "CRC32 signture error. recv signture:%x, local signture:%x", uiOrgCrc32, uiCrc32);
            return UNISTOR_ERR_ERROR;
        }
    }
    //get md5
    if ((pItem = reader->getKey(UNISTOR_KEY_MD5))){
        unsigned char szMd5[16];
        CwxMd5 md5;
        md5.update((unsigned char*)szData, pItem->m_szKey - szData - CwxPackageEx::getKeyOffset(pItem->m_unKeyLen, pItem->m_uiDataLen));
        md5.final(szMd5);
        if (memcmp(szMd5, pItem->m_szData, 16) != 0){
            if (szErr2K){
                char szTmp1[33];
                char szTmp2[33];
                CWX_UINT32 i=0;
                for (i=0; i<16; i++){
                    sprintf(szTmp1 + i*2, "%2.2x", (unsigned char)pItem->m_szData[i]);
                    sprintf(szTmp2 + i*2, "%2.2x", szMd5[i]);
                }
                CwxCommon::snprintf(szErr2K, 2047, "MD5 signture error. recv signture:%s, local signture:%s", szTmp1, szTmp2);
            }
            return UNISTOR_ERR_ERROR;
        }
    }
    return UNISTOR_ERR_SUCCESS;
}


///返回值：UNISTOR_ERR_SUCCESS：成功；其他都是失败
int UnistorPoco::packSyncDataReply(CwxPackageWriterEx* ,
                            CwxMsgBlock*& msg,
                            CWX_UINT32 uiTaskId,
                            CWX_UINT64 ullSeq,
                            CWX_UINT16 unMsgType,
                            char* szErr2K)
{
    char szBuf[9];
    setSeq(szBuf, ullSeq);
    CwxMsgHead head(0, 0, unMsgType, uiTaskId, sizeof(ullSeq));
    msg = CwxMsgBlockAlloc::pack(head, szBuf, sizeof(ullSeq));
    if (!msg){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "No memory to alloc msg, size:%u", sizeof(ullSeq));
        return UNISTOR_ERR_ERROR;
    }
    return UNISTOR_ERR_SUCCESS;
}


int UnistorPoco::parseSyncDataReply(CwxPackageReaderEx* ,
                             CwxMsgBlock const* msg,
                             CWX_UINT64& ullSeq,
                             char* szErr2K)
{
    if (msg->length() < sizeof(ullSeq)){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Data Length[%u] is too less, no seq id", msg->length());
        return UNISTOR_ERR_ERROR;
    }
    ullSeq = getSeq(msg->rd_ptr());
    return UNISTOR_ERR_SUCCESS;
}

