#ifndef __CWX_BIN_LOG_MGR_H__
#define __CWX_BIN_LOG_MGR_H__


#include "CwxType.h"
#include "CwxStl.h"
#include "CwxStlFunc.h"
#include "CwxCommon.h"
#include "CwxFile.h"
#include "CwxDate.h"
#include "CwxRwLock.h"
#include "CwxLockGuard.h"
#include "CwxLogger.h"
#include "UnistorMacro.h"


/**
@class CwxBinLogHeader
@brief binlog��header������
*/
class CwxBinLogHeader
{
public:
    enum{
        BIN_LOG_HEADER_SIZE = 32 ///<serialize�Ŀռ��ֽ���
    };
public:
    ///ȱʡ���캯��
    inline CwxBinLogHeader();
    ///���캯��
    inline CwxBinLogHeader(CWX_UINT64 ullSid, CWX_UINT32 uiLogNo, CWX_UINT32 uiDatetime, CWX_UINT32 uiOffset,
                   CWX_UINT32 uiLogLen, CWX_UINT32 uiPrevOffset,
                   CWX_UINT32 uiGroup);
    ///��������
    inline CwxBinLogHeader(CwxBinLogHeader const& header);
    ///��ֵ����
    inline CwxBinLogHeader& operator=(CwxBinLogHeader const& header);
    ///�Ƚϲ���,true��С�ڣ�false����С��
    inline bool operator<(CwxBinLogHeader const& header) const;
public:
    ///����sid
    inline void setSid(CWX_UINT64 ullSid);
    ///��ȡsid
    inline CWX_UINT64 getSid() const;
	///���ü�¼��
	inline void setLogNo(CWX_UINT32 uiLogNo);
	///��ȡ��¼��
	inline CWX_UINT32 getLogNo() const;
    ///����log��ʱ���
    inline void setDatetime(CWX_UINT32 uiDatetime);
    ///��ȡlog��ʱ���
    inline CWX_UINT32 getDatetime() const;
    ///����log���ļ�ƫ��
    inline void setOffset(CWX_UINT32 uiOffset);
    ///��ȡlog���ļ�ƫ��
    inline CWX_UINT32 getOffset() const;
    ///����log�ĳ��ȣ�������log header�ĳ���
    inline void setLogLen(CWX_UINT32 uiLogLen);
    ///��ȡlog�ĳ��ȣ�������log header�ĳ���
    inline CWX_UINT32 getLogLen() const;
    ///����ǰһ��log��offset
    inline void setPrevOffset(CWX_UINT32 uiPrevOffset);
    ///��ȡǰһ��log��offset
    inline CWX_UINT32 getPrevOffset() const;
    ///����binlog�ķ���
    inline void setGroup(CWX_UINT32 uiGroup);
    ///��ȡbinlog�ķ���
    inline CWX_UINT32 getGroup() const;
public:
    ///��log header�������л����������л�ռ�õĿռ��ֽ���
    inline CWX_UINT32 serialize(char* szBuf) const;
    ///��log header��buf�����л����������л�ռ�õĿռ��ֽ���
    inline CWX_UINT32 unserialize(char const* szBuf);
    ///��ն���
    inline void reset();
private:
    CWX_UINT64    m_ullSid; ///<ͬ�����к�
	CWX_UINT32    m_uiLogNo; ///<��¼��
    CWX_UINT32    m_uiDatetime; ///<��¼��ʱ���
    CWX_UINT32    m_uiOffset; ///<��¼���ļ�ƫ��
    CWX_UINT32    m_uiLogLen; ///<��¼�ĳ��ȣ�������log header�ĳ���
    CWX_UINT32    m_uiPrevOffset; ///<ǰһ����¼���ļ�ƫ��
    CWX_UINT32    m_uiGroup; ///<binlog��group
};


