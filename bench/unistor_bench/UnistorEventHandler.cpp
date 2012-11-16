#include "UnistorEventHandler.h"
#include "UnistorBenchApp.h"
#include "CwxMd5.h"

UnistorEventHandler::UnistorEventHandler(UnistorBenchApp* pApp):m_pApp(pApp){
	m_uiSendNum = 0;
	m_uiSuccessNum = 0;
	m_uiRecvNum = 0;
	CWX_UINT32 i=0;
	m_szBuf = new char[pApp->getConfig().m_uiDataSize + 1 + 1024];
	for (i=0; i<pApp->getConfig().m_uiDataSize + 1024; i++){
		m_szBuf[i] = 'a' + i % 26;
	}
	m_szBuf[i] = 0x00;
}

///连接建立
int UnistorEventHandler::onConnCreated(CwxMsgBlock*& msg, CwxTss* pThrEnv){
	UnistorTss* tss = (UnistorTss*)pThrEnv;
	///发送一个echo数据包
	sendNextMsg(tss, msg->event().getSvrId(),
		msg->event().getHostId(),
		msg->event().getConnId());
	return 1;
}

///echo请求的处理函数
int UnistorEventHandler::onRecvMsg(CwxMsgBlock*& msg, CwxTss* pThrEnv){
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
void UnistorEventHandler::getKey(CWX_UINT32 uiKey, char* szKey, bool bMd5){
    if (m_pApp->getConfig().m_uiDataMod) uiKey = (uiKey%m_pApp->getConfig().m_uiDataMod);
	if (!bMd5){
		sprintf(szKey, "0x%32.32x", uiKey);
	}else{
		sprintf(szKey, "%32.32x", uiKey);
		CwxMd5 md5;
		unsigned char szMd5[16];
		md5.update((unsigned char const*)szKey, strlen(szKey));
		md5.final(szMd5);
		strcpy(szKey, "0x");
		for (CWX_UINT32 i=0; i< 16; i++){
			sprintf(szKey + 2 + i * 2, "%2.2x", szMd5[i]);
		}
	}
}

///发送查询
void UnistorEventHandler::sendNextMsg(UnistorTss* tss,
                                      CWX_UINT32 uiSvrId,
                                      CWX_UINT32 uiHostId,
                                      CWX_UINT32 uiConnId)
{
	char szKey[64];
	CWX_UINT32 uiKey = 0;
	CwxMsgBlock* pBlock=NULL;
	CwxKeyValueItemEx data;
    CwxKeyValueItemEx key;
    int iRand = (rand()%m_pApp->getConfig().m_uiDataSize) + 16;

	m_uiSendNum++;

    if (m_pApp->getConfig().m_strOpr == "add"){
		uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex + m_pApp->getConfig().m_uiDataBase;
		getKey(uiKey, szKey, !m_pApp->getConfig().m_bKeyOrder);
        key.m_szData = szKey;
        key.m_uiDataLen = strlen(szKey);
        key.m_bKeyValue = false;
		memcpy(m_szBuf, szKey, 34);
		data.m_szData = m_szBuf;
		data.m_uiDataLen = iRand;
		data.m_bKeyValue = false;
		if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvAdd(tss->m_pWriter,
			pBlock,
			0,
			key,
            NULL,
            NULL,
			data,
			m_pApp->getConfig().m_uiExpire,
            0,
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
		uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex;
		getKey(uiKey, szKey, !m_pApp->getConfig().m_bKeyOrder);
        key.m_szData = szKey;
        key.m_uiDataLen = strlen(szKey);
        key.m_bKeyValue = false;
		memcpy(m_szBuf, szKey, 34);
		data.m_szData = m_szBuf;
		data.m_uiDataLen = iRand;
		data.m_bKeyValue = false;
		if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvSet(tss->m_pWriter,
			pBlock,
			0,
			key,
            NULL,
            NULL,
			data,
            0,
            m_pApp->getConfig().m_uiExpire,
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
		uiKey = m_pApp->getConfig().m_bKeyOrder?m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex:rand();
		getKey(uiKey, szKey, false);
        key.m_szData = szKey;
        key.m_uiDataLen = strlen(szKey);
        key.m_bKeyValue = false;
		memcpy(m_szBuf, szKey, 34);
		data.m_szData = m_szBuf;
		data.m_uiDataLen = iRand;
		data.m_bKeyValue = false;
		if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvUpdate(tss->m_pWriter,
			pBlock,
			0,
			key,
            NULL,
            NULL,
			data,
            false,
            m_pApp->getConfig().m_uiExpire,
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
        uiKey = m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex;
        getKey(uiKey, szKey, !m_pApp->getConfig().m_bKeyOrder);
        key.m_szData = szKey;
        key.m_uiDataLen = strlen(szKey);
        key.m_bKeyValue = false;
        memcpy(m_szBuf, szKey, 34);
        data.m_szData = m_szBuf;
        data.m_uiDataLen = iRand;
        data.m_bKeyValue = false;
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvImport(tss->m_pWriter,
            pBlock,
            0,
            key,
            NULL,
            data,
            m_pApp->getConfig().m_uiExpire,
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
	}else if (m_pApp->getConfig().m_strOpr == "delete"){
		uiKey = m_pApp->getConfig().m_bKeyOrder?m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex:rand();
		getKey(uiKey, szKey, false);
        key.m_szData = szKey;
        key.m_uiDataLen = strlen(szKey);
        key.m_bKeyValue = false;
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
		uiKey = m_pApp->getConfig().m_bGetOrder?m_uiSendNum * m_pApp->getConfig().m_uiKeyGroup + m_pApp->getConfig().m_uiKeyIndex:rand();
		getKey(uiKey, szKey, !m_pApp->getConfig().m_bKeyOrder);
        key.m_szData = szKey;
        key.m_uiDataLen = strlen(szKey);
        key.m_bKeyValue = false;
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


