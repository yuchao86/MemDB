#ifndef __CWX_ECHO_CHANNEL_CONFIG_H__
#define __CWX_ECHO_CHANNEL_CONFIG_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
#include "CwxGlobalMacro.h"
#include "CwxHostInfo.h"
#include "CwxCommon.h"
#include "CwxIniParse.h"

CWINUX_USING_NAMESPACE

///�����ļ����ض���
class CwxEchoChannelConfig
{
public:
    CwxEchoChannelConfig(){
        m_unThreadNum = 0;
    }
    
    ~CwxEchoChannelConfig(){}
public:
    //���������ļ�.-1:failure, 0:success
    int loadConfig(string const & strConfFile);
    //������ص������ļ���Ϣ
    void outputConfig(string & strConfig);
    //��ȡ�����ļ����ص�ʧ��ԭ��
    char const* getError() { return m_szError; };
public:
    string              m_strWorkDir;///<����Ŀ¼
    string              m_strUnixPathFile;///<unix domain�ļ��� path file
    CWX_UINT16           m_unThreadNum;///<echo�����echo�߳�����
    CwxHostInfo       m_listen;///<tcp�ļ���ip/port
    char                m_szError[2048];///<������Ϣ��buf
};

#endif
