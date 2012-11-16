#ifndef __CWX_CHARSET_H__
#define __CWX_CHARSET_H__
/*
��Ȩ������
    �������ѭGNU GPL V3��http://www.gnu.org/licenses/gpl.html����
    ��ϵ��ʽ��email:cwinux@gmail.com��΢��:http://t.sina.com.cn/cwinux
*/

/**
@file CwxCharset.h
@brief ����������ص�UTF8��GBK������GB3212����UTF16�����ַ���������һ�µĲ����ӿ�
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
@class CwxMultiString
@brief �ַ����Ľṹ�������ַ��������ݼ��ַ����ĳ��ȣ������ַ��滻���ַ�������ص���
*/
struct CwxMultiString
{
    CWX_UINT32    m_uiLength;///<m_szStr���ַ����ĳ���
    char      m_szStr[0];///<�ַ���
};

/**
@class CwxCharsetGbk
@brief GBK�ַ����Ľӿڶ��壬ȫ��Ϊ��̬�ӿڣ���ֹʵ����
*/
class CWX_API CwxCharsetGbk
{
public:
    /**
    @brief ��ȡ������������ַ��������֡�
    @return ���ء�gbk�����ַ���
    */
    inline static char const* charset()
    {
        return "gbk";
    }
    /**
    @brief �ж�һ���ַ����������Ƿ����CwxCharsetGbk��Ľӿ�������
    @param [in] szCharset �������ַ���������
    @return ����"gbk"��"gb2312"������true�����򷵻�false
    */
    inline static bool charset(char const* szCharset) 
    {
        return ((strcasecmp("gbk", szCharset)==0) || (strcasecmp("gb2312", szCharset)==0))?true:false;
    }
    /**
    @brief �ж�szGbkBuf�еĵ�һ���ַ����ֽ�����0��ʾ��Ч���ַ��������
    @param [in] szGbkBuf Ҫ�����ַ���buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @return 0��ʾ��Ч���ַ������������0��ʾszGbkBuf�е�һ���ַ����ֽڳ���
    */
    inline static CWX_UINT32 nextChrLen(char const* szGbkBuf, CWX_UINT32 uiBufLen)
    {
        if (uiBufLen)
        {
            if (szGbkBuf[0]<0)
            {
                return isCharset(szGbkBuf, uiBufLen)?2:0;
            }
            return szGbkBuf[0]?1:0;
        }
        return 0;
    }
    /**
    @brief ��szGbkBuf�еĵ�һ���ַ�ͨ��szUpper���ⲿBUF���أ�����Сд���Ϊ��д��
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @param [out] szUpper szGbkBuf�еĵ�һ���ַ�������Сд����任Ϊ��д����
    @return 0��ʾ��Ч���ַ������������0��ʾszUpper�з��صĵ�һ���ַ��ĳ���
    */
    inline static  CWX_UINT32 upperChr(char const* szGbkBuf, CWX_UINT32 uiBufLen, char* szUpper)
    {
        CWX_UINT32 uiNextLen = nextChrLen(szGbkBuf, uiBufLen);
        if (1 == uiNextLen)
        {
            if ((szGbkBuf[0]>='a') && (szGbkBuf[0]<='z'))
            {
                szUpper[0] = szGbkBuf[0] - ('a' - 'A');
            }
            else
            {
                szUpper[0] = szGbkBuf[0];
            }
        }
        else if (uiNextLen)
        {
           memcpy(szUpper, szGbkBuf, uiNextLen);
        }
        return uiNextLen;
    }

    /**
    @brief ��szGbkBuf�еĵ�һ���ַ�ͨ��szLower���ⲿBUF���أ����Ǵ�д���ΪСд��
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @param [out] szLower szGbkBuf�еĵ�һ���ַ������Ǵ�д����任ΪСд����
    @return 0��ʾ��Ч���ַ������������0��ʾszLower�з��صĵ�һ���ַ��ĳ���
    */
    inline static  CWX_UINT32 lowerChr(char const* szGbkBuf, CWX_UINT32 uiBufLen, char* szLower)
    {
        CWX_UINT32 uiNextLen = nextChrLen(szGbkBuf, uiBufLen);
        if (1 == uiNextLen)
        {
            if ((szGbkBuf[0]>='A') && (szGbkBuf[0]<='Z'))
            {
                szLower[0] = szGbkBuf[0] + ('a' - 'A');
            }
            else
            {
                szLower[0] = szGbkBuf[0];
            }
        }
        else if (uiNextLen)
        {
            memcpy(szLower, szGbkBuf, uiNextLen);
        }
        return uiNextLen;
    }

