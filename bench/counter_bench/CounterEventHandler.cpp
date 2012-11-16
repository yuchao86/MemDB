#include "CounterEventHandler.h"
#include "CounterBenchApp.h"
#include "CwxMd5.h"

CounterEventHandler::CounterEventHandler(CounterBenchApp* pApp):m_pApp(pApp){
	m_uiSendNum = 0;
	m_uiSuccessNum = 0;
	m_uiRecvNum = 0;
    m_uiBufLen = 0;
    if ((m_pApp->getConfig().m_strOpr == "add")||
        (m_pApp->getConfig().m_strOpr == "set") ||
        (m_pApp->getConfig().m_strOpr == "import") ||
        (m_pApp->getConfig().m_strOpr == "update"))
    {
        list<string>::const_iterator iter = m_pApp->getConfig().m_counters.begin();
        while(iter != m_pApp->getConfig().m_counters.end()){
            if (!m_uiBufLen){
                m_uiBufLen += sprintf(m_szBuf + m_uiBufLen, "%s=%u",
                    iter->c_str(), 
                    m_pApp->getConfig().m_uiValue);
            }else{
                m_uiBufLen += sprintf(m_szBuf + m_uiBufLen, ";%s=%u",
                    iter->c_str(), 
                    m_pApp->getConfig().m_uiValue);
            }
            iter++;
        }
        m_szBuf[m_uiBufLen] = 0x00;
    }
}

///连接建立
int CounterEventHandler::onConnCreated(CwxMsgBlock*& msg, CwxTss* pThrEnv){
	UnistorTss* tss = (UnistorTss*)pThrEnv;
	///发送一个echo数据包
	sendNextMsg(tss, msg->event().getSvrId(),
		msg->event().getHostId(),
		msg->event().getConnId());
	return 1;
}

///echo请求的处理函数
int CounterEventHandler::onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv){
	UnistorTss* tss = (UnistorTss*)pThrEnv;
	///收到的echo数据加1
	m_uiRecvNum++;
    if (tss->m_pReader->unpack(msg->rd_ptr(), msg->length(), false, true)){
		CwxKeyValueItemEx const* pItem = tss->m_pReader->getKey(UNISTOR_KEY_RET);
		if (pItem){
            if (UNISTOR_ERR_SUCCESS == atoi(pItem->m_szData)){
                m_uiSuccessNum++;
			}
		}
	}
	if (m_pApp->getConfig().m_bLasting){///如果是持久连接，则发送下一个echo请求数据包
		sendNextMsg(tss, msg->event().getSvrId(),
			msg->event().getHostId(),
			msg->event().getConnId());
    }else{
		m_pApp->noticeReconnect(msg->event().getConnId());
	}
	///若收到10000个数据包，则输出一条日志
	if (!(m_uiRecvNum%10000)){
		CWX_INFO(("opr[%s] Finish total-num=%u  send_num=%u  success-num=%u\n", m_pApp->getConfig().m_strOpr.c_str(),  m_uiRecvNum, m_uiSendNum, m_uiSuccessNum));
	}
    return 1;
}

//获取key
void CounterEventHandler::getKey(CWX_UINT32 uiKey, char* szKey){
    char const* szHex64="0000000000000000000000000000000000000000000000000000000000000000";
    char szBuf[32];
    if (m_pApp->getConfig().m_uiDataMod) uiKey = (uiKey%m_pApp->getConfig().m_uiDataMod);
    if (m_pApp->getConfig().m_strKeyType == "int32"){
        sprintf(szKey, "%u", uiKey);
    }else if(m_pApp->getConfig().m_strKeyType == "int64"){
        sprintf(szKey, "0x%x", uiKey);
    }else if(m_pApp->getConfig().m_strKeyType == "int128"){
        CWX_UINT32 uiLen = sprintf(szBuf, "%x", uiKey);
        memcpy(szKey, szHex64, 32-uiLen);
        memcpy(szKey + 32 - uiLen, szBuf, uiLen);
        szKey[32] = 0x00;
    }else if(m_pApp->getConfig().m_strKeyType == "int256"){
        CWX_UINT32 uiLen = sprintf(szBuf, "%x", uiKey);
        memcpy(szKey, szHex64, 64-uiLen);
        memcpy(szKey + 64 - uiLen, szBuf, uiLen);
        szKey[64] = 0x00;
    }else{//char
        sprintf(szKey, "0x%32.32x", uiKey);
    }
}

