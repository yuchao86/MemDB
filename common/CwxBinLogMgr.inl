
/***********************************************************************
                    CwxBinLogHeader  class
***********************************************************************/
inline CwxBinLogHeader::CwxBinLogHeader()
{
    memset(this, 0x00, sizeof(this));
}

inline CwxBinLogHeader::CwxBinLogHeader(CWX_UINT64 ullSid,
										CWX_UINT32 uiLogNo,
                                     CWX_UINT32 uiDatetime,
                                     CWX_UINT32 uiOffset,
                                     CWX_UINT32 uiLogLen,
                                     CWX_UINT32 uiPrevOffset,
                                     CWX_UINT32 uiGroup):
m_ullSid(ullSid), m_uiLogNo(uiLogNo), m_uiDatetime(uiDatetime), m_uiOffset(uiOffset),
m_uiLogLen(uiLogLen), m_uiPrevOffset(uiPrevOffset),
m_uiGroup(uiGroup)
{
}


inline CwxBinLogHeader::CwxBinLogHeader(CwxBinLogHeader const& header)
{
    memcpy(this, &header, sizeof(CwxBinLogHeader));
}

inline CwxBinLogHeader& CwxBinLogHeader::operator=(CwxBinLogHeader const& header)
{
    if (this != &header)
    {
        memcpy(this, &header, sizeof(CwxBinLogHeader));
    }
    return *this;
}

inline bool CwxBinLogHeader::operator<(CwxBinLogHeader const& header) const
{
    return this->m_ullSid<header.m_ullSid;
}

inline void CwxBinLogHeader::setSid(CWX_UINT64 ullSid)
{
    m_ullSid = ullSid;
}

inline CWX_UINT64 CwxBinLogHeader::getSid() const
{
    return m_ullSid;
}

///���ü�¼��
inline void CwxBinLogHeader::setLogNo(CWX_UINT32 uiLogNo)
{
	m_uiLogNo = uiLogNo;
}
///��ȡ��¼��
inline CWX_UINT32 CwxBinLogHeader::getLogNo() const
{
	return m_uiLogNo;
}

inline void CwxBinLogHeader::setDatetime(CWX_UINT32 uiDatetime)
{
    m_uiDatetime = uiDatetime;
}

inline CWX_UINT32 CwxBinLogHeader::getDatetime() const
{
    return m_uiDatetime;
}

inline void CwxBinLogHeader::setOffset(CWX_UINT32 uiOffset)
{
    m_uiOffset = uiOffset;
}

inline CWX_UINT32 CwxBinLogHeader::getOffset() const
{
    return m_uiOffset;
}

inline void CwxBinLogHeader::setLogLen(CWX_UINT32 uiLogLen)
{
    m_uiLogLen = uiLogLen;
}

inline CWX_UINT32 CwxBinLogHeader::getLogLen() const
{
    return m_uiLogLen;
}

inline void CwxBinLogHeader::setPrevOffset(CWX_UINT32 uiPrevOffset)
{
    m_uiPrevOffset = uiPrevOffset;
}

inline CWX_UINT32 CwxBinLogHeader::getPrevOffset() const
{
    return m_uiPrevOffset;
}

///����binlog�ķ���
inline void CwxBinLogHeader::setGroup(CWX_UINT32 uiGroup)
{
    m_uiGroup = uiGroup;
}

///��ȡbinlog�ķ���
inline CWX_UINT32 CwxBinLogHeader::getGroup() const
{
    return m_uiGroup;
}



inline CWX_UINT32 CwxBinLogHeader::serialize(char* szBuf) const
{
    CWX_UINT32 byte4;
    CWX_UINT8  pos = 0;
    //sid
    byte4 = (CWX_UINT32)(m_ullSid>>32);
    byte4 = CWX_HTONL(byte4); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    byte4 = (CWX_UINT32)(m_ullSid&0xFFFFFFFF);
    byte4 = CWX_HTONL(byte4); memcpy(szBuf+pos, &byte4, 4); pos+=4;
	//logno
	byte4 = CWX_HTONL(m_uiLogNo); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //datetime
    byte4 = CWX_HTONL(m_uiDatetime); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //offset
    byte4 = CWX_HTONL(m_uiOffset); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //log-length
    byte4 = CWX_HTONL(m_uiLogLen); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //prev-offset
	byte4 = CWX_HTONL(m_uiPrevOffset); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //group
    byte4 = CWX_HTONL(m_uiGroup); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    return pos;
}

