#ifndef __CWX_LOGGER_H__
#define __CWX_LOGGER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxLogger.h
@brief �ܹ���������־����
@author cwinux@gmail.com
@version 0.1
@date 2009-07-20
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include <stdio.h>
#include "CwxTss.h"
#include "CwxLockGuard.h"
#include "CwxMutexLock.h"
#include "CwxFile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "CwxCommon.h"
CWINUX_BEGIN_NAMESPACE
/**
@class CwxLogger
@brief �ܹ���־����Ķ��壬��������µ��ص㣺
1������־��Ϊinfo��debug��warning��error�ĸ����𡣿����������ȫ����ĳ�����������־��
2���������������־�Ĵ�С���������˴�С�󣬻��л���һ����־�ļ�
3������������־�ļ���������ʵ����־�ļ���ѭ��ʹ��
*/
class CWX_API CwxLogger
{
public:
    ///��־���Ͷ���
    enum{
        LEVEL_NONE = 0x00, ///<�������κ���־
        LEVEL_INFO = 0x01,///<info��־
        LEVEL_DEBUG = 0x02,///<debug��־
        LEVEL_WARNING = 0x04,///<warning��־
        LEVEL_ERROR = 0x08,///<error ��־
        LEVEL_ALL=0xFFFFFFFF ///<���ȫ����־
    };
    ///��־�ļ���������
    enum{
       MAX_LOG_FILE_NUM = 99,///<������־�ļ�����
       MIN_LOG_FILE_NUM = 4///<���ٵ���־�ļ�����
    };
    ///��������
    ~CwxLogger()
    {
        closeLog();
    }
public:
    ///��ʵ����ȡ
    static CwxLogger* instance();
    ///�ر���־
    static void close();
    /**
    @brief ��־�����ʼ��
    @param [in] base_name ��־�����ļ����֣�����·����
    @param [in] num ��־�ļ�������
    @param [in] max_size ÿ����־�ļ��Ĵ�С����λΪ�ֽ�
    @param [in] bAppend �Ƿ���յ�ǰʹ�õ���־�ļ�
    @return -1��ʧ�ܣ� 0���ɹ�
    */
    int  init(const char* base_name,
        CWX_UINT16 num,
        CWX_UINT32 max_size,
        bool bAppend);
    /**
    @brief �л���һ����־�ļ�
    @param [in] bAppend true���������һ����־�ļ�������append��ʽ��¼��־�������¼��־ǰ���
    @param [in] unFileNo ��һ����־���ļ��ţ���0��ʼ����Ϊ0����Ϊ��ǰ��־ϵͳ����һ����־�ļ�
    @return -1��ʧ�ܣ� 0���ɹ�
    */
    int  nextLog(bool bAppend, CWX_UINT16 unFileNo=0);
    /**
    @brief ��¼info���͵���־�������͵���־����¼�ļ������к�
    @param [in] format ��¼����־����
    @return void
    */
    void info(char const* format, ...);
    /**
    @brief ��¼debug���͵���־�������͵���־��¼�ļ������к�
    @param [in] format ��¼����־����
    @return void
    */
    void debug(char const* format, ...);
    /**
    @brief ��¼warning���͵���־�������͵���־��¼�ļ������к�
    @param [in] format ��¼����־����
    @return void
    */
    void warning(char const* format, ...);
    /**
    @brief ��¼error���͵���־�������͵���־��¼�ļ������к�
    @param [in] format ��¼����־����
    @return void
    */
    void error(char const* format, ...);
    ///��szMsg������log���ļ���
    void log(char const* szMsg);
    /**
    @brief ��uiLevel���͵���־���������Ϊ������͵Ļ�
    @param [in] uiLevel �򿪵���־����
    @return void
    */
    inline void enableLog(CWX_UINT32 uiLevel)
    {
        m_uiLevel |= uiLevel;
    }
    /**
    @brief �ر�uiLevel���͵���־���������Ϊ������͵Ļ�
    @param [in] uiLevel �رյ���־����
    @return void
    */
    inline void disableLog(CWX_UINT32 uiLevel)
    {
        m_uiLevel &=~uiLevel;
    }
    /**
    @brief ���uiLevel���͵���־����Ƿ�򿪣�����Ϊ������͵Ļ�
    @param [in] uiLevel ������־����
    @return true���򿪣�false��û�д�
    */
    inline bool isEnableLog(CWX_UINT32 uiLevel) const
    {
        return (m_uiLevel&(uiLevel))!=0;
    }
    ///��ȡ���ڵ���־�������
    inline CWX_UINT32 getLevel() const 
    {
        return m_uiLevel;
    }
    ///���õ�ǰ����־�������
    inline void setLevel(CWX_UINT32 uiLevel)
    {
        m_uiLevel = uiLevel;
    }
    ///��ȡ��ǰʹ�õ���־�ļ��Ĵ�С�� -1:��ʾ��ȡʧ��
    inline off_t getLogFileSize() const
    {
        return m_curLogFd?CwxFile::getFileSize(m_strCurLogFileName.c_str()):0;
    }
    ///��ȡ���õ���־�ļ�������
    inline CWX_UINT16 getLogFileNum() const
    {
        return this->m_unLogFileNum;
    }
    ///��ȡ��ǰ��־�ļ������
    inline CWX_UINT16 getCurLogFileNum() const 
    {
        return this->m_unCurLogFileNum;
    }
    ///��ȡ���õ���־�ļ��Ĵ�С
    inline CWX_UINT32 getMaxLogFileSize() const
    {
        return this->m_uiMaxFileSize;
    }
    ///��ȡ��ǰ��־�ļ���FILE handle
    inline FILE* getCurLogFileHandle() const
    {
        return this->m_curLogFd;
    }
private:
    ///���캯����ֻ��instance()���ܴ�����־���󣬱�֤�˵�ʵ����
    CwxLogger()
    {
        this->m_uiLevel = 0xFFFFFFFF;
        this->m_unLogFileNum = 0;
        this->m_unCurLogFileNum = 0;
        this->m_uiMaxFileSize = 0;
        this->m_curLogFd = NULL;
        this->m_prevLogFd = NULL;
        this->m_bInit = false;
    }
    ///�ر���־���󣬷���ֵ��0:success, -1:failure
    int closeLog();
    ///������һ����־�ļ��Ļ�����Ϣ
    void _nextLogFile();
    ///��ȡ���Ϊseq����־�ļ�������
    void _logFileName(int seq, std::string& strLog);
    ///����ʱ��������־�ļ�����־����ȡ��ʼʹ�õ���־�ļ���š�-1:��0��ʼ, otherwize, log's id
    int  _getStartLogNo();
    ///�л�����һ����־�ļ�������ֵ��0:success, -1:failure
    int _nextLog(bool append=true, CWX_UINT16 unFileNo=0);
    ///��ȡmsg header
    int _log_header(char const* szFile, int line, char* szBuf, CWX_UINT32 uiBufLen);

private:
    CWX_UINT32         m_uiLevel;///<��־�����LEVEL
    CWX_UINT32         m_uiMaxFileSize;///<���õ���־�ļ��ߴ�
    CWX_UINT16         m_unLogFileNum;///<���õ���־�ļ�����
    CWX_UINT16         m_unCurLogFileNum;///<��ǰʹ�õ���־�ļ���
    CwxMutexLock     m_mutex;///<������־�ļ��л�����
    string             m_strBaseName;///<��־�ļ������ļ���
    string             m_strCurLogFileName;///<������־�ļ����ļ���
    FILE *             m_curLogFd;///<��ǰ��־�ļ���fd
    FILE *             m_prevLogFd;///<��һ����־�ļ���fd
    bool               m_bInit;///<��־�����Ƿ��ʼ���ɹ�
    static CwxLogger* m_pInstance;///<��־����ĵ�ʵ������
};

CWINUX_END_NAMESPACE
///���һ��������Ϣ
#define CWX_ERROR(msg) do {\
    CWX_TSS_FILE_NO = __LINE__;\
    CWX_TSS_FILE_NAME = __FILE__;\
    CwxLogger::instance()->error msg;} while(0)
///���һ��������Ϣ
#define CWX_WARNING(msg) do {\
    CWX_TSS_FILE_NO = __LINE__;\
    CWX_TSS_FILE_NAME = __FILE__;\
    CwxLogger::instance()->warning msg;} while(0)
///���һ��������Ϣ
#define CWX_DEBUG(msg) do {\
    CWX_TSS_FILE_NO = __LINE__;\
    CWX_TSS_FILE_NAME = __FILE__;\
    CwxLogger::instance()->debug msg;} while(0)
///���һ��info��Ϣ
#define CWX_INFO(msg) do {\
    CWX_TSS_FILE_NO = 0;\
    CWX_TSS_FILE_NAME = NULL;\
    CwxLogger::instance()->info msg;} while(0)
///���һ��info��Ϣ
#define CWX_LOG(msg) do {\
    CwxLogger::instance()->log(msg);} while(0)
#include "CwxPost.h"
#endif
