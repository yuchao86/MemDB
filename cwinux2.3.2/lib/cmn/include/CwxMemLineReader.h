#ifndef __CWX_MEM_LINE_READER_H__
#define __CWX_MEM_LINE_READER_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxMemLineReader.h
@brief �����ڴ���ж�ȡ����Ķ���
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"

CWINUX_BEGIN_NAMESPACE
/**
@class CwxMemLineReader
@brief �ڴ���ж�ȡ��װ�����еķָ�ʱ\\n
*/
class CWX_API CwxMemLineReader
{
public:
    ///���캯��
    CwxMemLineReader()
    {
        m_uiCurLineNo = 0;
        m_uiCurLinePos = 0;
        m_uiNextLinePos = 0;
        m_szBuf = NULL;
        m_uiMaxLen = 0;
        m_bDataOwn = false;
        m_first = true;
    }
    ///��������
    ~CwxMemLineReader()
    {
        clear();
    }
public:
    /**
    @brief ����Ҫ�ж�ȡ���ڴ�
    @param [in] szData �и�ʽ������BUFFER
    @param [in] uiDataLen ���ݵĳ���
    @param [in] bDataOwn �ڴ��Ƿ��ɴ˶���ʵ�������ͷ�
    @return void
    */
    void setMem(char const* szData, CWX_UINT32 uiDataLen, bool bDataOwn=false);
    /**
    @brief ���ڴ�ָ���Ƶ���һ�еĿ�ʼλ�ã������ش�λ��,��ΪNULL����ʾ���������ڴ�<br>
           ���ڵ�һ�ε��ã���ָ���ڴ�Ŀ�ʼλ�ã�<br>
           ���ڴ��ܹ���N�У���ǰN�εĵ��ã�����ֵ��ΪNULL����N+1�η���NULL��
    @return NULL���ڴ��Ѿ����ꣻ����Ϊ��һ�еĿ�ʼλ�á�
    */
    char const* nextLine();
    ///��ȡ��ǰ���кţ���1��ʼ���
    CWX_UINT32 getCurLineNo() const;
    ///������ָ�룬�Ƶ��ڴ��ͷ����setMem()���״̬һ�¡�
    void seekHead();
    ///��ȡ��ǰ�е�ͷ
    char const* getCurLineBegin() const;
    ///��ȡ��ǰ�е�β������������ǰ���ַ�����Ϊ��һ�е�ͷ��
    char const* getCurLineEnd() const;
    ///��ն���
    void clear();
private:
    CWX_UINT32  m_uiCurLineNo;///<��ǰ�к�
    CWX_UINT32  m_uiCurLinePos;///<��ǰ����BUF�е�λ��
    CWX_UINT32  m_uiNextLinePos;///<��һ�еĿ�ʼ��buf�е�λ��
    char const* m_szBuf;///<�ڴ�BUF
    CWX_UINT32  m_uiMaxLen;///<�ڴ�BUF�ĳ���
    bool        m_bDataOwn;///<�Ƿ��ж�����BUF�Ŀռ��ͷ�
    bool        m_first;///<�Ƿ��һ�ε���nextLine() API
};

CWINUX_END_NAMESPACE

#include "CwxMemLineReader.inl"
#include "CwxPost.h"

#endif
