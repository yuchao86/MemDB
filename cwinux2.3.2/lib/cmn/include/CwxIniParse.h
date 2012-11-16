#ifndef __CWX_INI_PARSE_H__
#define __CWX_INI_PARSE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxIniParse.h
@brief ʵ������win.ini��ʽ�ļ�������Ϣ�ļ��ء�
@author cwinux@gmail.com
@version 0.1
@date 2011-09-13
@warning
@bug
*/
/***
1���ļ���[xxxx]��ʾ����Section�飬���е�key=value�����ã�������λ��ĳ��[xxxx]������Section���¡�
2������ͨ��key=value�ĸ�ʽ���塣�����λ��ĳ�����ÿ��¡�
3����#��ͷ���У�����ȫ�涼���ٿո񣩣���Ϊ��ע���У��ᱻ���ԡ�
4�����б����ԡ�
5��key=value�У�key��ǰ�á����ÿո�ᱻ���ԡ�key���ܰ���[=]��
   value��ǰ�á����ÿո�ᱻ���ԣ�\r��\nҲ�ᱻ���ԣ�
***/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxFile.h"
#include "CwxCommon.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxIniParse
@brief win.ini��ʽ�������ļ���Ϣ�Ľ�������ء�
*/
class CWX_API CwxIniParse
{
public:
    ///���캯��������ΪXML��encode�滻�������ⲿ���й���
    CwxIniParse(){
        memset(m_szErrMsg, 0x00, 512);
    }
    ///��������
    ~CwxIniParse(){
		map<string, list<pair<string, string> >*>::iterator iter = m_attrs.begin();
		while(iter != m_attrs.end()){
			delete iter->second;
			iter++;
		}
	}
public:
    /**
    *@brief  ���������ļ�.
    *@param [in] strFile �����ļ�����
    *@return false��ʧ�ܣ�true���ɹ�.
    */ 
    bool load(string const& strFile);
    /**
    *@brief  ��������.
    *@param [in] strConf ������Ϣ��
    *@return false��ʧ�ܣ�true���ɹ�.
    */
    bool parse(string const& strConf);
    /**
    *@brief  ��ȡ���������е�Section��
    *@param [out] sections �����е�section��
    *@return void.
    */ 
    void getSections(set<string>& sections) const;
    /**
    *@brief  ��ȡһ��section�µ���������
    *@param [in] strSection section�����֡�
    *@param [out] attr�µ��������Լ���value��
    *@return false��section�����ڣ�true��section����.
    */ 
    bool getAttr(string const& strSection,
		list<pair<string, string> >& attr) const;
    /**
    *@brief  ��ȡһ���ַ���������ֵ.
	*@param [in] strSection section�����֡�
	*@param [in] strAttr ��������
    *@param [out] strValue ����ֵ��
    *@return false�������ڣ�true������.
    */ 
    bool getAttr(string const& strSection,
        string const& strAttr,
		string& strValue) const;
	/**
	*@brief  �Ƿ����ָ����Section��
	*@param [int] strSection section�����֡�
	*@return true�����ڣ�false��������.
	*/ 
	bool isExistSection(string const& strSection){
		return m_attrs.find(strSection) != m_attrs.end();
	}

    ///���ش�����Ϣ
    char const* getErrMsg() const{ return m_szErrMsg;}
private:
	string							   m_strFile; ///<�����ļ�
	map<string, list<pair<string, string> >*>  m_attrs; ///<��������
    char m_szErrMsg[512];///<����buf
};

CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
