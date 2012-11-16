#ifndef __CWX_DATE_H__
#define __CWX_DATE_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxDate.h
@brief �������ڸ�ʽת��������CwxDate
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/

#include "CwxPre.h"
#include "CwxType.h"
#include "CwxGlobalMacro.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"

CWINUX_BEGIN_NAMESPACE

/**
@class CwxDate
@brief ʵ���˶Գ������ڸ�ʽ�ı任������ķ���ȫ��Ϊ��̬����������ʵ������<br>
       CwxDate��ȡ��ʱ�䣬ȫ���Ǳ���ʱ����ʱ��
*/
class CWX_API CwxDate
{
public:
    /**
    @brief ��tt����ָ����ʱ�䣬ת��Ϊformat����ĸ�ʽ�������ء�
    @param [in] tt ʱ�����
    @param [in] format ����ʱ���ʽ��format����ʽ��strftime()һ��
    @param [out] datetime format�����ʱ���ʽ
    @return ����datetime����
    */
    static string& format(time_t tt, char const* format, string& datetime);
    /**
    @brief ����ǰ�ı�׼ʱ�䣬ת��ΪYYYY-MM-DD H24:MM:SS�ĸ�ʽ�������ء�
    @param [out] datetime ���YYYY-MM-DD H24:MM:SS��ʽ�ַ�����string ����
    @return ����datetime����
    */
    static string& getDate(string& datetime);
    /**
    @brief ��tt����ָ����ʱ�䣬ת��ΪYYYY-MM-DD H24:MM:SS�ĸ�ʽ�������ء�
    @param [in] tt ʱ�����
    @param [out] datetime ���YYYY-MM-DD H24:MM:SS��ʽ�ַ�����string ����
    @return ����datetime����
    */
    static string& getDate(time_t tt, string& datetime);
    /**
    @brief ��YYYY-MM-DD H24:MM:SS�ĸ�ʽʱ�䣬ת��Ϊtime_t��ʱ�䲢���ء�
    @param [in] datetime ��ʽΪYYYY-MM-DD H24:MM:SS��ʽ�ַ�������
    @return datetime��Ӧ��time_t��ʱ��
    */
    static time_t getDate(string const& datetime);
    /**
    @brief ��tt��ʾ��ʱ�䣬ת��ΪYYYYMMDDH24MMSS��ʽ�ַ��������ء�
    @param [in] tt Ҫת����ʱ��
    @param [out] datetime ���ص�YYYYMMDDH24MMSS��ʽ���ַ���
    @return ����datetime����
    */
    static string& getDateY4MDHMS2(time_t tt, string& datetime);
    /**
    @brief ��YYYYMMDDH24MMSS��ʽ�ַ�����ת��Ϊtime_t����ʽ�����ء�
    @param [in] datetime YYYYMMDDH24MMSS��ʽ���ַ���
    @return datetime��Ӧ��ʱ��
    */
    static time_t getDateY4MDHMS2(const string& datetime);
    /**
    @brief ��tt��ʱ��ֵ����Сʱ��һ������ȥ������1Сʱ�Ĳ���
    @param [in] tt Ҫ�����ʱ��
    @return ���ذ�Сʱ��һ����time_t
    */
    static time_t trimToHour(time_t  tt);
    /**
    @brief ��tt��ʱ��ֵ�������һ������ȥ������1��Ĳ���
    @param [in] tt Ҫ�����ʱ��
    @return ���ذ����һ����time_t
    */
    static time_t trimToDay(time_t  tt);
    /**
    @brief ��ȡtime_t�е����
    @param [in] tt Ҫ��ȡ��ݵ�time_t
    @return ����ttʱ�����ڵ����
    */
    static int	getYear(time_t tt);
    /**
    @brief ��ȡtime_t�е��·�
    @param [in] tt Ҫ��ȡ�·ݵ�time_t
    @return ����ttʱ�����ڵ��·�
    */
    static int	getMonth(time_t tt);
    /**
    @brief ��ȡtime_t�е������·ݵ�����
    @param [in] tt Ҫ��ȡ�����·ݵ����ڵ�time_t
    @return ����ttʱ�������·ݵ�����
    */
    static int	getDay(time_t tt);
    /**
    @brief ��ȡtime_t��Ӧ���ڵ�Сʱ
    @param [in] tt Ҫ��ȡСʱ��time_t
    @return ����ttʱ���Сʱ
    */
    static int	getHour(time_t tt);
    /**
    @brief ��ȡtime_t��Ӧ���ڵķ���
    @param [in] tt Ҫ��ȡ���ӵ�time_t
    @return ����tt��Ӧ���ڵķ���
    */
    static int	getMinute(time_t tt);
    /**
    @brief ��ȡtime_t��Ӧ���ڵ���
    @param [in] tt Ҫ��ȡ���time_t
    @return ����tt��Ӧ���ڵ���
    */
    static int	getSecond(time_t tt);
    /**
    @brief ��ȡ��ȷ��΢��ĵ�ǰʱ�䣬Ϊ64λ���޷�����
    @return ���ؾ�ȷ��΢��ĵ�ǰʱ��
    */
    static CWX_UINT64 getTimestamp();
public:
    // Sat, 01 Jan 2005 12:00:00 +0100
    static const string HTTP_FORMAT;

private:
    ///�����ֹʵ����
    CwxDate(){}
};

CWINUX_END_NAMESPACE

#include "CwxDate.inl"
#include "CwxPost.h"

#endif
