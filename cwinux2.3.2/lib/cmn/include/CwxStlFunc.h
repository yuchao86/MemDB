#ifndef  __CWX_STL_FUNC_H__
#define  __CWX_STL_FUNC_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxStlFunc.h
*@brief STL contain's method
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-06-30
*@warning  none.
*/
#include "CwxPre.h"
#include "CwxType.h"
#include "CwxGlobalMacro.h"
#include "CwxStl.h"

CWINUX_BEGIN_NAMESPACE

/**
*@brief  ���Դ�Сд���ַ�����hash-code���γɺ���.
*@param [in] __s �ַ���.
*@return �ַ�����hash-code
*/
inline size_t __stl_hash_case_string(const char* __s)
{
    unsigned long __h = 0;
    for ( ; *__s; ++__s)
        __h = 5*__h + ((*__s>='a')||(*__s<='z'))?'A'+(*__s-'a'):*__s;
    return size_t(__h);
}

/**
*@class  CwxHash
*@brief  ��ʱһ�����������࣬����һ�������API��hash()��������hash-code��<br>
*        �˶�������ṩhash()��api.
*/
template<typename T> class CwxHash
{
public:
    /**
    *@brief  ����T��ָ�룬��ȡT��hash-code��<br>
    *        T �����ṩ [size_t hash() const]�ķ�����<br>
    *@return T��ָ������hash-code
    */
    inline size_t operator()(T const &key)const
    {
        return key.hash();
    }
};

/**
*@class  CwxPointHash
*@brief  ��ʱһ�����������࣬����һ�������ָ�룬������hash-code��<br>
*        �˶�������ṩhash()��api.
*/
template<typename T> class CwxPointHash
{
public:
    /**
    *@brief  ����T��ָ�룬��ȡT��hash-code��<br>
    *        T �����ṩ [size_t hash() const]�ķ�����<br>
    *@return T��ָ������hash-code
    */
    inline size_t operator()(T const *key)const 
    {
        return key->hash();
    }
};

/**
*@class  CwxPointEqual
*@brief  �Ƚ�����T��ָ�����������Ƿ���ȣ�����һ�����������ࡣ
*/
template<typename T> class CwxPointEqual
{
public:
    /**
    *@brief  �Ƚ�T��ָ��key1��key2�����ݶ�����ָ�뱾���Ƿ���ȡ�<br>
    *        ���ǶԶ����operator()���������ء�<br>
    *        ����T����֧�֡�==���������Ƚ���ʵ���Ƿ���ȡ�
    *@return true: ���; false: �����
    */
    inline bool operator()(T const *key1, T const *key2) const 
    {
        return *key1 == *key2;
    }
};

/**
*@class  CwxPointLess
*@brief  ���ж���ָ�����ݶ����ǵ�ַ�����less�Ƚϡ���ʱһ������������
*/
template<typename T> class CwxPointLess
{
public:
    /**
    *@brief  �Ƚ�*key1<*key2�Ƿ����
    *        ������CwxPointLess��operator()(T const *key1, T const *key2).
    *@return true:*key1<*key2��false��*key1��С��*key2
    */
    inline bool operator()(T const *key1, T const *key2) const 
    {
        return *key1 < *key2;
    }
};


/**
/@class  CwxCharHash
*@brief  ��ȡchar*��hash-code��ͬCwxPointHash.
**/
class CwxCharHash{
public:
    /**
    *@brief  ��ȡ�ַ���str��hash-code,
    *@return str��hash-key
    */
    inline size_t operator()(const char* str) const 
    {
        return __stl_hash_string(str);
    }
};

/**
/@class  CwxCharEqual
*@brief  �Ƚ������ַ����Ƿ���ȣ�ͬCwxPointLess.
**/
class CwxCharEqual
{
public:
    /**
    *@brief  �Ƚ�str1�������Ƿ����str2�����ݡ�
    *@return true: ����; false: ������.
    */
    inline bool operator()(const char* str1, const char* str2) const
    {
        return strcmp(str1,str2)==0;
    }
};

