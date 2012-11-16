#include "XmlParse.h"

XmlParser::XmlParser(CWX_UINT32 uiBufSize) 
{
    /* Initialize the validity of this parser to OFF */
    m_isReady = false;
    m_expatParser = NULL;
    /* Allocate a buffer for streaming in data */
    m_uiBufSize = uiBufSize;
    m_szGbkBuf = NULL;
    m_uiGbkBufLen = 0;

    /* Allocate a buffer for streaming in data */
    m_szXmlBuf = new XML_Char[m_uiBufSize];
    memset(m_szXmlBuf, 0, m_uiBufSize);

    m_szXmlPath = new XML_Char[PATH_BUF_ALIGN];
    m_uiPathBufLen = PATH_BUF_ALIGN;
    m_uiPathLen = 0;

}

XmlParser::~XmlParser(void) 
{
    m_isReady = false;
    if(m_expatParser)  XML_ParserFree(m_expatParser);
    m_expatParser = NULL;

    if(m_szXmlBuf) delete [] m_szXmlBuf;
    m_szXmlBuf = NULL;

    if (m_szXmlPath) delete [] m_szXmlPath;
    m_szXmlPath = NULL;

    if (m_szGbkBuf) delete [] m_szGbkBuf;
    m_szGbkBuf = NULL;
}


bool XmlParser::parse()
{
    ssize_t bytes_read;
    /* Ensure that the parser is ready */
    if(!prepare()) return false;
    m_bGbk = false;
    /* Loop, reading the XML source block by block */
    while((bytes_read = readBlock()) >= 0)
    {
        if(bytes_read > 0)
        {
            XML_Status local_status =  XML_Parse(m_expatParser, getBuf(), bytes_read, XML_FALSE);
            if(local_status != XML_STATUS_OK)
            {
                m_status = local_status;
                m_lastError = XML_GetErrorCode(m_expatParser);
                break;
            }
            /* Break on successful "short read", in event of EOF */
            if(getLastError() == XML_ERROR_FINISHED) break;
        }
    }
    /* Finalize the parser */
    if((getStatus() == XML_STATUS_OK) || (getLastError() == XML_ERROR_FINISHED)) 
    {
        XML_Parse(m_expatParser, NULL, 0, XML_TRUE);
        return true;
    }
    /* Return false in the event of an error. The parser is not finalized on error. */
    return false;
}

bool XmlParser::prepare()
{
    /* Initialize the validity of this parser to OFF */
    m_isReady = false;
    if (m_expatParser)
    {
        XML_ParserFree(m_expatParser);
        m_expatParser = NULL;
    }

    /* Allocate a new parser state-object */
    m_expatParser = XML_ParserCreate(NULL);
    /* Set the "ready" flag on this parser */
    m_isReady = true;
    XML_SetUserData(m_expatParser, (void*)this);
    regDefHandlers();

    return true;
}

char const* XmlParser::charsetValue(XML_Char const* value, CWX_UINT32 uiValueLen)
{
    if (uiValueLen >= m_uiGbkBufLen)
    {
        if (m_szGbkBuf) delete [] m_szGbkBuf;
        m_uiGbkBufLen = (uiValueLen + 1024)/1024;
        m_uiGbkBufLen *= 1024;
        m_szGbkBuf = new char[m_uiGbkBufLen];
    }
    if (!m_bGbk)
    {
        memcpy(m_szGbkBuf, value, uiValueLen);
        m_szGbkBuf[uiValueLen] = 0x00;
    }
    else
    {
        CWX_UINT32 uiGbkLen = m_uiGbkBufLen;
        CwxGbkUnicodeMap::utf8ToGbk(value, uiValueLen, m_szGbkBuf, uiGbkLen);
        m_szGbkBuf[uiGbkLen] = 0x00;
    }
    return m_szGbkBuf;
}


void XmlParser::startElement(const XML_Char *, const XML_Char **)
{
}

void XmlParser::endElement(const XML_Char *) 
{
}

void XmlParser::characterData(const XML_Char *, int ) 
{
}

void XmlParser::processingInstruction(const XML_Char *, const XML_Char *)
{
}

void XmlParser::commentData(const XML_Char *) 
{
}

void XmlParser::defaultHandler(const XML_Char *, int )
{
}

void XmlParser::startCData(void)
{
}

void XmlParser::endCData(void)
{
}

