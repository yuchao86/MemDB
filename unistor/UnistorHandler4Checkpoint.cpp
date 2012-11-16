#include "UnistorHandler4Checkpoint.h"
#include "UnistorApp.h"


///checkpoint检测
int UnistorHandler4Checkpoint::onTimeoutCheck(CwxMsgBlock*& , CwxTss* pThrEvn )
{
    UnistorTss* tss = (UnistorTss*)pThrEvn;

    CWX_INFO(("Starting check point............"));
	m_bCheckOuting = true;
	m_pApp->getStore()->checkpoint(tss);
	m_bCheckOuting = false;
	m_uiLastCheckTime = time(NULL);
	CWX_INFO(("End check point............"));
    return 1;
}

/// return -1：处理失败，0：不处理此事件，1：处理此事件。
int UnistorHandler4Checkpoint::onUserEvent(CwxMsgBlock*& msg, CwxTss* pThrEnv)
{
    UnistorTss* pTss = (UnistorTss*)pThrEnv;
    if (msg->event().getEvent() >= EVENT_STORE_MSG_START){
        if (0 != m_pApp->getStore()->storeEvent(pTss, msg)){
            CWX_ERROR(("UnistorHandler4Checkpoint: failure to deal store event, err:%s", pTss->m_szBuf2K));
        }
    }else{
        CWX_ERROR(("UnistorHandler4Checkpoint: unknown event type:%u", msg->event().getEvent()));
        return 0;
    }
    return 1;
}


///检测是否需要checkpoint
bool UnistorHandler4Checkpoint::isNeedCheckOut(time_t now) const {
    static CWX_UINT32 ttTimeBase = 0;
    if (UnistorApp::isClockBack(ttTimeBase, now)){
        return true;
    }
    if (m_bCheckOuting) return false;
    if (m_uiLastCheckTime + UNISTOR_CHECKOUT_INTERNAL < now) return true;
    return false;
}
