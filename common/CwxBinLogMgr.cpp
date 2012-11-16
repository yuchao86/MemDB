#include "CwxBinLogMgr.h"

/***********************************************************************
                    CwxBinLogCursor  class
***********************************************************************/
CwxBinLogCursor::CwxBinLogCursor()
{
    m_fd = -1;
    m_szHeadBuf[0] = 0x00;
    m_szErr2K[0] = 0x00;
	m_uiBlockNo = 0;
	m_uiBlockDataOffset = 0;
    m_ullSeekSid = 0; ///<seek��sid
	m_uiFileDay = 0; ///<�ļ�������
	m_uiFileNo = 0; ///<�ļ���
    m_ucSeekState = CURSOR_STATE_UNSEEK; ///<seek��״̬

}

CwxBinLogCursor::~CwxBinLogCursor()
{
    if (-1 != m_fd) ::close(m_fd);
}

int CwxBinLogCursor::open(char const* szFileName, CWX_UINT32 uiFileNo, CWX_UINT32 uiFileDay)
{
	m_ucSeekState = CURSOR_STATE_UNSEEK; ///<seek��״̬
    if (-1 != this->m_fd)
	{
		::close(m_fd);
		m_fd = -1;
	}
    m_fd = ::open(szFileName, O_RDONLY);
    if (-1 == m_fd)
    {
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Can't open file:%s.", szFileName);
        return -1;
    }
    m_strFileName = szFileName;
    m_curLogHeader.reset();
    m_szErr2K[0] = 0x00;
	m_uiBlockNo = 0;
	m_uiBlockDataOffset = 0;
	m_uiFileDay = uiFileDay;
	m_uiFileNo = uiFileNo;
    return 0;
}

/**
@brief ��ȡ��ǰlog��data
@return -1��ʧ�ܣ�>=0����ȡ���ݵĳ���
*/
int CwxBinLogCursor::data(char * szBuf, CWX_UINT32& uiBufLen)
{
    int iRet;
    if (-1 == m_fd)
    {
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Cursor's file handle is invalid");
        return -1;
    }
    
	if (m_ucSeekState == CURSOR_STATE_UNSEEK)
    {
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Cursor is not ready.");
        return -1;
    }

	if (m_ucSeekState == CURSOR_STATE_ERROR){
		return -1;
	}

	///������Щ���󣬲�Ӱ��cursor״̬�ĸı�
    if (m_curLogHeader.getLogLen())
    {
        if (uiBufLen < m_curLogHeader.getLogLen())
        {
            CwxCommon::snprintf(this->m_szErr2K, 2047, "Buf is too small, buf-size[%u], data-size[%u]",
                uiBufLen, m_curLogHeader.getLogLen());
            return -1;
        }
        iRet = pread(this->m_fd,
            szBuf,
            m_curLogHeader.getLogLen(),
            m_curLogHeader.getOffset() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE);
        if (iRet != (int)m_curLogHeader.getLogLen())
        {
            if (-1 == iRet)
				return -1;
            uiBufLen = iRet;
            CwxCommon::snprintf(this->m_szErr2K, 2047, "Log's dat is less, log-data-size[%u], read-size[%u]",
                m_curLogHeader.getLogLen(), iRet);
            return -2;
        }
    }
    uiBufLen = m_curLogHeader.getLogLen();
    return uiBufLen;
}

void CwxBinLogCursor::close()
{
    if (-1 != m_fd) ::close(m_fd);
    m_fd = -1;
	m_ucSeekState = CURSOR_STATE_UNSEEK; ///<seek��״̬
}


///-2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0��������1����ȡһ��
int CwxBinLogCursor::header(CWX_UINT32 uiOffset)
{
    int iRet;
    iRet = pread(this->m_fd, m_szHeadBuf, CwxBinLogHeader::BIN_LOG_HEADER_SIZE, uiOffset);
    if (iRet != CwxBinLogHeader::BIN_LOG_HEADER_SIZE)
    {
        if (0 == iRet)
        {
            return 0;
        }
        else if(-1 == iRet)
        {
            return -1;
        }
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Log is incomplete");
        return -2;
    }
    m_curLogHeader.unserialize(m_szHeadBuf);
    if (uiOffset != m_curLogHeader.getOffset())
    {
		m_ucSeekState = CURSOR_STATE_ERROR; ///<seek��״̬
        CwxCommon::snprintf(this->m_szErr2K, 2047, "Invalid binlog, offset of header[%u] is different with it's file-offset[%u].",
            uiOffset, m_curLogHeader.getOffset());
        return -1;
    }
    m_ucSeekState = CURSOR_STATE_READY;
    return 1;
}

inline bool CwxBinLogCursor::preadPage(int fildes, CWX_UINT32 uiBlockNo, CWX_UINT32 uiOffset)
{
	CWX_ASSERT(uiOffset<=BINLOG_READ_BLOCK_SIZE);
	ssize_t ret = 0;
	if (uiBlockNo != m_uiBlockNo)
	{
		m_uiBlockNo = uiBlockNo;
		m_uiBlockDataOffset = 0;
	}
	if ((uiBlockNo == m_uiBlockNo)&&(uiOffset <= m_uiBlockDataOffset)) return true;
	do 
	{
		ret = ::pread(fildes, m_szReadBlock + m_uiBlockDataOffset, BINLOG_READ_BLOCK_SIZE - m_uiBlockDataOffset, (uiBlockNo<<BINLOG_READ_BLOCK_BIT) + m_uiBlockDataOffset);
		if (-1 != ret)
		{
			m_uiBlockDataOffset += ret;
			return true;
		}
		if (EOVERFLOW == errno) return true;
		if (EINTR == errno) continue;
		break;
	} while(1);
	CwxCommon::snprintf(this->m_szErr2K, 2047, "Failure to read bin-log file:%s, errno=%d.", m_strFileName.c_str(), errno);
	return false;
}

//��ȡ����
ssize_t CwxBinLogCursor::pread(int fildes, void *buf, size_t nbyte, CWX_UINT32 offset)
{
	size_t pos = 0;
	if (nbyte)
	{
		CWX_UINT32 uiStartBlock = offset>>BINLOG_READ_BLOCK_BIT;
		CWX_UINT32 uiEndBlock = (offset + nbyte - 1)>>BINLOG_READ_BLOCK_BIT;
		CWX_UINT32 uiBlockNum = uiEndBlock - uiStartBlock + 1;
		CWX_UINT32 uiBlockStartOffset = 0;
		CWX_UINT32 uiBlockEndOffset = 0;

		//get first block
		uiBlockStartOffset = offset - (uiStartBlock << BINLOG_READ_BLOCK_BIT);
		uiBlockEndOffset = uiBlockNum==1?uiBlockStartOffset + nbyte:(CWX_UINT32)BINLOG_READ_BLOCK_SIZE;
		if (!preadPage(fildes, uiStartBlock, uiBlockEndOffset)) return -1;
		if (uiBlockEndOffset <= m_uiBlockDataOffset)
		{//�����㹻
			memcpy(buf, m_szReadBlock + uiBlockStartOffset, uiBlockEndOffset - uiBlockStartOffset);
			pos = uiBlockEndOffset - uiBlockStartOffset;
			if (uiStartBlock == uiEndBlock) return pos;
		}
		else
		{//not enough data
			if (uiBlockStartOffset < m_uiBlockDataOffset)
			{
				memcpy(buf, m_szReadBlock + uiBlockStartOffset, m_uiBlockDataOffset - uiBlockStartOffset);
				pos = m_uiBlockDataOffset - uiBlockStartOffset;
				return pos;
			}
			return 0;
		}
		//get middle block
		if (uiBlockNum > 2)
		{//read middle
			ssize_t ret = 0;
			do 
			{
				ret = ::pread(fildes, (char*)buf+pos, (uiBlockNum - 2)<<BINLOG_READ_BLOCK_BIT, (uiStartBlock + 1)<<BINLOG_READ_BLOCK_BIT);
				if (-1 != ret)
				{
					pos += ret;
					if ((CWX_UINT32)ret != ((uiBlockNum - 2)<<BINLOG_READ_BLOCK_BIT)) //finish
						return pos;
					break;
				}
				if (EOVERFLOW == errno) return pos;
				if (EINTR == errno) continue;
				CwxCommon::snprintf(this->m_szErr2K, 2047, "Failure to read bin-log file:%s, errno=%d.", m_strFileName.c_str(), errno);
				return -1;
			} while(1);
		}		
		//get last block
		{
			CWX_ASSERT(nbyte > pos);
			uiBlockEndOffset = nbyte - pos;
			if (!preadPage(fildes, uiEndBlock, uiBlockEndOffset)) return -1;
			if (uiBlockEndOffset <= m_uiBlockDataOffset)
			{//�����㹻
				memcpy((char*)buf + pos, m_szReadBlock, uiBlockEndOffset);
				pos += uiBlockEndOffset;
			}
			else
			{//not enough data
				memcpy((char*)buf + pos, m_szReadBlock, m_uiBlockDataOffset);
				pos += m_uiBlockDataOffset;
			}

		}
	}
	return pos;
}

/***********************************************************************
                    CwxBinLogIndexWriteCache  class
***********************************************************************/
CwxBinLogIndexWriteCache::CwxBinLogIndexWriteCache(int indexFd,
                                         CWX_UINT32 uiIndexOffset,
                                         CWX_UINT64 ullSid)
{
    m_indexFd = indexFd;
    m_ullPrevIndexSid = ullSid;
    m_ullMinIndexSid = 0;
    m_uiIndexFileOffset = uiIndexOffset;
    m_indexBuf = new unsigned char[BINLOG_WRITE_INDEX_CACHE_RECORD_NUM * CwxBinLogIndex::BIN_LOG_INDEX_SIZE];
    m_uiIndexLen = 0;
    m_ullMaxSid = ullSid;
}

CwxBinLogIndexWriteCache::~CwxBinLogIndexWriteCache()
{
    if (m_indexBuf) delete [] m_indexBuf;
}