/* 
This function causes Expat to register this's default static handlers
with the Expat events.
*/
void XmlParser::regDefHandlers(void)
{
    XML_SetElementHandler(m_expatParser,
        &XmlParser::elementStartHandler,
        &XmlParser::elementEndHandler);
    XML_SetCharacterDataHandler(m_expatParser,
        &XmlParser::characterDataHandler);
    XML_SetProcessingInstructionHandler(m_expatParser,
        &XmlParser::processingInstrHandler);
    XML_SetCommentHandler(m_expatParser, &XmlParser::commentHandler);
    XML_SetCdataSectionHandler(m_expatParser,
        &XmlParser::startCDatahandler,
        &XmlParser::endCDatahandler);
    XML_SetDefaultHandler(m_expatParser, &XmlParser::defaultHandler);
    XML_SetUnknownEncodingHandler(m_expatParser, &XmlParser::encodingHandler, this);
}

ssize_t XmlParser::readBlock(void)
{
    m_lastError = XML_ERROR_NO_ELEMENTS;
    m_status = XML_STATUS_ERROR;
    return -1;
}

/* 
**** INTERNAL HANDLER FUNCTIONS *****
The expatmm protocol is to pass (this) as the userData argument
in the XML_Parser structure. These static methods will convert 
handlers into upcalls to the instantiated class's virtual members
to do the actual handling work.
*/
void XMLCALL XmlParser::elementStartHandler(void *userData, const XML_Char *name,
                                               const XML_Char **atts)
{
    XmlParser *me = (XmlParser*)userData;
    CWX_UINT32 uiNameLen = strlen(name);
    if(me != NULL)
    {
        if (me->m_uiPathLen + uiNameLen + 1 >= me->m_uiPathBufLen)
        {
            me->m_uiPathBufLen = ((me->m_uiPathBufLen + uiNameLen + PATH_BUF_ALIGN -1)/PATH_BUF_ALIGN)*PATH_BUF_ALIGN * 2;
            XML_Char* buf = new XML_Char[me->m_uiPathBufLen];
            memcpy(buf, me->m_szXmlPath, me->m_uiPathLen);
            delete [] me->m_szXmlPath;
            me->m_szXmlPath = buf;
        }
        if (me->m_uiPathLen)
        {
            me->m_szXmlPath[me->m_uiPathLen]=':';
            me->m_uiPathLen++;
        }
        memcpy(me->m_szXmlPath + me->m_uiPathLen, name, uiNameLen);
        me->m_uiPathLen += uiNameLen;
        me->m_szXmlPath[me->m_uiPathLen] = 0x00;
        me->startElement(name, atts);
    }
}

void XMLCALL XmlParser::elementEndHandler(void *userData, const XML_Char *name)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL)
    {
        CWX_UINT32 i =0;
        for (i=me->m_uiPathLen; i>0; i--)
        {
            if (me->m_szXmlPath[i-1] == ':')
            {
                i--;
                break;
            }
        }
        me->m_szXmlPath[i] = 0x00;
        me->m_uiPathLen = i;
        me->endElement(name);
    }
}

void XMLCALL XmlParser::characterDataHandler(void *userData, const XML_Char *s, int len)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL) me->characterData(s, len);
}

void XMLCALL XmlParser::processingInstrHandler(void *userData, const XML_Char *target, const XML_Char *data)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL) me->processingInstruction(target, data);
}

void XMLCALL XmlParser::commentHandler(void *userData, const XML_Char *data)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL) me->commentData(data);
}

void XMLCALL XmlParser::defaultHandler(void *userData, const XML_Char *s, int len)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL) me->defaultHandler(s, len);
}

void XMLCALL XmlParser::startCDatahandler(void *userData)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL) me->startCData();
}

void XMLCALL XmlParser::endCDatahandler(void *userData)
{
    XmlParser *me = (XmlParser*)userData;
    if(me != NULL) me->endCData();
}

int XMLCALL XmlParser::convert(void* , char const* s)
{
    CWX_UINT16 unGbk = s[0];
    unGbk <<= 8;
    unGbk += (CWX_UINT8)s[1];
    return CwxGbkUnicodeMap::gbkToUtf16(unGbk);
}
int  XMLCALL XmlParser::encodingHandler(void* userData, XML_Char const* name, XML_Encoding* info)
{
    int i;
    if (!name) return XML_STATUS_ERROR;
    if((0==strcasecmp(name,"gb2312")) || (0 == strcasecmp(name,"gbk")))
    {
        XmlParser *me = (XmlParser*)userData;
        me->m_bGbk = true;
        for(i=0;i<128;i++) info->map[i]	= i;
        for(;i<256;i++) info->map[i]= -2;
        info->convert = convert;
        info->release = NULL;
        return XML_STATUS_OK;
    }
    return XML_STATUS_ERROR;
}

