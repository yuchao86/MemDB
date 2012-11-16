#ifndef __CWX_SINGLETON_H__
#define __CWX_SINGLETON_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxSingleton.h
@brief ��ʵ������ӿڵĶ��塣
@author cwinux@gmail.com
@version 0.1
@date 2009-10-02
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"

CWINUX_BEGIN_NAMESPACE
class CwxSingletonMgr;
/**
@class CwxSingleton
@brief ��ʵ������ӿڡ�
*/
class CWX_API CwxSingleton
{
public:
    inline string const& getName() const
    {
        return m_strName;
    }
protected:
    ///���캯��������CwxSingletonMgr����ע��, strNameΪ��������֣��Ա����
    CwxSingleton(string const& strName);
    ///���������������ʵ������CwxSingletonMgr�ͷ�
    virtual ~CwxSingleton();
    friend class CwxSingletonMgr;
private:
    CwxSingleton*      m_next;
    string             m_strName;

};

CWINUX_END_NAMESPACE


#include "CwxPost.h"

#endif
