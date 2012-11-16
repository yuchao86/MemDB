#ifndef __CWX_ECHO_CLIENT_CONFIG_H__
#define __CWX_ECHO_CLIENT_CONFIG_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/
#include "CwxHostInfo.h"
#include "CwxCommon.h"
#include "CwxIniParse.h"

CWINUX_USING_NAMESPACE

///echoѹ�����Ե������ļ����ض���
class CwxEchoClientConfig
{
public:
    CwxEchoClientConfig(){
        m_bTcp = true;
        m_unConnNum = 0;
        m_uiDataSize = 0;
        m_bLasting = true;
    }
    
    ~CwxEchoClientConfig(){}
public:
    //���������ļ�.-1:failure, 0:success
    int loadConfig(string const & strConfFile);
    //��������ļ�
    void outputConfig(string & strConfig);
    //��ȡ���������ļ���ʧ�ܴ�����Ϣ
    char const* getError() { return m_szError; };
    
public:
    bool                m_bTcp; ///<�Ƿ�ͨ��tcp��������
    string              m_strUnixPathFile;///<������unix domain���ӣ���Ϊ���ӵ�path-file
    string              m_strWorkDir;///<����Ŀ¼
    CWX_UINT16           m_unConnNum;///<���ӵ�����
    CWX_UINT32           m_uiDataSize;///<echo�����ݰ��Ĵ�С
    bool                m_bLasting;///<�Ƿ�Ϊ�־����ӣ�����HTTP��keep-alive
    CwxHostInfo          m_listen;///<tcp���ӵĶԷ�listen��ַ
    char                m_szError[2048];///<������Ϣbuf
};

#endif