XmlFileParser::XmlFileParser(string const& strFileName)
{
    m_fd = NULL;
    m_strFileName = strFileName;
}

XmlFileParser::~XmlFileParser() 
{
    if (m_fd) fclose(m_fd);
    m_fd = NULL;
}

bool XmlFileParser::prepare()
{
    if (!XmlParser::prepare()) return false;
    if (m_fd) fclose(m_fd);
    m_fd = fopen(m_strFileName.c_str(), "r");
    if (!m_fd)
    {
        setReady(false);
        return false;
    }
    return true;
}

ssize_t XmlFileParser::readBlock(void)
{
    if (!isReady()) return -1;
    size_t size = fread(getBuf(), sizeof(XML_Char), getBufSize(), m_fd);
    ssize_t code = (ssize_t)size;
    if(size < getBufSize())
    {
        if(feof(m_fd))
        {
            setLastError(XML_ERROR_FINISHED);
            return size;
        }
        if(ferror(m_fd))
        {
            setStatus(XML_STATUS_ERROR);
            setLastError(XML_ERROR_NO_ELEMENTS);
        }
    }
    if(size == 0)
    {
        setStatus(XML_STATUS_OK);
        setLastError(XML_ERROR_FINISHED);
        code = -1;
    }
    return code;
}

XmlConfigParser::XmlConfigParser(CWX_UINT32 uiAvgTokenLen, CWX_UINT32 uiAvgXmlSize)
:m_memPool(uiAvgTokenLen*4, uiAvgXmlSize) 
{
    m_root = NULL;
    m_pCur = NULL;
    m_szGbkBuf = NULL;
    m_uiGbkBufLen = 0;
    m_expatParser = NULL;
}

XmlConfigParser::~XmlConfigParser(void)
{
    if(m_expatParser)  XML_ParserFree(m_expatParser);
    m_expatParser = NULL;
    if (m_root) delete m_root;
    m_root = NULL;
    if (m_szGbkBuf) delete [] m_szGbkBuf;
    m_szGbkBuf = NULL;
}

bool XmlConfigParser::parse(char const* szXml)
{
    /* Allocate a new parser state-object */
    if (m_expatParser) XML_ParserFree(m_expatParser);
    m_expatParser = XML_ParserCreate(NULL);
    XML_SetUserData(m_expatParser, (void*)this);
    regDefHandlers();

    if (m_root) delete m_root;
    m_root = NULL;
    m_pCur = NULL;
    m_memPool.reset();
    m_bGbk = false;
    //parse
    if (szXml)
    {
        XML_Status local_status =  XML_Parse(m_expatParser, szXml, strlen(szXml), XML_TRUE);
        if(local_status != XML_STATUS_OK)
        {
            return false;
        }
    }
    return true;
}

///get a attribute value for a element specified by szPath which's format is e1:e2:...
char const* XmlConfigParser::getElementAttr(char const* szPath, char const* szAttr) const
{
    XmlTreeNode const * pNode = getElementNode(szPath);
    if (pNode && szAttr)
    {
        list<pair<char*, char*> >::const_iterator iter = pNode->m_lsAttrs.begin();
        while(iter != pNode->m_lsAttrs.end())
        {
            if (strcmp(szAttr, iter->first) == 0)
            {
                return iter->second;
            }
            iter++;
        }
    }
    return NULL;

}
bool XmlConfigParser::getElementAttrs(char const* szPath, list<pair<char*, char*> >& attrs) const
{
    XmlTreeNode const * pNode = getElementNode(szPath);
    attrs.clear();
    if (pNode)
    {
        list<pair<char*, char*> >::const_iterator iter = pNode->m_lsAttrs.begin();
        while(iter != pNode->m_lsAttrs.end())
        {
            attrs.push_front(*iter);
            iter++;
        }
        return true;
    }
    return false;
}

