#ifndef __CWX_HEAP_H__
#define __CWX_HEAP_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxHeap.h
@brief ͨ��STL��HEAP �ӿڣ�ʵ����Сheap��
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
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
@class CwxHeap
@brief ͨ��STL��HEAP �ӿڣ�ʵ����Сheapģ���ࣻ<br>
TYPEΪ����Ķ������ͣ�CMPΪTYPE�ıȽϺ�������<br>
����CMP(T1,T2)�ıȽϷ���ֵ��˵����������,ȱʡΪstl::less
T1<T2������true
T1>=T2������false
*/
template <class TYPE, class CMP=less<TYPE> >
class CWX_API CwxHeap
{
public:
    /**
    @brief ���캯��
    @param [in] num heap������Ԫ������
    @param [in] cmp �ȽϺ���
    */
    CwxHeap(CWX_UINT32 num, CMP const& cmp=less<TYPE>()):m_cmp(cmp)
    {
        m_uiNum = num;
        m_uiCurNum = 0;
        m_pElement = NULL;
        m_bHeap = true;
    }
    ///��������
    ~CwxHeap()
    {
        if (m_pElement) delete [] m_pElement;
    }
public:
    /**
    @brief ��ʼ��HEAP
    @return -1��ʧ�ܣ�0���ɹ�
    */
    int init()
    {
        if (!m_uiNum) return -1;
        if (NULL == m_pElement) m_pElement = new TYPE[m_uiNum];
        m_uiCurNum = 0;
        m_bHeap = true;
        return 0;
    }
    /**
    @brief ��HEAP�����һ��Ԫ�أ���HEAP�������������С��һ��Ԫ��
    @param [in] in ��ӵ�Ԫ��
    @param [out] out ��HEAP�������������С��Ԫ��
    @return true���������С��Ԫ�أ�false��û��Ԫ�ش�HEAP�����
    */
    bool push(TYPE const& in, TYPE& out)
    {
        CWX_ASSERT(m_bHeap);
        if (m_uiCurNum == m_uiNum)
        {
            if (!m_cmp(in, m_pElement[0]))
            {
                out = in;
                return true;
            }
            pop_heap(m_pElement, m_pElement + m_uiNum, m_cmp);
            out = m_pElement[m_uiNum - 1];
            m_pElement[m_uiNum - 1] = in;
            push_heap(m_pElement, m_pElement + m_uiNum, m_cmp);
            return true;
        }
        m_pElement[m_uiCurNum] = in;
        m_uiCurNum++;
        push_heap(m_pElement, m_pElement + m_uiCurNum, m_cmp);
        return false;
    }
    /**
    @brief �����Ĵ�HEAP��POPһ����С��Ԫ��
    @param [out] out pop������С��Ԫ��
    @return true��pop����һ����С��Ԫ�أ�false��heap�ѿ�
    */
    bool pop(TYPE& out)
    {
        CWX_ASSERT(m_bHeap);
        if (0 == m_uiCurNum) return false;
        pop_heap(m_pElement, m_pElement + m_uiCurNum, m_cmp);
        m_uiCurNum --;
        out = m_pElement[m_uiCurNum];
        return true;
    }
    ///��HEAP����CMP�����������򣬰������������
    void sort()
    {
        sort_heap(m_pElement, m_pElement+m_uiCurNum, m_cmp);
        m_bHeap = false;
    }
    ///��ȡHEAP�еĵ�index��Ԫ�أ�������NULL����ʾԪ�ز�����
    TYPE const* operator[](CWX_UINT32 index) const
    {
        if (index<m_uiCurNum) return m_pElement + index;
        return NULL;
    }
    ///��ȡHEAP��Ԫ�ص�����
    CWX_UINT32 count() const
    { 
        return m_uiCurNum;
    }
    ///true��HEAP�е�Ԫ����HEAP�Ĵ���false��HEAP�е�Ԫ�ز���HEAP�Ĵ���
    bool isHeap() const 
    { 
        return m_bHeap;
    }
private:
    CWX_UINT32  m_uiNum;///<heap������
    CWX_UINT32  m_uiCurNum;///<heap��Ԫ�ص�����
    TYPE*   m_pElement;///<heap�����飬��СΪm_uiNum��
    bool    m_bHeap;///<m_pElement�е����ݣ��Ƿ���HEAP�Ĵ���
    CMP     m_cmp;///<�ȽϺ�������
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"


#endif
