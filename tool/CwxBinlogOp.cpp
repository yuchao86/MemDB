#include "CwxBinlogOp.h"
#include "CwxZlib.h"

CwxBinlogOp::CwxBinlogOp(){
    m_pCursor = NULL;
    m_pBuf = NULL;
    m_uiBufLen = 0;
    m_pUnBuf = NULL;
    m_uiUnBufLen = 0;
    m_pDumpBuf = NULL;
    m_uiDumpBufLen = 0;
    m_ullMinSid = 0;
    m_ullMaxSid = 0;
    m_uiRecNum = 0;
    m_szErr2K[0] = 0x00;
}

CwxBinlogOp::~CwxBinlogOp(){
    if (m_pCursor) delete m_pCursor;
    if (m_pBuf) delete [] m_pBuf;
    if (m_pUnBuf) delete [] m_pUnBuf;
    if (m_pDumpBuf) delete [] m_pDumpBuf;
}

int CwxBinlogOp::init(string const& strBinLogFileName){
    clear();
    m_strLogFileName = strBinLogFileName;

    m_pCursor = new CwxBinLogCursor();
    printf("Begin to load binlog file, please waiting......\n");
    if (0 != m_pCursor->open(strBinLogFileName.c_str(), 0, 0)){
        printf("Failure to load binlog file, err:%s\n", m_pCursor->getErrMsg());
        return -1;
    }
    //create index
    CWX_UINT32 uiOffset=0;
    CWX_UINT32 uiEndOffset=0;
    int iRet = 0;
    while(1 == (iRet=m_pCursor->next())){
        m_uiRecNum++;
        uiOffset = m_pCursor->getHeader().getOffset() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE + m_pCursor->getHeader().getLogLen();
        if (1 == m_uiRecNum){
            m_ullMinSid = m_pCursor->getHeader().getSid();
        }
        m_ullMaxSid = m_pCursor->getHeader().getSid();
        uiEndOffset = m_pCursor->getHeader().getOffset();
        if ((1==m_uiRecNum) || (0 == m_uiRecNum%BINLOG_INDEX_INTERNAL)){
            m_sidIndex[m_pCursor->getHeader().getSid()] = m_pCursor->getHeader().getOffset();
            m_recIndex[m_uiRecNum] = m_pCursor->getHeader().getOffset();
        }
    }
    //add the end record
    if (m_uiRecNum > 1){
        m_sidIndex[m_ullMaxSid] = uiEndOffset;
        m_recIndex[m_uiRecNum] = uiEndOffset;
    }


    if (0 != iRet){
        printf("Failure to read binlog(offset:%u, record:%u), err:%s\n", uiOffset, m_uiRecNum, m_pCursor->getErrMsg());
        return -1;
    }else{
        printf("Success to load binlog file\n");
    }
    m_pCursor->seek(0);
    return 0;
}

void CwxBinlogOp::run(){
    char szInput[1024];
    while(true){
        prompt();
        fgets(szInput, 1023, stdin);
        if (strlen(szInput) == 0) continue;
        szInput[strlen(szInput)-1] = 0x00;
        if (1 != doCommand(szInput)) break;
    }
}


void CwxBinlogOp::clear(){
    if (m_pCursor) delete m_pCursor;
    m_pCursor = NULL;
    if (m_pBuf) delete [] m_pBuf;
    m_pBuf = NULL;
    if (m_pUnBuf) delete [] m_pUnBuf;
    m_pUnBuf = NULL;
    if (m_pDumpBuf) delete [] m_pDumpBuf;
    m_pDumpBuf = NULL;
    m_uiBufLen = 0;
    m_uiUnBufLen = 0;
    m_uiDumpBufLen = 0;
    m_ullMinSid = 0;
    m_ullMaxSid = 0;
    m_uiRecNum = 0;
    m_szErr2K[0] = 0x00;
}

void CwxBinlogOp::prompt(){
    char szBuf[64];
    printf("%u/%s>",
        m_pCursor->getHeader().getLogNo(),
        CwxCommon::toString(m_pCursor->getHeader().getSid(), szBuf));
}

