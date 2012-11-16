#ifndef  __CWX_STL_H__
#define  __CWX_STL_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxStl.h
*@brief sgi header
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-06-28
*@warning  ��.
*/
#include "CwxPre.h"

#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <map>
#ifdef __GNUC__
#include <ext/hash_set>
#include <ext/hash_map>
using namespace __gnu_cxx;
#define HAVE_STL_HASH  1
#else
#undef HAVE_STL_HASH
#endif

using namespace std;

#include "CwxPost.h"
#endif