inline CWX_UINT32 CwxBinLogHeader::unserialize(char const* szBuf)
{
    CWX_UINT32 byte4;
    CWX_UINT8  pos = 0;
    //sid
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_ullSid = CWX_NTOHL(byte4);
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_ullSid <<=32;
    m_ullSid += CWX_NTOHL(byte4);
	//log no
	memcpy(&byte4, szBuf+pos, 4); pos += 4;
	m_uiLogNo = CWX_NTOHL(byte4);
    //datetime
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_uiDatetime = CWX_NTOHL(byte4);
    //offset
	memcpy(&byte4, szBuf+pos, 4); pos += 4;
	m_uiOffset = CWX_NTOHL(byte4);
    //log-length
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_uiLogLen = CWX_NTOHL(byte4);
    //prev-offset
	memcpy(&byte4, szBuf+pos, 4); pos += 4;
	m_uiPrevOffset = CWX_NTOHL(byte4);
    //group
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_uiGroup = CWX_NTOHL(byte4);
    return pos;
}

///��ն���
inline void CwxBinLogHeader::reset()
{
    memset(this, 0x00, sizeof(CwxBinLogHeader));
}

/***********************************************************************
                    CwxBinLogIndex  class
***********************************************************************/
inline CwxBinLogIndex::CwxBinLogIndex()
{
    memset(this, 0x00, sizeof(CwxBinLogIndex));
}

inline CwxBinLogIndex::CwxBinLogIndex(CWX_UINT64 ullSid, CWX_UINT32 uiDatetime, CWX_UINT32 uiOffset, CWX_UINT32 uiLogLen):
m_ullSid(ullSid), m_uiDatetime(uiDatetime), m_uiOffset(uiOffset), m_uiLogLen(uiLogLen)
{
}

inline CwxBinLogIndex::CwxBinLogIndex(CwxBinLogIndex const& index)
{
    memcpy(this, &index, sizeof(CwxBinLogIndex));
}

inline CwxBinLogIndex::CwxBinLogIndex(CwxBinLogHeader const& header)
{
    m_ullSid = header.getSid();
    m_uiDatetime = header.getDatetime();
    m_uiOffset = header.getOffset();
    m_uiLogLen = header.getLogLen();
}

inline CwxBinLogIndex& CwxBinLogIndex::operator=(CwxBinLogIndex const& index)
{
    if (this != &index)
    {
        memcpy(this, &index, sizeof(CwxBinLogIndex));
    }
    return *this;
}

inline CwxBinLogIndex& CwxBinLogIndex::operator=(CwxBinLogHeader const& header)
{
    m_ullSid = header.getSid();
    m_uiDatetime = header.getDatetime();
    m_uiOffset = header.getOffset();
    m_uiLogLen = header.getLogLen();
    return *this;
}

inline bool CwxBinLogIndex::operator<(CwxBinLogIndex const& index) const
{
    return m_ullSid < index.m_ullSid;
}

inline void CwxBinLogIndex::setSid(CWX_UINT64 ullSid)
{
    m_ullSid = ullSid;
}

inline CWX_UINT64 CwxBinLogIndex::getSid() const
{
    return m_ullSid;
}

inline void CwxBinLogIndex::setDatetime(CWX_UINT32 uiDatetime)
{
    m_uiDatetime = uiDatetime;
}

inline CWX_UINT32 CwxBinLogIndex::getDatetime() const
{
    return m_uiDatetime;
}


inline void CwxBinLogIndex::setOffset(CWX_UINT32 uiOffset)
{
    m_uiOffset = uiOffset;
}

inline CWX_UINT32 CwxBinLogIndex::getOffset() const
{
    return m_uiOffset;
}