    /**
    @brief ���szGbkBuf�еĵ�һ���ַ����Ƿ�һ��������˫�ֽ�GBK�ַ���
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @return true���ǣ�false������
    */
    inline static  bool isCharset(char const* szGbkBuf, CWX_UINT32 uiBufLen)
    {
        if ( ((unsigned char)szGbkBuf[0] >=0x81) &&
            ((unsigned char)szGbkBuf[0] <=0xFE) &&
            ((unsigned char)szGbkBuf[1] >=0x40) &&
            ((unsigned char)szGbkBuf[1] <=0xFE) && uiBufLen>1)
        {
            return true;
        }
        return false;
    }

    /**
    @brief ���szGbkBuf���ַ����Ƿ�ΪGBK�ַ���֧�ֵ��ַ���
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @return true����GBK�ַ�����false������GBK�ַ���
    */
    inline static  bool isValid(char const * szGbkBuf, CWX_UINT32 uiBufLen)
    {
        if(uiBufLen)
        {
            CWX_UINT32 uiPos = 0;
            CWX_UINT32 uiLen = 0;
            while(uiPos < uiBufLen)
            {
                uiLen = nextChrLen(szGbkBuf+uiPos, uiBufLen - uiPos);
                if(0 == uiLen) return false;
                uiPos += uiLen;
            }
            return (uiPos == uiBufLen)?true:false;
        }
        return true;
    }

    /**
    @brief ��ȡszGbkBuf�е��ַ�����
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @return szGbkBuf���ַ��ĸ���
    */
    inline static  CWX_UINT32 chrlen(char const* szGbkBuf, CWX_UINT32 uiBufLen)
    {
        CWX_UINT32 src_pos =0;
        CWX_UINT32 count =0;
        CWX_UINT32 len;
        while (src_pos < uiBufLen)
        {
            len =nextChrLen(szGbkBuf + src_pos, uiBufLen-src_pos);
            if (0 == len) return count;
            count ++;
            src_pos +=len;
        }
        return count;
    }

    /**
    @brief �ڲ����ַ��ضϵ�����£������szGbkBuf�н�ȡuiFetchLen���ֽڵ��ֽڽ���λ�á�
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @param [in] uiFetchLen Ҫ��ȡ������ֽ�����
    @return ���ؽ�ȡ���ֽ�λ��
    */
    inline static  CWX_UINT32 getPosByByteLen(char const* szGbkBuf, CWX_UINT32 uiBufLen, CWX_UINT32 uiFetchLen)
    {
        CWX_UINT32 pos =0;
        CWX_UINT32 len =0;
        while(1)
        {
            len =nextChrLen(szGbkBuf + pos, uiBufLen - pos);
            if (!len || (pos + len > uiBufLen) || (pos + len > uiFetchLen)) break;
            pos +=len;
        }
        return pos;
    }
    /**
    @brief �ڲ����ַ��ضϵ�����£������szGbkBuf�н�ȡuiFetchNum���ַ����ֽڽ���λ�á�
    @param [in] szGbkBuf �ַ�����buffer
    @param [in] uiBufLen szGbkBuf���ֽڳ���
    @param [in] uiFetchNum Ҫ��ȡ������ַ�����
    @return ���ؽ�ȡ���ֽ�λ��
    */
    inline static  CWX_UINT32 getPosByChrLen(char const* szGbkBuf, CWX_UINT32 uiBufLen, CWX_UINT32 uiFetchNum)
    {
        CWX_UINT32 pos =0;
        CWX_UINT32 len =0;
        CWX_UINT32 num =0;
        while(1)
        {
            len =nextChrLen(szGbkBuf + pos, uiBufLen - pos);
            if (!len || (pos + len > uiBufLen) || (num + 1 > uiFetchNum)) break;
            num ++;
            pos +=len;
        }
        return pos;
    }
private:
    ///˽�л����캯������ֹʵ����
    CwxCharsetGbk() {};
    ///˽�л�������������ֹʵ����
    ~CwxCharsetGbk() {};
};


