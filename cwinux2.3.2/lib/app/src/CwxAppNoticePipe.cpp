#include "CwxAppNoticePipe.h"
CWINUX_BEGIN_NAMESPACE

///���캯��
CwxAppNoticePipe::CwxAppNoticePipe()
{
    m_bPipeEmpty = true;
    m_noticeHead = NULL;
    m_noticeTail = NULL;
    m_pipeReader = CWX_INVALID_HANDLE;
    m_pipeWriter = CWX_INVALID_HANDLE;
}
///��������
CwxAppNoticePipe::~CwxAppNoticePipe()
{
    clear();
}

CWINUX_END_NAMESPACE
