#ifndef  __CWX_GET_OPT_H__
#define  __CWX_GET_OPT_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file CwxGetOpt.h
*@brief CwxGetOpt����
*@author cwinux@gmail.com
*@version 1.0
*@date  2009-06-05
*@warning  nothing
*@bug    
*/

#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include <getopt.h>
#include <stdlib.h>

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxGetOpt
*
* @brief ��os��getopt_long�ķ�װ.
*/
class CWX_API CwxGetOpt
{
public:
    enum {
        /// Doesn't take an argument.
        NO_ARG = 0,
        /// Requires an argument, same as passing ":" after a short option
        /// character in optstring.
        ARG_REQUIRED = 1,
        /// Argument is optional, same as passing "::" after a short
        /// option character in optstring.
        ARG_OPTIONAL = 2
    };
    ///���캯��
    CwxGetOpt (int argc,
        char **argv,
        char const* optstring = "");
    ///��������
    ~CwxGetOpt (void);
public:
    ///����long option��Ӧ��short option
    int long_option (char const *name, int short_option, int has_arg = NO_ARG);
    /**
    @brief ��ȡ��һ������
    @return ��getopt_long()��ͬ
    */
    int next();
    ///��ȡoption�Ĳ���
    char *opt_arg (void) const;
    ///���ص�ǰ��option
    int opt_opt (void) const;
    ///���ص�ǰ������index
    int opt_ind (void) const;
    ///���ص�ǰ��long option��
    char const* long_option() const;
private:
    int             m_argc;
    char **         m_argv;
    char const*     m_optString;
    struct option*   m_longOptions;
    int             m_longindex;
    int             m_longOptionsNum;
    int             m_longOptionsSize;
    int             m_opt;
};



CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif
