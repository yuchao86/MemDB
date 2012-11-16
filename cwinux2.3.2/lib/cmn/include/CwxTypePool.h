#ifndef __CWX_TYPE_POOL_H__
#define __CWX_TYPE_POOL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxTypePool.h
@brief ����ֻ���䣬���ͷŵ��ڴ��ģ�壺CwxTypePool��
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
@class CwxTypePool
@brief ֻ���䣬���ͳһ�ͷŵ��ڴ��ģ���ࡣ
*/
template <typename T > 
class CwxTypePool
{
public:
    enum{
        MIN_POOL_OBJECT_NUM = 64///<ÿ���ڴ������С�Ķ��������
    };
private:
    vector<char*>  m_vector;///<�ڴ���vector
    CWX_UINT32     m_uiSize;///<����Ķ��������
    CWX_UINT32     m_uiCapacity;///<������ڴ����ܶ�������
    CWX_UINT32     m_uiPoolSize;///<ÿ���ڴ��Ĵ�С
    CWX_UINT32     m_uiLeft;///<��ǰ�ڴ��ʣ��Ŀ��ж�������
    char*          m_curPool;///<��ǰ���ڴ��
public:
    ///���캯��������ÿ���ڴ��Ķ�������
    CwxTypePool(CWX_UINT32 uiPoolSize=MIN_POOL_OBJECT_NUM)
    {
        m_uiSize = 0;
        m_uiCapacity = 0;
        m_uiLeft = 0;
        m_curPool = NULL;
        m_uiPoolSize = uiPoolSize>(CWX_UINT32)MIN_POOL_OBJECT_NUM?uiPoolSize:(CWX_UINT32)MIN_POOL_OBJECT_NUM;
    }
    ///��������
    ~CwxTypePool()
    {
        clear();
    }
public:
    ///���ڴ���з���һ������
    T* malloc()
    {
        if (!m_uiLeft)
        {//new pool
            m_curPool = (char*)::malloc(sizeof(T) * m_uiPoolSize);
            if (NULL == m_curPool) return NULL;
            m_vector.push_back(m_curPool);
            m_uiCapacity += m_uiPoolSize;
            m_uiLeft = m_uiPoolSize;
		}
        T* obj = new(m_curPool) T();
        m_uiLeft --;
        m_curPool += sizeof(T);
        m_uiSize ++;
        return obj;
    }
    ///���ڴ���з���һ�����󣬲��������ʼֵ����ʱ�Ķ�����붨�忽������
    T* malloc(T const& item)
    {
        if (!m_uiLeft)
        {//new pool
            m_curPool = (char*)::malloc(sizeof(T) * m_uiPoolSize);
            if (NULL == m_curPool) return NULL;
            m_vector.push_back(m_curPool);
            m_uiCapacity += m_uiPoolSize;
            m_uiLeft = m_uiPoolSize;
        }
        T* obj = new(m_curPool) T(item);
        m_uiLeft --;
        m_curPool += sizeof(T);
        m_uiSize ++;
        return obj;
    }
    ///��ȡ��ǰ������ܵĶ��������
    CWX_UINT32 size() const
    {
        return m_uiSize;
    }
    ///��ȡ��ǰ������ڴ������ɵ��ܵĶ��������
    CWX_UINT32 capacity() const
    {
        return m_uiCapacity;
    }
    ///��ȡ��uiIndex���󣬷���ֵ��NULL�������ڣ�����Ϊ��uiIndex�������ַ
    T* at(CWX_UINT32 uiIndex)
    {
        if (uiIndex >= m_uiSize) return NULL;
        CWX_UINT32 uiPoolIndex = uiIndex/m_uiPoolSize;
        CWX_UINT32 uiPos = uiIndex%m_uiPoolSize;
        return ((T*)m_vector[uiPoolIndex]) + uiPos;
    }
    ///�ͷ��ڴ���еĶ��󼰷�����ڴ��
    void clear()
    {
        T* pObj = NULL;
        CWX_UINT32 uiPoolIndex = m_uiSize/m_uiPoolSize;
        CWX_UINT32 uiPos = m_uiSize%m_uiPoolSize;
        for (CWX_UINT32 i=0; i<uiPoolIndex; i++)
        {
            pObj = (T*)m_vector[i];
            for (CWX_UINT32 j=0; j<m_uiPoolSize; j++)
            {
                pObj->~T();
                pObj++;
            }
            free(m_vector[i]);
        }
        if (uiPos)
        {
            m_curPool = m_vector[uiPoolIndex];
        }
        else
        {
            m_curPool = NULL;
        }
        m_vector.clear();

        pObj = (T*)m_curPool;
        if (m_curPool)
        {
            for (CWX_UINT32 j=0; j<uiPos; j++)
            {
                pObj->~T();
                pObj++;
            }
            if (m_curPool) free(m_curPool);
            m_curPool = NULL;
        }
        m_uiCapacity = 0;
        m_uiLeft = 0;
        m_uiSize = 0;
    }
};


CWINUX_END_NAMESPACE

#endif