//0£ºexit£¬1£ºcontinue
int CwxBinlogOp::doCommand(char* szCmd){
    string strCmd;
    list<string> value;
    CwxCommon::trim(szCmd);
    strCmd = szCmd;
    CwxCommon::split(strCmd, value, ' ');
    list<string>::iterator iter = value.begin();
    //remove the empty value
    while(iter != value.end()){
        if ((*iter).length() == 0){
            value.erase(iter);
            iter = value.begin();
            continue;
        }
        iter++;
    }
    if (value.size() == 0) return 1;

    iter = value.begin();
    CWX_UINT32 uiItemNum = value.size();
    if(0 == strcasecmp((*iter).c_str(), "help")){
        doHelp();
    }else if(0 == strcasecmp((*iter).c_str(), "info")){
        doInfo();
    }else if(0 == strcasecmp((*iter).c_str(), "next")){
        CWX_UINT32 uiNum = 1;
        if (1 != uiItemNum){
            iter++;
            uiNum = strtoul((*iter).c_str(), NULL, 0);
        }
        doNext(uiNum);
    }else if(0 == strcasecmp((*iter).c_str(), "prev")){
        CWX_UINT32 uiNum = 1;
        if (1 != uiItemNum){
            iter++;
            uiNum = strtoul((*iter).c_str(), NULL, 0);
        }
        doPrev(uiNum);
    }else if(0 == strcasecmp((*iter).c_str(), "sid")){
        if (2 != uiItemNum){
            printf("Invalid sid command, using: sid value.\n");
            return 1;
        }
        iter++;
        doSid(strtoull((*iter).c_str(), NULL, 0));
    }else if(0 == strcasecmp((*iter).c_str(), "rec")){
        if (2 != uiItemNum){
            printf("Invalid rec command, using: rec value.\n");
            return 1;
        }
        iter++;
        doRecord(strtoul((*iter).c_str(), NULL, 0));
    }else if(0 == strcasecmp((*iter).c_str(), "group")){
        if (2 != uiItemNum){
            printf("Invalid group command, using: group value.\n");
            return 1;
        }
        iter++;
        doGroup(strtoul((*iter).c_str(), NULL, 0));
    }else if(0 == strcasecmp((*iter).c_str(), "key")){
        if (3 != uiItemNum){
            printf("Invalid key command, using: key k v.\n");
            return 1;
        }
        iter++;
        string strKey = *iter;
        doKey(strKey.c_str(), (*iter).c_str());
    }else if(0 == strcasecmp((*iter).c_str(), "head")){
        doHead();
    }else if(0 == strcasecmp((*iter).c_str(), "data")){
        doData();
    }else if(0 == strcasecmp((*iter).c_str(), "save")){
        if (1 == uiItemNum){
            printf("Invalid save command, using: save file.\n");
            return 1;
        }
        iter++;
        doSave(*iter);
/*    }else if(0 == strcasecmp((*iter).c_str(), "dump")){
        bool bHead = false;
        if (1 == uiItemNum){
            printf("Invalid dump command, using: save [-h] file.\n");
            return 1;
        }
        iter++;
        if (0 == strcasecmp((*iter).c_str(), "-h")){
            bHead = true;
            if (2 == uiItemNum){
                printf("Invalid dump command, using: save [-h] file.\n");
                return 1;
            }
            iter++;
        }
        doDump(bHead, *iter);*/
    }else if(0 == strcasecmp((*iter).c_str(), "exit")){
        return 0;
    }else{
        printf("Invalid command %s\n", (*iter).c_str());
        doHelp();
    }
    return 1;    
}

void CwxBinlogOp::doHelp(){
    printf("Binlog command help:\n");
    printf("help:             show the help information\n");
    printf("info:             show the binlog file information\n");
    printf("next [n]:         skip [n] binlog backward\n");
    printf("prev [n]:         skip [n] binlog forward\n");
    printf("sid n:            jump to binlog with sid=n\n");
    printf("rec n:            jump the Nth binlog\n");
    printf("group n:          find the next binlog with group=n\n");
    printf("key k v:          find the next binlog with key[k]'s value is v\n");
    printf("head:             print binlog's header\n");
    printf("data:             print binlog's data.\n");
    printf("save file:        save the current binlog to the file.\n");
//    printf("dump [-h] file:   dump binlog file to [file].-h:just dump binlog head.\n");
    printf("zip n:            set the compress bit[0~31] in head's attr. clear if no bit.\n");
    printf("exit:             exit.\n");
}

void CwxBinlogOp::doInfo(){
    char szBuf[64];
    printf("Binlog file info:\n");
    printf("Min Sid: %s\n", CwxCommon::toString(m_ullMinSid, szBuf));
    printf("Max Sid: %s\n", CwxCommon::toString(m_ullMaxSid, szBuf));
    printf("Record Num: %u\n", m_uiRecNum);
}
void CwxBinlogOp::doNext(CWX_UINT32 uiNum){
    int iRet = 0;
    printf("Skip %u binlog backward\n", uiNum);
    while(uiNum){
        iRet = m_pCursor->next();
        if (1 != iRet) break;
        uiNum --;
    }
    if (0 > iRet){
        printf("Failure to skip [%u] binlog backword, err:%s\n", uiNum, m_pCursor->getErrMsg());
    }else if (!iRet && uiNum){
        printf("Skip to end\n");
    }
}
void CwxBinlogOp::doPrev(CWX_UINT32 uiNum){
    int iRet = 0;
    printf("Skip %u binlog forward\n", uiNum);
    while(uiNum){
        iRet = m_pCursor->prev();
        if (1 != iRet) break;
        uiNum --;
    }
    if (0 > iRet){
        printf("Failure to skip [%u] binlog forward, err:%s\n", uiNum, m_pCursor->getErrMsg());
    }else if (!iRet && uiNum){
        printf("Skip to begin\n");
    }
}
void CwxBinlogOp::doSid(CWX_UINT64 ullSid){
    int iRet = 0;
    CWX_UINT64 offset=0;
    char szBuf1[64], szBuf2[64];
    printf("Seeking to binlog, sid=%s\n", CwxCommon::toString(ullSid, szBuf1));
    map<CWX_UINT64/*sid*/, CWX_UINT32/*offset*/>::iterator iter;

    if (ullSid < m_ullMinSid){
        printf("Sid[%s] is less than min sid[%s], it doesn't exist\n", 
            CwxCommon::toString(ullSid, szBuf1),
            CwxCommon::toString(m_ullMinSid, szBuf2));
        return;
    }
    if (ullSid > m_ullMaxSid){
        printf("Sid[%s] is more than max sid[%s], it doesn't exist\n", 
            CwxCommon::toString(ullSid, szBuf1),
            CwxCommon::toString(m_ullMaxSid, szBuf2));
        return;
    }
    iter = m_sidIndex.lower_bound(ullSid);

    CWX_ASSERT(iter != m_sidIndex.end());
    if (iter->first == ullSid){
        offset = iter->second;
    }else{
        iter--;
        CWX_ASSERT(iter != m_sidIndex.end());
        offset = iter->second;
    }

    CWX_UINT32 i=0;
    CWX_UINT64 ullOldSid = m_pCursor->getHeader().getOffset();
    bool bFind = false;
    if (1 != m_pCursor->seek(offset)){
        printf("Failure to seek binlog, error:%s\n", m_pCursor->getErrMsg());
        m_pCursor->seek(ullOldSid);
        return;
    }
    for (i=0; i<BINLOG_INDEX_INTERNAL; i++){
        if (m_pCursor->getHeader().getSid() == ullSid){
            bFind = true;
            printf("Find sid=%s\n", CwxCommon::toString(ullSid, szBuf1));
            return;
        }
        iRet = m_pCursor->next();
        if (1 == iRet) continue;
        if (0 == iRet){
            printf("Can't find sid=%s\n", CwxCommon::toString(ullSid, szBuf1));
            m_pCursor->seek(ullOldSid);
            return;
        }else if (0 > iRet){
            printf("Failure to seek binlog, err=%s\n", m_pCursor->getErrMsg());
            m_pCursor->seek(ullOldSid);
            return;
        }
    }
    printf("Can't find binlog, sid=%s\n", CwxCommon::toString(ullSid, szBuf1));
    m_pCursor->seek(ullOldSid);

}
void CwxBinlogOp::doRecord(CWX_UINT32 uiRecord){
    int iRet = 0;
    CWX_UINT64 offset=0;
    printf("Seeking to binlog, record index=%u\n", uiRecord);
    map<CWX_UINT32/*rec_no*/, CWX_UINT32/*offset*/>::iterator iter;
    if (uiRecord + 1 > m_uiRecNum){
        printf("Record Index[%u] doesn't exist, total record is %u\n",
            uiRecord,
            m_uiRecNum);
        return;
    }
    iter = m_recIndex.lower_bound(uiRecord+1);

    CWX_ASSERT(iter != m_recIndex.end());
    CWX_UINT32 uiRecordSkip = 0;
    if (iter->first == uiRecord + 1){
        offset = iter->second;
        uiRecordSkip = 0;
    }else{
        iter--;
        CWX_ASSERT(iter != m_recIndex.end());
        offset = iter->second;
        uiRecordSkip = uiRecord + 1 - iter->first;
    }

    CWX_UINT64 ullOldSid = m_pCursor->getHeader().getOffset();
    if (1 != m_pCursor->seek(offset))
    {
        printf("Failure to seek binlog, error:%s\n", m_pCursor->getErrMsg());
        m_pCursor->seek(ullOldSid);
        return;
    }
    while(uiRecordSkip){
        iRet = m_pCursor->next();
        if (0 == iRet){
            printf("Skip to end.\n");
            return;
        }else if (0 > iRet){
            printf("Failure to seek binlog, record index [%u], err=%s\n", uiRecord, m_pCursor->getErrMsg());
            m_pCursor->seek(ullOldSid);
            return;
        }
        uiRecordSkip --;
    }
}