///0:�ɹ���-1��д����ʧ�ܡ�
int CwxBinLogIndexWriteCache::append(CwxBinLogHeader const& header, char* szErr2K)
{
    unsigned char* pos = NULL;
    //д��index��cache
    CwxBinLogIndex index(header);
    if (m_uiIndexLen + CwxBinLogIndex::BIN_LOG_INDEX_SIZE > CwxBinLogIndex::BIN_LOG_INDEX_SIZE * BINLOG_WRITE_INDEX_CACHE_RECORD_NUM){
        if (0 != flushIndex(szErr2K)) return -1;
    }
    pos = m_indexBuf + m_uiIndexLen;
    index.serialize((char*)pos);
    m_uiIndexLen += CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
    if (0 == m_ullMinIndexSid) m_ullMinIndexSid =  header.getSid();
    m_indexSidMap[header.getSid()] = pos;
    m_ullMaxSid = header.getSid();
    return 1;
}


inline int CwxBinLogIndexWriteCache::flushIndex(char* szErr2K)
{
	if (m_uiIndexLen)
	{
		if (m_uiIndexLen != (CWX_UINT32)::pwrite(m_indexFd, m_indexBuf, m_uiIndexLen, m_uiIndexFileOffset))
		{
			if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to write index data to binlog index, errno=%d", errno);
			return -1;
		}
		m_ullPrevIndexSid = m_ullMaxSid;
		m_ullMinIndexSid = 0;
		m_uiIndexFileOffset += m_uiIndexLen;
		m_uiIndexLen = 0;
		m_indexSidMap.clear();
	}
	return 0;
}

/***********************************************************************
                    CwxBinLogFile  class
***********************************************************************/
CwxBinLogFile::CwxBinLogFile(CWX_UINT32 ttDay, CWX_UINT32 uiFileNo, CWX_UINT32 uiMaxFileSize)
{
    m_bValid = false;
    if (uiMaxFileSize<MIN_BINLOG_FILE_SIZE)
    {
        m_uiMaxFileSize = MIN_BINLOG_FILE_SIZE;
    }
    else
    {
        m_uiMaxFileSize = uiMaxFileSize;
    }
    m_ullMinSid = 0;
    m_ullMaxSid = 0;
    m_ttMinTimestamp = 0;
    m_ttMaxTimestamp = 0;
    m_uiLogNum = 0;
    m_bReadOnly = true;
    m_fd = -1;
    m_indexFd = -1;
    m_uiFileSize = 0;
    m_uiIndexFileSize = 0;
    m_uiPrevLogOffset = 0;
    m_ttDay = ttDay;
    m_uiFileNo = uiFileNo;
    m_writeCache = NULL;
}

CwxBinLogFile::~CwxBinLogFile()
{
    close();
}


//0: success, 
//-1:failure,
int CwxBinLogFile::open(char const* szPathFile,
                        bool bReadOnly,
                        bool bCreate,
                        char* szErr2K)
{
    string strIndexPathFileName;
    //�رն���
    close();
    m_bReadOnly = bCreate?false:bReadOnly;
    m_strPathFileName = szPathFile;
    m_strIndexFileName = m_strPathFileName + ".idx";

    //��ȡbinlog�ļ��Ĵ�С
    if (CwxFile::isFile(m_strPathFileName.c_str()))
    {
        m_uiFileSize = CwxFile::getFileSize(m_strPathFileName.c_str());
        if (-1 == (CWX_INT32)m_uiFileSize)
        {
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to get binlog file's size, file:%s, errno=%d", m_strPathFileName.c_str(), errno);
            return -1;
        }
    }
    else
    {
        m_uiFileSize = -1;
    }
    //��ȡbinlog�����ļ��Ĵ�С
    if (CwxFile::isFile(m_strIndexFileName.c_str()))
    {
        m_uiIndexFileSize = CwxFile::getFileSize(m_strIndexFileName.c_str());
        if (-1 == (CWX_INT32)m_uiIndexFileSize)
        {
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to get binlog index file's size, file:%s, errno=%d", m_strIndexFileName.c_str(), errno);
            return -1;
        }
    }
    else
    {
        m_uiIndexFileSize = -1;
    }

    //����binlog�ļ����������ļ�
    if (bCreate)
    {
        if (-1 == mkBinlog(szErr2K)) return -1;
    }
    else
    {
        if (-1 == (CWX_INT32)m_uiFileSize)
        {//binlog �ļ�������
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Binlog file doesn't exist, file:%s", m_strPathFileName.c_str());
            return -1;
        }
    }
    //��binlog�ļ�
    m_fd = ::open(m_strPathFileName.c_str(),  O_RDWR);
    if (-1 == m_fd)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Can't open binlog file:%s", m_strPathFileName.c_str());
        return -1;
    }
    //�������ļ�
    if (-1 != (CWX_INT32)m_uiIndexFileSize)
    {//�����ļ�����
        m_indexFd = ::open(m_strIndexFileName.c_str(),  O_RDWR);
    }
    else
    {//�����ļ�������
        m_indexFd = ::open(m_strIndexFileName.c_str(),  O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        //����index�ļ��Ĵ�СΪ0.
        m_uiIndexFileSize = 0;
    }
    if (-1 == m_indexFd)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Can't open binlog's index file:%s", m_strIndexFileName.c_str());
        return -1;
    }
    //��֤binlog�ļ��������ļ�һ��
    if (-1 == prepareFile(szErr2K)) return -1;
    //�����ֻ������ر�binlog��������io handle
    if (m_bReadOnly)
    {
        if (-1 != m_fd) ::close(m_fd);
        m_fd = -1;
        if (-1 != m_indexFd) ::close(m_indexFd);
        m_indexFd = -1;
        m_writeCache = NULL;
    }else{
        m_writeCache = new CwxBinLogIndexWriteCache(m_indexFd, m_uiIndexFileSize, getMaxSid());
    }
    m_bValid = true;
    return 0;
}

//-1��ʧ�ܣ�0����־�ļ����ˣ�1���ɹ���
int CwxBinLogFile::append(CWX_UINT64 ullSid,
                          CWX_UINT32 ttTimestamp,
                          CWX_UINT32 uiGroup,
                          char const* szData,
                          CWX_UINT32 uiDataLen,
                          char* szErr2K)
{
//    char szBuf[CwxBinLogHeader::BIN_LOG_HEADER_SIZE];
    if (m_uiFileSize + FREE_BINLOG_FILE_SIZE + CwxBinLogHeader::BIN_LOG_HEADER_SIZE + uiDataLen >= m_uiMaxFileSize) return 0; //it's full.

    if (this->m_bReadOnly)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "The log is opened in read mode, can't append record.file:%s", m_strPathFileName.c_str());
        return -1;
    }
    if (!m_bValid)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "The log is not valid, can't append record. file:%s", m_strPathFileName.c_str());
        return -1;
    }
    //sid��������
    if (!m_uiLogNum)
    {
        if (ullSid <= m_ullMaxSid)
        {
            char szBuf1[64];
            char szBuf2[64];
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "binlog[%s]'s sid [%s] is more than the appending binlog's sid[%s]",
                m_strPathFileName.c_str(),
                CwxCommon::toString(m_ullMaxSid, szBuf1),
                CwxCommon::toString(ullSid, szBuf2));
            return -1;
        }
    }
    CwxBinLogHeader header(ullSid,
		m_uiLogNum,
        (CWX_UINT32)ttTimestamp,
        m_uiFileSize,
        uiDataLen,
        m_uiPrevLogOffset,
        uiGroup);

	char szBuf[CwxBinLogHeader::BIN_LOG_HEADER_SIZE];
	header.serialize(szBuf);
	if (CwxBinLogHeader::BIN_LOG_HEADER_SIZE != pwrite(m_fd, szBuf, CwxBinLogHeader::BIN_LOG_HEADER_SIZE, m_uiFileSize))
	{
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to write log header to binlog:%s, errno=%d", m_strPathFileName.c_str(), errno);
		return -1;
	}
	if (uiDataLen)
	{
		if ((int)uiDataLen != pwrite(m_fd, szData, uiDataLen, m_uiFileSize + CwxBinLogHeader::BIN_LOG_HEADER_SIZE))
		{
			if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to write log data to binlog:%s, errno=%d", m_strPathFileName.c_str(), errno);
			return -1;
		}
	}
    int ret = m_writeCache->append(header, szErr2K);
    if (-1 == ret){
        m_bValid = false;
        return -1;		
    }

    ///����ǰһ��binlog���ļ�offset
    m_uiPrevLogOffset = m_uiFileSize;
    ///�޸��ļ��Ĵ�С
    m_uiFileSize += CwxBinLogHeader::BIN_LOG_HEADER_SIZE + uiDataLen;
    ///�޸������ļ��Ĵ�С
    m_uiIndexFileSize += CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
    //����sid��ʱ���
    m_ullMaxSid = ullSid;
    m_ttMaxTimestamp = ttTimestamp; ///ʱ��Ӧ��������ģ�����������˵��ʱ�����˵���������������ֵ�����
    if (!m_uiLogNum)
    {
        m_ullMinSid = ullSid;
        m_ttMinTimestamp = ttTimestamp;
    }
	if (m_ttMinTimestamp > ttTimestamp) ///����ǰʱ��С����Сʱ�䣬���޸���Сʱ��
	{
		m_ttMinTimestamp = ttTimestamp;
	}
    //��¼����1
    m_uiLogNum ++;
    return 1;
}

/**
@brief ȷ����cache������д�뵽Ӳ��
@param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
@return -1��ʧ�ܣ�0���ɹ���
*/
int CwxBinLogFile::flush_cache(char* szErr2K)
{
    if (this->m_bReadOnly)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "The log is opened in read mode, can't commit.file:%s", m_strPathFileName.c_str());
        return -1;
    }
    if (!m_bValid)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "The log is not valid, can't commit. file:%s", m_strPathFileName.c_str());
        return -1;
    }
    //flush data 
    if (0 != m_writeCache->flushIndex(szErr2K))
    {
        m_bValid = false;
        return -1;
    }
    return 0;
}

int CwxBinLogFile::fsync(bool bFlushAll, char*)
{
    if (-1 != m_fd)	::fdatasync(m_fd);
	if (bFlushAll && (-1 != m_indexFd)){
		::fdatasync(m_indexFd);
	}
    return 0;
}