/**
@class CwxCharsetUtf8
@brief UTF8�ַ����Ľӿڶ��壬ȫ��Ϊ��̬�ӿڣ���ֹʵ����
*/
class CWX_API CwxCharsetUtf8
{
public:
    /**
    @brief ��ȡ������������ַ��������֡�
    @return ���ء�utf-8�����ַ���
    */
    inline static char const* charset()
    {
        return "utf-8";
    }
    /**
    @brief �ж�һ���ַ����������Ƿ����CwxCharsetUtf8��Ľӿ�������
    @param [in] szCharset �������ַ���������
    @return ����"utf-8"������true�����򷵻�false
    */
    inline static bool charset(char const* szCharset)
    {
        return strcasecmp("utf-8", szCharset)==0?true:false;
    }
    /**
    @brief �ж�szUtf8Buf�еĵ�һ���ַ����ֽ�����0��ʾ��Ч���ַ��������
    @param [in] szUtf8Buf Ҫ�����ַ���buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @return 0��ʾ��Ч���ַ������������0��ʾszUtf8Buf�е�һ���ַ����ֽڳ���
    */
    inline static  CWX_UINT32 nextChrLen(char const* szUtf8Buf, CWX_UINT32 uiBufLen)
    {
        CWX_UINT32 i;
        unsigned char c = szUtf8Buf[0];
        if (uiBufLen)
        {
            if ( (c & 0x80) == 0) return 1;
            for (i=1; i<8; i++)
            {
                c <<= 1;
                if ( (c & 0x80) == 0) break;
                if ((szUtf8Buf[i] &0xC0) != 0x80) return 0; //error, skip it.
            }
            return i<=uiBufLen?i:0;
        }
        return 0;
    }
    /**
    @brief ��szUtf8Buf�еĵ�һ���ַ�ͨ��szUpper���ⲿBUF���أ�����Сд���Ϊ��д��
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @param [out] szUpper szUtf8Buf�еĵ�һ���ַ�������Сд����任Ϊ��д����
    @return 0��ʾ��Ч���ַ������������0��ʾszUpper�з��صĵ�һ���ַ��ĳ���
    */
    inline static  CWX_UINT32 upperChr(char const* szUtf8Buf, CWX_UINT32 uiBufLen, char* szUpper)
    {
        CWX_UINT32 uiNextLen = nextChrLen(szUtf8Buf, uiBufLen);
        if (1 == uiNextLen)
        {
            if ((szUtf8Buf[0]>='a') && (szUtf8Buf[0]<='z'))
            {
                szUpper[0] = szUtf8Buf[0] - ('a' - 'A');
            }
            else
            {
                szUpper[0] = szUtf8Buf[0];
            }
        }
        else if (uiNextLen)
        {
            memcpy(szUpper, szUtf8Buf, uiNextLen);
        }
        return uiNextLen;
    }

    /**
    @brief ��szUtf8Buf�еĵ�һ���ַ�ͨ��szLower���ⲿBUF���أ����Ǵ�д���ΪСд��
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @param [out] szLower szUtf8Buf�еĵ�һ���ַ������Ǵ�д����任ΪСд����
    @return 0��ʾ��Ч���ַ������������0��ʾszLower�з��صĵ�һ���ַ��ĳ���
    */
    inline static  CWX_UINT32 lowerChr(char const* szUtf8Buf, CWX_UINT32 uiBufLen, char* szLower)
    {
        CWX_UINT32 uiNextLen = nextChrLen(szUtf8Buf, uiBufLen);
        if (1 == uiNextLen)
        {
            if ((szUtf8Buf[0]>='A') && (szUtf8Buf[0]<='Z'))
            {
                szLower[0] = szUtf8Buf[0] + ('a' - 'A');
            }
            else
            {
                szLower[0] = szUtf8Buf[0];
            }
        }
        else if (uiNextLen)
        {
            memcpy(szLower, szUtf8Buf, uiNextLen);
        }
        return uiNextLen;
    }

    /**
    @brief ���szUtf8Buf�еĵ�һ���ַ����Ƿ�һ��UTF8�ַ���
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @return true���ǣ�false������
    */
    inline static  bool isCharset(char const* szUtf8Buf, CWX_UINT32 uiBufLen)
    {
        int ret = nextChrLen(szUtf8Buf, uiBufLen);
        return ret==0?false:true;
    }
    /**
    @brief ���szUtf8Buf���ַ����Ƿ�ΪUTF8�ַ���֧�ֵ��ַ���
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @return true����UTF8�ַ�����false������UTF8�ַ���
    */
    inline static bool isValid(char const * szUtf8Buf, CWX_UINT32 uiBufLen)
    {
        if(uiBufLen)
        {
            CWX_UINT32 uiPos = 0;
            CWX_UINT32 uiLen = 0;
            while(uiPos < uiBufLen)
            {
                uiLen = nextChrLen(szUtf8Buf+uiPos, uiBufLen - uiPos);
                if(0 == uiLen) return false;
                uiPos += uiLen;
            }
            return uiPos == uiBufLen?true:false;
        }
        return true;
    }
    /**
    @brief ��ȡszUtf8Buf�е��ַ�����
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @return szUtf8Buf���ַ��ĸ���
    */
    inline static CWX_UINT32 chrlen(char const* szUtf8Buf, CWX_UINT32 uiBufLen)
    {
        CWX_UINT32 pos =0;
        CWX_UINT32 count =0;
        CWX_UINT32 len =0;
        while (pos < uiBufLen)
        {
            len =nextChrLen(szUtf8Buf + pos, uiBufLen - pos);
            if (0 == len) return count;
            count ++;
            pos +=len;
        }
        return count;
    }