void CwxBinlogOp::doGroup(CWX_UINT32 uiGroup){
    int iRet = 0;
    printf("Finding binlog with group %u\n", uiGroup);
    while(1){
        if (m_pCursor->getHeader().getGroup() == uiGroup){
            return ;
        }
        iRet = m_pCursor->next();
        if (0 == iRet){
            printf("Can't find binlog with group %u\n", uiGroup);
            return ;
        }else if (0 > iRet){
            printf("Failure to iterator binlog file, error: %s\n", m_pCursor->getErrMsg());
            return ;
        }
    }
}


void CwxBinlogOp::doKey(char const* szKey, char const* szValue){
    int iRet = 1;
    CwxPackageReaderEx reader(false);
    char* pBuf = NULL;
    CwxKeyValueItemEx const* pItem;
    CWX_UINT32 uiLen = 0;
    printf("Finding binlog with [%s]=%s\n", szKey, szValue);
    while(1 == iRet){
        pBuf = getData(uiLen);
        if (!pBuf) return;
        pBuf[uiLen] = 0x00;
        if (CwxPackageEx::isValidPackage(pBuf, uiLen)){
           reader.unpack(pBuf, uiLen, false, false);
           pItem = reader.getKey(szKey, true);
           if (pItem){
               if (strcasecmp(szValue, pItem->m_szData) == 0){
                   return;
               }
           }
        }
        iRet = m_pCursor->next();
    }
    if (0 == iRet){
        printf("Can't finding binlog with [%s]=%s\n", szKey, szValue);
    }else{
        printf("Failure to find binlog, err=%s\n", m_pCursor->getErrMsg());
    }
}

void CwxBinlogOp::doHead(){
    char szBuf[64];
    string strDatetime;
    CwxDate::getDate((time_t)m_pCursor->getHeader().getDatetime(), strDatetime);
    printf("binlog head info:\n");
    printf("sid: %s\n", CwxCommon::toString(m_pCursor->getHeader().getSid(), szBuf));
    printf("timestamp: %s[%d]\n", strDatetime.c_str(), m_pCursor->getHeader().getDatetime());
    printf("offset: %u\n", m_pCursor->getHeader().getOffset());
    printf("log length: %u\n", m_pCursor->getHeader().getLogLen());
    printf("log no: %u\n", m_pCursor->getHeader().getLogNo());
    printf("prev offset: %u\n", m_pCursor->getHeader().getPrevOffset());
    printf("group : %u\n", m_pCursor->getHeader().getGroup());
}

void CwxBinlogOp::doData(){
    char* pBuf = NULL;
    CWX_UINT32 uiLen = 0;
    CWX_UINT32 uiDumpLen = 0;

    printf("Binlog data:\n");

    pBuf = getData(uiLen);
    if (!pBuf) return;
    pBuf[uiLen] = 0x00;
    if (CwxPackageEx::isValidPackage(pBuf, uiLen)){
        if (!prepareDumpBuf(uiLen + 64 * 1024)) return;
        uiDumpLen = m_uiDumpBufLen;
        CwxPackageEx::dump(pBuf, uiLen, m_pDumpBuf, uiDumpLen);
        m_pDumpBuf[uiDumpLen] = 0x00;
        printf("%s\n", m_pDumpBuf);
    }else{
        printf("%s\n", pBuf);
    }
}