///发送查询
void CounterEventHandler::sendNextMsg(UnistorTss* tss,
                                      CWX_UINT32 uiSvrId,
                                      CWX_UINT32 uiHostId,
                                      CWX_UINT32 uiConnId)
{
	char szKey[256];
	CWX_UINT32 uiKey = 0;
    CwxMsgBlock* pBlock=NULL;
    CwxKeyValueItemEx data;
    CwxKeyValueItemEx key;
    CwxKeyValueItemEx field;

    m_uiSendNum++;

    key.m_szData = szKey;
    key.m_bKeyValue = false;
    if (m_pApp->getConfig().m_strOpr == "add"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        data.m_szData = m_szBuf;
        data.m_uiDataLen = m_uiBufLen;
        data.m_bKeyValue = false;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvAdd(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            NULL,
            data,
            0,
            1,
            0,
            m_pApp->getConfig().m_bCache,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack add msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }else if (m_pApp->getConfig().m_strOpr == "set"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        data.m_szData = m_szBuf;
        data.m_uiDataLen = m_uiBufLen;
        data.m_bKeyValue = false;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvSet(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            NULL,
            data,
            0,
            0,
            0,
            m_pApp->getConfig().m_bCache,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack set msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }else if (m_pApp->getConfig().m_strOpr == "update"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        data.m_szData = m_szBuf;
        data.m_uiDataLen = m_uiBufLen;
        data.m_bKeyValue = false;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvUpdate(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            NULL,
            data,
            false,
            0,
            0,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack update msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }else if (m_pApp->getConfig().m_strOpr == "import"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        data.m_szData = m_szBuf;
        data.m_uiDataLen = m_uiBufLen;
        data.m_bKeyValue = false;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvImport(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            data,
            0,
            0,
            m_pApp->getConfig().m_bCache,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack import msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }else if (m_pApp->getConfig().m_strOpr == "inc"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        field.m_szData = m_pApp->getConfig().m_counters.begin()->c_str();
        field.m_uiDataLen = m_pApp->getConfig().m_counters.begin()->length();
        field.m_bKeyValue = false;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvInc(tss->m_pWriter,
            pBlock,
            0,
            key,
            &field,
            NULL,
            m_pApp->getConfig().m_uiValue,
            0,
            0,
            0,
            0,
            0,
            0,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack inc msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }else if (m_pApp->getConfig().m_strOpr == "delete"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvDel(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            NULL,
            0,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack del msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }else if (m_pApp->getConfig().m_strOpr == "get"){
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
        if (m_pApp->getConfig().m_uiDataMod){
            uiKey %= m_pApp->getConfig().m_uiDataMod;
        }
        getKey(uiKey, szKey);
        key.m_uiDataLen = strlen(szKey);

        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packGetKey(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            NULL,
            false,
            m_pApp->getConfig().m_strUser.c_str(),
            m_pApp->getConfig().m_strPasswd.c_str(),
            m_pApp->getConfig().m_bGetMaster,
            0,
            tss->m_szBuf2K))
        {
            CWX_ERROR(("Failure to pack get msg, err=%s", tss->m_szBuf2K));
            m_pApp->stop();
            return;
        }
    }

	///设置消息的发送方式
	///设置消息的svr-id
	pBlock->send_ctrl().setSvrId(uiSvrId);
	///设置消息的host-id
	pBlock->send_ctrl().setHostId(uiHostId);
	///设置消息发送的连接id
	pBlock->send_ctrl().setConnId(uiConnId);
	///设置消息发送的user-data
	pBlock->send_ctrl().setUserData(NULL);
	///设置消息发送阶段的行为，包括开始发送是否通知、发送完成是否通知、发送失败是否通知
	pBlock->send_ctrl().setMsgAttr(CwxMsgSendCtrl::NONE);
	///发送echo请求
	if (0 != m_pApp->sendMsgByConn(pBlock)){
		CWX_ERROR(("Failure to send msg"));
		m_pApp->noticeCloseConn(uiConnId);
		return ;
	}
}


