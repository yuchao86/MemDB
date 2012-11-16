#ifndef __CWX_BINLOG_OP_H__
#define __CWX_BINLOG_OP_H__

/**
@file CwxBinlogOp.h
@brief binlog �ļ����������
@author cwinux@gmail.com
@version 0.1
@date 2009-10-10
@warning
@bug
*/
#include "CwxPackageReaderEx.h"
#include "CwxBinLogMgr.h"

CWINUX_USING_NAMESPACE

class CwxBinlogOp{
public:
    enum{
        BINLOG_INDEX_INTERNAL = 100, ///<binlog�������������ȣ�N,2N,3N......
        BINLOG_COMPRESS_RATE  = 10   ///<����ѹ�����ݣ���Ĭ��ѹ����Ϊ10��
    };
public:
    CwxBinlogOp();
    ~CwxBinlogOp();
    int init(string const& strBinLogFileName);
    void run();
private:
    void clear();
    void prompt();
    //0��exit��1��continue
    int doCommand(char* szCmd);
    void doHelp();
    void doInfo();
    void doNext(CWX_UINT32 uiNum);
    void doPrev(CWX_UINT32 uiNum);
    void doSid(CWX_UINT64 ullSid);
    void doRecord(CWX_UINT32 uiRecord);
    void doGroup(CWX_UINT32 uiGroup);
    void doKey(char const* szKey, char const* szValue);
    void doHead();
    void doData();
    void doSave(string const& strFileName);
//    void doDump(bool bHead, string const& strFileName);

    bool prepareBuf(CWX_UINT32 uiLen);
    bool prepareUnBuf(CWX_UINT32 uiLen);
    bool prepareDumpBuf(CWX_UINT32 uiLen);
    char* getData(CWX_UINT32& uiLen);

private:
    map<CWX_UINT64/*sid*/, CWX_UINT32/*offset*/>  m_sidIndex; ///<ÿBINLOG_INDEX_INTERNAL����¼����һ������
    map<CWX_UINT32/*rec_no*/, CWX_UINT32/*offset*/> m_recIndex; ///<ÿBINLOG_INDEX_INTERNAL����¼��������
    CwxBinLogCursor*     m_pCursor; ///<binlog�Ķ�ȡcursor
    char*               m_pBuf; ///<data��ȡ��buf
    CWX_UINT32          m_uiBufLen; ///<data��ȡbuf�Ŀռ��С
    char*               m_pUnBuf; ///<data��ѹbuf
    CWX_UINT32          m_uiUnBufLen; ///<data��ѹbuf�Ŀռ��С
    char*               m_pDumpBuf; ///<data dump buf
    CWX_UINT32           m_uiDumpBufLen; ///<data dump buf�Ŀռ��С
    CWX_UINT64          m_ullMinSid; ///<��С��sidֵ
    CWX_UINT64          m_ullMaxSid; ///<����sidֵ
    CWX_UINT32          m_uiRecNum; ///<binlog������
    string              m_strLogFileName; ///<binlog�ļ�������
    char                m_szErr2K[2048]; ///<����ʱ�Ĵ�����Ϣ
};

#endif