/**
@class CwxBinLogIndex
@brief binlog��index������
*/
class CwxBinLogIndex
{
public:
    enum{
        BIN_LOG_INDEX_SIZE = 20 ///<serialize�Ŀռ��ֽ���
    };
public:
    ///ȱʡ����
    inline CwxBinLogIndex();
    ///���캯��
    inline CwxBinLogIndex(CWX_UINT64 ullSid, CWX_UINT32 uiDatetime, CWX_UINT32 uiOffset, CWX_UINT32 uiLogLen);
    ///��������
    inline CwxBinLogIndex(CwxBinLogIndex const& index);
    ///��������
    inline CwxBinLogIndex(CwxBinLogHeader const& header);
    ///��ֵ����
    inline CwxBinLogIndex& operator=(CwxBinLogIndex const& index);
    ///��ֵ����
    inline CwxBinLogIndex& operator=(CwxBinLogHeader const& header);
    ///�Ƚϲ�����true��С�ڣ�false����С��
    inline bool operator<(CwxBinLogIndex const& index) const;
public:
    ///����sid
    inline void setSid(CWX_UINT64 ullSid);
    ///��ȡsid
    inline CWX_UINT64 getSid() const;
    ///����log��ʱ���
    inline void setDatetime(CWX_UINT32 uiDatetime);
    ///��ȡlog��ʱ���
    inline CWX_UINT32 getDatetime() const;
    ///����log���ļ�ƫ��
    inline void setOffset(CWX_UINT32 uiOffset);
    ///��ȡlog���ļ�ƫ��
    inline CWX_UINT32 getOffset() const;
    ///����log�ĳ��ȣ�������log header�ĳ���
    inline void setLogLen(CWX_UINT32 uiLogLen);
    ///��ȡlog�ĳ��ȣ�������log header�ĳ���
    inline CWX_UINT32 getLogLen() const;
public:
    ///��log header�������л����������л�ռ�õĿռ��ֽ���
    inline CWX_UINT32 serialize(char* szBuf) const;
    ///��log header��buf�����л����������л�ռ�õĿռ��ֽ���
    inline CWX_UINT32 unserialize(char const* szBuf);
    ///��ն���
    inline void reset();
private:
    CWX_UINT64    m_ullSid; ///<ͬ�����к�
    CWX_UINT32    m_uiDatetime; ///<binlog��timestamp
    CWX_UINT32    m_uiOffset; ///<��¼���ļ�ƫ��
	CWX_UINT32    m_uiLogLen; ///<��¼�ĳ���
};

class CwxBinLogFile;
class CwxBinLogMgr;


/**
@class CwxBinLogCursor
@brief BinLog �ļ��Ķ�ȡ�α�
*/
class CwxBinLogCursor
{
public:
	enum
	{
		BINLOG_READ_BLOCK_BIT = 16, ///<64K�Ķ�ȡbuf��
		BINLOG_READ_BLOCK_SIZE = 1<<BINLOG_READ_BLOCK_BIT ///<64K�Ķ�ȡbuf��
	};
	enum{
		CURSOR_STATE_UNSEEK = 0, ///<cursor����δ��λ��״̬
		CURSOR_STATE_READY = 1, ///<cursor���ڶ�λ��״̬
		CURSOR_STATE_ERROR = 2 ///<cursor���ڳ����״̬
	};
public:
    ///���캯��
    CwxBinLogCursor();
    ///��������
    ~CwxBinLogCursor();
public:
    /**
    @brief ��bin-log�ļ�����ʱcursorû�ж�λ���κμ�¼����ʱ������next��prev����ᶨλ����һ����¼�ϡ�
           ����seek�ᶨλ��ָ����offset�ϡ�
    @param [in] szFileName bin-log���ļ�����
    @return -1��ʧ�ܣ�0���ɹ�
    */
    int open(char const* szFileName, CWX_UINT32 uiFileNo, CWX_UINT32 uiFileDay);
    /**
    @brief �Ƶ���һ��log
    @return -2��log��header��������-1����ȡʧ�ܣ�0����ǰlogΪ���һ��log��1���Ƶ���һ��log
    */
    inline int next();
    /**
    @brief �Ƶ�ǰһ��log
    @return -2��log��header��������-1����ȡʧ�ܣ�0����ǰlogΪ��һ��log��1���Ƶ�ǰһ��log
    */
    inline int prev();
    /**
    @brief �ļ�ƫ�Ƶ�ָ����offset��offset��λ�ñ���Ϊһ��log�Ŀ�ʼλ��
    @param [in] uiOffset binlog���ļ��е�offset��
    @return -2��log��header��������-1����ȡʧ�ܣ�0��������Χ��1���Ƶ�ָ����offset
    */
    inline int seek(CWX_UINT32 uiOffset);
    /**
    @brief ��ȡ��ǰlog��data
    @param [in] szBuf binlog��buf��
    @param [in,out] uiBufLen ����szBuf�Ĵ�С�����ض�ȡ���ݵĳ��ȡ�
    @return  -2�����ݲ���ɣ�-1��ʧ�ܣ�>=0����ȡ���ݵĳ���
    */
    int data(char * szBuf, CWX_UINT32& uiBufLen);
    ///�ر�cursor
    void close();
    ///��ȡ����log�ļ�¼ͷ
    inline CwxBinLogHeader const& getHeader() const;
    ///��ȡ�ļ�������
    inline string const& getFileName() const;
    ///��ȡ�ļ�������
    inline CWX_UINT32 getFileDay() const;
    ///��ȡ�ļ������
    inline CWX_UINT32 getFileNo() const;
    ///��ȡ��ǰ�Ĵ�����Ϣ
    inline char* getErrMsg();
    ///��ȡcursor��SEEK STATE
    inline CWX_UINT8 getSeekState() const;
	///����cursor��SEEK STATE
	inline void setSeekState(CWX_UINT8 ucSeekState);
	///��ȡcursor��SEEK SID
	inline CWX_UINT64 getSeekSid() const;
	///����cursor��SEEK SID
	inline void setSeekSid(CWX_UINT64 ullSid);
	///�Ƿ�ready
	inline bool isReady() const;
	///�Ƿ�unseek
	inline bool isUnseek() const;
	///�Ƿ����
	inline bool isError() const;
private:
    /**
    @brief ��ָ��λ�ã���ȡlog��header��
    @param [in] uiOffset binlog���ļ��е�offset��
    @return -2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0��������1����ȡһ��
    */
    int header(CWX_UINT32 uiOffset);
    //��ȡcursor���ļ� io handle
    inline int getHandle() const;
	//��ȡ����
	ssize_t pread(int fildes, void *buf, size_t nbyte, CWX_UINT32 offset);
	//��ȡһҳ
	bool preadPage(int fildes, CWX_UINT32 uiBlockNo, CWX_UINT32 uiOffset);
private:
    friend class CwxBinLogMgr;
    string             m_strFileName; ///<�ļ�������
    int                m_fd;///<�ļ���handle
    CwxBinLogHeader     m_curLogHeader; ///<��ǰlog��header
    char               m_szHeadBuf[CwxBinLogHeader::BIN_LOG_HEADER_SIZE]; ///<log header��buf�ռ�
    char               m_szErr2K[2048];///<������Ϣbuf
	char			   m_szReadBlock[BINLOG_READ_BLOCK_SIZE];  ///<�ļ���ȡ��buf
	CWX_UINT32         m_uiBlockNo;   ///<��ǰcache��block no
	CWX_UINT32		   m_uiBlockDataOffset; ///<�������ݽ���ƫ��
	CWX_UINT32		   m_uiFileDay; ///<�ļ�������
	CWX_UINT32         m_uiFileNo; ///<�ļ���
    //��CwxBinLogMgrʹ�õ�״ֵ̬
    CWX_UINT64          m_ullSeekSid; ///<seek��sid
    CWX_UINT8           m_ucSeekState; ///<seek��״̬
};



/**
@class CwxBinLogIndexWriteCache
@brief BinLog�ļ���дcache����ʵ�ֶ�binlog������дcache��
*/
class CwxBinLogIndexWriteCache
{
public:
    enum
    {
        BINLOG_WRITE_INDEX_CACHE_RECORD_NUM = 16384 ///<д������320K�� 20 * 16K=320K
    };
public:
    /**
    @brief ���캯��
    @param [in] indexFd �����ļ���fd handle��
    @param [in] ullIndexOffset �����ļ���ǰ�Ĵ�С��
    @param [in] ullSid  �����ļ���ǰ�����sid��
    */
    CwxBinLogIndexWriteCache(int indexFd,
        CWX_UINT32 uiIndexOffset,
        CWX_UINT64 ullSid);
    ///��������
    ~CwxBinLogIndexWriteCache();

public:
    /**
    @brief ��cache������ˢ�µ������ļ���
    @param [out] szErr2K ����ʱ�Ĵ�����Ϣ��
    @return 0:�ɹ���-1��ʧ�ܡ���ʱ�����ļ�д����
    */
    int flushIndex(char* szErr2K=NULL);
    /**
    @brief �������ļ�д��cache�С�
    @param [int] header ��Ϣ��header��
    @param [out] szErr2K ����ʱ�Ĵ�����Ϣ��
    @return 0:�ɹ���-1��д����ʧ�ܡ�
    */
    int append(CwxBinLogHeader const& header, char* szErr2K=NULL);

    friend class CwxBinLogFile;
    friend class CwxBinLogMgr;

private:
    int					m_indexFd;///<cache��Ӧ�������ļ���fd
    //binlog дcache����Ϣ
    CWX_UINT64			m_ullPrevIndexSid; ///<ǰһ��sid�����Ƕ�Ӧ�������ļ�����sid����Ϊ0����ʾȫ�����ڴ档
    CWX_UINT64			m_ullMinIndexSid; ///<����cache����Сsid����Ϊ0��ʾû��cache
    CWX_UINT32          m_uiIndexFileOffset; ///<�������ļ�д��ƫ��
    unsigned char*		m_indexBuf;  ///<����cache��buf��
    CWX_UINT32			m_uiIndexLen;  ///<����buf�ĳ���
    map<CWX_UINT64/*sid*/, unsigned char*>  m_indexSidMap; ///<index���ݵ�sid����
    CWX_UINT64			m_ullMaxSid;   ///<��ǰ����sid
};