void CwxBinlogOp::doSave(string const& strFileName){
    char* pBuf = NULL;
    CWX_UINT32 uiLen = 0;
    CWX_UINT32 uiDumpLen = 0;

    printf("Save binlog \n");

    pBuf = getData(uiLen);
    if (!pBuf) return;
    pBuf[uiLen] = 0x00;
    if (CwxPackageEx::isValidPackage(pBuf, uiLen)){
        if (!prepareDumpBuf(uiLen + 64 * 1024)) return;
        uiDumpLen = m_uiDumpBufLen;
        CwxPackageEx::dump(pBuf, uiLen, m_pDumpBuf, uiDumpLen);
        m_pDumpBuf[uiDumpLen] = 0x00;
        pBuf = m_pDumpBuf;
    }

    FILE* pFile = fopen(strFileName.c_str(), "a+");
    if (!pFile){
        printf("Failure to open file:%s, errno=%d\n", strFileName.c_str(), errno);
        return;
    }
    char szBuf[64];
    string strDatetime;
    fprintf(pFile, "Head:\n");
    fprintf(pFile, "sid: %s\n", CwxCommon::toString(m_pCursor->getHeader().getSid(), szBuf));
    CwxDate::getDate(m_pCursor->getHeader().getDatetime(), strDatetime);
    fprintf(pFile, "timestamp: %s[%d]\n", strDatetime.c_str(), m_pCursor->getHeader().getDatetime());
    fprintf(pFile, "offset: %u\n", m_pCursor->getHeader().getOffset());
    fprintf(pFile, "Log Length: %u\n", m_pCursor->getHeader().getLogLen());
    fprintf(pFile, "Log NO: %u\n", m_pCursor->getHeader().getLogNo());
    fprintf(pFile, "prev offset: %u\n", m_pCursor->getHeader().getPrevOffset());
    fprintf(pFile, "Data:\n");
    fprintf(pFile, "%s\n", pBuf);
    fclose(pFile);
}
/*
void CwxBinlogOp::doDump(bool bHead, string const& strFileName){

}
*/

bool CwxBinlogOp::prepareBuf(CWX_UINT32 uiLen){
    if (uiLen + 1 > m_uiBufLen){
        if (m_pBuf) delete [] m_pBuf;
        m_uiBufLen = uiLen + 2048;
        m_pBuf = new char[m_uiBufLen];
        if (!m_pBuf){
            printf("Failure to malloc memory, size:%u\n", m_uiBufLen);
            m_uiBufLen = 0;
            return false;
        }
    }
    return true;
}
bool CwxBinlogOp::prepareUnBuf(CWX_UINT32 uiLen){
    if (uiLen + 1 > m_uiUnBufLen){
        if (m_pUnBuf) delete [] m_pUnBuf;
        m_uiUnBufLen = uiLen + 2048;
        m_pUnBuf = new char[m_uiUnBufLen];
        if (!m_pUnBuf){
            printf("Failure to malloc memory, size:%u\n", m_uiUnBufLen);
            m_uiUnBufLen = 0;
            return false;
        }
    }
    return true;
}

bool CwxBinlogOp::prepareDumpBuf(CWX_UINT32 uiLen){
    if (uiLen + 1 > m_uiDumpBufLen){
        if (m_pDumpBuf) delete [] m_pDumpBuf;
        m_uiDumpBufLen = uiLen + 2048;
        m_pDumpBuf = new char[m_uiDumpBufLen];
        if (!m_pDumpBuf){
            printf("Failure to malloc memory, size:%u\n", m_uiDumpBufLen);
            m_uiDumpBufLen = 0;
            return false;
        }
    }
    return true;
}


char* CwxBinlogOp::getData(CWX_UINT32& uiLen){
    char* pBuf = NULL;
    if (!prepareBuf(m_pCursor->getHeader().getLogLen())) return NULL;
    
    uiLen = m_uiBufLen;
    if (0 > m_pCursor->data(m_pBuf, uiLen)){
        printf("Failure to read data, err:%s\n", m_pCursor->getErrMsg());
        return NULL;
    }
	pBuf = m_pBuf;
    return pBuf;
}
