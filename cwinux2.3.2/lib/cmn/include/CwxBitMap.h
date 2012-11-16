#ifndef __CWX_BIT_MAP_H__
#define __CWX_BIT_MAP_H__

/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxBitMap.h
@brief ������CwxBitMap�࣬�书����ʵ�����2^32��λ�����á�������
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
@class CwxBitMap
@brief �����2^32��λ�������á����Ĳ��������в���Ϊ��̬��inline������
*/
class CWX_API CwxBitMap
{
public:
    /**
    @brief ���bitMapָ����ڴ��У���bitλ�Ƿ�����Ϊ1��
    @param [in] bitMap Ҫ����λ���ڵ��ڴ�
    @param [in] bit Ҫ����λ
    @return true������Ϊ1��false��û������Ϊ1
    */
    inline static bool isEnable(void* bitMap, CWX_UINT32 bit)
    {
        return ((((char*)bitMap)[bit>>3]) & (1<<(bit&0x07))) != 0;
    }
    /**
    @brief ��bitMapָ����ڴ�ĵ�bitλ����Ϊ1��
    @param [in] bitMap Ҫ���õ�λ���ڵ��ڴ�
    @param [in] bit Ҫ���õ�λ
    @return void
    */
    inline static void setBit(void* bitMap, CWX_UINT32 bit)
    {
        (((char*)bitMap)[bit>>3]) |= (1<<(bit&0x07));
    }
    /**
    @brief ��bitMapָ����ڴ�ĵ�bitλ���Ϊ0��
    @param [in] bitMap Ҫ���õ�λ���ڵ��ڴ�
    @param [in] bit Ҫ��յ�λ
    @return void
    */
    inline static void clrBit(void* bitMap, CWX_UINT32 bit)
    {
        (((char*)bitMap)[bit>>3]) &=~(1<<(bit&0x07));
    }
private:
    ///˽�й��캯������ֹʵ����
    CwxBitMap(){}
    ///˽����������
    ~CwxBitMap(){}
};

CWINUX_END_NAMESPACE
#include "CwxPost.h"

#endif