    /**
    @brief �ڲ����ַ��ضϵ�����£������szUtf8Buf�н�ȡuiFetchLen���ֽڵ��ֽڽ���λ�á�
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @param [in] uiFetchLen Ҫ��ȡ������ֽ�����
    @return ���ؽ�ȡ���ֽ�λ��
    */
    inline static  CWX_UINT32 getPosByByteLen(char const* szUtf8Buf, CWX_UINT32 uiBufLen, CWX_UINT32 uiFetchLen)
    {
        CWX_UINT32 pos =0;
        CWX_UINT32 len =0;
        while(1)
        {
            len =nextChrLen(szUtf8Buf + pos, uiBufLen - pos);
            if (!len || (pos + len > uiBufLen) || (pos + len > uiFetchLen)) break;
            pos +=len;
        }
        return pos;
    }

    /**
    @brief �ڲ����ַ��ضϵ�����£������szUtf8Buf�н�ȡuiFetchNum���ַ����ֽڽ���λ�á�
    @param [in] szUtf8Buf �ַ�����buffer
    @param [in] uiBufLen szUtf8Buf���ֽڳ���
    @param [in] uiFetchNum Ҫ��ȡ������ַ�����
    @return ���ؽ�ȡ���ֽ�λ��
    */
    inline static  CWX_UINT32 getPosByChrLen(char const* szUtf8Buf, CWX_UINT32 uiBufLen, CWX_UINT32 uiFetchNum)
    {
        CWX_UINT32 pos =0;
        CWX_UINT32 len =0;
        CWX_UINT32 num =0;
        while(1)
        {
            len =nextChrLen(szUtf8Buf + pos, uiBufLen - pos);
            if (!len || (pos + len > uiBufLen) || (num + 1 > uiFetchNum)) break;
            num ++;
            pos +=len;
        }
        return pos;
    }
private:
    ///˽�л����캯������ֹʵ����
    CwxCharsetUtf8(){}
    ///˽�л�������������ֹʵ����
    ~CwxCharsetUtf8(){}

};

/**
@class CwxCharsetUtf16
@brief UTF16�ַ����Ľӿڶ��壬ȫ��Ϊ��̬�ӿڣ���ֹʵ����
*/
class CWX_API CwxCharsetUtf16
{
public:
    /**
    @brief ��ȡ������������ַ��������֡�
    @return ���ء�utf-16�����ַ���
    */
    inline static char const* charset()
    { 
        return "utf-16";
    }
    /**
    @brief �ж�һ���ַ����������Ƿ����CwxCharsetUtf8��Ľӿ�������
    @param [in] szCharset �������ַ���������
    @return ����"utf-16"������true�����򷵻�false
    */
    inline static bool charset(char const* szCharset)
    {
        return strcasecmp("utf-16", szCharset)==0?true:false;
    }
    /**
    @brief �ж�szUtf16Buf�еĵ�һ���ַ����ֽ�����0��ʾ��Ч���ַ��������
    @param [in] szUtf16Buf Ҫ�����ַ���buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @return 0��ʾ��Ч���ַ������������0��ʾszUtf16Buf�е�һ���ַ����ֽڳ���
    */
    inline static  CWX_UINT32 nextChrLen(char const* szUtf16Buf, CWX_UINT32 uiBufLen)
    {
        return szUtf16Buf?(uiBufLen>=2?2:0):0;
    }
    /**
    @brief ��szUtf16Buf�еĵ�һ���ַ�ͨ��szUpper���ⲿBUF���أ�����Сд���Ϊ��д��
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @param [out] szUpper szUtf16Buf�еĵ�һ���ַ�������Сд����任Ϊ��д����
    @return 0��ʾ��Ч���ַ������������0��ʾszUpper�з��صĵ�һ���ַ��ĳ���
    */
    inline static  CWX_UINT32 upperChr(char const* szUtf16Buf, CWX_UINT32 uiBufLen, char* szUpper)
    {
        CWX_UINT32 uiNextLen = nextChrLen(szUtf16Buf, uiBufLen);
        if (uiNextLen)
        {
            if (szUtf16Buf[0])
            {
                szUpper[0] = 0;
                if ((szUtf16Buf[1]>='a') && (szUtf16Buf[1]<='z'))
                {
                    szUpper[1] = szUtf16Buf[1] - ('a' - 'A');
                }
                else
                {
                    szUpper[1] = szUtf16Buf[1];
                }
            }
            else
            {
                szUpper[0] = szUtf16Buf[0];
                szUpper[1] = szUtf16Buf[1];
            }
        }
        return uiNextLen;
    }

