#ifndef __CWX_KEY_VALUE_MAP_H__
#define __CWX_KEY_VALUE_MAP_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxKeyValueMap.h
@brief ��װSTL��multimapʵ��Key��Value����string���͵�map������Key��ѡ���Ƿ��Сд����
@author cwinux@gmail.com
@version 0.1
@date 2009-10-17
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxKeyValueMap
@brief ��װSTL��MAPʵ��Key��Value����string���͵�map������Key��ѡ���Ƿ��Сд����
*/
class CWX_API CwxKeyValueMap
{
public:
    typedef multimap<std::string, std::string, CwxCaseStringLess2> MAP;
    typedef MAP::iterator iterator;
    typedef MAP::const_iterator const_iterator;
    /**
    @brief ���캯��
    @param [in] bCaseSensive key�Ƿ��Сд���С�true���ǣ������ǡ�
    */
    CwxKeyValueMap(bool bCaseSensive)
        :m_bCaseSensive(bCaseSensive)
    {
        m_index = new MAP(CwxCaseStringLess2(bCaseSensive));
    }
    ///��������
    ~CwxKeyValueMap()
    {
        if (m_index) delete m_index;
    }
    ///��������
    CwxKeyValueMap(CwxKeyValueMap const& obj);
    ///��ֵ����
    CwxKeyValueMap& operator=(CwxKeyValueMap const& obj);
public:
    ///������map��key/value���н���
    void swap(CwxKeyValueMap& obj);
    MAP& map();
    bool isCaseSensive() const;
private:
    bool           m_bCaseSensive;
    MAP*           m_index;
};

CWINUX_END_NAMESPACE

#include "CwxKeyValueMap.inl"
#include "CwxPost.h"


#endif
