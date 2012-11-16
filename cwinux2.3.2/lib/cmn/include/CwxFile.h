#ifndef __CWX_FILE_H__
#define __CWX_FILE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxFile.h
@brief �����ļ�ϵͳ������ص��ࣺCwxFile
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

CWINUX_BEGIN_NAMESPACE

/**
@class CwxFile
@brief ʵ�����ļ�ϵͳ�ĳ��ò���������ķ���ȫ��Ϊ��̬������������ʵ����
*/
class CWX_API CwxFile
{
public:
    /**
    @brief ���pathָ�������һ���ļ�Ŀ¼�����ڣ��򴴽���
    @param [in] path �ļ�Ŀ¼
    @param [in] mode �ļ���Ȩ��ģʽ
    @return true��Ŀ¼�Ѿ����ڻ򴴽��ɹ���false��Ŀ¼����ʧ�ܻ�ָ����·���Դ��ڵ�����Ŀ¼��
    */
    static bool createMissDir(char const* path, int mode=S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
    /**
    @brief ����pathָ����Ŀ¼��Ŀ¼���벻���ڡ�
    @param [in] path �ļ�Ŀ¼
    @param [in] mode �ļ���Ȩ��ģʽ
    @return true�������ɹ���false������ʧ�ܡ�
    */
    static bool createDir(char const* path, int mode=S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
    /**
    @brief �ж�path��Ӧ��·���Ƿ�Ϊһ���ļ���
    @param [in] path Ҫ�жϵ�·����
    @return true�����ļ���false�������ļ���·�������ڡ�
    */
    static bool isFile(char const* path);
    /**
    @brief �ж�path��Ӧ��·���Ƿ�Ϊһ��Ŀ¼��
    @param [in] path Ҫ�жϵ�·����
    @return true����Ŀ¼��false������Ŀ¼��·�������ڡ�
    */
    static bool isDir(char const* path);
    /**
    @brief ��ȡstrFile��Ӧ���ļ��Ĵ�С��
    @param [in] strFile �ļ���
    @return -1����ȡʧ�ܣ�����Ϊ�ļ��Ĵ�С��
    */
    static off_t getFileSize(char const* strFile);
    /**
    @brief ��ȡ�ļ�������޸�ʱ�䡣
    @param [in] strFile �ļ���
    @return 0����ȡʧ�ܣ�����Ϊ�ļ�������޸�ʱ�䡣
    */
    static time_t getFileMTime(char const* strFile);
    /**
    @brief ��srcָ�����ļ����Ƶ�dest��·����dest�а����ļ�����
    @param [in] src Ҫ�����Ƶ��ļ�
    @param [in] dest �µ��ļ���
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool moveFile(char const*  src, char const*  dest);
    /**
    @brief ɾ��strFile��Ӧ���ļ���
    @param [in] strFile Ҫɾ�����ļ���
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool rmFile(char const*  strFile);
    /**
    @brief ɾ��strPath��Ӧ��Ŀ¼��
    @param [in] strPath Ҫɾ����Ŀ¼
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool rmDir(char const*  strPath);
    /**
    @brief �л���ǰ���̵�Ŀ¼��
    @param [in] strPath Ҫ�л�����Ŀ¼��
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool changeDir(char const* strPath);
    /**
    @brief ��ȡstrPathĿ¼�µ�����ֱ���ļ�����������Ŀ¼��
    @param [in] strPath Ҫ��ȡ�ļ���Ŀ¼��
    @param [out] files �ļ����б��б��е��ļ�������·����
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool getDirFile(const string& strPath, list<string>& files);
    /**
    @brief ��ȡ/xxx/yyyy/zzzz��ʽ���һ�������֣���zzzz
    @param [in] strPath Ҫ��ȡ���һ�����ֵ�Ŀ¼��
    @param [out] strLast ���һ��������
    @return �������һ�������֣���strLast��
    */
    static string& getLastDirName(const string& strPath, string& strLast);
    /**
    @brief ��fd��Ӧ���ı��ļ��еĵ�ǰ�ļ�ָ��λ�ã���һ�����ݡ�
    @param [in] fd �ı��ļ����ļ����
    @param [out] strLine ��ȡ���У������سɹ���strLine.empty()==true�����ʾ�����ļ�β
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool readTxtLine(FILE* fd, string& strLine);
    /**
    @brief ��ȡ�����ı��ļ������ݡ�
    @param [in] strFileName Ҫ��ȡ���ı��ļ�������
    @param [out] strData �ı��ļ�������
    @return true���ɹ���false��ʧ�ܡ�
    */
    static bool readTxtFile(string const& strFileName, string& strData);
    /**
    @brief ��һ���ļ���ȫ�ֵ�д����
    @param [in] fd ����дȨ�޵��ļ�handle
    @return true���ɹ���false��ʧ�ܣ�errno��������ԭ��
    */
    static bool lock(int fd);
    /**
    @brief ����һ���ļ��ϼӵ�ȫ��д����
    @param [in] fd �������ļ�handle
    @return true���ɹ���false��ʧ�ܣ�errno��������ԭ��
    */
    static bool unlock(int fd);

private:
    ///˽�л��Ĺ��캯������ֹʵ����
    CwxFile(){}
};
CWINUX_END_NAMESPACE

#include "CwxFile.inl"
#include "CwxPost.h"

#endif