bool XmlConfigParser::getElementData(char const* szPath, string& strData) const
{
    XmlTreeNode const * pNode = getElementNode(szPath);
    if (pNode && !pNode->m_listData.empty() && pNode->m_lsAttrs.empty() && !pNode->m_pChildHead)
    {
        list<char*> ::const_iterator iter = pNode->m_listData.begin();
        strData.clear();
        while(iter != pNode->m_listData.end())
        {
            strData += *iter;
            iter++;
        }
        return true;
    }
    return false;
}
///get a path's element node, path's format is e1:e2:...
XmlTreeNode const* XmlConfigParser::getElementNode(char const* szPath) const
{
    char const *elem_name_start = szPath;
    char const *elem_name_end = NULL;
    CWX_UINT32 uiLen = 0;
    XmlTreeNode* pNode = m_root;
    while (elem_name_start && pNode)
    {
        elem_name_end = strchr(elem_name_start, ':');
        if ( elem_name_end == NULL)
        {
            uiLen = strlen(elem_name_start);
        }
        else
        {
            uiLen = elem_name_end - elem_name_start;
            elem_name_end++;
        }
        while(pNode)
        {
            if ((strlen(pNode->m_szElement) != uiLen)||
                (0 != memcmp(elem_name_start, pNode->m_szElement, uiLen)))
            {
                pNode = pNode->m_next;
            }
            else
            {
                break;
            }
        }
        if (!pNode) return NULL;
        elem_name_start = elem_name_end;
        if (elem_name_start) pNode = pNode->m_pChildHead;
    }
    return pNode;

}


void XmlConfigParser::startElement(const XML_Char *name, const XML_Char **atts) 
{
    CWX_UINT32 uiAttrNum = 0;
    CWX_UINT32 uiLen = 0;
    pair<char*, char*> item;
    XmlTreeNode* pNode = new XmlTreeNode();
    char const* pName = NULL;
    if (m_bGbk)
    {
        pName = charsetValue(name, strlen(name));
    }
    else
    {
        pName = name;
    }
    uiLen = strlen(pName);
    pNode->m_szElement = m_memPool.malloc(uiLen + 1);
    memcpy(pNode->m_szElement, pName, uiLen);
    pNode->m_szElement[uiLen] = 0x00;
    while(atts[uiAttrNum*2])
    {
        if (m_bGbk)
        {
            pName = charsetValue(atts[uiAttrNum*2], strlen(atts[uiAttrNum*2]));
        }
        else
        {
            pName = atts[uiAttrNum*2];
        }
        uiLen = strlen(pName);
        item.first = m_memPool.malloc(uiLen + 1);
        memcpy(item.first, pName, uiLen);
        item.first[uiLen] = 0x00;
        if (m_bGbk)
        {
            pName = charsetValue(atts[uiAttrNum*2 + 1], strlen(atts[uiAttrNum*2 + 1]));
        }
        else
        {
            pName = atts[uiAttrNum*2 + 1];
        }
        uiLen = strlen(pName);
        item.second = m_memPool.malloc(uiLen + 1);
        memcpy(item.second, pName, uiLen);
        item.second[uiLen] = 0x00;
        pNode->m_lsAttrs.push_back(item);
        uiAttrNum++;
    }
    if (!m_pCur)
    {
        m_root = pNode;
        m_pCur = pNode;
    }
    else
    {
        pNode->m_pParent = m_pCur;
        if (!m_pCur->m_pChildTail)
        {
            m_pCur->m_pChildTail = m_pCur->m_pChildHead = pNode;
        }
        else
        {//add to tail for child
            m_pCur->m_pChildTail->m_next = pNode;
            pNode->m_prev = m_pCur->m_pChildTail;
            m_pCur->m_pChildTail = pNode;
        }
    }
    m_pCur = pNode;
}

void XmlConfigParser::endElement(const XML_Char* )
{
    m_pCur = m_pCur->m_pParent;
}

void XmlConfigParser::characterData(const XML_Char *s, int len)
{
    char const* pName = NULL;
    CWX_UINT32 uiLen = len;
    if (isGbk())
    {
        pName = charsetValue(s, len);
        uiLen = strlen(pName);
    }
    else
    {
        pName = s;
    }
    char* pData = m_memPool.malloc(uiLen + 1);
    memcpy(pData, pName, uiLen);
    pData[uiLen] = 0x00;
    m_pCur->m_listData.push_back(pData);
}

