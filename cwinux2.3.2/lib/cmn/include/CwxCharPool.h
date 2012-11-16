#ifndef __CWX_CHAR_POOL_H__
#define __CWX_CHAR_POOL_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxCharPool.h
@brief ֻ���䣬���ͷŵ�char pool����Ŀ���Ƿ�ֹ�ڴ����Ƭ��
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

CWINUX_BEGIN_NAMESPACE
/**
@class CwxCharPool
@brief ֻ���䣬���ͷŵ�char pool����Ŀ���Ƿ�ֹ�ڴ����Ƭ��
*/
class CWX_API CwxCharPool
{
public:
    enum{
        DEF_MEM_TRUNK_SIZE = 4096,///<ȱʡ�ڴ��Ĵ�С
        DEF_KEEP_MEM_SIZE = 16 * DEF_MEM_TRUNK_SIZE,///<��reset��ʱ�򣬳��е��ڴ��С
        MEM_ALIGN_SIZE = 1024,///<�ڴ��Ķ����С
        POINTER_SIZE = sizeof(void*),///<ָ����ֽ���
        TRUNK_HEAD_SIZE = POINTER_SIZE + sizeof(CWX_UINT32)///<�ڴ��ͷ�Ĵ�С
    };
public:
    /**
    @brief ���캯����
    @param [in] uiMemTrunkPoolSize �ڴ��Ĵ�С
    @param [in] uiKeepMemSize ��reset��ʱ�򣬳��е��ڴ��С
    */
    CwxCharPool(CWX_UINT32 uiMemTrunkPoolSize=DEF_MEM_TRUNK_SIZE, CWX_UINT32 uiKeepMemSize = DEF_KEEP_MEM_SIZE)
    {
        m_uiMemTrunkSize = ((uiMemTrunkPoolSize+MEM_ALIGN_SIZE-1)/MEM_ALIGN_SIZE)*MEM_ALIGN_SIZE;
        m_uiMemTrunkDataSize = m_uiMemTrunkSize - TRUNK_HEAD_SIZE;
        m_uiKeepMemSize = uiKeepMemSize;
        if (m_uiKeepMemSize < m_uiMemTrunkSize) m_uiKeepMemSize = m_uiMemTrunkSize;
        m_pUsedHead = NULL;
        m_pUsedTail = NULL;
        m_pFreeHead = NULL;
        m_uiTailLeft = 0;
        m_uiTotalSize = 0;
        m_uiFreeSize = 0;
    }
    ///��������
    ~CwxCharPool()
    {
        free();
    }
    ///����uiNum��С���ڴ�
    char* malloc(CWX_UINT32 uiNum);
    ///�Ƿ��ڴ���ж�����ڴ��
    void reset();
    ///�ͷ����е��ڴ��
    void free();
private:
    ///�ͷ��ڴ������
    inline void free(char* pChain) const
    {
        char* pNext = pChain;
        while(pNext)
        {
            pChain = getNext(pNext);
            delete [] pNext;
            pNext = pChain;
        }
    }
    ///��ȡ�ڴ�������ͷ�ڴ��
    inline char* getNext(char const* pTrunk) const
    {
        char* pNext = NULL;
        memcpy(&pNext, pTrunk, POINTER_SIZE);
        return pNext;
    }
    ///��һ���ڴ��ŵ��ڴ�������ͷ��
    inline void setNext(char* pTrunk, char const* pNext) const
    {
        memcpy(pTrunk, &pNext, POINTER_SIZE);
    }
    ///��ȡ��¼���ڴ���ͷ���ڴ��С��Ϣ
    inline CWX_UINT32 getSize(char const* pTrunk) const
    {
        CWX_UINT32 uiSize = 0;
        memcpy(&uiSize, pTrunk + POINTER_SIZE, sizeof(uiSize));
        return uiSize;
    }
    ///���ü�¼���ڴ��ͷ���ڴ��С��Ϣ
    inline void setSize(char* pTrunk, CWX_UINT32 uiSize) const
    {
        memcpy(pTrunk + POINTER_SIZE, &uiSize, sizeof(uiSize));
    }

private:
    CWX_UINT32       m_uiMemTrunkSize;///<��С���ڴ���С
    CWX_UINT32       m_uiKeepMemSize;///<��reset��ʱ����е��ڴ��С
    CWX_UINT32       m_uiMemTrunkDataSize;///<�ڴ�������ݲ��ֵĴ�С
    char*           m_pUsedHead;///<ʹ�õ��ڴ�������ͷ
    char*           m_pUsedTail;///<ʹ�õ��ڴ�������β
    char*           m_pFreeHead;///<���е��ڴ�������ͷ
    CWX_UINT32      m_uiTailLeft;///<��ǰ�ڴ��Ŀ����ڴ��С
    CWX_UINT32      m_uiTotalSize;///<������ڴ����ڴ�����
    CWX_UINT32      m_uiFreeSize;///<���е��ڴ����ڴ��С
};

CWINUX_END_NAMESPACE

#endif
