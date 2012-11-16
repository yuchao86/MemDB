#ifndef __CWX_TYPE_POOL_EX_H__
#define __CWX_TYPE_POOL_EX_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxTypePoolEx.h
@brief �������;�ͷŵ��ڴ�ض���ģ�壺CwxTypePoolEx��
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxStl.h"
#include "CwxCommon.h"
#include "CwxGlobalMacro.h"
#include <new>

CWINUX_BEGIN_NAMESPACE

/**
@class CwxTypePoolEx
@brief �ɷ��䡢���ͷŵ��ڴ��ģ���ࡣ
*/
template <typename T > 
class CwxTypePoolEx
{
private:
    ///�����������ͣ�Ϊ����T�Ķ�������һ��CwxTypePoolExItem*��ָ��m_next
    class CwxTypePoolExItem
    {
    private:
        ///���������Ķ���
        CwxTypePoolExItem()
            :m_next(NULL)
        {

        }
        ~CwxTypePoolExItem()
        {

        }
    public:
        T      m_data;
        CwxTypePoolExItem*     m_next;
    };
    enum{
        MIN_POOL_OBJECT_NUM = 64///<ÿ���ڴ������С�Ķ��������
    };
private:
    vector<char*> m_vector;///<�ڴ���vector
    CWX_UINT32    m_uiSize;///<����Ķ��������
    CWX_UINT32    m_uiCapacity;///<������ڴ����ܶ�������
    CWX_UINT32     m_uiPoolSize;///<ÿ���ڴ��Ĵ�С
    CwxTypePoolExItem*  m_freeList;///<���ж����б�
public:
    ///���캯��������ÿ���ڴ��Ķ�������
    CwxTypePoolEx(CWX_UINT32 uiPoolSize=MIN_POOL_OBJECT_NUM)
    {
        m_uiSize = 0;
        m_uiCapacity = 0;
        m_uiPoolSize = uiPoolSize;
        if (m_uiPoolSize < MIN_POOL_OBJECT_NUM) m_uiPoolSize = MIN_POOL_OBJECT_NUM;
        m_freeList = NULL;
    }
    ///��������
    ~CwxTypePoolEx()
    {
        clear();
    }
public:
    ///���ڴ���з���һ������
    T* malloc()
    {
        CwxTypePoolExItem* obj = NULL;
        if (!m_freeList) createFreeObj();
        if (!m_freeList) return NULL;
        obj = m_freeList;
        m_freeList = m_freeList->m_next;
        obj->m_next = (CwxTypePoolExItem*)-1;///used sign
        m_uiSize++;
        return new(obj) T();
    }
    ///���ڴ���з���һ�����󣬲��������ʼֵ����ʱ�Ķ�����붨�忽������
    T* malloc(T const& item)
    {
        CwxTypePoolExItem* obj = NULL;
        if (!m_freeList) createFreeObj();
        if (!m_freeList) return NULL;
        obj = m_freeList;
        m_freeList = m_freeList->m_next;
        obj->m_next = (CwxTypePoolExItem*)-1;///used sign
        m_uiSize++;
        return new(obj) T(item);
    }
    ///�ͷ�һ������
    void free(T* obj)
    {
        obj->~T();
        CwxTypePoolExItem* pItem = (CwxTypePoolExItem*)obj;
        pItem->m_next = m_freeList;
        m_freeList = pItem;
        m_uiSize--;
    }
    ///��ȡ����Ķ��������
    CWX_UINT32 size() const
    {
        return m_uiSize;
    }
    ///��ȡ���ж��������
    unsigned int freeSize() const
    {
        return m_uiCapacity - m_uiSize;
    }
    ///��ȡ�ڴ�ص�ǰ�Ķ�������
    unsigned int capacity() const
    {
        return m_uiCapacity;
    }
    ///�ͷ������ڴ�ص��ڴ�
    void clear()
    {
        CwxTypePoolExItem* pObj = NULL;
        CWX_UINT32 uiPoolIndex = m_vector.size();
        for (CWX_UINT32 i=0; i<uiPoolIndex; i++)
        {
            pObj = (CwxTypePoolExItem*)m_vector[i];
            for (CWX_UINT32 j=0; j<m_uiPoolSize; j++)
            {
                if (pObj->m_next == (CwxTypePoolExItem*)-1) ((T*)pObj)->~T();
                pObj++;
            }
            ::free(m_vector[i]);
        }
        m_vector.clear();
        m_uiCapacity = 0;
        m_uiSize = 0;
        m_freeList = NULL;
    }
private:
    ///�����µĶ����ڴ��
    void createFreeObj()
    {
        char* szTrunk = (char*)::malloc(sizeof(CwxTypePoolExItem) * m_uiPoolSize);
        if (szTrunk)
        {
           m_vector.push_back(szTrunk);
           CwxTypePoolExItem* pItem = (CwxTypePoolExItem*)szTrunk;
           CWX_UINT32 i=0;
           for (i=0; i<m_uiPoolSize-1; i++)
           {
               pItem[i].m_next = &pItem[i+1];
           }
           pItem[i].m_next = m_freeList;
           m_freeList = pItem;
           m_uiCapacity += m_uiPoolSize;
        }
    }
};

CWINUX_END_NAMESPACE

#endif