char const* XmlConfigParser::charsetValue(XML_Char const* value, CWX_UINT32 uiValueLen)
{
    if (uiValueLen >= m_uiGbkBufLen)
    {
        if (m_szGbkBuf) delete [] m_szGbkBuf;
        m_uiGbkBufLen = (uiValueLen + 1024)/1024;
        m_uiGbkBufLen *= 1024;
        m_szGbkBuf = new char[m_uiGbkBufLen];
    }
    if (!m_bGbk)
    {
        memcpy(m_szGbkBuf, value, uiValueLen);
        m_szGbkBuf[uiValueLen] = 0x00;
    }
    else
    {
        CWX_UINT32 uiGbkLen = m_uiGbkBufLen;
        CwxGbkUnicodeMap::utf8ToGbk(value, uiValueLen, m_szGbkBuf, uiGbkLen);
        m_szGbkBuf[uiGbkLen] = 0x00;
    }
    return m_szGbkBuf;
}

/* 
This function causes Expat to register this's default static handlers
with the Expat events.
*/
void XmlConfigParser::regDefHandlers(void)
{
    XML_SetElementHandler(m_expatParser,
        &XmlConfigParser::elementStartHandler,
        &XmlConfigParser::elementEndHandler);
    XML_SetCharacterDataHandler(m_expatParser,
        &XmlConfigParser::characterDataHandler);
    XML_SetUnknownEncodingHandler(m_expatParser, &XmlConfigParser::encodingHandler, this);
}


void XMLCALL XmlConfigParser::elementStartHandler(void *userData, const XML_Char *name,
                                                     const XML_Char **atts)
{
    XmlConfigParser *me = (XmlConfigParser*)userData;
    if(me != NULL)
    {
        me->startElement(name, atts);
    }
}

void XMLCALL XmlConfigParser::elementEndHandler(void *userData, const XML_Char *name)
{
    XmlConfigParser *me = (XmlConfigParser*)userData;
    if(me != NULL)
    {
        me->endElement(name);
    }
}

void XMLCALL XmlConfigParser::characterDataHandler(void *userData, const XML_Char *s, int len)
{
    XmlConfigParser *me = (XmlConfigParser*)userData;
    if(me != NULL) me->characterData(s, len);
}

int XMLCALL XmlConfigParser::convert(void* , char const* s)
{
    CWX_UINT16 unGbk = s[0];
    unGbk <<= 8;
    unGbk += (CWX_UINT8)s[1];
    return CwxGbkUnicodeMap::gbkToUtf16(unGbk);
}
int  XMLCALL XmlConfigParser::encodingHandler(void* userData, XML_Char const* name, XML_Encoding* info)
{
    int i;
    if (!name) return XML_STATUS_ERROR;
    if((0==strcasecmp(name,"gb2312")) || (0 == strcasecmp(name,"gbk")))
    {
        XmlConfigParser *me = (XmlConfigParser*)userData;
        me->m_bGbk = true;
        for(i=0;i<128;i++) info->map[i]	= i;
        for(;i<256;i++) info->map[i]= -2;
        info->convert = convert;
        info->release = NULL;
        return XML_STATUS_OK;
    }
    return XML_STATUS_ERROR;
}

XmlFileConfigParser::XmlFileConfigParser(CWX_UINT32 uiAvgTokenLen, CWX_UINT32 uiAvgXmlSize)
:m_parser(uiAvgTokenLen, uiAvgXmlSize)
{
    m_fd = NULL;
    m_szBuf = NULL;
}

XmlFileConfigParser::~XmlFileConfigParser(void) 
{
    if (m_fd) fclose(m_fd);
    if (m_szBuf) delete []m_szBuf;
}

///Attempts to parse an entire XML source
bool XmlFileConfigParser::parse(string const& strFileName)
{
    m_strFileName = strFileName;
    if (m_fd) fclose(m_fd);
    m_fd = fopen(m_strFileName.c_str(), "r");
    if (!m_fd) return false;
    fseek(m_fd, 0, SEEK_END);
    CWX_UINT32 uiFileSize = ftell(m_fd);
    if (m_szBuf) delete []m_szBuf;
    m_szBuf = new char[uiFileSize + 1];
    fseek(m_fd, 0, SEEK_SET);
    if (uiFileSize != fread(m_szBuf, 1, uiFileSize, m_fd))
    {
        delete []m_szBuf;
        return false;
    }
    m_szBuf[uiFileSize] = 0x00;
    bool bResult = m_parser.parse(m_szBuf);
    delete [] m_szBuf;
    m_szBuf = NULL;
    return bResult;
}


