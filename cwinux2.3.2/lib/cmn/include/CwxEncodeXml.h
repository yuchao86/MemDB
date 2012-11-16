#ifndef __CWX_ENCODE_XML_H__
#define __CWX_ENCODE_XML_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
*@file  CwxEncodeXml.h
*@brief ��XML�������ַ�����ת�Ƶ�CwxEncodeXml����Ķ���
*@author cwinux@gmail.com
*@version 0.1
*@date  2009-10-18
*@warning  ��.
*/
#include "CwxPre.h"
#include "CwxGlobalMacro.h"
#include "CwxType.h"
#include "CwxStl.h"
#include "CwxReplMachine.h"

CWINUX_BEGIN_NAMESPACE

/**
* @class CwxEncodeXml
*
* @brief ��XML�������ַ�����ת�ƣ�Ĭ����Ϊ���£�
  1����<��-->��&lt;��
  2����>��-->��&gt;��
  3����&��-->��&amp;��
  4����'��-->��&#39;��
  5����1~31���У�����\\t,\\r,\\n�⣬ȫ���滻Ϊ�գ���ɾ����

 */

class CWX_API CwxEncodeXml
{
public:
    ///���캯����Ĭ��Ϊ��GBK����ΪUTF-8�������б����滻��ʱ�򣬴�Сд���С�
    CwxEncodeXml(bool bGbk=false, bool bCaseSensive=true);
    ///��������
    ~CwxEncodeXml();
public:
    /**
    @brief ��ʼ��xml�����ַ����滻��
    @param [in] append ����Ĭ�ϵģ������ӵ��滻�ַ���
    @param [in] remove �Ƴ�Ĭ���е��滻�ַ���
    @return true:��ӳɹ���false���ַ����������
    */
    bool init(map<string, string> const& append = emptyXml, map<string, string> const& remove = emptyXml);
    /**
    @brief ��XML�е������ַ������滻���������ַ�����0x00������
    @param [in] szIn ��Ҫ�����滻�������ַ�����
    @param [in,out] uiInLen ����szIn���ַ����ĳ��ȣ����سɹ�ִ�б�����ַ������ȣ�
                            �����ڳɹ�������£����ڿռ䲻���ֻ����˲���ת�롣
    @param [in] szOut ������û����ַ�����buf��
    @param [in,out] uiOutLen ����szOut�Ŀռ��С������XML�������ַ����ĳ��ȡ�
    @return true���ɹ���false��ʧ�ܡ�ʧ�ܵ�ԭ����������ַ�����Ĵ���
    */
    bool encode(char const* szIn, CWX_UINT32& uiInLen, char* szOut, CWX_UINT32& uiOutLen) const;
    ///�����Ƿ�GBK
    bool isGbk() const;
    ///�����Ƿ��Сд����
    bool isCaseSensive() const;
private:
    ///��ӱ���ת���ַ���������false��ʾ��ӵ�Դ�ַ����ı������
    bool addStr(char const* src, char const* dst, map<string, string> const& append, map<string, string> const& remove);
private:
    static  map<string, string> const emptyXml;
    CwxReplBase*    m_pRepl;
    bool           m_bGbk;
    bool           m_bCaseSensive;
};



CWINUX_END_NAMESPACE

#include "CwxEncodeXml.inl"
#include "CwxPost.h"

#endif