inline void CwxBinLogIndex::setLogLen(CWX_UINT32 uiLogLen)
{
    m_uiLogLen = uiLogLen;
}

inline CWX_UINT32 CwxBinLogIndex::getLogLen() const
{
    return m_uiLogLen;
}

inline CWX_UINT32 CwxBinLogIndex::serialize(char* szBuf) const
{
    CWX_UINT32 byte4;
    CWX_UINT8  pos = 0;
    //sid
    byte4 = (CWX_UINT32)(m_ullSid>>32);
    byte4 = CWX_HTONL(byte4); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    byte4 = (CWX_UINT32)(m_ullSid&0xFFFFFFFF);
    byte4 = CWX_HTONL(byte4); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //datetime
    byte4 = CWX_HTONL(m_uiDatetime); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //offset
	byte4 = CWX_HTONL(m_uiOffset); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    //log-length
    byte4 = CWX_HTONL(m_uiLogLen); memcpy(szBuf+pos, &byte4, 4); pos+=4;
    return pos;
}

inline CWX_UINT32 CwxBinLogIndex::unserialize(char const* szBuf)
{
    CWX_UINT32 byte4;
    CWX_UINT8  pos = 0;
    //sid
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_ullSid = CWX_NTOHL(byte4);
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_ullSid <<=32;
    m_ullSid += CWX_NTOHL(byte4);
    //datetime
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_uiDatetime = CWX_NTOHL(byte4);
    //offset
	memcpy(&byte4, szBuf+pos, 4); pos += 4;
	m_uiOffset = CWX_NTOHL(byte4);
    //log-length
    memcpy(&byte4, szBuf+pos, 4); pos += 4;
    m_uiLogLen = CWX_NTOHL(byte4);
    return pos;
}

///��ն���
inline void CwxBinLogIndex::reset()
{
    memset(this, 0x00, sizeof(CwxBinLogIndex));
}



/***********************************************************************
                    CwxBinLogCursor  class
***********************************************************************/
inline int CwxBinLogCursor::next()
{
	if (CURSOR_STATE_ERROR == m_ucSeekState) return -1;
    if (-1 == m_fd){
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Cursor's file handle is invalid");
        return -1;
    }
	if (CURSOR_STATE_UNSEEK == m_ucSeekState) {
        return header(0);
	}
	return header(m_curLogHeader.getOffset() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE + m_curLogHeader.getLogLen());
}

inline int CwxBinLogCursor::prev()
{
	if (CURSOR_STATE_ERROR == m_ucSeekState) return -1;
    if (-1 == m_fd){
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Cursor's file handle is invalid");
        return -1;
    }
	if (CURSOR_STATE_UNSEEK == m_ucSeekState) {
        return header(0);
	}
    if (0 != m_curLogHeader.getOffset()){
		return header(m_curLogHeader.getPrevOffset());
	}
	return 0;
}

inline int CwxBinLogCursor::seek(CWX_UINT32 uiOffset)
{
	m_ucSeekState = CURSOR_STATE_UNSEEK;
	if (-1 == m_fd)
	{
		CwxCommon::snprintf(this->m_szErr2K, 2047, "Cursor's file handle is invalid");
		return -1;
	}
    int iRet = header(uiOffset);
    if (1 == iRet){
		m_ucSeekState = CURSOR_STATE_READY;
    }
    return iRet;
}


inline CwxBinLogHeader const& CwxBinLogCursor::getHeader() const
{
    return m_curLogHeader;
}

inline string const& CwxBinLogCursor::getFileName() const
{
    return m_strFileName;
}

///��ȡ�ļ�������
inline CWX_UINT32 CwxBinLogCursor::getFileDay() const
{
    return m_uiFileDay;
}

///��ȡ�ļ������
inline CWX_UINT32 CwxBinLogCursor::getFileNo() const
{
    return m_uiFileNo;
}


inline char* CwxBinLogCursor::getErrMsg()
{
    return m_szErr2K;
}

inline int CwxBinLogCursor::getHandle() const
{
    return m_fd;
}