/**
*@class  CwxCharLess
*@brief  �ַ���less�Ƚϣ�ͬCwxPointLess  
*/
class CwxCharLess
{
public:
    /**
    *@brief  �Ƚ�str1���ַ����Ƿ�С��str2���ַ���.
    *@return true:С�ڣ�false����С�ڡ�
    */
    inline bool operator()(const char* str1, const char* str2) const
    {
        return strcmp(str1,str2)<0;
    }
};


/**
/@class  CwxCharCaseHash
*@brief  ��ȡchar*�Ĵ�Сд������hash-code��ͬCwxCharHash.
**/
class CwxCharCaseHash
{
public:
    /**
    *@brief  ��ȡ�ַ���str�Ĵ�Сд������hash-code
    *@return str��hash-code
    */
    inline size_t operator()(const char* str) const 
    {
        return __stl_hash_case_string(str);
    }
};

/**
*@class  CwxCharCaseEqual
*@brief  �Ƚ�������Сд�����е��ַ����Ƿ���ȣ�ͬCwxCharEqual.
*/
class CwxCharCaseEqual
{
public:
    /**
    *@brief  �Ƚ�������Сд�����е��ַ����Ƿ����.
    *@return true: ���; false: �����.
    */
    inline bool operator()(const char* str1, const char* str2) const 
    {
        return strcasecmp(str1,str2)==0;
    }
};

/**
*@class  CwxCharCaseLess
*@brief  �Ƚϴ�Сд�����е��ַ���str1�Ƿ�С��str2.ͬCwxCharLess.
*/
class CwxCharCaseLess
{
public:
    /**
    *@brief  �Ƚϴ�Сд�����е��ַ���str1�Ƿ�С��str2��
    *@return true��С�ڣ�false����С��
    */
    inline bool operator()(const char* str1, const char* str2) const 
    {
        return strcasecmp(str1,str2)<0;
    }
};



/**
/@class  CwxCharCaseHash2
*@brief  ͨ���������ƻ�ȡ��ȡ��Сд���л��ǲ����е�hash-code��ͬCwxCharHash.
**/
class CwxCharCaseHash2
{
public:
    ///ȱʡ���캯����Ĭ�ϴ�Сд����
    CwxCharCaseHash2():m_bCaseSensive(true)
    {

    }
    ///���캯���������Ƿ��Сд���С�bCaseSensive��true����Сд���У�false����Сд������
    CwxCharCaseHash2(bool bCaseSensive):m_bCaseSensive(bCaseSensive)
    {
    }
    ///��������
    CwxCharCaseHash2(CwxCharCaseHash2 const& obj)
    {
        m_bCaseSensive = obj.m_bCaseSensive;
    }
public:
    /**
    *@brief  ��ȡ�ַ���str�Ĵ�Сд�����л����е�hash-code
    *@return str��hash-code
    */
    inline size_t operator()(const char* str) const 
    {
        return m_bCaseSensive?__stl_hash_string(str):__stl_hash_case_string(str);
    }
private:
    bool    m_bCaseSensive;
};

/**
*@class  CwxCharCaseEqual2
*@brief  ͨ�����������������ַ������Ǵ�Сд���е���Ȼ��ǲ����е���ȱȽ�.
*/
class CwxCharCaseEqual2
{
public:
    ///ȱʡ���캯��,ȱʡ��Сд����
    CwxCharCaseEqual2():m_bCaseSensive(true)
    {

    }
    ///��ȷָ����Сд���в����Ĺ��캯��.bCaseSensive��true����Сд���У�false����Сд������
    CwxCharCaseEqual2(bool bCaseSensive):m_bCaseSensive(bCaseSensive)
    {
    }
    ///��������
    CwxCharCaseEqual2(CwxCharCaseEqual2 const& obj)
    {
        m_bCaseSensive = obj.m_bCaseSensive;
    }
public:
    /**
    *@brief  �Ƚ�������Сд���л����е��ַ����Ƿ����.
    *@return true: ���; false: �����.
    */
    inline bool operator()(const char* str1, const char* str2) const
    {
        return m_bCaseSensive?strcmp(str1, str2)==0:strcasecmp(str1,str2)==0;
    }
private:
    bool    m_bCaseSensive;
};