/**
@class CwxBinLogFile
@brief BinLog�ļ���ϵ���󣬴˶�����һ��binlog�����ݡ������ļ��Ĺ�������binlog�ļ��Ķ�������
*/
class CwxBinLogFile
{
public:
    enum{
        SEEK_START = 0, ///<������Ƶ��ļ��Ŀ�ͷ
        SEEK_TAIL = 1, ///<������Ƶ��ļ������λ��
        SEEK_SID = 2 ///<������Ƶ��ļ���ָ����SID
    };
    enum{
        MIN_BINLOG_FILE_SIZE = 32 * 1024 * 1024, ///<����binlog�ļ���СΪ256M
        FREE_BINLOG_FILE_SIZE = 4 * 1024 * 1024 ///<binlogԤ���Ŀռ�Ϊ4M
    };

public:
    /**
    @brief ���캯��
    @param [in] ttDay binlog�ļ������ڡ�
    @param [in] uiFileNo binlog�ļ�����š�
    @param [in] uiMaxFileSize binlog�ļ�������С��
    @return 0���ɹ���-1��ʧ�ܡ�
    */
    CwxBinLogFile(CWX_UINT32 ttDay, CWX_UINT32 uiFileNo=0, CWX_UINT32 uiMaxFileSize=512*1024*1024);
    ///��������
    ~CwxBinLogFile();
    ///С�ڱȽ�
    inline bool operator < (CwxBinLogFile const& obj) const;

public:
    /**
    @brief �򿪻򴴽�����־�ļ�
    @param [in] szPathFile ��־�ļ�����
    @param [in] bReadOnly �Ƿ���ֻ����ʽ�򿪣���ʱ���ļ�������ڡ�
    @param [in] bCreate �Ƿ񴴽�����־�ļ�����ʱ����־�ļ����벻���ڡ�
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return 0���ɹ���-1��ʧ�ܡ�
    */
    int open(char const* szPathFile, bool bReadOnly = true, bool bCreate=false, char* szErr2K=NULL);
    /**
    @brief ����־�ļ����һ������־
    @param [in] ullSid ��־��sid�š�
    @param [in] ttTimestamp ��־������
    @param [in] uiGroup binlog�ķ���
    @param [in] uiAttr ��־������
    @param [in] szData ��־����
    @param [in] uiDataLen ��־���ȡ�
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0����־�ļ����ˣ�1���ɹ���
    */
    int append(CWX_UINT64 ullSid,
        CWX_UINT32 ttTimestamp,
        CWX_UINT32 uiGroup,
        char const* szData,
        CWX_UINT32 uiDataLen,
        char* szErr2K=NULL);
    /**
    @brief ȷ����cache������д�뵽Ӳ��
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int flush_cache(char* szErr2K=NULL);
    /**
    @brief ȷ��д�����־���浽Ӳ��
	@param [in] bFlushAll true:��������Ҳ��Ҫfsync��Ӳ�̣�false��ֻ����fsync��Ӳ�̡�
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int fsync(bool bFlushAll=false, char* szErr2K=NULL);

	/**
	@brief ��ȡ����ullSid����Сbinlog header
	@param [in] ullSid Ҫ���ҵ�sid��
	@param [out] item ����������binlog index��
	@param [out] szErr2K ����ʱ�Ĵ�����Ϣ��
	@return -1��ʧ�ܣ�0�������ڣ�1������
	*/
	int upper(CWX_UINT64 ullSid, CwxBinLogIndex& item, char* szErr2K=NULL);

	/**
	@brief ��ȡ������ullSid�����binlog header
	@param [in] ullSid Ҫ���ҵ�sid��
	@param [out] item ����������binlog index��
	@param [out] szErr2K ����ʱ�Ĵ�����Ϣ��
	@return -1��ʧ�ܣ�0�������ڣ�1������
	*/
	int lower(CWX_UINT64 ullSid, CwxBinLogIndex&item, char* szErr2K=NULL);

	/**
	@brief ��λCursor��λ��
	@param [in] cursor ��־��handle��
	@param [in] ucMode ��λ��ģʽ��SEEK_START����λ���ļ��Ŀ�ͷ��SEEK_TAIL����λ���ļ������SEEK_SID����λ����һ������cursor.getSid()����־����
	@return -2����������ɵļ�¼ͷ��-1��ʧ�ܣ�0�������ڣ�1����λ��ָ����λ��
	*/
	int seek(CwxBinLogCursor& cursor, CWX_UINT8 ucMode=SEEK_SID);
	///������trim��ָ����sid��0���ɹ���-1��ʧ��
	int trim(CWX_UINT64 ullSid, char* szErr2K=NULL);

