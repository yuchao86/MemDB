#ifndef __CWX_COMMON_H__
#define __CWX_COMMON_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxCommon.h
@brief ������CwxCommon�࣬���ඨ����һ�����õĹ�����̬API��
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
@class CwxCommon
@brief ʵ����һЩ�����ġ����ɹ���ĳ���API��
*/
class CWX_API CwxCommon
{
public:
    /**
    @brief ��src�����ݿ�����dest�У������ĳ��Ȳ��ܳ���len,dest���ַ�����\\0������
    @param [in,out] dest ������Ŀ��buf
    @param [in] src Դ�ַ���
    @param [in] len ��������󳤶�
    @return ����dest���ַ����ĳ���
    */
    static size_t copyStr(char* dest, char const* src, size_t len);
    /**
    @brief ��src��ǰsrc_len���ַ���������dest�У��������ܳ���dest_len�ĳ��ȣ�dest���ַ�����\\0������
    @param [in,out] dest ������Ŀ��buf
    @param [in] dest_len dest��buffer�ܽ��ܵ�����ַ�����
    @param [in] src Դ�ַ���
    @param [in] src_len ����src��ǰsrc_len���ַ�
    @return ����dest���ַ����ĳ���
    */
    static size_t copyStr(char* dest, size_t dest_len, char const* src, size_t src_len);
    /**
    @brief ����key=value����ʽ��src�ַ�������Ϊkey��value�����֣����ڷ��ص�kv��pair�����С�
    @param [in] src ��key=value����ʽ���ַ���
    @param [out] kv �����key/value��
    @return false��src������Ч��key/value�ԣ�true���ɹ����key/value��
    */
    static bool keyValue(string const& src, pair<string, string>& kv);
    /**
    @brief ����ĳ���ָ������зָ���ַ��������շָ�������ֶΣ�Ĭ�Ϸָ���Ϊ'|����
    @param [in] src ��Ҫ�ָ���ַ���
    @param [out] value ���շָ�����ɵ��ֶ��б���ĳ���ֶ�Ϊ�գ���ʾ�����ָ���������ָ���λ��ͷ����β��
    @param [in] ch �ָ�����Ĭ��Ϊ'|'
    @return �ָ�ɵ��ֶ�����
    */
    static int split(string const& src, list<string>& value, char ch='|');
    /**
    @brief ����ĳ���ָ������зָ��KEY/VALUE���ַ��������շָ������KEY/VALUE�Ե��б�Ĭ�Ϸָ���Ϊ'|����
    @param [in] src ��Ҫ�ָ��KEY/VALUE���ַ���
    @param [out] value ���շָ�����ɵ�KEY/VALUE�ԣ���ĳ������KEY/VALUE�ԣ������
    @param [in] ch �ָ�����Ĭ��Ϊ'|'
    @return �ָ�ɵ�KEY/VALUE�Ե�����
    */
    static int split(string const& src, list< pair<string, string> >& value, char ch='|');
    /**
    @brief ��һ��KEY/VALUE�Ե�LIST�У�����KEYΪname��key/value�ԡ���API��nameΪ��Сд���бȽ�
    @param [in] values KEY/VALUE���б�
    @param [in] name Ҫ���ҵ�Key������
    @param [out] item �����ҵ��ĵ�һ��Key/value�ԡ�
    @return true���ҵ���false��û���ҵ�
    */
    static bool findKey(list< pair<string, string> > const & values, string const& name, pair<string, string>& item);
    /**
    @brief ��һ��KEY/VALUE�Ե�LIST�У�����KEYΪname��key/value�ԡ���API��nameΪ��Сд�����бȽ�
    @param [in] values KEY/VALUE���б�
    @param [in] name Ҫ���ҵ�Key������
    @param [out] item �����ҵ��ĵ�һ��Key/value�ԡ�
    @return true���ҵ���false��û���ҵ�
    */
    static bool findCaseKey(list< pair<string, string> > const & values, string const& name, pair<string, string>& item);
    /**
    @brief ��һ��KEY/VALUE�Ե�LIST�У�����KEYΪname��key/value�ԡ���API��nameΪ��Сд���бȽ�
    @param [in] values KEY/VALUE���б�
    @param [in] name Ҫ���ҵ�Key������
    @param [out] item �����ҵ��ĵ�һ��Key/value�ԡ�
    @return true���ҵ���false��û���ҵ�
    */
    static bool findKey(list< pair<char*, char*> > const & values, char const* name, pair<char*, char*>& item);
    /**
    @brief ��һ��KEY/VALUE�Ե�LIST�У�����KEYΪname��key/value�ԡ���API��nameΪ��Сд���бȽ�
    @param [in] values KEY/VALUE���б�
    @param [in] name Ҫ���ҵ�Key������
    @param [out] item �����ҵ��ĵ�һ��Key/value�ԡ�
    @return true���ҵ���false��û���ҵ�
    */
    static bool findCaseKey(list< pair<char*, char*> > const & values, char const* name, pair<char*, char*>& item);
    /**
    @brief ��һ���ڴ��н�src��ʼ���ַ��ƶ���dest��λ�ã��ƶ��ĳ���Ϊn
    @param [in] dest Ҫ�Ƶ���λ��
    @param [in] src Ҫ�ƶ����ַ��Ŀ�ʼλ��
    @param [in] n �ƶ����ַ����ȡ�
    @return ����dest
    */
    static char* memMove(char *dest, char const *src, size_t n);
    /**
    @brief ȥ��value���ַ�����ͷ����β�Ŀո���chrs��Ϊ�գ���chrs�а������ַ�Ҳ���Ƴ���Χ�ڡ�
    @param [in] value ��TRIM���ַ���
    @param [in] chrs ���ո��⣬chrs�������ַ�Ҳ���Ƴ��ķ�Χ�ڡ�
    @return ����value
    */
    static string& trim(string& value, char const* chrs=NULL);
    /**
    @brief ȥ��value���ַ�����β�Ŀո���chrs��Ϊ�գ���chrs�а������ַ�Ҳ���Ƴ���Χ�ڡ�
    @param [in] value ��rtrim���ַ���
    @param [in] chrs ���ո��⣬chrs�������ַ�Ҳ���Ƴ��ķ�Χ�ڡ�
    @return ����value
    */
    static string& rtrim(string& value, char const* chrs=NULL);
    /**
    @brief ȥ��value���ַ�����ͷ�Ŀո���chrs��Ϊ�գ���chrs�а������ַ�Ҳ���Ƴ���Χ�ڡ�
    @param [in] value ��ltrim���ַ���
    @param [in] chrs ���ո��⣬chrs�������ַ�Ҳ���Ƴ��ķ�Χ�ڡ�
    @return ����value
    */
    static string& ltrim(string& value, char const* chrs=NULL);
    /**
    @brief ȥ��value���ַ�����ͷ����β�Ŀո���chrs��Ϊ�գ���chrs�а������ַ�Ҳ���Ƴ���Χ�ڡ�
    @param [in] value ��TRIM���ַ���
    @param [in] chrs ���ո��⣬chrs�������ַ�Ҳ���Ƴ��ķ�Χ�ڡ�
    @return ����value
    */
    static char* trim(char* value, char const* chrs=NULL);
    /**
    @brief ȥ��value���ַ�����β�Ŀո���chrs��Ϊ�գ���chrs�а������ַ�Ҳ���Ƴ���Χ�ڡ�
    @param [in] value ��rtrim���ַ���
    @param [in] chrs ���ո��⣬chrs�������ַ�Ҳ���Ƴ��ķ�Χ�ڡ�
    @return ����value
    */
    static char* rtrim(char* value, char const* chrs=NULL);
    /**
    @brief ȥ��value���ַ�����ͷ�Ŀո���chrs��Ϊ�գ���chrs�а������ַ�Ҳ���Ƴ���Χ�ڡ�
    @param [in] value ��ltrim���ַ���
    @param [in] chrs ���ո��⣬chrs�������ַ�Ҳ���Ƴ��ķ�Χ�ڡ�
    @return ����value
    */
    static char* ltrim(char* value, char const* chrs=NULL);
    /**
    @brief ��strSrc�г��ֵ�str1�ַ����滻Ϊstr2�ַ�����
    @param [in] strSrc ִ���û����ַ���
    @param [in] str1 ���滻���ַ�����
    @param [in] str2 �滻�ɵ��ַ�����
    @return ����strSrc
    */
    static string& replaceAll(string& strSrc, string const& str1, string const& str2);
    /**
    @brief ��src�г��ֵ�from�ַ����滻Ϊto�ַ������滻����ַ����ŵ�dest��buf�У�dest��\\0������
    @param [in] src ִ���û����ַ���
    @param [in] dest �γɵ��滻���ַ���
    @param [in,out] dest_len ����dest��buffer��С������滻����ַ�����
    @param [in] from ���滻���ַ�����
    @param [in] to �滻�ɵ��ַ�����
    @return ����dest
    */
    static char* replaceAll(char const* src, char* dest, size_t& dest_len, char const* from, char const* to);
    /**
    @brief ��AF12BC��ʽ��16�����ַ�������Ϊ�������ַ�����dest��\\0������
    @param [in] src 16�����ַ���
    @param [in] dest �γɵ��ַ���
    @param [in,out] dest_len ����dest��buffer��С������滻����ַ�����
    @return ����dest
    */
    static char* hexToString(char const* src, char* dest, size_t& dest_len);
    /**
    @brief ��src�еĴ�д�ַ���ΪСд�ַ���
    @param [in,out] src ��ΪСд���ַ���
    @return ����src
    */
    static char* lower(char* src);
    /**
    @brief ��src�е�Сд�ַ���Ϊ��д�ַ���
    @param [in,out] src ��Ϊ��д���ַ���
    @return ����src
    */
    static char* upper(char* src);
    ///��szIP�ĵ��ʽ�������ַ��Ϊ�����ֽ��������
    static CWX_UINT32 ipDot2Int(char const* szIp);
    ///�������ֽ����������Ϊ���ʽ�ĵ�ַ�ַ���
    static char* ipInt2Doc(CWX_UINT32 ip, char* szIp);
    ///ʵ��snprintf,ʵ��window��unixƽ̨��ͳһ
    static int snprintf(char *buf, size_t maxlen, const char *format, ...);
    ///���long long unsigned��ֵ���ַ���, baseΪ���ƣ�֧��16����base=16��10����base��=16
    static char const* toString(CWX_UINT64 ullNum, char* szBuf, int base=0);
    ///���long long��ֵ���ַ���, baseΪ���ƣ�֧��16����base=16��10����base��=16
    static char const* toString(CWX_INT64 llNum, char* szBuf, int base=0);
private:
    ///��ֹʵ����
    CwxCommon(){};
};

CWINUX_END_NAMESPACE

#include "CwxCommon.inl"
#include "CwxPost.h"

#endif