/**
*@class  CwxCharCaseLess2
*@brief  �Ƚϴ�Сд���л����е��ַ���str1�Ƿ�С��str2.ͬCwxCharLess.
*/
class CwxCharCaseLess2
{
public:
    ///ȱʡ���캯������ʱ���ַ���Сд����
    CwxCharCaseLess2():m_bCaseSensive(true)
    {
    }
    ///ָ����Сд���л����в����Ĺ��캯��
    CwxCharCaseLess2(bool bCaseSensive):m_bCaseSensive(bCaseSensive)
    {
    }
    ///��������
    CwxCharCaseLess2(CwxCharCaseLess2 const& obj)
    {
        m_bCaseSensive = obj.m_bCaseSensive;
    }
public:
    /**
    *@brief  �Ƚϴ�Сд���л����е��ַ���str1�Ƿ�С��str2��
    *@return true��С�ڣ�false����С��
    */
    inline bool operator()(const char* str1, const char* str2) const
    {
        return m_bCaseSensive?strcmp(str1, str2)<0:strcasecmp(str1,str2)<0;
    }
private:
    bool       m_bCaseSensive;
};


/**
*@class  CwxStringHash
*@brief  ��ȡstring�����ַ�����hash-code��ͬCwxCharHash
*/
class CwxStringHash
{
public:
    /**
    *@brief  ��ȡstring�����ַ�����hash-code.
    *@return str�� hash-key
    */
    inline size_t operator()(string const& str) const
    {
        return __stl_hash_string(str.c_str()); 
    }
};

/**
*@class  CwxCaseStringHash
*@brief  ��ȡstring�����ַ����Ĵ�Сд�����е�hash-code.
*/
class CwxCaseStringHash
{
public:
    /**
    *@brief  ��ȡstring�����ַ����Ĵ�Сд�����е�hash-code.
    *@return str�� hash-key
    */
    inline size_t operator()(string const & str) const 
    {
        return __stl_hash_case_string(str.c_str()); 
    }
};

/**
*@class  CwxCaseStringEqual
*@brief  �Ƚϴ�Сд�����е�����string�����ַ����Ƿ���ȡ�
*/
class CwxCaseStringEqual
{
public:
    /**
    *@brief  �Ƚϴ�Сд�����е�����string�����ַ����Ƿ���ȡ�
    *@return true����ȣ�false�������
    */
    inline bool operator()(string const & str1, string const& str2) const 
    {
        return strcasecmp(str1.c_str(), str2.c_str())==0; 
    }
};
/**
*@class  CwxCaseStringLess
*@brief  �Ƚϴ�Сд�����е�string���͵�str1�Ƿ�С��str2��
*/
class CwxCaseStringLess
{
public:
    /**
    *@brief  �Ƚϴ�Сд�����е�string���͵�str1�Ƿ�С��str2��
    *@return true��С�ڣ�false����С��
    */
    inline bool operator()(string const & str1, string const& str2) const
    {
        return strcasecmp(str1.c_str(), str2.c_str())<0; 
    }
};