    /**
    @brief ɾ��ָ����binlog�ļ����������ļ�
    @param [in] szPathFileName binlog�ļ�����
    @return void��
    */
    static void remove(char const* szPathFileName);
    //�ر�
    void close();
    /**
    @brief ��ȡ���µĵ�N����¼��sidֵ
    @param [in] uiNo ��¼��
    @param [out] ullSid sid��ֵ
    @return true���ɹ���false��ʧ�ܡ�
    */
    bool getLastSidByNo(CWX_UINT32 uiNo, CWX_UINT64& ullSid, char* szErr2K);
public:
    ///��ȡ��С��sid
    inline CWX_UINT64 getMinSid() const;
    ///��ȡ����sid
    inline CWX_UINT64 getMaxSid() const;
    ///��ȡbinlog����Сʱ���
    inline CWX_UINT32 getMinTimestamp() const;
    ///��ȡbinlog�����ʱ���
    inline CWX_UINT32 getMaxTimestamp() const;
    ///��ȡbinlog�ļ�������
    inline CWX_UINT32 getFileDay() const;
    ///��ȡbinlog���ļ����
    inline CWX_UINT32 getFileNo() const;
    ///��ȡbinlog�ļ���log��¼��
    inline CWX_UINT32 getLogNum() const;
    ///��ȡbinlog�ļ��Ĵ�С
    inline CWX_UINT32 getFileSize() const;
    ///�ж��Ƿ�ֻ��
    inline bool readOnly() const;
    ///����ֻ��
    inline void setReadOnly();
    ///�ж���־�ļ��Ƿ�Ϊ��
    inline bool empty() const;
    ///��ȡ�����ļ�������
    inline string const& getDataFileName() const;
    ///��ȡ�����ļ�������
    inline string const& getIndexFileName() const;
private:
    ///��ն���
    void reset();
    /**
    @brief ��ȡָ��λ�õ�������¼
    @param [in] fd �����ļ���fd��
    @param [out] index ���ص�������
    @param [in] uiOffset ������λ�á�
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    inline int readIndex(int fd, CwxBinLogIndex& index, CWX_UINT32 uiOffset, char* szErr2K=NULL) const;
    /**
    @brief ��ָ����λ��д������
    @param [in] fd �����ļ���fd��
    @param [in] index д���������
    @param [in] uiOffset ������λ�á�
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    inline int writeIndex(int fd, CwxBinLogIndex const& index, CWX_UINT32 uiOffset, char* szErr2K=NULL) const;
    /**
    @brief ����ָ����binlog�ļ�
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int mkBinlog(char* szErr2K=NULL);
    /**
    @brief ���binlog�ļ����������ļ��Ƿ�һ�£�����һ������д���
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int prepareFile(char* szErr2K=NULL);
    /**
    @brief ����Ƿ���Ҫ��������
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0������Ҫ��1����Ҫ��
    */
    int isRebuildIndex(char* szErr2K=NULL);
    /**
    @brief ��������
    @param [in] szErr2K ������Ϣbuf����ΪNULL�򲻷��ش�����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int createIndex(char* szErr2K=NULL);
    friend class CwxBinLogMgr;
private:
    bool					m_bValid;       ///<�Ƿ���Ч
    string					m_strPathFileName; ///<binlog�ļ�������
    string					m_strIndexFileName; ///<index�ļ�������
    CWX_UINT32				m_uiMaxFileSize; ///<�½�����binlog�ļ�������С��
    CWX_UINT64				m_ullMinSid; ///<binlog�ļ�����Сsid
    volatile CWX_UINT64     m_ullMaxSid; ///<binlog�ļ������sid
    CWX_UINT32              m_ttMinTimestamp; ///<binlog�ļ���log��ʼʱ��
    volatile CWX_UINT32     m_ttMaxTimestamp; ///<binlog�ļ���log����ʱ��
    volatile CWX_UINT32		m_uiLogNum; ///<binlog�ļ���log����
    bool					m_bReadOnly; ///<�Ƿ�Ϊֻ��
    int						m_fd; ///<log�ļ���io handle
    int						m_indexFd; ///<�����ļ���io handle
    volatile CWX_UINT32     m_uiFileSize; ///<binlog�����ļ���С��-1��ʾ������
    volatile CWX_UINT32     m_uiIndexFileSize; ///<�����ļ��Ĵ�С��-1��ʾ������
    volatile CWX_UINT32     m_uiPrevLogOffset; ///<ǰһ��binlog��ƫ��
    CWX_UINT32				m_ttDay; ///��־�ļ�������
    CWX_UINT32				m_uiFileNo; ///<��־��š�
    CwxBinLogIndexWriteCache*  m_writeCache; ///<write ģʽ�µ�дcache��
};


/**
@class CwxBinLogMgr
@brief BinLog�Ĺ�����󣬹���binlog�Ķ���д��binlog����������Ϊprex_xxxxxxxxxx.log��<br>
       ���Ӧ�������ļ�������Ϊprex_xxxxxxxxxx.log.idx��
*/
class CwxBinLogMgr
{
private:
    class CwxBinLogFileItem
    {
    public:
        CwxBinLogFileItem(CWX_UINT32 day, CWX_UINT32 no):m_uiDay(day), m_uiNo(no){}
        bool operator == (CwxBinLogFileItem const& item) const
        {
            return m_uiNo == item.m_uiNo;
        }
        bool operator < (CwxBinLogFileItem const& item) const
        {
            return m_uiNo < item.m_uiNo;
        }
    public:
        CWX_UINT32 getDay() const { return m_uiDay;}
        CWX_UINT32 getFileNo() const { return m_uiNo;}