int CwxBinLogFile::upper(CWX_UINT64 ullSid, CwxBinLogIndex& item, char* szErr2K)
{
	CWX_ASSERT(m_bValid);
	if (!m_bValid)
	{
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "CwxBinlogFile is invalid, file:%s", m_strPathFileName.c_str());
		return -1;
	}
	if (!m_uiLogNum) return 0;

	if (ullSid >= m_ullMaxSid)
	{
		return 0;///������
	}
    if (m_writeCache && m_writeCache->m_indexSidMap.size())
    {
        if (m_writeCache->m_ullPrevIndexSid <= ullSid)
        {//��ullSid���sid��һ����write cache�С�
            map<CWX_UINT64/*sid*/, unsigned char*>::const_iterator iter = m_writeCache->m_indexSidMap.upper_bound(ullSid);
            CWX_ASSERT((iter != m_writeCache->m_indexSidMap.end()));
            item.unserialize((char const*)iter->second);
            return 1;
        }
    }
	//����ָ����SID��λ
	int fd = -1;
	CWX_UINT32 uiOffset = 0;
	//�۰����
	fd = ::open(m_strIndexFileName.c_str(), O_RDONLY);
	if (-1 == fd)
	{
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to open index file:%s, errno=%d", m_strIndexFileName.c_str(), errno);
		return -1;
	}
	if (ullSid < m_ullMinSid)
	{
		if (0 != readIndex(fd, item, 0, szErr2K))
		{
			::close(fd);
			return -1;
		}
		::close(fd);
		return 1;
	}

	CWX_UINT32 uiStart = 0;
	CWX_UINT32 uiEnd = m_uiLogNum - 1 - (m_writeCache?m_writeCache->m_indexSidMap.size():0);
	CWX_UINT32 uiMid = 0;
	while(uiEnd >= uiStart)
	{
		uiMid = (uiStart + uiEnd)/2;
		uiOffset = uiMid;
		uiOffset *= CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
		if (0 != readIndex(fd, item, uiOffset, szErr2K))
		{
			::close(fd);
			return -1;
		}
		if (ullSid == item.getSid())
		{
			break;
		}
		else if (ullSid < item.getSid())
		{
			uiEnd = uiMid-1;
		}
		else
		{
			uiStart = uiMid+1;
		}
	}
	if (ullSid >= item.getSid())
	{//next item
		uiOffset += CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
		if (0 != readIndex(fd, item, uiOffset, szErr2K))
		{
			::close(fd);
			return -1;
		}
	}
	::close(fd);
	return 1;
}

// -1��ʧ�ܣ�0�������ڣ�1������
int CwxBinLogFile::lower(CWX_UINT64 ullSid, CwxBinLogIndex& item, char* szErr2K)
{
	CWX_ASSERT(m_bValid);
	if (!m_bValid)
	{
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "CwxBinlogFile is invalid, file:%s", m_strPathFileName.c_str());
		return -1;
	}
	if (!m_uiLogNum) return 0;
	if (ullSid < m_ullMinSid)
	{
		return 0;///������
	}
    if (m_writeCache && m_writeCache->m_indexSidMap.size())
    {
        if (m_writeCache->m_ullMinIndexSid < ullSid)
        {//������ullSid���sid��һ����write cache�С�
            map<CWX_UINT64/*sid*/, unsigned char*>::const_iterator iter = m_writeCache->m_indexSidMap.lower_bound(ullSid);
            if(iter == m_writeCache->m_indexSidMap.end()){///ȡ���һ��
                iter = m_writeCache->m_indexSidMap.find(m_writeCache->m_ullMaxSid);
            }
            if (iter->first >= ullSid) iter--;
            item.unserialize((char const*)iter->second);
            return 1;
        }
    }

	//����ָ����SID��λ
	int fd = -1;
	CWX_UINT32 uiOffset = 0;
	//�۰����
	fd = ::open(m_strIndexFileName.c_str(), O_RDONLY);
	if (-1 == fd)
	{
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to open index file:%s, errno=%d", m_strIndexFileName.c_str(), errno);
		return -1;
	}
	if (ullSid >= m_ullMaxSid)
	{
		//��ȡ���һ��
		if (0 != readIndex(fd, item, (m_uiLogNum - 1) * CwxBinLogIndex::BIN_LOG_INDEX_SIZE, szErr2K))
		{
			::close(fd);
			return -1;
		}
		::close(fd);
		return 1;
	}

	CWX_UINT32 uiStart = 0;
	CWX_UINT32 uiEnd = m_uiLogNum - 1 - (m_writeCache?m_writeCache->m_indexSidMap.size():0);
	CWX_UINT32 uiMid = 0;
	while(uiEnd >= uiStart)
	{
		uiMid = (uiStart + uiEnd)/2;
		uiOffset = uiMid;
		uiOffset *= CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
		if (0 != readIndex(fd, item, uiOffset, szErr2K))
		{
			::close(fd);
			return -1;
		}
		if (ullSid == item.getSid())
		{
			break;
		}
		else if (ullSid < item.getSid())
		{
			uiEnd = uiMid-1;
		}
		else
		{
			uiStart = uiMid+1;
		}
	}
	if (ullSid < item.getSid())
	{//next item
		uiOffset -= CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
		if (0 != readIndex(fd, item, uiOffset, szErr2K))
		{
			::close(fd);
			return -1;
		}
	}
	::close(fd);
	return 1;
}


//-2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0�������ڣ�1����λ��ָ����λ��
int CwxBinLogFile::seek(CwxBinLogCursor& cursor, CWX_UINT8 ucMode)
{
    CWX_ASSERT(m_bValid);
	if (!m_bValid)
	{
		CwxCommon::snprintf(cursor.getErrMsg(), 2047, "CwxBinlogFile is invalid, file:%s", m_strPathFileName.c_str());
		return -1;
	}
    int iRet = cursor.open(m_strPathFileName.c_str(), m_uiFileNo, m_ttDay);
    if (-1 == iRet) return -1;

	if (SEEK_START == ucMode){
		return cursor.seek(0);
	}
	if (SEEK_TAIL == ucMode){
		return cursor.seek(m_uiPrevLogOffset);
	}

	CwxBinLogIndex item;
	iRet = upper(cursor.getSeekSid(), item, cursor.getErrMsg());
	if (1 != iRet) return iRet;
	return cursor.seek(item.getOffset());
}

///������trim��ָ����sid��0���ɹ���-1��ʧ��
int CwxBinLogFile::trim(CWX_UINT64 ullSid, char* szErr2K)
{
	CWX_ASSERT(m_bValid);
	if (!m_bValid)
	{
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "CwxBinlogFile is invalid, file:%s", m_strPathFileName.c_str());
		return -1;
	}
	CwxBinLogCursor cursor;
	int iRet = cursor.open(m_strPathFileName.c_str(), m_uiFileNo, m_ttDay);
	if (-1 == iRet) return -1;

	CwxBinLogIndex item;
	iRet = lower(ullSid, item, szErr2K);
	if (-1 == iRet) return iRet;
	
	if (1 == iRet)
	{
		iRet = cursor.seek(item.getOffset());
		if (1 != iRet){
			if (szErr2K){
				if ((0 == iRet) || (-2 == iRet)){
					CwxCommon::snprintf(szErr2K, 2047, "File size is less %u", item.getOffset() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE);
				}else{
					strcpy(szErr2K, cursor.getErrMsg());
				}
			}
			return -1;
		}
		///����ǰһ��binlog���ļ�offset
		m_uiPrevLogOffset = cursor.getHeader().getOffset();
		///�޸��ļ��Ĵ�С
		m_uiFileSize = cursor.getHeader().getOffset() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE + cursor.getHeader().getLogLen();
		///�޸������ļ��Ĵ�С
		m_uiIndexFileSize = (cursor.getHeader().getLogNo() + 1) * CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
		//����sid��ʱ���
		m_ullMaxSid = cursor.getHeader().getSid();
		m_ttMaxTimestamp = cursor.getHeader().getDatetime();
		m_uiLogNum = cursor.getHeader().getLogNo() + 1;
	}else{
		///����ǰһ��binlog���ļ�offset
		m_uiPrevLogOffset = 0;
		///�޸��ļ��Ĵ�С
		m_uiFileSize = 0;
		///�޸������ļ��Ĵ�С
		m_uiIndexFileSize = 0;
		//����sid��ʱ���
		m_ullMaxSid = 0;
		m_ullMinSid = 0;
		m_ttMaxTimestamp = 0;
		m_ttMinTimestamp = 0;
		m_uiLogNum = 0;
	}
	//�������ļ�
	int fd = ::open(m_strIndexFileName.c_str(),  O_RDWR);
	if (-1 == fd){
		m_bValid = true;
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to open index file:%s, errno=%d", m_strIndexFileName.c_str(), errno);
		return -1;
	}
	ftruncate(fd, CwxBinLogIndex::BIN_LOG_INDEX_SIZE * (cursor.getHeader().getLogNo() + 1));
	::close(fd);
	fd = ::open(m_strPathFileName.c_str(), O_RDWR);
	if (-1 == fd){
		m_bValid = true;
		if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to open file:%s, errno=%d", m_strPathFileName.c_str(), errno);
		return -1;
	}
	ftruncate(fd, cursor.getHeader().getOffset() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE + cursor.getHeader().getLogLen());
	::close(fd);
	return 1;
}



void CwxBinLogFile::reset()
{
    m_bValid = false;
    m_strPathFileName.erase();
    m_strIndexFileName.erase();
    m_ullMinSid = 0;
    m_ullMaxSid = 0;
    m_ttMinTimestamp = 0;
    m_ttMaxTimestamp = 0;
    m_uiLogNum = 0;
    m_bReadOnly = true;
    m_fd = -1;
    m_indexFd = -1;
    m_uiFileSize = 0;
    m_uiIndexFileSize = 0;
    m_uiPrevLogOffset = 0;
    if (m_writeCache) delete m_writeCache;
    m_writeCache = NULL;
}

void CwxBinLogFile::remove(char const* szPathFileName)
{
    //ɾ��binlog�ļ�
    CwxFile::rmFile(szPathFileName);
    //ɾ�������ļ�
    string strIndexFile=szPathFileName;
    strIndexFile += ".idx";
    CwxFile::rmFile(strIndexFile.c_str());
}

