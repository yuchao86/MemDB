#include "CwxIniParse.h"

CWINUX_BEGIN_NAMESPACE
/**
*@brief  ���������ļ�.
*@param [in] strFile �����ļ�����
*@return false��ʧ�ܣ�true���ɹ�.
*/ 
bool CwxIniParse::load(string const& strFile){
	string strConf;
    if (!CwxFile::readTxtFile(strFile, strConf)){
		CwxCommon::snprintf(m_szErrMsg, 511, "Failure to read conf file:%s, errno=%d", strFile.c_str(), errno);
		return false;
    }
    return parse(strConf);
}

bool CwxIniParse::parse(string const& strConf)
{
    //�������
    {
        map<string, list<pair<string, string> >*>::iterator iter = m_attrs.begin();
        while(iter != m_attrs.end()){
            delete iter->second;
            iter++;
        }
        m_attrs.clear();
    }
    string line;
    list<string>lines;
    list<string>::iterator iter;
    string strSection;
    string strKey;
    string strValue;
    list<pair<string, string> >* pAttr=NULL;
    CwxCommon::split(strConf, lines, '\n');
    iter = lines.begin();
    while(iter != lines.end()){
        line = *iter;
        CwxCommon::trim(line, "\r\n");
        if (!line.length()){
            iter++;
            continue;
        }
        if ('#' == line[0]){
            iter++;
            continue;
        }
        if (('['==line[0])&&(']'==line[line.length()-1])){
            strSection = line.substr(1, line.length()-2);
            CwxCommon::trim(strSection);
            if (m_attrs.find(strSection) != m_attrs.end()){
                CwxCommon::snprintf(m_szErrMsg, 511, "Section[%s] is duplicate.", strSection.c_str());
                return false;
            }
            pAttr = new list<pair<string, string> >;
            m_attrs[strSection] = pAttr;
            iter++;
            continue;
        }
        if (!pAttr){
            CwxCommon::snprintf(m_szErrMsg, 511, "Property[%s] is in secion.", line.c_str());
            return false;
        }
        if (line.find('=')==string::npos){
            CwxCommon::snprintf(m_szErrMsg, 511, "Property[%s] isn't key=value format.", line.c_str());
            return false;
        }
        if ('=' == line[0]){
            CwxCommon::snprintf(m_szErrMsg, 511, "Property[%s]'s key is empty.", line.c_str());
            return false;
        }
        strKey = line.substr(0, line.find('='));
        strValue = line.substr(line.find('=')+1);
        CwxCommon::trim(strKey);
        CwxCommon::trim(strValue);
        if (!strKey.length()){
            CwxCommon::snprintf(m_szErrMsg, 511, "Property[%s]'s key is empty.", line.c_str());
            return false;
        }
        pAttr->push_back(pair<string, string>(strKey, strValue));
        iter++;
    }
    return true;
}

/**
*@brief  ��ȡ���������е�Section��
*@param [out] sections �����е�section��
*@return void.
*/ 
void CwxIniParse::getSections(set<string>& sections) const
{
	sections.clear();
	map<string, list<pair<string, string> >*>::const_iterator iter = m_attrs.begin();
	while(iter != m_attrs.end()){
		sections.insert(iter->first);
		iter++;
	}
}

/**
*@brief  ��ȡһ��section�µ���������
*@param [in] strSection section�����֡�
*@param [out] attr�µ��������Լ���value��
*@return false��section�����ڣ�true��section����.
*/ 
bool CwxIniParse::getAttr(string const& strSection,
				 list<pair<string, string> >& attr) const
{
	map<string, list<pair<string, string> >*>::const_iterator iter = m_attrs.find(strSection);
	if (iter == m_attrs.end()) return false;
	attr.clear();
	attr = *iter->second;
	return true;
}
/**
*@brief  ��ȡһ���ַ���������ֵ.
*@param [in] strSection section�����֡�
*@param [in] strAttr ��������
*@param [out] strValue ����ֵ��
*@return false�������ڣ�true������.
*/ 
bool CwxIniParse::getAttr(string const& strSection,
				 string const& strAttr,
				 string& strValue) const
{
	map<string, list<pair<string, string> >*>::const_iterator iter = m_attrs.find(strSection);
	if (iter == m_attrs.end()) return false;
	list<pair<string, string> >::const_iterator attr_iter = iter->second->begin();
	while(attr_iter != iter->second->end())
	{
		if (attr_iter->first == strAttr)
		{
			strValue = attr_iter->second;
			return true;
		}
		attr_iter++;
	}
	return false;
}


CWINUX_END_NAMESPACE