    private:
        CWX_UINT32   m_uiDay;
        CWX_UINT32   m_uiNo;
    };
public:
    enum{
        DEF_MANAGE_FILE_NUM=10, ///<ȱʡ����binlog������
        MIN_MANAGE_FILE_NUM = 1, ///<����binlog����С����
        MAX_MANAGE_FILE_NUM = 2048, ///<����binlog�����Сʱ��
		START_FILE_NUM = 1, ///<��ʼ���ļ����
        MIN_SID_NO = 1, ///<��С��sid���
		MAX_BINLOG_FILE_SIZE = 0X7FFFFFFF, ///<2G
        SKIP_SID_NUM = 10000 ///<��next sidС�����ֵʱ����Ծ������
    };
public:
    /**
    @brief ���캯����
    @param [in] szLogPath binlog�ļ����ڵ�Ŀ¼��
    @param [in] szFilePrex binlog�ļ���ǰ׺���γɵ��ļ���ΪszFilePrex_xxxxxxxxxx��xxxxxxxxxxΪ�ļ���š�
    @param [in] uiMaxFileSize binlog�ļ�������С��
    @param [in] uiBinlogFlushNum binlogд���������Զ�flush
    @param [in] uiBinlogFlushSecond binlog����ʱ�䣬�Զ�flush
    @param [in] bDelOutManageLogFile �Ƿ�ɾ�����ٹ���Χ�ڵ��ļ���
    @return �ޡ�
    */
    CwxBinLogMgr(char const* szLogPath,
        char const* szFilePrex,
        CWX_UINT32 uiMaxFileSize,
        CWX_UINT32 uiBinlogFlushNum,
        CWX_UINT32 uiBinlogFlushSecond,
        bool       bDelOutManageLogFile = false
        );
    ///��������
    ~CwxBinLogMgr();
public:
    /**
    @brief ��ʼ��binlog����������
    @param [in] uiMaxFileNum �����binlog�����������
	@param [in] bCache �Ƿ��д������ݽ���cache��
    @param [out] szErr2K ����ʼ��ʧ�ܣ�����ʧ�ܵĴ�����Ϣ����ΪNULL������ʧ��Ҳ�����ش����ԭ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int init(CWX_UINT32 uiMaxFileNum, bool bCache, char* szErr2K=NULL);
    /**
    @brief ���һ��binlog��
    @param [in] ullSid binlog��sid����ֵ������ڵ�ǰ���е����ֵ��
    @param [in] ttTimestamp binlog��ʱ�����ͨ����ʱ���������binlogͬ����������
    @param [in] uiType ��־�ķ���
    @param [in] uiType ��־������
    @param [in] szData binlog�����ݡ�
    @param [in] uiDataLen binlog�����ݵĳ��ȡ�
    @param [in] szErr2K �����ʧ�ܣ���Ϊʧ�ܵ�ԭ����Ϣ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int append(CWX_UINT64& ullSid,
        CWX_UINT32 ttTimestamp,
        CWX_UINT32 uiGroup,
        char const* szData,
        CWX_UINT32 uiDataLen,
        char* szErr2K=NULL);
    /**
    @brief ˢ���Ѿ�д���binlog��ȷ�����浽Ӳ�̡�
	@param [in] bAll �Ƿ�����Ҳcommit��
    @param [in] szErr2K ��ˢ��ʧ�ܣ��򷵻�ʧ�ܵ�ԭ��
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int commit(bool bAll= false, char* szErr2K=NULL);
    ///���binlog������
    void clear();
    ///�������
    void removeAllBinlog();
	///������trim��ָ����sid��0���ɹ���-1��ʧ��
//	int trim(CWX_UINT64 ullSid, char* szErr2K=NULL);
public:
	/**
	@brief ��ȡ����ullSid����Сbinlog header
	@param [in] ullSid Ҫ���ҵ�sid��
	@param [out] index ����������binlog index��
	@return -1��ʧ�ܣ�0�������ڣ�1������
	*/
	int upper(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K=NULL);
	/**
	@brief ��ȡ������ullSid�����binlog header
	@param [in] ullSid Ҫ���ҵ�sid��
	@param [out] index ����������binlog index��
	@return -1��ʧ�ܣ�0�������ڣ�1������
	*/
	int lower(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K=NULL);

	/**
    @brief ����binlog��ȡ���α�
    @return NULL��ʧ�ܣ����򷵻��α�����ָ�롣
    */
	CwxBinLogCursor* createCurser(CWX_UINT64 ullSid=0, CWX_UINT8 ucState=CwxBinLogCursor::CURSOR_STATE_UNSEEK);
    
    /**
    @brief ��binlog��ȡ���α��Ƶ�>ullSid��binlog����
    @param [in] pCursor binlog�Ķ�ȡ�αꡣ
    @param [in] ullSid ��λ�α��sid���α꽫��λ��>ullSid��binlog����
    @return -1��ʧ�ܣ�0���޷���λ��ullSid��һ��binlog��1����λ��ullSid��һ����binlog�ϡ�
    */
    int seek(CwxBinLogCursor* pCursor, CWX_UINT64 ullSid);

    /**
    @brief ���α��Ƶ���һ��binlog��¼�������д�����ͨ��pCursor��getErrMsg()��ȡ��
    @param [in] pCursor �α�Ķ���ָ�롣
    @return -1��ʧ�ܣ�0���Ƶ����1���ɹ��Ƶ���һ��binlog��
    */
    int next(CwxBinLogCursor* pCursor);
    /**
    @brief ���α��Ƶ�ǰһ��binlog��¼�������д�����ͨ��pCursor��getErrMsg()��ȡ��
    @param [in] pCursor �α�Ķ���ָ�롣
    @return -1��ʧ�ܣ�0���Ƶ��ʼ��1���ɹ��Ƶ�ǰһ��binlog��
    */
    int prev(CwxBinLogCursor* pCursor);