//�ر�
void CwxBinLogFile::close()
{
	if (!m_bReadOnly)
	{
        flush_cache(NULL);
		fsync(true, NULL);
	}
    if (-1 != m_fd) ::close(m_fd);
    m_fd = -1;
    if (-1 != m_indexFd) ::close(m_indexFd);
    m_indexFd = -1;
    reset();
}

bool CwxBinLogFile::getLastSidByNo(CWX_UINT32 uiNo, CWX_UINT64& ullSid, char* szErr2K){
    if (m_uiLogNum < uiNo){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "File's record num[%u] is less than the num[%u], file:%s", m_uiLogNum, uiNo, m_strPathFileName.c_str());
        return false;
    }
    if (0 == uiNo) return m_ullMaxSid;
    CwxBinLogIndex index;
    if (m_writeCache){
        CWX_UINT32 uiCacheNum = m_writeCache->m_uiIndexLen/CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
        if (uiCacheNum >= uiNo){
            index.unserialize((char const*)(m_writeCache->m_indexBuf + (uiCacheNum - uiNo) * CwxBinLogIndex::BIN_LOG_INDEX_SIZE));
            ullSid = index.getSid();
            return true;
        }
    }
    ///���ļ���
    int fd = -1;
    //�۰����
    fd = ::open(m_strIndexFileName.c_str(), O_RDONLY);
    if (-1 == fd){
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to open file:%s, errno=%d", m_strPathFileName.c_str(), errno);
        return false;
    }
    if (0 != readIndex(fd, index, (m_uiLogNum - uiNo) * CwxBinLogIndex::BIN_LOG_INDEX_SIZE, szErr2K)){
        ::close(fd);
        return false;
    }
    ullSid = index.getSid();
    ::close(fd);
    return true;
}

int CwxBinLogFile::mkBinlog(char* szErr2K)
{
    //pretect the sync-log file
    if ((-1 != (CWX_INT32)m_uiFileSize) && (m_uiFileSize > CwxBinLogHeader::BIN_LOG_HEADER_SIZE * 2))
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "binlog file [%s] exists, can't create.", m_strPathFileName.c_str());
        return -1;
    }
    int fd=-1;
    //������ļ����ݵķ�ʽ���ļ�
    fd = ::open(m_strPathFileName.c_str(),  O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if (-1 == fd)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to create binlog file [%s] .", m_strPathFileName.c_str());
        return -1;
    }
    //���������ļ���СΪ0
    m_uiFileSize = 0;
    ::close(fd);

    //������ļ����ݵķ�ʽ���ļ�
    fd = ::open(m_strIndexFileName.c_str(),  O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    if (-1 == fd)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to create binlog's index file [%s] .", m_strIndexFileName.c_str());
        return -1;
    }
    //���������ļ���СΪ0
    m_uiIndexFileSize = 0; 
    ::close(fd);
    return 0;
}

//-1��ʧ�ܣ�0���ɹ���
int CwxBinLogFile::prepareFile(char* szErr2K)
{
    int iRet = isRebuildIndex(szErr2K);
    if (-1 == iRet) return -1;
    if (1 == iRet)
    {//rebuilt index
        if (0 != createIndex(szErr2K)) return -1;
    }
    //��ȡbinlog����
    CWX_ASSERT(!(m_uiIndexFileSize%CwxBinLogIndex::BIN_LOG_INDEX_SIZE));
    m_uiLogNum = m_uiIndexFileSize /CwxBinLogIndex::BIN_LOG_INDEX_SIZE;

    //�������һ����¼�Ŀ�ʼλ��
    m_uiPrevLogOffset = 0;

    if (m_uiLogNum)
    {//��¼����
        //��ȡ��С��sid��timestamp
        CwxBinLogIndex index;
        if (0 != readIndex(m_indexFd, index, 0, szErr2K)) return -1;
        m_ullMinSid = index.getSid();
        m_ttMinTimestamp = index.getDatetime();
        //��ȡ����sid, timestamp
        if (0 != readIndex(m_indexFd, index, m_uiIndexFileSize - CwxBinLogIndex::BIN_LOG_INDEX_SIZE)) return -1;
        m_ullMaxSid = index.getSid();
        m_ttMaxTimestamp = index.getDatetime();
        m_uiPrevLogOffset = index.getOffset();
        if (m_ullMinSid > m_ullMaxSid)
        {
            char szBuf1[64];
            char szBuf2[64];
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Binlog file's min-sid[%s] more than max sid[%s], file:%s",
                CwxCommon::toString(m_ullMinSid, szBuf1),
                CwxCommon::toString(m_ullMaxSid, szBuf2),
                m_strPathFileName.c_str());
            return -1;
        }
    }
    else
    {
        m_ullMinSid = 0;
        m_ullMaxSid = 0;
        m_ttMinTimestamp = 0;
        m_ttMaxTimestamp = 0;
    }
    return 0;
}

//-1��ʧ�ܣ�0������Ҫ��1����Ҫ��
int CwxBinLogFile::isRebuildIndex(char* szErr2K)
{
    //������������
    if (m_uiIndexFileSize%CwxBinLogIndex::BIN_LOG_INDEX_SIZE) return 1;

    //����Ϊ�գ������ݲ�Ϊ��
    if (!m_uiIndexFileSize) return 1;

    //���������¼��binlog�ļ���С��binlog�ļ�����Ĵ�С��ϵ
    char szBuf[CwxBinLogIndex::BIN_LOG_INDEX_SIZE];
    CwxBinLogIndex index;
    //��ȡ����sid, timestamp
    if (CwxBinLogIndex::BIN_LOG_INDEX_SIZE != pread(m_indexFd,
        &szBuf,
        CwxBinLogIndex::BIN_LOG_INDEX_SIZE,
        m_uiIndexFileSize - CwxBinLogIndex::BIN_LOG_INDEX_SIZE))
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to read binlog index, file:%s, errno=%d", this->m_strIndexFileName.c_str(), errno);
        return -1;
    }
    
    index.unserialize(szBuf);
    //�������Ҫ�ؽ�
    if (index.getOffset() + index.getLogLen() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE != m_uiFileSize) return 1;
    //����Ҫ�ؽ�
    return 0;
}

//-1��ʧ�ܣ�0���ɹ���
int CwxBinLogFile::createIndex(char* szErr2K)
{
    CwxBinLogIndex index;
    CwxBinLogCursor* cursor = new CwxBinLogCursor();
    int iRet = cursor->open(m_strPathFileName.c_str(), m_uiFileNo, m_ttDay);
    m_uiIndexFileSize = 0;
    if (-1 == iRet)
    {
        if (szErr2K) strcpy(szErr2K, cursor->getErrMsg());
		delete cursor;
        return -1;
    }

    while(1 == (iRet = cursor->next()))
    {
        index = cursor->getHeader();
        if (0 != writeIndex(m_indexFd, index, m_uiIndexFileSize, szErr2K)) return -1;
        m_uiIndexFileSize += CwxBinLogIndex::BIN_LOG_INDEX_SIZE;
    }
    if (-1 == iRet)
    {
        if (szErr2K) strcpy(szErr2K, cursor->getErrMsg());
		delete cursor;
        return -1;
    }
    if (m_uiIndexFileSize)
    {//������Ч��binlog
        m_uiFileSize = index.getOffset() + index.getLogLen() + CwxBinLogHeader::BIN_LOG_HEADER_SIZE;
    }
    else
    {//��������Ч��binlog
        m_uiFileSize = 0;
    }
    //truncate binlog �ļ�
	CWX_INFO(("Truncate file %s to size %u", m_strPathFileName.c_str(), m_uiFileSize));
    ftruncate(m_fd, m_uiFileSize);
    //truncate index �ļ�
	CWX_INFO(("Truncate file %s to size %u", m_strIndexFileName.c_str(), m_uiIndexFileSize));
    ftruncate(m_indexFd, m_uiIndexFileSize);
	delete cursor;
    return 0;
}


/***********************************************************************
                    CwxBinLogMgr  class
***********************************************************************/

CwxBinLogMgr::CwxBinLogMgr(char const* szLogPath,
                           char const* szFilePrex,
                           CWX_UINT32 uiMaxFileSize,
                           CWX_UINT32 uiBinlogFlushNum,
                           CWX_UINT32 uiBinlogFlushSecond,
                           bool bDelOutManageLogFile)
{
    m_bValid = false;
    strcpy(m_szErr2K, "Not init.");
    m_pCurBinlog = NULL;
    m_strLogPath = szLogPath;
    if ('/' !=m_strLogPath[m_strLogPath.length() - 1]) m_strLogPath += "/";
    m_strPrexLogPath = m_strLogPath + szFilePrex + "/";
    m_strFilePrex = szFilePrex;
    m_uiMaxFileSize = uiMaxFileSize;
	if (m_uiMaxFileSize > MAX_BINLOG_FILE_SIZE) m_uiMaxFileSize = MAX_BINLOG_FILE_SIZE;
    m_uiMaxFileNum = DEF_MANAGE_FILE_NUM;
	m_bCache = true;
    m_bDelOutManageLogFile = bDelOutManageLogFile;
    m_fdLock = -1;
    m_ullMinSid = 0; ///<binlog�ļ�����Сsid
    m_ullMaxSid = 0; ///<binlog�ļ������sid
    m_ttMinTimestamp = 0; ///<binlog�ļ���log��ʼʱ��
    m_ttMaxTimestamp = 0; ///<binlog�ļ���log����ʱ��
    m_ullNextSid = 0;
    m_uiFlushBinLogNum = (uiBinlogFlushNum==0?1:uiBinlogFlushNum);
    m_uiFlushBinLogTime = (uiBinlogFlushSecond==0?1:uiBinlogFlushSecond); ///<����ʱ���Զ�flush
    m_uiUnFlushBinlog = 0;
    m_ttLastFlushBinlogTime = time(NULL);
}