///��ȡcursor��SEEK STATE
inline CWX_UINT8 CwxBinLogCursor::getSeekState() const{
	return m_ucSeekState;
}
///����cursor��SEEK STATE
inline void CwxBinLogCursor::setSeekState(CWX_UINT8 ucSeekState){
	m_ucSeekState = ucSeekState;
}
///��ȡcursor��SEEK SID
inline CWX_UINT64 CwxBinLogCursor::getSeekSid() const{
	return m_ullSeekSid;
}
///����cursor��SEEK SID
inline void CwxBinLogCursor::setSeekSid(CWX_UINT64 ullSid){
	m_ullSeekSid = ullSid;
}

///�Ƿ�ready
inline bool CwxBinLogCursor::isReady() const{
	return CURSOR_STATE_READY == m_ucSeekState;
}
///�Ƿ�unseek
inline bool CwxBinLogCursor::isUnseek() const{
	return CURSOR_STATE_UNSEEK == m_ucSeekState;
}
///�Ƿ����
inline bool CwxBinLogCursor::isError() const{
	return CURSOR_STATE_ERROR == m_ucSeekState;
}


/***********************************************************************
                    CwxBinLogFile  class
***********************************************************************/
///С�ڱȽ�
inline bool CwxBinLogFile::operator < (CwxBinLogFile const& obj) const
{
    return m_ullMinSid < obj.m_ullMaxSid;
}

///��ȡ��С��sid
inline CWX_UINT64 CwxBinLogFile::getMinSid() const
{
    return m_ullMinSid;
}

///��ȡ����sid
inline CWX_UINT64 CwxBinLogFile::getMaxSid() const
{
    return m_ullMaxSid;
}

///��ȡbinlog����Сʱ���
inline CWX_UINT32 CwxBinLogFile::getMinTimestamp() const
{
    return m_ttMinTimestamp;
}

///��ȡbinlog�����ʱ���
inline CWX_UINT32 CwxBinLogFile::getMaxTimestamp() const
{
    return m_ttMaxTimestamp;
}

inline CWX_UINT32 CwxBinLogFile::getFileDay() const
{
    return m_ttDay;
}

///��ȡbinlog���ļ����
inline CWX_UINT32 CwxBinLogFile::getFileNo() const
{
    return m_uiFileNo;
}

///��ȡbinlog�ļ���log��¼��
inline CWX_UINT32 CwxBinLogFile::getLogNum() const
{
    return m_uiLogNum;
}

///��ȡbinlog�ļ��Ĵ�С
inline CWX_UINT32 CwxBinLogFile::getFileSize() const
{
    return m_uiFileSize;
}

///�ж��Ƿ�ֻ��
inline bool CwxBinLogFile::readOnly() const
{
    return m_bReadOnly;
}

///����ֻ��
inline void CwxBinLogFile::setReadOnly()
{
    if (!m_bReadOnly)
    {
        flush_cache(NULL);
		fsync(true,NULL);
        m_bReadOnly = true;
        if (-1 != m_fd) ::close(m_fd);
        m_fd = -1;
        if (-1 != m_indexFd) ::close(m_indexFd);
        m_indexFd = -1;
        if (m_writeCache) delete m_writeCache;
        m_writeCache = NULL;

    }
}

///�ж���־�ļ��Ƿ�Ϊ��
inline bool CwxBinLogFile::empty() const
{
    return !m_uiLogNum;
}

///��ȡ�����ļ�������
inline string const& CwxBinLogFile::getDataFileName() const
{
    return m_strPathFileName;
}
///��ȡ�����ļ�������
inline string const& CwxBinLogFile::getIndexFileName() const
{
    return m_strIndexFileName;
}


