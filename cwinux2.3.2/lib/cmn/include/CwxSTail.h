#ifndef __CWX_STAIL_H__
#define __CWX_STAIL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxSTail.h
*@brief ���������ģ����
*@author cwinux@gmail.com
*@version 1.0
*@date  2010-09-19
*@warning  ��.
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxSTail
*
* @brief ���������ģ���࣬���������m_next�Ķ���ָ�룬ָ����һ������Ԫ�ء�
*/
template <typename CELL>
class CwxSTail
{
public:
    ///���캯������lock��Ϊ�գ������
    CwxSTail():m_count(0),m_head(NULL),m_tail(NULL)
    {
    }
    ///������������������Ϊ�գ������
    ~CwxSTail()
    {
    }
public:
    ///���ؼ�¼������
    inline CWX_UINT32 count() const
    {
        return m_count;
    }
    ///��ȡ�����ͷ��������������ɾ����NULL��ʾû��
    inline CELL const* head() const
    {
        return m_head;
    }
    ///pushһ��Ԫ�ص������ͷ
    inline void push_head(CELL* cell)
    {
        m_count++;
        cell->m_next = m_head;
        m_head = cell;
        if (!m_tail) m_tail = m_head;
    }
    ///�������ͷ��popһ��Ԫ�ء�
    inline CELL* pop_head()
    {
        CELL* obj = NULL;
        if (m_head)
        {
            m_count--;
            obj = m_head;
            m_head = m_head->m_next;
            obj->m_next = NULL;
            if (!m_head)
            {
                m_tail = NULL;
                m_count = 0;
            }
        }
        return obj;
    }
    ///��ȡ�����β��������������ɾ����NULL��ʾû��
    CELL const* tail() const
    {
        return m_tail;
    }
    ///pushһ��Ԫ�ص������β
    void push_tail(CELL* cell)
    {
        count++;
        cell->m_next = NULL;
        if (m_tail)
        {
            m_tail->m_next = cell;
        }
        else
        {
            m_head = cell;
        }
        m_tail = cell;
    }
private:
    CWX_UINT32   m_count; ///<��¼������
    CELL*       m_head; ///<�����ͷ
    CELL*       m_tail; ///<�����β
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"
#endif