CwxBinLogMgr::~CwxBinLogMgr()
{
    if (m_pCurBinlog){
        m_pCurBinlog->flush_cache(NULL);
        m_pCurBinlog->fsync(true, NULL);
    }
	map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.begin();
	while(iter != m_binlogMap.end()){
		delete iter->second;
		iter++;
	}
    m_binlogMap.clear();
	set<CwxBinLogCursor*>::iterator set_iter =  m_cursorSet.begin();
	while(set_iter != m_cursorSet.end()){
		delete *set_iter;
		set_iter++;
	}
}

// -1��ʧ�ܣ�0���ɹ���
int CwxBinLogMgr::init(CWX_UINT32 uiMaxFileNum, bool bCache, char* szErr2K)
{
    ///д������
    CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
    this->_clear();
    m_bValid = false;
    strcpy(m_szErr2K, "Not init.");
    if (uiMaxFileNum < MIN_MANAGE_FILE_NUM) m_uiMaxFileNum = MIN_MANAGE_FILE_NUM;
    if (uiMaxFileNum > MAX_MANAGE_FILE_NUM) m_uiMaxFileNum = MAX_MANAGE_FILE_NUM;
    m_uiMaxFileNum = uiMaxFileNum;

	m_bCache = bCache;

    //���binlog��Ŀ¼�����ڣ��򴴽���Ŀ¼
    if (!CwxFile::isDir(m_strLogPath.c_str()))
    {
        if (!CwxFile::createDir(m_strLogPath.c_str()))
        {
            CwxCommon::snprintf(m_szErr2K, 2047, "Failure to create binlog path:%s, errno=%d", m_strLogPath.c_str(), errno);
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
    }
    if (!CwxFile::isDir(m_strPrexLogPath.c_str()))
    {
        if (!CwxFile::createDir(m_strPrexLogPath.c_str()))
        {
            CwxCommon::snprintf(m_szErr2K, 2047, "Failure to create binlog path:%s, errno=%d", m_strPrexLogPath.c_str(), errno);
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
    }
    //��ȡϵͳ���ļ�
    string strLockFile=m_strLogPath + m_strFilePrex + ".lock";
    if (!CwxFile::isFile(strLockFile.c_str()))
    {///�������ļ�
        m_fdLock = ::open(strLockFile.c_str(),  O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
    }
    else
    {
        m_fdLock = ::open(strLockFile.c_str(),  O_RDWR);
    }
    if (-1 == m_fdLock)
    {
        CwxCommon::snprintf(m_szErr2K, 2047, "Failure to open  binlog lock file:%s, errno=%d", strLockFile.c_str(), errno);
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }
    if (!CwxFile::lock(m_fdLock))
    {
        CwxCommon::snprintf(m_szErr2K, 2047, "Failure to lock  binlog lock file:%s, errno=%d", strLockFile.c_str(), errno);
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }

    //��ȡĿ¼�µ������ļ�
    list<string> files;
    if (!CwxFile::getDirFile(m_strPrexLogPath, files))
    {
        CwxCommon::snprintf(m_szErr2K, 2047, "Failure to create binlog path:%s, errno=%d", m_strPrexLogPath.c_str(), errno);
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }
    //��ȡĿ¼�µ�����binlog�ļ������ŵ�map�У�����map������������ļ�
    string strPathFile;
    list<string>::iterator iter=files.begin();
    map<CwxBinLogFileItem, string> fileMap;
    CWX_UINT32 ttDay = 0;
    CWX_UINT32 uiFileNo = 0;
    while(iter != files.end())
    {
        strPathFile = m_strPrexLogPath + *iter;
        if (isBinLogFile(strPathFile))
        {
            uiFileNo = getBinLogFileNo(strPathFile, ttDay);
			if (uiFileNo >= START_FILE_NUM){
				CwxBinLogFileItem item(ttDay, uiFileNo);
				fileMap[item] = strPathFile;
			}
        }
        iter++;
    }
    map<CwxBinLogFileItem, string>::reverse_iterator map_iter = fileMap.rbegin();

    CwxBinLogFile* pBinLogFile = NULL;
    while(map_iter != fileMap.rend())
    {
        pBinLogFile = new CwxBinLogFile(map_iter->first.getDay(), map_iter->first.getFileNo(), m_uiMaxFileSize);
        if (0 != pBinLogFile->open(map_iter->second.c_str(),
            m_pCurBinlog?true:false,
            false,
            m_szErr2K))
        {
            delete pBinLogFile;
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
		if (!pBinLogFile->getLogNum()){///�������ļ���ɾ��
			CWX_INFO(("Remove binlog file for empty, file:%s", pBinLogFile->getDataFileName().c_str()));
			CwxBinLogFile::remove(pBinLogFile->getDataFileName().c_str());
			delete pBinLogFile;
			map_iter++;
			continue;
		}
        if (!m_pCurBinlog){
            m_ttMaxTimestamp = pBinLogFile->getMaxTimestamp();
            m_ttMinTimestamp = pBinLogFile->getMinTimestamp();
        }else{
			//������С��ʱ���
			m_ttMinTimestamp = pBinLogFile->getMinTimestamp();
		}
        if (!m_pCurBinlog){
            m_pCurBinlog = pBinLogFile;
            m_ullMaxSid = pBinLogFile->getMaxSid();
            m_ullMinSid = pBinLogFile->getMinSid();
        }else{
            ///���ս�����ȡ�ļ�����Ϊ���������binlog�ļ���sidӦ��С�����е�sid
            if (pBinLogFile->getMaxSid() >= getMinSid()){
                char szBuf1[64];
                char szBuf2[64];
                CwxCommon::snprintf(m_szErr2K, 2047, "BinLog file[%s]'s max sid[%s] is more than the existing min sid[%s]",
                    pBinLogFile->getDataFileName().c_str(),
                    CwxCommon::toString(pBinLogFile->getMaxSid(), szBuf1),
                    CwxCommon::toString(getMinSid(), szBuf2)
                    );
                delete pBinLogFile;
                if (szErr2K) strcpy(szErr2K, m_szErr2K);
                return -1;
            }
            m_ullMinSid = pBinLogFile->getMinSid();
        }
        m_binlogMap[pBinLogFile->getFileNo()] = pBinLogFile;
        map_iter++;
    }
    _outputManageBinLog();
    m_bValid = true;
    m_szErr2K[0] = 0x00;
    return 0;
}

//-1��ʧ�ܣ�0���ɹ���
int CwxBinLogMgr::append(CWX_UINT64& ullSid,
                         CWX_UINT32 ttTimestamp,
                         CWX_UINT32 uiGroup,
                         char const* szData,
                         CWX_UINT32 uiDataLen,
                         char* szErr2K)
{
    bool bNeedFlush = false;
    {
        ///д������
        CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
        if(!m_bValid){
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
        if (0 == ullSid){
            m_ullNextSid ++;
            ullSid = m_ullNextSid;
        }
        int ret = _append(ullSid, ttTimestamp, uiGroup, szData, uiDataLen, szErr2K);
        if (0 != ret) return -1;
        m_uiUnFlushBinlog++;
        CWX_UINT32 uiNow = time(NULL);
        if ((m_uiUnFlushBinlog > m_uiFlushBinLogNum) ||
            (m_ttLastFlushBinlogTime + m_uiFlushBinLogTime <= uiNow))
        {
            bNeedFlush = true;
            m_uiUnFlushBinlog = 0;
            m_ttLastFlushBinlogTime = uiNow;
        }
    }
    if (bNeedFlush){
        return commit(true, szErr2K);
    }
    return 0;

}

int CwxBinLogMgr::_append(CWX_UINT64 ullSid,
            CWX_UINT32 ttTimestamp,
            CWX_UINT32 uiGroup,
            char const* szData,
            CWX_UINT32 uiDataLen,
            char* szErr2K)
{
    //���û��binlog�ļ����򴴽���ʼbinlog�ļ�����ʼ�ļ����Ϊ0
    if (!m_pCurBinlog)
    {
        string strPathFile;
        m_pCurBinlog = new CwxBinLogFile(CwxDate::trimToDay(ttTimestamp), START_FILE_NUM, m_uiMaxFileSize);
        getFileNameByFileNo(START_FILE_NUM, m_pCurBinlog->getFileDay(), strPathFile);
        if (0 != m_pCurBinlog->open(strPathFile.c_str(),
            false,
            true,
            m_szErr2K))
        {
            delete m_pCurBinlog;
            m_pCurBinlog = NULL;
            m_bValid = false;
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
        m_binlogMap[m_pCurBinlog->getFileNo()] = m_pCurBinlog;
    }

    if (!m_bValid)
    {
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Binlog manage is invalid, error:%s", m_szErr2K);
        return -1;
    }

    if (ullSid <= m_ullMaxSid)
    {
        char szBuf1[64];
        char szBuf2[64];
        CwxCommon::snprintf(m_szErr2K, 2047, "Log's sid[%s] is no more than max sid[%s]", CwxCommon::toString(ullSid, szBuf1), CwxCommon::toString(m_ullMaxSid, szBuf2));
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }
    if (MIN_SID_NO > ullSid)
    {
        char szBuf1[64];
        char szBuf2[64];
        CwxCommon::snprintf(m_szErr2K, 2047, "Log's sid[%s] is less than min sid[%s]", CwxCommon::toString(ullSid, szBuf1), CwxCommon::toString((CWX_UINT64)MIN_SID_NO, szBuf2));
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }

    if ((CWX_UINT32)CwxDate::trimToDay(ttTimestamp) > m_pCurBinlog->getFileDay())///new day
    {
        string strPathFile;
        CWX_UINT32 uiFileNum = m_pCurBinlog->getFileNo() + 1; 
        getFileNameByFileNo(uiFileNum, CwxDate::trimToDay(ttTimestamp), strPathFile);
        CwxBinLogFile* pBinLogFile = new CwxBinLogFile(CwxDate::trimToDay(ttTimestamp), uiFileNum, m_uiMaxFileSize);
        if (0 != pBinLogFile->open(strPathFile.c_str(),
            false,
            true,
            m_szErr2K))
        {
            delete pBinLogFile;
            m_bValid = false;
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
        ///���õ�ǰ��binlogΪֻ��
        m_pCurBinlog->setReadOnly();
        m_pCurBinlog = pBinLogFile;
        ///����ǰ��binlog���ŵ�binlog map��
        m_binlogMap[m_pCurBinlog->getFileNo()] = m_pCurBinlog;

        //����Ƿ��г�������Χ��binlog�ļ�
        while(m_binlogMap.size())
        {
            if (_isManageBinLogFile(m_binlogMap.begin()->second)) break;
            if (m_bDelOutManageLogFile)
            {
                CWX_INFO(("Remove binlog file for outdate, file:%s", m_binlogMap.begin()->second->getDataFileName().c_str()));
                CwxBinLogFile::remove(m_binlogMap.begin()->second->getDataFileName().c_str());
            }
            delete m_binlogMap.begin()->second;
            m_binlogMap.erase(m_binlogMap.begin());
        }
        if (m_binlogMap.size()){
            m_ullMinSid = m_binlogMap.begin()->second->getMinSid();
            m_ttMinTimestamp = m_binlogMap.begin()->second->getMinTimestamp();
        }else{
            m_ullMinSid = m_pCurBinlog->getMinSid();
            m_ttMinTimestamp =m_pCurBinlog->getMinTimestamp();
        }
        ///������������binlog�ļ���Ϣ
        _outputManageBinLog();
    }

    int iRet = m_pCurBinlog->append(ullSid,
        ttTimestamp,
        uiGroup,
        szData,
        uiDataLen,
        m_szErr2K);
    if (-1 == iRet)
    {
        if (!m_pCurBinlog->m_bValid)
        {
            m_bValid = false;
        }
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }
    if (1 == iRet)
    {
        m_ullMaxSid = ullSid;
        m_ttMaxTimestamp = ttTimestamp;
        if ( 0 == m_ullMinSid)
        {
            m_ullMinSid = ullSid;
            m_ttMinTimestamp = ttTimestamp;
        }
        //���ܻ����ʱ��
        if (m_ttMinTimestamp > ttTimestamp) m_ttMinTimestamp = ttTimestamp;
        return 0;
    }
    if (0 == iRet)
    {
        string strPathFile;
        CWX_UINT32 uiFileNum = m_pCurBinlog->getFileNo() + 1;
        getFileNameByFileNo(uiFileNum,
            m_pCurBinlog->getFileDay(),
            strPathFile);
        CwxBinLogFile* pBinLogFile = new CwxBinLogFile(m_pCurBinlog->getFileDay(),
            uiFileNum, 
            m_uiMaxFileSize);
        if (0 != pBinLogFile->open(strPathFile.c_str(),
            false,
            true,
            m_szErr2K))
        {
            delete pBinLogFile;
            m_bValid = false;
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return -1;
        }
        ///���õ�ǰ��binlogΪֻ��
        m_pCurBinlog->setReadOnly();
        m_pCurBinlog = pBinLogFile;
        ///����ǰ��binlog���ŵ�binlog map��
        m_binlogMap[m_pCurBinlog->getFileNo()] = m_pCurBinlog;
        //����Ƿ��г�������Χ��binlog�ļ�
        while(m_binlogMap.size())
        {
            if (_isManageBinLogFile(m_binlogMap.begin()->second)) break;
            if (m_bDelOutManageLogFile)
            {
                CWX_INFO(("Remove binlog file for outdate, file:%s", m_binlogMap.begin()->second->getDataFileName().c_str()));
                CwxBinLogFile::remove(m_binlogMap.begin()->second->getDataFileName().c_str());
            }
            delete m_binlogMap.begin()->second;
            m_binlogMap.erase(m_binlogMap.begin());
        }
        if (m_binlogMap.size())
        {
            m_ullMinSid = m_binlogMap.begin()->second->getMinSid();
            m_ttMinTimestamp = m_binlogMap.begin()->second->getMinTimestamp();
        }
        else
        {
            m_ullMinSid = m_pCurBinlog->getMinSid();
            m_ttMinTimestamp =m_pCurBinlog->getMinTimestamp();
        }
        ///������������binlog�ļ���Ϣ
        _outputManageBinLog();
    }
    ///����¼������ӵ���binlog�ļ���
    iRet = m_pCurBinlog->append(ullSid,
        ttTimestamp,
        uiGroup,
        szData,
        uiDataLen,
        m_szErr2K);
    if (-1 == iRet)
    {
        if (!m_pCurBinlog->m_bValid)
        {
            m_bValid = false;
        }
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
        return -1;
    }
    if (1 == iRet)
    {
        m_ullMaxSid = ullSid;
        m_ttMaxTimestamp = ttTimestamp;
        if ( 0 == m_ullMinSid)
        {
            m_ullMinSid = ullSid;
            m_ttMinTimestamp = ttTimestamp;
        }
        //���ܻ����ʱ��
        if (m_ttMinTimestamp > ttTimestamp) m_ttMinTimestamp = ttTimestamp;
        return 0;
    }
    ///���ļ��޷�����һ����¼
    CwxCommon::snprintf(m_szErr2K, 2047, "Binlog's length[%d] is too large, can't be put binlog file", uiDataLen);
    if (szErr2K) strcpy(szErr2K, m_szErr2K);
    m_bValid = false;
    return -1;

}


//-1��ʧ�ܣ�0���ɹ���
int CwxBinLogMgr::commit(bool bAlL, char* szErr2K){
    int iRet = 0;
	if(!m_bValid){
		if (szErr2K) strcpy(szErr2K, m_szErr2K);
		return -1;
	}
    CwxBinLogFile* pCurBinLog = NULL;
    {
        ///д������
        CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
        if (!m_pCurBinlog) return 0;
        iRet = m_pCurBinlog->flush_cache(m_szErr2K);
        if (0 != iRet)
        {
            if (szErr2K) strcpy(szErr2K, m_szErr2K);
            return iRet;
        }
        m_uiUnFlushBinlog = 0;
        m_ttLastFlushBinlogTime = time(NULL);
        pCurBinLog = m_pCurBinlog;
    }
	iRet = pCurBinLog->fsync(bAlL, m_szErr2K); 
    if (0 != iRet){
        if (szErr2K) strcpy(szErr2K, m_szErr2K);
    }
    return iRet;
}


///���binlog������
void CwxBinLogMgr::clear()
{
    ///д������
    CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
    _clear();
}

///�������
void CwxBinLogMgr::removeAllBinlog(){
    ///д������
    CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
    m_bValid = true;
    {
        map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.begin(); ///<������ǰbinlog�ļ���binlog�ļ���map
        while(iter != m_binlogMap.end()){
            CWX_INFO(("Remove binlog file, file:%s", iter->second->getDataFileName().c_str()));
            CwxBinLogFile::remove(iter->second->getDataFileName().c_str());
            delete iter->second;
            iter++;
        }
        m_binlogMap.clear();
    }
    ///��������cursor��״̬
    {
        set<CwxBinLogCursor*>::iterator iter = m_cursorSet.begin(); ///<����������cursor�ļ���
        while(iter != m_cursorSet.end()){
            if (-1 != (*iter)->m_fd){
                ::close((*iter)->m_fd);
                (*iter)->m_fd = -1;
            }
            (*iter)->m_ucSeekState = CwxBinLogCursor::CURSOR_STATE_ERROR;
            strcpy((*iter)->m_szErr2K, "Cursor is reset for deleting binlog.");
            iter++;
        }
        m_cursorSet.clear();
    }
    
    m_pCurBinlog = NULL;///<��ǰд��binlog�ļ�
    m_ullMinSid = 0; ///<binlog�ļ�����Сsid
    m_ullMaxSid = 0; ///<binlog�ļ������sid
    m_ttMinTimestamp = 0; ///<binlog�ļ���log��ʼʱ��
    m_ttMaxTimestamp = 0; ///<binlog�ļ���log����ʱ��
    m_ullNextSid = 1; ///<һ��һ��sid��ֵ
    m_uiUnFlushBinlog = 0; ///<δflush��binlog������
    m_ttLastFlushBinlogTime = time(NULL); ///<��һ��flushbinlog��ʱ��

}


///������trim��ָ����sid��0���ɹ���-1��ʧ��
/*int CwxBinLogMgr::trim(CWX_UINT64 ullSid, char* szErr2K){
	CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
	CwxBinLogFile* pBinLogFile = NULL;
	if(!m_bValid)
	{
		if (szErr2K) strcpy(szErr2K, m_szErr2K);
		return -1;
	}
	///��С���������ʷ����
	map<CWX_UINT32, CwxBinLogFile*>::iterator iter = m_binlogMap.begin();
	while (iter != m_binlogMap.end()){
		if (ullSid < iter->second->getMaxSid()) ///���С�����ֵ����һ������
		{
			pBinLogFile = iter->second;
			break;
		}
		iter++;
	}
	if (!pBinLogFile)
	{
		if (m_pCurBinlog && (ullSid < m_pCurBinlog->getMaxSid())) ///��ǰbinglog���ڶ���С�����ֵ
			pBinLogFile = m_pCurBinlog;
	}
	if (!pBinLogFile)
	{///û����Ҫtrim��record
		return 0;
	}

	CWX_UINT32 uiCurFileNo = m_pCurBinlog->getFileNo();
	CWX_UINT32 uiCurDay = m_pCurBinlog->getFileDay();
	CWX_UINT32 uiFileNo = pBinLogFile->getFileNo();
	if (0 != pBinLogFile->trim(ullSid, m_szErr2K))
	{
		m_bValid = true;
		if (szErr2K) strcpy(szErr2K, m_szErr2K);
		return -1;
	}

	if (!pBinLogFile->getLogNum()){ ///�ļ�Ϊ��
		//delete file
		CwxBinLogFile::remove(pBinLogFile->getDataFileName().c_str());
		m_binlogMap.erase(uiFileNo);
		delete pBinLogFile;
		uiFileNo--;
	}
	if (uiFileNo < uiCurFileNo) m_pCurBinlog = NULL;
	//ɾ�����д��ڴ�file no���ļ�
	iter = m_binlogMap.upper_bound(uiFileNo);
	while(iter != m_binlogMap.end()){
		CwxBinLogFile::remove(iter->second->getDataFileName().c_str());
		delete iter->second;
		m_binlogMap.erase(iter);
		iter = m_binlogMap.upper_bound(uiFileNo);
	}
	if (!m_pCurBinlog){
		string strPathFile;
		uiFileNo += 1;
		m_pCurBinlog = new CwxBinLogFile(CwxDate::trimToDay(uiCurDay), uiFileNo, m_uiMaxFileSize);
		getFileNameByFileNo(START_FILE_NUM, m_pCurBinlog->getFileDay(), strPathFile);
		if (0 != m_pCurBinlog->open(strPathFile.c_str(),
			false,
			true,
			m_szErr2K))
		{
			delete m_pCurBinlog;
			m_pCurBinlog = NULL;
			m_bValid = false;
			if (szErr2K) strcpy(szErr2K, m_szErr2K);
			return -1;
		}
	}

	///�����α�
	set<CwxBinLogCursor*>::iterator cursor_iter = m_cursorSet.begin();
	while(cursor_iter != m_cursorSet.end()){
		if ((*cursor_iter)->isReady()&&
			((*cursor_iter)->getHeader().getSid() >= ullSid))
		{
			(*cursor_iter)->setSeekState(CwxBinLogCursor::CURSOR_STATE_UNSEEK);
			(*cursor_iter)->setSeekSid((*cursor_iter)->getHeader().getSid());
		}
		cursor_iter++;
	}
	return 0;
}
*/

void CwxBinLogMgr::_clear()
{
	set<CwxBinLogCursor*>::iterator iter = m_cursorSet.begin();
	while(iter != m_cursorSet.end())
	{
		delete *iter;
		iter++;
	}
	m_cursorSet.clear();

    if (m_pCurBinlog)
	{
        m_pCurBinlog->flush_cache(NULL);
		m_pCurBinlog->fsync(true, NULL);
	}
    m_pCurBinlog = NULL;
    while(m_binlogMap.size())
    {
        delete m_binlogMap.begin()->second;
		m_binlogMap.erase(m_binlogMap.begin());
    }
    if (-1 != m_fdLock)
    {
        CwxFile::unlock(m_fdLock);
        ::close(m_fdLock);
        m_fdLock = -1;
    }
    m_uiMaxFileNum = DEF_MANAGE_FILE_NUM;
    m_ullMinSid = 0; ///<binlog�ļ�����Сsid
    m_ullMaxSid = 0; ///<binlog�ļ������sid
    m_ttMinTimestamp = 0; ///<binlog�ļ���log��ʼʱ��
    m_ttMaxTimestamp = 0; ///<binlog�ļ���log����ʱ��
    m_bValid = false;
    strcpy(m_szErr2K, "Not init.");
}


//NULL ʧ�ܣ����򷵻��α�����ָ�롣
CwxBinLogCursor* CwxBinLogMgr::createCurser(CWX_UINT64 ullSid, CWX_UINT8 ucState)
{
	///��������
	CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
    CwxBinLogCursor* pCursor =  new CwxBinLogCursor();
    pCursor->setSeekSid(ullSid);
    pCursor->setSeekState(ucState);
	m_cursorSet.insert(pCursor);
    return pCursor;
}

/**
@brief ��ȡ��С��ullSid����Сbinlog header
@param [in] ullSid Ҫ���ҵ�sid��
@param [out] index ����������binlog index��
@return -1��ʧ�ܣ�0�������ڣ�1������
*/
int CwxBinLogMgr::upper(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K)
{
	///��������
	CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
	return _upper(ullSid, index, szErr2K);
}

///-1��ʧ�ܣ�0�������ڣ�1�����֡�
int CwxBinLogMgr::_upper(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K)
{
	CwxBinLogFile* pBinLogFile = NULL;
	int iRet = 0;

	if(!m_bValid)
	{
		if (szErr2K) strcpy(szErr2K, m_szErr2K);
		return -1;
	}
	//��λsid���ڵ�binlog�ļ�
	///��С���������ʷ����
	map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.begin();
	while (iter != m_binlogMap.end()){
		if (ullSid < iter->second->getMaxSid()) ///���С�����ֵ����һ������
		{
			pBinLogFile = iter->second;
			break;
		}
		iter++;
	}
	if (!pBinLogFile)
	{///�������ֵ
		return 0;
	}
	//��λcursor
	iRet = pBinLogFile->upper(ullSid, index, szErr2K);
	return iRet;
}

/**
@brief ��ȡ������ullSid�����binlog header
@param [in] ullSid Ҫ���ҵ�sid��
@param [out] index ����������binlog index��
@return -1��ʧ�ܣ�0�������ڣ�1������
*/
int CwxBinLogMgr::lower(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K)
{
	///��������
	CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
	return _lower(ullSid, index, szErr2K);
}

///-1��ʧ�ܣ�0�������ڣ�1�����֡�
int CwxBinLogMgr::_lower(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K)
{
	CwxBinLogFile* pBinLogFile = NULL;
	int iRet = 0;
	if(!m_bValid)
	{
		if (szErr2K) strcpy(szErr2K, m_szErr2K);
		return -1;
	}

	//��λsid���ڵ�binlog�ļ����Ӵ�С����
	if (m_pCurBinlog && (ullSid>=m_pCurBinlog->getMinSid())) ///<�����С����Сֵ
	{
		map<CWX_UINT32/*file no*/, CwxBinLogFile*>::reverse_iterator iter = m_binlogMap.rbegin();
		while (iter != m_binlogMap.rend()){
			if (ullSid >= iter->second->getMinSid()){ ///<�����С����Сֵ
				pBinLogFile = iter->second;
				break;
			}
			iter++;
		}
	}
	if (!pBinLogFile)
	{///û�м�¼
		return 0;
	}
	//��λcursor
	iRet = pBinLogFile->lower(ullSid, index, szErr2K);
	return iRet;
}

/// -1��ʧ�ܣ�0���޷���λ��ullSid��һ��binlog��1����λ��ullSid��һ����binlog�ϡ�
int CwxBinLogMgr::seek(CwxBinLogCursor* pCursor, CWX_UINT64 ullSid)
{
    ///��������
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    return _seek(pCursor, ullSid);
}

/// -1��ʧ�ܣ�0���޷���λ��ullSid��һ��binlog��1����λ��ullSid��һ����binlog�ϡ�
int CwxBinLogMgr::_seek(CwxBinLogCursor* pCursor, CWX_UINT64 ullSid)
{
    CwxBinLogFile* pBinLogFile = NULL;
    int iRet = 0;
    pCursor->setSeekSid(ullSid);
	if(!m_bValid)
	{
		strcpy(pCursor->getErrMsg(), m_szErr2K);
		return -1;
	}
    if (!m_pCurBinlog || 
        (ullSid >= m_pCurBinlog->getMaxSid()))
    {///�������ֵ
        pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_UNSEEK);
        return 0;
    }
    //��λsid���ڵ�binlog�ļ�
    if (ullSid>=m_pCurBinlog->getMinSid())
    {///�ڵ�ǰbinlog�ļ���
        pBinLogFile = m_pCurBinlog;
    }else{
        map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.begin();
        while(iter != m_binlogMap.end()){
            if (ullSid < iter->second->getMaxSid()){
                pBinLogFile = iter->second;
                break;
            }
            iter++;
        }
    }
    if (!pBinLogFile) pBinLogFile = m_pCurBinlog;
    //��λcursor
    iRet = pBinLogFile->seek(*pCursor, CwxBinLogFile::SEEK_SID);
    if (1 == iRet){
        return 1;
    }

	pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);   
    if ((-1 == iRet) || (-2 == iRet)) return -1; 
    //iRet == 0
    char szBuf1[64], szBuf2[64];
    CwxCommon::snprintf(pCursor->getErrMsg(), 2047, "Sid[%s] should be found, but not. binlog-file's max-sid[%s], binlog file:%s",
        CwxCommon::toString(ullSid, szBuf1),
        CwxCommon::toString(pBinLogFile->getMaxSid(), szBuf2),
        pBinLogFile->getDataFileName().c_str());
    return -1;
}


//-1��ʧ�ܣ�0���Ƶ����1���ɹ��Ƶ���һ��binlog��
int CwxBinLogMgr::next(CwxBinLogCursor* pCursor)
{
    ///��������
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
	if(!m_bValid)
	{
		strcpy(pCursor->getErrMsg(), m_szErr2K);
		return -1;
	}
    if (!pCursor->isReady())
    {
        if (pCursor->isUnseek()) strcpy(pCursor->getErrMsg(), "Cursor is unseek.");
        return -1;
    }
    if (pCursor->isUnseek())
    {
        if (!m_pCurBinlog ||
            (pCursor->getSeekSid() >= m_pCurBinlog->getMaxSid())) return 0;
        if (0 != _seek(pCursor, pCursor->getSeekSid())) return -1;
        CWX_ASSERT(pCursor->isReady());
    }
    int iRet = 0;
    if (_isOutRange(pCursor))
    {
		CwxBinLogFile* pBinLogFile = _getMinBinLogFile();
        iRet = pBinLogFile->seek(*pCursor, CwxBinLogFile::SEEK_START);
        //iRet -2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0�������ڣ�1����λ��ָ����λ��
        if (1 == iRet) return 1;
		pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
        if (0 == iRet)
        {
            CwxCommon::snprintf(pCursor->getErrMsg(), 2047, "Seek to binlog file's start, but return 0, LogNum=%d, file=%s",
                pBinLogFile->getLogNum(),
                pBinLogFile->getDataFileName().c_str());
        }
        return -1;
    }
	
	iRet = pCursor->next();//-2��log��header��������-1����ȡʧ�ܣ�0����ǰlogΪ���һ��log��1���Ƶ���һ��log
    if (1 == iRet) return 1;
    if ((-1==iRet) || (-2==iRet))
    {
		pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
        return -1;
    }
    //Now, iRet=0
	map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.upper_bound(pCursor->getFileNo());
    if (iter != m_binlogMap.end())
    {
        iRet = iter->second->seek(*pCursor, CwxBinLogFile::SEEK_START);
        //iRet -2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0�������ڣ�1����λ��ָ����λ��
        if (1 == iRet) return 1;
		pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
        if (0 == iRet)
        {
            CwxCommon::snprintf(pCursor->getErrMsg(), 2047, "Seek to binlog file's start, but return 0, LogNum=%d, file=%s",
                iter->second->getLogNum(),
                iter->second->getDataFileName().c_str());
        }
        return -1;
    }
    //the end
    return 0;
}

// -1��ʧ�ܣ�0���Ƶ��ʼ��1���ɹ��Ƶ�ǰһ��binlog��
int CwxBinLogMgr::prev(CwxBinLogCursor* pCursor)
{
    ///��������
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
	if(!m_bValid)
	{
		strcpy(pCursor->getErrMsg(), m_szErr2K);
		return -1;
	}
    if (!pCursor->isReady())
    {
        if (pCursor->isUnseek()) strcpy(pCursor->getErrMsg(), "Cursor is unseek.");
        return -1;
    }
    if (pCursor->isUnseek())
    {
        if (!m_pCurBinlog ||
            (pCursor->getSeekSid() <= m_pCurBinlog->getMinSid())) return 0;
        if (pCursor->getSeekSid() >= m_pCurBinlog->getMaxSid()){
            if (m_pCurBinlog->getMaxSid()){
                pCursor->setSeekSid(m_pCurBinlog->getMaxSid() - 1);
                if (0 != _seek(pCursor, pCursor->getSeekSid())) return -1;
            }else{
                return 0;
            }
        }
        CWX_ASSERT(pCursor->isReady());
    }

    int iRet = 0;
    if (_isOutRange(pCursor))
    {
		CwxBinLogFile * pBinLogFile = _getMinBinLogFile();
        iRet = pBinLogFile->seek(*pCursor, CwxBinLogFile::SEEK_START);
        //iRet -2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0�������ڣ�1����λ��ָ����λ��
        if (1 == iRet) return 0;
		pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
        if (0 == iRet)
        {
            CwxCommon::snprintf(pCursor->getErrMsg(), 2047, "Seek to binlog file's start, but return 0, LogNum=%d, file=%s",
                pBinLogFile->getLogNum(),
                pBinLogFile->getDataFileName().c_str());
        }
        return -1;
    }

	iRet = pCursor->prev();//-2��log��header��������-1����ȡʧ�ܣ�0����ǰlogΪ���һ��log��1���Ƶ���һ��log
    if (1 == iRet) return 1;
    if ((-1==iRet) || (-2==iRet))
    {
		pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
        return -1;
    }
    //Now, iRet=0
	map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.lower_bound(pCursor->getFileNo());
	CWX_ASSERT(iter != m_binlogMap.end());
	if (iter->first == pCursor->getFileNo()) iter--;
    if (iter != m_binlogMap.end())
    {
        CWX_ASSERT(iter->second);
        iRet = iter->second->seek(*pCursor, CwxBinLogFile::SEEK_TAIL);
        //iRet -2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0�������ڣ�1����λ��ָ����λ��
        if (1 == iRet) return 1;
		pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
        if (0 == iRet)
        {
            CwxCommon::snprintf(pCursor->getErrMsg(), 2047, "Seek to binlog file's end, but return 0, LogNum=%d, file=%s",
                iter->second->getLogNum(),
                iter->second->getDataFileName().c_str());
        }
        return -1;
    }
    //the end
    return 0;
}

//-1��ʧ�ܣ�0���ɹ���ȡ��һ��binlog��
int CwxBinLogMgr::fetch(CwxBinLogCursor* pCursor, char* szData, CWX_UINT32& uiDataLen)
{
	///��������
	{
		CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
		if(!m_bValid)
		{
			strcpy(pCursor->getErrMsg(), m_szErr2K);
			return -1;
		}
		if (!pCursor->isReady())
		{
			if (pCursor->isUnseek()) strcpy(pCursor->getErrMsg(), "Cursor is unseek.");
			return -1;
		}
		if (pCursor->isUnseek())
		{
			strcpy(pCursor->getErrMsg(), "the cursor doesn't seek.");
			return -1;
		}
	}
    int iRet = pCursor->data(szData, uiDataLen);
    //iRet: -2�����ݲ���ɣ�-1��ʧ�ܣ�>=0����ȡ���ݵĳ���
    if (iRet >= 0) return 0;
	pCursor->setSeekState(CwxBinLogCursor::CURSOR_STATE_ERROR);
    return -1;
}

// -1��ʧ�ܣ�0���Ƶ����1���ɹ���ȡ��һ��binlog��
int CwxBinLogMgr::next(CwxBinLogCursor* pCursor, char* szData, CWX_UINT32& uiDataLen)
{
    int iRet = next(pCursor);
    //iRet -1��ʧ�ܣ�0���Ƶ����1���ɹ��Ƶ���һ��binlog��
    if (1 == iRet)
    {
        iRet = fetch(pCursor, szData, uiDataLen);
        //iRet -1��ʧ�ܣ�0���ɹ���ȡ��һ��binlog��
        if (0 == iRet) return 1;
        return -1;
    }
    if (0 == iRet) return 0;
    return -1;
}

// -1��ʧ�ܣ�0���Ƶ��ʼ��1���ɹ���ȡǰһ��binlog��
int CwxBinLogMgr::prev(CwxBinLogCursor* pCursor, char* szData, CWX_UINT32& uiDataLen)
{
    int iRet = prev(pCursor);
    //iRet -1��ʧ�ܣ�0���Ƶ���ʼ��1���ɹ��Ƶ���һ��binlog��
    if (1 == iRet)
    {
        iRet = fetch(pCursor, szData, uiDataLen);
        //iRet -1��ʧ�ܣ�0���ɹ���ȡ��һ��binlog��
        if (0 == iRet) return 1;
        return -1;
    }
    if (0 == iRet) return 0;
    return -1;
}

//-1��ʧ�ܣ�0���ɹ���
int CwxBinLogMgr::destoryCurser(CwxBinLogCursor*& pCursor)
{
	CwxWriteLockGuard<CwxRwLock> lock(&m_rwLock);
	set<CwxBinLogCursor*>::iterator iter =  m_cursorSet.find(pCursor);
	if (iter != m_cursorSet.end())
	{
		m_cursorSet.erase(iter);
		delete pCursor;
		pCursor = NULL;
		return 0;
	}
	return -1;
}

CWX_INT64 CwxBinLogMgr::leftLogNum(CwxBinLogCursor const* pCursor)
{
    ///��������
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    if (!pCursor) return -1;
    if (pCursor->isReady()) return -1;
    CWX_INT64 num = m_binlogMap.find(pCursor->getFileNo())->second->getLogNum() - pCursor->getHeader().getLogNo();
	map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.upper_bound(pCursor->getFileNo());
    while(iter != m_binlogMap.end())
    {
        num += iter->second->getLogNum();
		iter++;
    }
    return num;
}

CWX_UINT64 CwxBinLogMgr::getFileStartSid(CWX_UINT32 ttTimestamp)
{
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
	map<CWX_UINT32/*file no*/, CwxBinLogFile*>::iterator iter = m_binlogMap.begin();
    while (iter != m_binlogMap.end()){
        if (iter->second->getMaxTimestamp() > ttTimestamp)
            return iter->second->getMinSid();
        iter++;
    }
    if (m_pCurBinlog) return m_pCurBinlog->getMinSid();
    return 0;
}

bool CwxBinLogMgr::getLastSidByNo(CWX_UINT32 uiNo, CWX_UINT64& ullSid, char* szErr2K){
    CwxReadLockGuard<CwxRwLock> lock(&m_rwLock);
    map<CWX_UINT32/*file no*/, CwxBinLogFile*>::reverse_iterator iter = m_binlogMap.rbegin();
    while (iter != m_binlogMap.rend()){
        if (iter->second->getLogNum() >= uiNo) break;
        uiNo -= iter->second->getLogNum();
        iter++;
    }
    if (iter == m_binlogMap.rend()){
        ullSid = 0;
        return true;
    }
    return iter->second->getLastSidByNo(uiNo, ullSid, szErr2K);
}

//void
void CwxBinLogMgr::_outputManageBinLog()
{
    char szBuf[2048];
    char szBuf1[64];
    char szBuf2[64];
    string strTimeStamp1;
    string strTimeStamp2;
    string strFileName = m_strPrexLogPath + m_strFilePrex + "_mgr.inf";
    FILE* fd = fopen(strFileName.c_str(), "wb");
    if (fd)
    {
        CwxCommon::snprintf(szBuf, 2047, "FileNo\t\tMinSid\t\tMaxSid\t\tMinTimestamp\t\tMaxTimestamp\t\tLogNo\t\tFile\n");
        fwrite(szBuf, 1, strlen(szBuf), fd);
        if (m_pCurBinlog)
        {
            CwxCommon::snprintf(szBuf,
                2047,
                "%d\t\t%s\t\t%s\t\t%s\t\t%s\t\t%u\t\t%s\n",
                m_pCurBinlog->getFileNo(),
                CwxCommon::toString(m_pCurBinlog->getMinSid(), szBuf1),
                CwxCommon::toString(m_pCurBinlog->getMaxSid(), szBuf2),
                CwxDate::getDate(m_pCurBinlog->getMinTimestamp(), strTimeStamp1).c_str(),
                CwxDate::getDate(m_pCurBinlog->getMaxTimestamp(), strTimeStamp2).c_str(),
                m_pCurBinlog->getLogNum(),
                m_pCurBinlog->getDataFileName().c_str()
                );
            fwrite(szBuf, 1, strlen(szBuf), fd);
        }
		map<CWX_UINT32/*file no*/, CwxBinLogFile*>::reverse_iterator iter =  m_binlogMap.rbegin();
        while(iter != m_binlogMap.rend()){
            CwxCommon::snprintf(szBuf,
                2047,
                "%d\t\t%s\t\t%s\t\t%s\t\t%s\t\t%u\t\t%s\n",
                iter->second->getFileNo(),
                CwxCommon::toString(iter->second->getMinSid(), szBuf1),
                CwxCommon::toString(iter->second->getMaxSid(), szBuf2),
                CwxDate::getDate(iter->second->getMinTimestamp(), strTimeStamp1).c_str(),
                CwxDate::getDate(iter->second->getMaxTimestamp(), strTimeStamp2).c_str(),
                iter->second->getLogNum(),
                iter->second->getDataFileName().c_str()
                );
            fwrite(szBuf, 1, strlen(szBuf), fd);
			iter++;
        }
    }
    fclose(fd);
}