//-1��ʧ�ܣ�0���ɹ���
inline int CwxBinLogFile::readIndex(int fd, CwxBinLogIndex& index, CWX_UINT32 uiOffset, char* szErr2K) const
{
    char szBuf[CwxBinLogIndex::BIN_LOG_INDEX_SIZE];
	ssize_t ret = pread(fd, &szBuf, CwxBinLogIndex::BIN_LOG_INDEX_SIZE, uiOffset);
    if (CwxBinLogIndex::BIN_LOG_INDEX_SIZE != ret)
    {
		if (szErr2K)
		{
			if (-1 == ret)
			{
				CwxCommon::snprintf(szErr2K, 2047, "Failure to read binlog index, file:%s, errno=%d", this->m_strIndexFileName.c_str(), errno);
			}
			else
			{
				CwxCommon::snprintf(szErr2K, 2047, "No complete index record, offset:%u", uiOffset);
			}
		}
        return -1;
    }
    index.unserialize(szBuf);
    return 0;
}

// -1��ʧ�ܣ�0���ɹ���
inline int CwxBinLogFile::writeIndex(int fd, CwxBinLogIndex const& index, CWX_UINT32 uiOffset, char* szErr2K) const
{
    char szBuf[CwxBinLogIndex::BIN_LOG_INDEX_SIZE];
    index.serialize(szBuf);
    if (CwxBinLogIndex::BIN_LOG_INDEX_SIZE != pwrite(fd, szBuf, CwxBinLogIndex::BIN_LOG_INDEX_SIZE, uiOffset))
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to write binlog index, file:%s, errno=%d", m_strIndexFileName.c_str(), errno);
        return -1;
    }
    return 0;
}

/***********************************************************************
                    CwxBinLogMgr  class
***********************************************************************/
///��ȡ�������Ƿ���Ч
inline bool CwxBinLogMgr::isInvalid() const
{
    return !m_bValid;
}

inline bool CwxBinLogMgr::isOutRange(CwxBinLogCursor* pCursor)
{
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    return _isOutRange(pCursor);
}
///�Ƿ���unseek
inline bool CwxBinLogMgr::isUnseek(CwxBinLogCursor* pCursor)
{
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    return pCursor->isUnseek();
}

///cursor��Ӧ���ļ����Ƿ��ڹ���ķ�Χ֮��
inline bool CwxBinLogMgr::_isOutRange(CwxBinLogCursor*& pCursor)
{
    bool ret = pCursor->getHeader().getSid() < getMinSid();
	CWX_ASSERT(!ret);
	return ret;
}


///��ȡ��������Ч��ԭ��
inline char const* CwxBinLogMgr::getInvalidMsg() const
{
    return m_szErr2K;
}

///��ȡ��С��sid
inline CWX_UINT64 CwxBinLogMgr::getMinSid()
{
    return m_ullMinSid;
}
///��ȡ����sid
inline CWX_UINT64 CwxBinLogMgr::getMaxSid()
{
    return m_ullMaxSid;
}
///��ȡbinlog����Сʱ���
inline CWX_UINT32 CwxBinLogMgr::getMinTimestamp()
{
    return m_ttMinTimestamp;
}

///��ȡbinlog�����ʱ���
inline CWX_UINT32 CwxBinLogMgr::getMaxTimestamp()
{
    return m_ttMaxTimestamp;
}

///��ȡ�����binlog����С�ļ����
inline string& CwxBinLogMgr::getMinFile(string& strFile)
{
    ///��������
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    CwxBinLogFile* pFile = _getMinBinLogFile();
    strFile = pFile?pFile->getDataFileName():"";
    return strFile;
}
///��ȡ�����binlog������ļ����
inline string& CwxBinLogMgr::getMaxFile(string& strFile)
{
    ///��������
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    CwxBinLogFile* pFile = _getMaxBinLogFile();
    strFile = pFile?pFile->getDataFileName():"";
    return strFile;
}
///����Ƿ�Ϊ��
inline bool CwxBinLogMgr::empty()
{
    return !getMaxSid();
}
///��ȡ�ļ��Ŷ�Ӧ��binlog�ļ���
inline string& CwxBinLogMgr::getFileNameByFileNo(CWX_UINT32 uiFileNo,
                                                 CWX_UINT32 ttDay,
                                                 string& strFileName)
{
    char szPathFile[512];
    string strDay;
    CwxDate::getDateY4MDHMS2(ttDay, strDay);
    snprintf(szPathFile, 511, "%s%s.%10.10d.%s.log",
        m_strPrexLogPath.c_str(),
        m_strFilePrex.c_str(),
		uiFileNo,
        strDay.substr(0,8).c_str());
    strFileName = szPathFile;
    return strFileName;
}
///��ȡ�ļ��Ŷ�Ӧ��binlog�ļ��������ļ���
inline string& CwxBinLogMgr::getIndexFileNameByFileNo(CWX_UINT32 uiFileNo,
                                                      CWX_UINT32 ttDay,
                                                      string& strFileName)
{
    getFileNameByFileNo(uiFileNo, ttDay, strFileName);
    strFileName += ".idx";
    return strFileName;
}