bool CwxXmlPackageConv::xmlToPackage(char const * szSrc,
                                     char* szOut,
                                     CWX_UINT32& uiOutLen)
{
    XmlConfigParser parser;
    if (!parser.parse(szSrc))
    {
        CwxCommon::snprintf(this->m_szErrMsg, 511, "%s", "Failure to parse xml");
        return false;
    }
    XmlTreeNode const* pRoot = parser.getRoot();
    if (pRoot)
    {
        return xmlToPackage(pRoot, szOut, uiOutLen);
    }
    else
    {
        strcpy(this->m_szErrMsg, "Failure to parse xml.");
        return false;
    }
    return true;
}

bool CwxXmlPackageConv::packageToXml(char const* szRootName,
                                     char const * szSrc,
                                     CWX_UINT32 uiSrcLen,
                                     char* szOut,
                                     CWX_UINT32& uiOutLen,
                                     char const* szXmlTitile)
{
    CwxPackageReaderEx package;
    if (!package.unpack(szSrc, uiSrcLen, false, true))
    {
        CwxCommon::snprintf(m_szErrMsg, 511, "Failure to unpack msg, err=%s", package.getErrMsg());
        return false;
    }
    return packageToXml(szRootName, package, szOut, uiOutLen, szXmlTitile);
}

bool CwxXmlPackageConv::packageToXml(char const* szRootName,
                                     CwxPackageReaderEx& package,
                                     char* szOut,
                                     CWX_UINT32& uiOutLen,
                                     char const* szXmlTitile)
{
    CWX_UINT32 pos = 0;
    CWX_UINT32 uiTmp = 0;

    memset(szOut, 0x00, uiOutLen);

    if (!szXmlTitile)
    {
        if (m_xmlEncode->isGbk())
        {
            szXmlTitile = "<?xml version='1.0' encoding=\"gbk\" ?>";
        }
        else
        {
            szXmlTitile = "<?xml version='1.0' encoding=\"utf-8\" ?>";
        }
    }
    uiTmp = strlen(szXmlTitile);
    if (pos + uiTmp >= uiOutLen)
    {
        strcpy(this->m_szErrMsg, "Buf is too less.");
        return false;
    }
    memcpy(szOut + pos, szXmlTitile, uiTmp);
    pos += uiTmp;
    uiTmp = uiOutLen - pos;
    if (!packageToXmlNode(szRootName, package, szOut + pos, uiTmp)) return false;
    pos += uiTmp;
    uiOutLen = pos;
    szOut[uiOutLen] = 0x00;
    return true;
}

bool CwxXmlPackageConv::packageToXmlNode(char const* szNodeName,
                                         char const * szSrc,
                                         CWX_UINT32 uiSrcLen,
                                         char* szOut,
                                         CWX_UINT32& uiOutLen)
{
    CwxPackageReaderEx package;
    if (!package.unpack(szSrc, uiSrcLen, false, true))
    {
        CwxCommon::snprintf(m_szErrMsg, 511, "Failure to unpack msg, err=%s", package.getErrMsg());
        return false;
    }
    return packageToXmlNode(szNodeName, package, szOut, uiOutLen);
}

bool CwxXmlPackageConv::packageToXmlNode(char const* szNodeName,
                                         CwxPackageReaderEx& package,
                                         char* szOut,
                                         CWX_UINT32& uiOutLen)
{
    CwxKeyValueItemEx const* pItem;
    CWX_UINT32 pos = 0;
    CWX_UINT32 uiTmp = 0;
    memset(szOut, 0x00, uiOutLen);
    uiTmp = uiOutLen - pos;
    if (szNodeName)
    {
        if (!appendXmlKeyBegin(szNodeName, strlen(szNodeName), szOut + pos, uiTmp)) return false;
        pos += uiTmp;
    }
    for (CWX_UINT32 i=0; i<package.getKeyNum(); i++)
    {
        pItem = package.getKey(i);
        uiTmp = uiOutLen - pos;
        if (!packageToXml(*pItem, szOut + pos, uiTmp)) return false;
        pos += uiTmp;
    }
    if (szNodeName)
    {
        uiTmp = uiOutLen - pos;
        if (!appendXmlKeyEnd(szNodeName, strlen(szNodeName), szOut + pos, uiTmp)) return false;
        pos += uiTmp;
    }
    uiOutLen = pos;
    szOut[uiOutLen] = 0x00;
    return true;
}


