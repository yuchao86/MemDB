#ifndef __CWX_DTAIL_H__
#define __CWX_DTAIL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxDTail.h
*@brief ˫�������ģ����
*@author cwinux@gmail.com
*@version 1.0
*@date  2010-09-19
*@warning  ��.
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxDTail
*
* @brief ˫�������ģ���࣬���������m_next, m_prev�Ķ���ָ�룬�ֱ�ָ����һ����ǰһ������Ԫ�ء�
*/
template <typename CELL>
class CwxDTail
{
public:
    ///���캯������lock��Ϊ�գ������
    CwxDTail():m_count(0),m_head(NULL),m_tail(NULL)
    {
    }
    ///������������������Ϊ�գ������
    ~CwxDTail()
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
        cell->m_prev = NULL;
        cell->m_next = m_head;
        if (m_head)
        {
            m_head->m_prev = cell;
        }
        else
        {
            m_tail = cell;            
        }
        m_head = cell;
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
            if (m_head)
            {
                m_head->m_prev = NULL;
            }
            else
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
        cell->m_prev = m_tail;
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
    ///�������β��popһ��Ԫ��
    CELL* pop_tail()
    {
        CELL* obj = NULL;
        if (m_tail)
        {
            m_count--;
            obj = m_tail;
            m_tail = m_tail->m_prev;
            obj->m_prev = NULL;
            if (m_tail)
            {
                m_tail->m_next =NULL;
            }
            else
            {
                m_head = NULL;
                m_count = 0;
            }
        }
        return obj;
    }
    ///ɾ��һ��Ԫ��
    inline void remove(CELL* cell)
    {
        if (cell == m_head)
        {
            pop_head();
        }
        else if (cell == m_tail)
        {
            pop_tail();
        }
        else
        {
            if (cell->m_prev && cell->m_next)
            {
                cell->m_prev->m_next = cell->m_next;
                cell->m_next->m_prev = cell->m_prev;
                cell->m_prev=cell->m_next=NULL;
                m_count--;
            }
        }
    }
    ///��һ��Ԫ���Ƶ�ͷ��
    inline void to_head(CELL* cell)
    {
        if (cell != m_head)
        {
            if (cell->m_next)
            {
                cell->m_prev->m_next = cell->m_next;
                cell->m_next->m_prev = cell->m_prev;
            }
            else
            {
                m_tail=m_tail->m_prev;
                m_tail->m_next = NULL;
            }
            cell->m_prev = NULL;
            cell->m_next = m_head;
            m_head = cell;
        }
    }
    ///��һ��Ԫ���Ƶ�β��
    inline void to_tail(CELL* cell)
    {
        if (cell != m_tail)
        {
            if (cell->m_prev)
            {
                cell->m_prev->m_next = cell->m_next;
                cell->m_next->m_prev = cell->m_prev;
            }
            else
            {
                m_head = m_head->m_next;
                m_head->m_prev = NULL;
            }
            cell->m_next = NULL;
            cell->m_prev = m_tail;
            m_tail = cell;
        }

    }
private:
    CWX_UINT32   m_count; ///<��¼������
    CELL*       m_head; ///<�����ͷ
    CELL*       m_tail; ///<�����β
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"
#endif