inline CWX_UINT32 CwxBinLogMgr::getBinLogFileNo(string const& strFileName, CWX_UINT32& ttDay)
{
    
    if (strFileName.length() <= m_strPrexLogPath.length() + m_strFilePrex.length() + 13) return 0;
	string strFileNum = strFileName.substr(m_strPrexLogPath.length() + m_strFilePrex.length() + 1, 10);
    string strDay = strFileName.substr(m_strPrexLogPath.length() + m_strFilePrex.length() + 1 + 10 + 1, 8);
    strDay += "000000";
    ttDay = CwxDate::getDateY4MDHMS2(strDay);
    return strtoul(strFileNum.c_str(), NULL, 10);
}

inline bool CwxBinLogMgr::isBinLogFile(string const& strFileName)
{
    string strTmp;
    CWX_UINT32  ttDay=0;
    CWX_UINT32 uiFileNo = getBinLogFileNo(strFileName, ttDay);
    return getFileNameByFileNo(uiFileNo, ttDay, strTmp) == strFileName;
}

///�ж�һ���ļ����Ƿ���һ��binlog�������ļ�
inline bool CwxBinLogMgr::isBinLogIndexFile(string const& strFileName)
{
    string strTmp;
    CWX_UINT32 ttDay = 0;
    CWX_UINT32 uiFileNo = getBinLogFileNo(strFileName, ttDay);
    return getIndexFileNameByFileNo(uiFileNo, ttDay, strTmp) == strFileName;
}
///��ȡbinlog��ǰ׺��
inline  string const& CwxBinLogMgr::getBinlogPrexName() const
{
    return m_strFilePrex;
}
///�Ƿ���Ч��ǰ׺��
inline bool CwxBinLogMgr::isValidPrexName(char const* szName)
{
    if (!szName) return false;
    if (!strlen(szName)) return false;
    size_t i=0;
    while(szName[i])
    {
        if (((szName[i] >= 'a') && (szName[i]<='z')) ||
            ((szName[i] >= 'A') && (szName[i]<='Z')) ||
            ((szName[i] >= '0') && (szName[i]<='9')) ||
            (szName[i] == '.') || (szName[i] == '_') || (szName[i] == '-'))
            continue;
        return false;
    }
    return true;
}


inline CwxBinLogFile* CwxBinLogMgr::_getMinBinLogFile() 
{
    if (m_binlogMap.begin() != m_binlogMap.end()) return m_binlogMap.begin()->second;
    return m_pCurBinlog;
}

inline CwxBinLogFile* CwxBinLogMgr::_getMaxBinLogFile()
{
    return m_pCurBinlog;
}

inline bool CwxBinLogMgr::_isManageBinLogFile(CwxBinLogFile* pBinLogFile)
{
    if (!m_pCurBinlog) return true;
    ///����ļ���cursorʹ�ã��򱻹���
	if (m_binlogMap.size() <= m_uiMaxFileNum) return true;
	//����Ƿ���cursor��ʹ��
	set<CwxBinLogCursor*>::iterator iter = m_cursorSet.begin();
	while(iter != m_cursorSet.end()){
		if ((*iter)->isReady()){//cursor��headerһ����Ч
			if ((*iter)->getHeader().getSid() <= pBinLogFile->getMaxSid()){
				return true;
			}
        }else if ((*iter)->getSeekSid() <= pBinLogFile->getMaxSid()){//cursor��������״̬
			return true;
		}
		iter++;
	}
	return false;

}