///xml转换为package
bool CwxXmlPackageConv::xmlToPackage(XmlTreeNode const * treeNode, char* szOut, CWX_UINT32& uiOutLen)
{
    CWX_UINT32 pos = 0;
    CWX_UINT16 key_len=0;
    CWX_UINT32 data_len=0;
    CWX_UINT32 total_data_len=0;
    CWX_UINT32 byte4;
    char* pKeyValue=NULL;
    int       ret;
    list<char*>::const_iterator text_iter;
    list<pair<char*, char*> >::const_iterator attr_iter;
    XmlTreeNode const* child;
    bool bKeyValue = false;
    if (!treeNode)
    {
        uiOutLen = 0;
        return true;
    }
    //add key
    key_len = strlen(treeNode->m_szElement);
    if (uiOutLen < pos + CwxPackageEx::getKvLen(key_len, 0))
    {
        strcpy(this->m_szErrMsg, "Package's buf is too small.");
        return false;
    }
    CwxPackageEx::encodeUint16(key_len, (unsigned char*)(szOut + pos), data_len);
    pos += data_len;
    //append the key/value sign
    pKeyValue = szOut + pos;
    pos++;
    //append key with null
    memcpy(szOut + pos, treeNode->m_szElement, key_len + 1);
    pos += key_len + 1;

    if ((treeNode->m_listData.begin() != treeNode->m_listData.end()) &&
        (treeNode->m_lsAttrs.begin() == treeNode->m_lsAttrs.end()) &&
        !treeNode->m_pChildHead)
    {//it's [<name>xxxxx</name>]'s item
        bKeyValue = false;
        text_iter = treeNode->m_listData.begin();
        while(text_iter != treeNode->m_listData.end())
        {
            data_len = strlen(*text_iter);
            if (uiOutLen < pos + data_len + 1)
            {
                strcpy(this->m_szErrMsg, "Buf is too small.");
                return false;
            }
            memcpy(szOut + pos, *text_iter, data_len);
            pos += data_len;
            total_data_len += data_len;
            text_iter++;
        }
        szOut[pos++]=0x00;//append the \0
    }
    else
    {
        bKeyValue = true;
        //add attr
        attr_iter = treeNode->m_lsAttrs.begin();
        while(attr_iter != treeNode->m_lsAttrs.end())
        {
            ret = CwxPackageEx::appendKey(szOut + pos, uiOutLen-pos, attr_iter->first, strlen(attr_iter->first), attr_iter->second, strlen(attr_iter->second), false);
            if (-1 == ret)
            {
                strcpy(this->m_szErrMsg, "Buf is too small.");
                return false;
            }
            pos += ret;
            total_data_len += ret;
            attr_iter ++;
        }
        //add child
        child = treeNode->m_pChildHead;
        if (child)
        {
            while(child)
            {
                data_len = uiOutLen - pos;
                if (!xmlToPackage(child, szOut + pos, data_len)) return false;
                pos += data_len;
                total_data_len += data_len;
                child = child->m_next;
            }
        }
        szOut[pos++] = 0x00;
    }
    *pKeyValue=bKeyValue?1:0;
    byte4 = CwxPackageEx::getKvLen(key_len, total_data_len);
    unsigned char szBuf[10];
    CwxPackageEx::encodeUint32(byte4, (unsigned char*)szBuf, data_len);
    memmove(szOut+data_len, szOut, pos);
    memcpy(szOut, szBuf, data_len);
    uiOutLen = pos + data_len;
    return true;
}

