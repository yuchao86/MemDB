#ifndef __CWX_APP_PROCESS_MGR_H__
#define __CWX_APP_PROCESS_MGR_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxAppProcessMgr.h
@brief ˫����ģʽ�Ľ��̹�����������ᴴ��˫���̣�һ���ǹ������̣�һ���ǹ������
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxAppMacro.h"
#include "CwxAppConfig.h"
#include "CwxStl.h"
#include "CwxAppFramework.h"

CWINUX_BEGIN_NAMESPACE

/**
@class CwxAppProcessInfo
@brief ����������Ϣ����
*/
class CWX_API CwxAppProcessInfo
{
public:
    ///��������״̬
    enum{
        PROC_STATE_RUNNING = 1,///<����״̬
        PROC_STATE_RESTARTING = 2,///<��������
        PROC_STATE_STOPPING = 3,///<����ֹͣ
        PROC_STATE_STOPPED = 4///<ֹͣ
    };
public:
    ///���캯��
    CwxAppProcessInfo()
    {
        m_pApp = NULL;
        m_unProcId = 0;
        m_pid = 0;
        m_ttLastChildHeat = 0;
        m_unState = PROC_STATE_STOPPED;
    }
    ///��������
    ~CwxAppProcessInfo()
    {
        if (m_pApp) delete m_pApp;
    }
public:
    CwxAppFramework*   m_pApp;///<�������̵Ķ���
    CWX_UINT16         m_unProcId;///<�������̵�ID
    pid_t              m_pid;///<�������̵�pid
    time_t             m_ttLastChildHeat;///<���������ϴη����������¼�
    CWX_UINT8          m_unState;///<�������̵�״̬
};

/**
@class CwxAppProcessMgr
@brief ˫����ģʽ�Ľ��̹�����������ᴴ��˫���̣�
һ���ǹ������̣�һ���ǹ�����̡��������̻ᶨ�ڵĸ���ؽ��̷���HUP�������ź�
*/

class CWX_API CwxAppProcessMgr
{
public:
    /**
    @brief ��ʼ�����̹�����
    @param [in] pApp �������̵�APP
    @return -1��ʧ�ܡ� 0���ɹ�
    */
    static int init(CwxAppFramework* pApp);
    /**
    @brief �����������̼���ؽ���
    @param [in] argc ��������ʱ�Ĳ�������
    @param [in] argv ��������ʱ�Ĳ���
    @param [in] unMaxHeartbeatInternal �������̷����������������s��
    @param [in] unDelaySec �����ĳ��ڣ���ʱ����������ʱ�䣨s��
    @return -1������ʧ�ܡ� 0�������ɹ���
    */
    static int start(int argc, char** argv, CWX_UINT16 unMaxHeartbeatInternal, CWX_UINT16 unDelaySec);
private:
    ///ע���źŴ���handle
    static int regSigHandle();
    ///�����������������в���
    static int checkRunCmd(int argc, char** argv);
    ///������������
    static int startProcess(int argc, char** argv, CWX_UINT16 unDelaySec);
    ///SIGQUIT �źž��
    static void stopHandler(int , siginfo_t *info, void *);
    ///SIGINT �źž��
    static void restartHandler(int , siginfo_t *info, void *);
    ///SIGCHLD �źž������Ϊ���������˳����ź�
    static void childExitHandler(int , siginfo_t *info, void *);
    ///SIGHUP�������źž��
    static void childHeatHandler(int , siginfo_t *info, void *);
    ///����ָ�����ź�
    static void blockSignal(int signal);
    ///ȡ������ָ�����ź�
    static void unblockSignal(int signal);
    ///��ס�����ļ������ؽ����ļ���FILE
    static FILE* lock();
    ///���������ļ���ͬʱ�ر��ļ�
    static void unlock(FILE* fd);
    ///�жϽ����Ƿ����,-1��ʧ�ܣ�0�������ڣ�1������
    static int isExistProc(char const* szProcName, int pid);
    ///��ȡpid�ļ��еĽ���id��-1��ʧ��
    static int getProcId(char const* szPidFile);

private:
    ///��ֹ��������ʵ��
    CwxAppProcessMgr()
    {

    }
private:
    static CwxAppProcessInfo*  m_pProcess;///<��������ʵ��
    static string      m_strPorcfile;///<�����ļ���
    static string      m_strAppName;///<������
    static string      m_strAppLockFile;///<�������ļ���
    static bool        m_bExit;///<�Ƿ�ֹͣ����
};


CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