    /**
    @brief ��ȡ�α�ĵ�ǰbinlog�����д�����ͨ��pCursor��getErrMsg()��ȡ��
    @param [in] pCursor �α�Ķ���ָ�롣
    @param [out] szData binlog��data��
    @param [in,out] uiDataLen ����szData��buf��С������szData�Ĵ�С��
    @return -1��ʧ�ܣ�0���ɹ���ȡ��һ��binlog��
    */
    int fetch(CwxBinLogCursor* pCursor, char* szData, CWX_UINT32& uiDataLen);
    /**
    @brief ��ȡ��һ����¼�����д�����ͨ��pCursor��getErrMsg()��ȡ��
    @param [in] pCursor �α�Ķ���ָ�롣
    @param [out] szData binlog��data��
    @param [in,out] uiDataLen ����szData��buf��С������szData�Ĵ�С��
    @return -1��ʧ�ܣ�0���Ƶ����1���ɹ���ȡ��һ��binlog��
    */
    int next(CwxBinLogCursor* pCursor, char* szData, CWX_UINT32& uiDataLen);
    /**
    @brief ��ȡǰһ��binlog��¼�����д�����ͨ��pCursor��getErrMsg()��ȡ��
    @param [in] pCursor �α�Ķ���ָ�롣
    @param [out] szData binlog��data��
    @param [in,out] uiDataLen ����szData��buf��С������szData�Ĵ�С��
    @return -1��ʧ�ܣ�0���Ƶ��ʼ��1���ɹ���ȡǰһ��binlog��
    */
    int prev(CwxBinLogCursor* pCursor, char* szData, CWX_UINT32& uiDataLen);
    /**
    @brief �ͷ�cursor��
    @param [in] pCursor Ҫ�ͷŵ��αꡣ
    @return -1��ʧ�ܣ�0���ɹ���
    */
    int destoryCurser(CwxBinLogCursor*& pCursor);
    /**
    @brief ��ȡ��û�ж�ȡ����־������
    @param [in] pCursor �αꡣ
    @return -1���α���Ч������Ϊ��¼��������
    */
    CWX_INT64 leftLogNum(CwxBinLogCursor const* pCursor);
    /**
    @brief ��ȡ����ָ��ʱ����־��log�ļ�����ʼsid
    @param [in] ttTimestamp  ��־ʱ��
    @return ����sid��
    */
    CWX_UINT64 getFileStartSid(CWX_UINT32 ttTimestamp);
    /**
    @brief ��ȡ���µĵ�N����¼��sidֵ
    @param [in] uiNo ��¼��
    @param [out] ullSid sid��ֵ
    @return true���ɹ���false��ʧ�ܡ�
    */
    bool getLastSidByNo(CWX_UINT32 uiNo, CWX_UINT64& ullSid, char* szErr2K);


public:
    ///���ó�ʼ��SID
    inline void setNextSid(CWX_UINT64 ullSid) {
        if (ullSid < getMaxSid())
            ullSid = getMaxSid() + SKIP_SID_NUM;
        m_ullNextSid = ullSid;
    }
    ///��ȡ��ǰ��sid
    inline CWX_UINT64 getCurNextSid() const{
        return m_ullNextSid;
    }
    ///��ȡ�������Ƿ���Ч
    inline bool isInvalid() const;
    ///cursor��Ӧ���ļ����Ƿ��ڹ���ķ�Χ֮��
    inline bool isOutRange(CwxBinLogCursor* pCursor);
    ///�Ƿ���unseek
    inline bool isUnseek(CwxBinLogCursor* pCursor);
    ///��ȡ��������Ч��ԭ��
    char const* getInvalidMsg() const;
    ///��ȡ��С��sid
    inline CWX_UINT64 getMinSid() ;
    ///��ȡ����sid
    inline CWX_UINT64 getMaxSid() ;
    ///��ȡbinlog����Сʱ���
    inline CWX_UINT32 getMinTimestamp() ;
    ///��ȡbinlog�����ʱ���
    inline CWX_UINT32 getMaxTimestamp();
    ///��ȡ�����binlog����С�ļ����
    inline string& getMinFile(string& strFile) ;
    ///��ȡ�����binlog������ļ����
    inline string& getMaxFile(string& strFile);
    ///����Ƿ�Ϊ��
    inline bool empty() ;
    ///��ȡ�ļ��Ŷ�Ӧ��binlog�ļ���
    inline string& getFileNameByFileNo(CWX_UINT32 uiFileNo, CWX_UINT32 ttDay, string& strFileName);
    ///��ȡ�ļ��Ŷ�Ӧ��binlog�ļ��������ļ���
    inline string& getIndexFileNameByFileNo(CWX_UINT32 uiFileNo, CWX_UINT32 ttDay, string& strFileName);
    ///����binlog�ļ�������ȡ�ļ���
    inline CWX_UINT32 getBinLogFileNo(string const& strFileName, CWX_UINT32& ttDay);
    ///�ж�һ���ļ����Ƿ���һ��binlog�ļ�
    inline bool isBinLogFile(string const& strFileName);
    ///�ж�һ���ļ����Ƿ���һ��binlog�������ļ�
    inline bool isBinLogIndexFile(string const& strFileName);
    ///��ȡbinlog��ǰ׺��
    inline  string const& getBinlogPrexName() const;
    ///�Ƿ���Ч��ǰ׺��
    static inline bool isValidPrexName(char const* szName);

private:
    ///���binlog������
    void _clear();
	/**
	@brief ��ȡ����ullSid����Сbinlog header
	@param [in] ullSid Ҫ���ҵ�sid��
	@param [out] index ����������binlog index��
	@return -1��ʧ�ܣ�0�������ڣ�1������
	*/
	int _upper(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K=NULL);
	/**
	@brief ��ȡ������ullSid�����binlog header
	@param [in] ullSid Ҫ���ҵ�sid��
	@param [out] index ����������binlog index��
	@return -1��ʧ�ܣ�0�������ڣ�1������
	*/
	int _lower(CWX_UINT64 ullSid, CwxBinLogIndex& index, char* szErr2K=NULL);
    /**
    @brief ��binlog��ȡ���α��Ƶ�>ullSid��binlog����
    @param [in] pCursor binlog�Ķ�ȡ�αꡣ
    @param [in] ullSid ��λ�α��sid���α꽫��λ��>ullSid��binlog����
    @return -1��ʧ�ܣ�0���޷���λ��ullSid��һ��binlog��1����λ��ullSid��һ����binlog�ϡ�
    */
    int _seek(CwxBinLogCursor* pCursor, CWX_UINT64 ullSid);
    ///���һ��binlog�ļ��Ƿ�Ӧ�ñ�����
    inline bool _isManageBinLogFile(CwxBinLogFile* pBinLogFile) ;
    ///cursor��Ӧ���ļ����Ƿ��ڹ���ķ�Χ֮��
    inline bool _isOutRange(CwxBinLogCursor*& pCursor);
    ///��ȡ��С��binlog�ļ�
    inline CwxBinLogFile* _getMinBinLogFile() ;
    ///��ȡ����binlog�ļ�
    inline CwxBinLogFile* _getMaxBinLogFile() ;
    ///��������binlog�ļ���Ϣ
    void  _outputManageBinLog() ;
    ///append����
    int _append(CWX_UINT64 ullSid,
        CWX_UINT32 ttTimestamp,
        CWX_UINT32 uiGroup,
        char const* szData,
        CWX_UINT32 uiDataLen,
        char* szErr2K=NULL);

private:
    string					  m_strLogPath; ///<binlog�ļ��ĸ�Ŀ¼
    string                    m_strPrexLogPath; ///<ָ��ǰ׺��binlog�ļ���Ŀ¼
    string					  m_strFilePrex; ///<binlog�ļ���ǰ׺��
    bool                      m_bDelOutManageLogFile; ///<�Ƿ�ɾ�����ڹ����ڵ��ļ�
    CWX_UINT32                m_uiMaxFileSize; ///<binlog�ļ�������С
    CWX_UINT32                m_uiMaxFileNum; ///<�����binlog���������
	bool					  m_bCache;  ///<�Ƿ��д������ݽ���cache
    char                      m_szErr2K[2048]; ///<binlog ��������Ч��ԭ��
    int                       m_fdLock; ///<ϵͳ���ļ����
    CwxRwLock                 m_rwLock; ///<binlog�Ķ�д��
    ///һ�±������ڶ�д������֮��
    bool                      m_bValid; ///<binlog �������Ƿ���Ч��
    map<CWX_UINT32/*file no*/, CwxBinLogFile*>   m_binlogMap; ///<������ǰbinlog�ļ���binlog�ļ���map
	set<CwxBinLogCursor*>     m_cursorSet; ///<����������cursor�ļ���
    CwxBinLogFile*            m_pCurBinlog;///<��ǰд��binlog�ļ�
    CWX_UINT64                m_ullMinSid; ///<binlog�ļ�����Сsid
    CWX_UINT64                m_ullMaxSid; ///<binlog�ļ������sid
    CWX_UINT32                m_ttMinTimestamp; ///<binlog�ļ���log��ʼʱ��
    CWX_UINT32                m_ttMaxTimestamp; ///<binlog�ļ���log����ʱ��
    CWX_UINT64                m_ullNextSid; ///<һ��һ��sid��ֵ
    CWX_UINT32                m_uiFlushBinLogNum; ///<����binlog�Զ�flush
    CWX_UINT32                m_uiFlushBinLogTime; ///<����ʱ���Զ�flush
    CWX_UINT32				  m_uiUnFlushBinlog; ///<δflush��binlog������
    CWX_UINT32				  m_ttLastFlushBinlogTime; ///<��һ��flushbinlog��ʱ��

};


#include "CwxBinLogMgr.inl"

#endif