    /**
    @brief ��szUtf16Buf�еĵ�һ���ַ�ͨ��szLower���ⲿBUF���أ����Ǵ�д���ΪСд��
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @param [out] szLower szUtf16Buf�еĵ�һ���ַ������Ǵ�д����任ΪСд����
    @return 0��ʾ��Ч���ַ������������0��ʾszLower�з��صĵ�һ���ַ��ĳ���
    */
    inline static  CWX_UINT32 lowerChr(char const* szUtf16Buf, CWX_UINT32 uiBufLen, char* szLower)
    {
        CWX_UINT32 uiNextLen = nextChrLen(szUtf16Buf, uiBufLen);
        if (uiNextLen)
        {
            if (szUtf16Buf[0])
            {
                szLower[0] = 0;
                if ((szUtf16Buf[1]>='A') && (szUtf16Buf[1]<='Z'))
                {
                    szLower[1] = szUtf16Buf[1] + ('a' - 'A');
                }
                else
                {
                    szLower[1] = szUtf16Buf[1];
                }
            }
            else
            {
                szLower[0] = szUtf16Buf[0];
                szLower[1] = szUtf16Buf[1];
            }
        }
        return uiNextLen;
    }
    /**
    @brief ���szUtf16Buf�еĵ�һ���ַ����Ƿ�һ��szUtf16Buf�ַ���
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @return true���ǣ�false������
    */
    inline static  bool isCharset(char const* szUtf16Buf, CWX_UINT32 uiBufLen)
    {
        return szUtf16Buf?(uiBufLen>=2?true:false):false;
    }
    /**
    @brief ���szUtf16Buf���ַ����Ƿ�ΪUTF16�ַ���֧�ֵ��ַ���
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @return true����UTF16�ַ�����false������UTF16�ַ���
    */
    inline static  bool isValid(char const * szUtf16Buf, CWX_UINT32 uiBufLen)
    {
        return szUtf16Buf?(uiBufLen%2?false:true):true;
    }
    /**
    @brief ��ȡszUtf16Buf�е��ַ�����
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @return szUtf16Buf���ַ��ĸ���
    */
    inline static CWX_UINT32 chrlen(char const* szUtf16Buf, CWX_UINT32 uiBufLen)
    {
        if (szUtf16Buf) return uiBufLen/2;
        return 0;
    }
    /**
    @brief �ڲ����ַ��ضϵ�����£������szUtf16Buf�н�ȡuiFetchLen���ֽڵ��ֽڽ���λ�á�
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @param [in] uiFetchLen Ҫ��ȡ������ֽ�����
    @return ���ؽ�ȡ���ֽ�λ��
    */
    inline static  CWX_UINT32 getPosByByteLen(char const* szUtf16Buf, CWX_UINT32 uiBufLen, CWX_UINT32 uiFetchLen)
    {
        if (szUtf16Buf) return uiBufLen>uiFetchLen?uiFetchLen:uiBufLen;
        return 0;
    }

    /**
    @brief �ڲ����ַ��ضϵ�����£������szUtf16Buf�н�ȡuiFetchNum���ַ����ֽڽ���λ�á�
    @param [in] szUtf16Buf �ַ�����buffer
    @param [in] uiBufLen szUtf16Buf���ֽڳ���
    @param [in] uiFetchNum Ҫ��ȡ������ַ�����
    @return ���ؽ�ȡ���ֽ�λ��
    */
    inline static  CWX_UINT32 getPosByChrLen(char const* szUtf16Buf, CWX_UINT32 uiBufLen, CWX_UINT32 uiFetchNum)
    {
        if (szUtf16Buf) return uiBufLen>uiFetchNum*2?uiFetchNum*2:uiBufLen;
        return 0;
    }
private:
    ///˽�л����캯������ֹʵ����
    CwxCharsetUtf16() {}
    ///˽�л�������������ֹʵ����
    ~CwxCharsetUtf16() {}
};

CWINUX_END_NAMESPACE

#include "CwxPost.h"

#endif