///package转换为xml
bool CwxXmlPackageConv::packageToXml(CwxKeyValueItemEx const& item, char* szOut, CWX_UINT32& uiOutLen)
{
    CWX_UINT32 pos = 0;
    CWX_UINT32 left = uiOutLen;
    CWX_UINT32 in_len = 0;
    CWX_UINT32 out_len = 0;
    CwxKeyValueItemEx const* pItem;
    CwxPackageReaderEx package;
    if (NULL == item.m_szData)
    {
        CwxCommon::snprintf(this->m_szErrMsg, 511, "key[%s]'s value is null.", item.m_szKey);
        return false;
    }
    if (!item.m_bKeyValue)
    {
        //add [<]
        out_len = left;
        if (!appendXmlKeyBegin(item.m_szKey, item.m_unKeyLen, szOut + pos, out_len)) return false;
        pos += out_len;
        left -= out_len;
        //append data
        in_len = item.m_uiDataLen;
        out_len = left;
        if (!m_xmlEncode->encode(item.m_szData, in_len, szOut + pos, out_len))
        {
            CwxCommon::snprintf(m_szErrMsg, 511, "Failure to xml-encode data:%s", item.m_szData);
            return false;
        }
        if (in_len != item.m_uiDataLen)
        {
            strcpy(this->m_szErrMsg, "Buff is to small.");
            return false;
        }
        pos+= out_len;
        left-=out_len;

        //add </key>\n
        out_len = left;
        if (!appendXmlKeyEnd(item.m_szKey, item.m_unKeyLen, szOut + pos, out_len)) return false;
        pos += out_len;
        left -= out_len;
        uiOutLen = pos;
        szOut[pos]=0x00;
        return true;
    }
    //it's key value
    if (!package.unpack(item.m_szData, item.m_uiDataLen, false, true))
    {
        CwxCommon::snprintf(m_szErrMsg, 511, "Failure to unpack key-value, key=%s, err=%s", item.m_szKey, package.getErrMsg());
        return false;
    }
    //add <item.key>
    out_len = left;
    if (!appendXmlKeyBegin(item.m_szKey, item.m_unKeyLen, szOut + pos, out_len)) return false;
    pos += out_len;
    left -= out_len;

    for (CWX_UINT32 i=0; i<package.getKeyNum(); i++)
    {
        pItem = package.getKey(i);
        out_len = left;
        if (!packageToXml(*pItem, szOut + pos, out_len)) return false;
        pos+=out_len;
        left-=out_len;
    }

    //add </item.key>
    out_len = left;
    if (!appendXmlKeyEnd(item.m_szKey, item.m_unKeyLen, szOut + pos, out_len)) return false;
    pos += out_len;
    left -= out_len;
    uiOutLen = pos;
    szOut[pos]=0x00;
    return true;
}

///append <key>
bool CwxXmlPackageConv::appendXmlKeyBegin(char const* szKey, CWX_UINT16 unKeyLen, char* szOut, CWX_UINT32& uiOutLen)
{
    CWX_UINT32 left = uiOutLen;
    CWX_UINT32 pos = 0;
    CWX_UINT32 in_len = 0;
    CWX_UINT32 out_len = 0;
    //add [<]
    if (left < 2)
    {
        strcpy(this->m_szErrMsg, "Buff is to small.");
        return false;
    }
    szOut[pos++]='<';
    left--;
    //add key
    in_len = unKeyLen;
    out_len = left;
    if (!m_xmlEncode->encode(szKey, in_len, szOut + pos, out_len))
    {
        CwxCommon::snprintf(m_szErrMsg, 511, "Failure to xml-encode key:%s", szKey);
        return false;
    }
    if (in_len != unKeyLen)
    {
        strcpy(this->m_szErrMsg, "Buff is to small.");
        return false;
    }
    pos+= out_len;
    left-=out_len;
    //append other '>'
    if (left < 2)
    {
        strcpy(this->m_szErrMsg, "Buff is to small.");
        return false;
    }
    szOut[pos++]='>';
    uiOutLen = pos;
    return true;
}
///append </key>
bool CwxXmlPackageConv::appendXmlKeyEnd(char const* szKey, CWX_UINT16 unKeyLen, char* szOut, CWX_UINT32& uiOutLen)
{
    CWX_UINT32 left = uiOutLen;
    CWX_UINT32 pos = 0;
    CWX_UINT32 in_len = 0;
    CWX_UINT32 out_len = 0;
    //add [</]
    if (left < 3)
    {
        strcpy(this->m_szErrMsg, "Buff is to small.");
        return false;
    }
    szOut[pos++]='<';
    left--;
    szOut[pos++]='/';
    left--;
    //add key
    in_len = unKeyLen;
    out_len = left;
    if (!m_xmlEncode->encode(szKey, in_len, szOut + pos, out_len))
    {
        CwxCommon::snprintf(m_szErrMsg, 511, "Failure to xml-encode key:%s", szKey);
        return false;
    }
    if (in_len != unKeyLen)
    {
        strcpy(this->m_szErrMsg, "Buff is to small.");
        return false;
    }
    pos+= out_len;
    left-=out_len;
    //append other '>\n'
    if (left < 3)
    {
        strcpy(this->m_szErrMsg, "Buff is to small.");
        return false;
    }
    szOut[pos++]='>';
    left--;
    szOut[pos++]='\n';
    left--;
    uiOutLen = pos;
    return true;
}