/**
*@class  CwxCaseStringHash2
*@brief  ��ȡstring�����ַ����Ĵ�Сд���л����е�hash-code.
*/
class CwxCaseStringHash2
{
public:
    ///Ĭ�Ϲ��캯����Ϊ��Сд���е�hash-code
    CwxCaseStringHash2():m_bCaseSensive(true)
    {
    }
    ///��ָ����Сд���п��صĹ��캯����bCaseSensive��true����Сд���У�false����Сд������
    CwxCaseStringHash2(bool bCaseSensive)
        :m_bCaseSensive(bCaseSensive)
    {

    }
    ///��������
    CwxCaseStringHash2(CwxCaseStringHash2 const& obj)
    {
        m_bCaseSensive = obj.m_bCaseSensive;
    }
public:
    /**
    *@brief  ��ȡstring�����ַ����Ĵ�Сд���л����е�hash-code.
    *@return str�� hash-key
    */
    inline size_t operator()(string const & str) const 
    {
        return m_bCaseSensive? __stl_hash_string(str.c_str()):__stl_hash_case_string(str.c_str());
    }
private:
    bool     m_bCaseSensive;
};

/**
*@class  CwxCaseStringEqual2
*@brief  �Ƚϴ�Сд���л����е�����string�����ַ����Ƿ���ȡ�
*/
class CwxCaseStringEqual2
{
public:
    ///Ĭ�Ϲ��캯����ȱʡΪ��Сд����
    CwxCaseStringEqual2():m_bCaseSensive(true)
    {

    }
    ///����ָ����Сд���л����п��صĹ��캯��
    CwxCaseStringEqual2(bool bCaseSensive)
        :m_bCaseSensive(bCaseSensive)
    {

    }
    ///��������
    CwxCaseStringEqual2(CwxCaseStringEqual2 const& obj)
    {
        m_bCaseSensive=obj.m_bCaseSensive;
    }
public:
    /**
    *@brief  �Ƚϴ�Сд���л����е�����string�����ַ����Ƿ���ȡ�
    *@return true����ȣ�false�������
    */
    inline bool operator()(string const & str1, string const& str2) const
    {
        return m_bCaseSensive?strcmp(str1.c_str(), str2.c_str())==0:strcasecmp(str1.c_str(), str2.c_str())==0;
    }
private:
    bool       m_bCaseSensive;
};

/**
*@class  CwxCaseStringLess2
*@brief  �Ƚϴ�Сд���л����е�string���͵�str1�Ƿ�С��str2��
*/
class CwxCaseStringLess2
{
public:
    ///Ĭ�Ϲ��캯��
    CwxCaseStringLess2():m_bCaseSensive(true)
    {

    }
    ///����ָ�����л����п��صĹ��캯��
    CwxCaseStringLess2(bool bCaseSensive)
        :m_bCaseSensive(bCaseSensive)
    {

    }
    ///��������
    CwxCaseStringLess2(CwxCaseStringLess2 const& obj)
    {
        m_bCaseSensive=obj.m_bCaseSensive;
    }
public:
    /**
    *@brief  �Ƚϴ�Сд���л����е�string���͵�str1�Ƿ�С��str2��
    *@return true��С�ڣ�false����С��
    */
    inline bool operator()(string const & str1, string const& str2) const
    {
        return m_bCaseSensive?strcmp(str1.c_str(), str2.c_str())<0:strcasecmp(str1.c_str(), str2.c_str())<0;
    }
private:
    bool       m_bCaseSensive;
};



/**
*@class  CwxNumHash
*@brief  ���������typedef�����hash���岻���ڵ��������͵�hash.
*/
template <typename T>
class CwxNumHash
{
public:
    /**
    *@brief  ��÷��������͵�hash-code
    *@return hash-code
    */
    inline size_t operator()(T const key)const
    { 
        return (size_t)key;
    }
};

/**
*@class  CwxSize
*@brief  ����һ�������sizeof�Ĵ�Сģ�庯������.
*/
template<typename T>
class CwxSize
{
public:
    /**
    *@brief  ���ض���T��sizeof()
    *@return �����sizeof
    */
    inline size_t operator()(T const key) const
    {
        return sizeof(key);
    }
};



CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif

