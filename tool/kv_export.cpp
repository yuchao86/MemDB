#include "CwxSocket.h"
#include "CwxINetAddr.h"
#include "CwxSockStream.h"
#include "CwxSockConnector.h"
#include "CwxGetOpt.h"
#include "UnistorPoco.h"
#include "CwxFile.h"
#include "UnistorStore.h"

using namespace cwinux;
string g_strHost;
CWX_UINT16 g_unPort = 0;
string g_confFile;
string g_key;
string g_extra;
string g_subscribe;
CWX_UINT32 g_uiExpire=0;
CWX_UINT32 g_uiChunkSize = 64;
string g_user;
string g_passwd;
string g_output;
CWX_UINT64 g_lastStartSid = 0;
CWX_UINT64 g_lastEndSid = 0;
bool       g_bNewSid = false;
CWX_UINT64 g_startSid = 0;
CWX_UINT64 g_endSid = 0;
CWX_UINT64 g_session = 0;

FILE* g_fdout = NULL; ///日志输出文件句柄
UnistorConfig g_config; ///配置文件信息
UnistorStore g_store;  ///存储引擎
UnistorTss   g_tss;  ///tss对象
CwxSockStream  g_stream; ///连接的stream
CwxSockConnector g_conn; ///连接
CwxPackageReaderEx  g_reader1; ///reader1
CwxPackageReaderEx  g_reader2; ///reader2


///-1：失败；0：help；1：成功
int parseArg(int argc, char**argv)
{
    CwxGetOpt cmd_option(argc, argv, "H:P:C:c:k:X:S:t:u:p:o:B:E:Nh");
    int option;
    while( (option = cmd_option.next()) != -1)
    {
        switch (option)
        {
        case 'h':
            printf("Export data from unistor.\n");
            printf("-H: Server host\n");
            printf("-P: Server port\n");
            printf("-C: The conf file for local store engine.\n");
            printf("-c: Chunk Kbyte for export data, 64 for default.\n");
            printf("-k: The start key to export.\n");
            printf("-X: Import data's engine extra data.\n");
            printf("-S: The subscribe for export.\n");
			printf("-t: Key's timeout second to set.\n");
            printf("-B: The last start sid. \n");
            printf("-E: The last end sid.\n");
            printf("-N: re-set the binlog's sid.\n");
            printf("-u: User name.\n");
            printf("-p: User password.\n");
            printf("-o: Output log file.\n");
            printf("-h: Help\n");
            return 0;
        case 'H':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-H requires an argument.\n");
                return -1;
            }
            g_strHost = cmd_option.opt_arg();
            break;
        case 'P':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-P requires an argument.\n");
                return -1;
            }
            g_unPort = strtoul(cmd_option.opt_arg(), NULL, 10);
            break;
        case 'C':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-C requires an argument.\n");
                return -1;
            }
            g_confFile = cmd_option.opt_arg();
            break;
        case 'c':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-c requires an argument.\n");
                return -1;
            }
            g_uiChunkSize = strtoul(cmd_option.opt_arg(), NULL, 10);
            break;
        case 'k':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-k requires an argument.\n");
                return -1;
            }
            g_key = cmd_option.opt_arg();
            break;
        case 'X':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-X requires an argument.\n");
                return -1;
            }
            g_extra = cmd_option.opt_arg();
            break;
        case 'S':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-S requires an argument.\n");
                return -1;
            }
            g_subscribe = cmd_option.opt_arg();
            break;
        case 'o':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-o requires an argument.\n");
                return -1;
            }
            g_output = cmd_option.opt_arg();
            break;
		case 't':
			if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
			{
				printf("-t requires an argument.\n");
				return -1;
			}
			g_uiExpire = strtoul(cmd_option.opt_arg(), NULL, 10);
            if (g_uiExpire < 365 * 24 * 3600){
                g_uiExpire += (CWX_UINT32)time(NULL);
            }
			break;
        case 'B':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-B requires an argument.\n");
                return -1;
            }
            g_lastStartSid = strtoull(cmd_option.opt_arg(), NULL, 10);
            break;
        case 'E':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-E requires an argument.\n");
                return -1;
            }
            g_lastEndSid = strtoull(cmd_option.opt_arg(), NULL, 10);
            break;
        case 'N':
            g_bNewSid = true;
            break;
        case 'u':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-u requires an argument.\n");
                return -1;
            }
            g_user = cmd_option.opt_arg();
            break;
        case 'p':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-p requires an argument.\n");
                return -1;
            }
            g_passwd = cmd_option.opt_arg();
            break;
        case ':':
            printf("%c requires an argument.\n", cmd_option.opt_opt ());
            return -1;
        case '?':
            break;
        default:
            printf("Invalid arg %s.\n", argv[cmd_option.opt_ind()-1]);
            return -1;
        }
    }
    if (-1 == option)
    {
        if (cmd_option.opt_ind()  < argc)
        {
            printf("Invalid arg %s.\n", argv[cmd_option.opt_ind()]);
            return -1;
        }
    }
    if (!g_strHost.length())
    {
        printf("No host, set by -H\n");
        return -1;
    }
    if (!g_unPort)
    {
        printf("No port, set by -P\n");
        return -1;
    }
    if (!g_confFile.length())
    {
        printf("No conf file, set by -C\n");
        return -1;
    }
    return 1;
}

int msg_notice_fn(void* , CwxMsgBlock* msg,  bool , char* ){
    CwxMsgBlockAlloc::free(msg);
    return 0;
}

int get_sys_info_fn(void* , ///<app对象
                  char const* , ///<要获取的key
                  CWX_UINT16 , ///<key的长度
                  char* , ///<若存在，则返回数据。内存有存储引擎分配
                  CWX_UINT32&   ///<szData数据的字节数
                  )
{
    return 0;
}

bool initOutFile(FILE*& fd){
    if (g_output.length()){
        fd = fopen(g_output.c_str(), "a+");
        if (!fd){
            printf("Failure to open output log file:%s", g_output.c_str());
            return false;
        }
    }
    fd = stdout;
    return true;
}

bool initEnv(){
    ///初始化日志输出
    if (g_output.length()){
        g_fdout = fopen(g_output.c_str(), "a+");
        if (!g_fdout){
            printf("Failure to open output log file:%s", g_output.c_str());
            return false;
        }
    }else{
        g_fdout = stdout;
    }
    ///初始化tss
    if (0 != g_tss.init(NULL)){
        fprintf(g_fdout, "Failure to init tss\n");
        return false;
    }
    ///初始化配置文件
    if (0 != g_config.init(g_confFile)){
        fprintf(g_fdout, "Failure to init conf file:%s, err=%s\n", g_confFile.c_str(), g_config.getErrMsg());
        return false;
    }
    ///初始化存储引擎
    string strEngine=g_config.getCommon().m_strWorkDir + "engine/";
    if (0 != g_store.init(msg_notice_fn, get_sys_info_fn, NULL, &g_config, strEngine, g_tss.m_szBuf2K)){
        fprintf(g_fdout, "Failure to init engine, err=%s\n", g_tss.m_szBuf2K);
        return false;
    }
    return true;
}

bool connect(){
    ///初始化网路
    if (g_stream.getHandle() != -1){
        g_stream.close();
    }
    CwxINetAddr  addr(g_unPort, g_strHost.c_str());
    if (0 != g_conn.connect(g_stream, addr)){
        fprintf(g_fdout, "Failure to connect ip:port: %s:%u, errno=%d\n", g_strHost.c_str(), g_unPort, errno);
        return false;
    }
    return true;
}
///重新计算开始的sid
void reCalcStartSid(){
    if (g_lastStartSid){
        if (!g_bNewSid){
            if (g_startSid < g_store.getBinLogMgr()->getMaxSid()){
                g_startSid = g_store.getBinLogMgr()->getMaxSid();
            }else{
                g_startSid = g_lastStartSid;
            }
        }else{
            g_startSid = g_lastStartSid;
        }
    }
}
///报告export点
bool reportExport(){
    CwxMsgBlock* block=NULL;
    CwxMsgHead head;
    int iRet = 0;
    char const* szErrMsg = NULL;
    ///report
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packExportReport(g_tss.m_pWriter,
        block,
        0,
        g_uiChunkSize,
        g_subscribe.c_str(),
        g_key.length()?g_key.c_str():NULL,
        g_extra.length()?g_extra.c_str():NULL,
        g_user.c_str(),
        g_passwd.c_str(),
        g_tss.m_szBuf2K))
    {
        fprintf(g_fdout, "Failure to pack export report package, err=%s\n", g_tss.m_szBuf2K);
        return false;
    }
    if (block->length() != (CWX_UINT32)CwxSocket::write_n(g_stream.getHandle(),
        block->rd_ptr(),
        block->length()))
    {
        fprintf(g_fdout, "failure to send export report message, errno=%d\n", errno);
        CwxMsgBlockAlloc::free(block);
        return false;
    }
    CwxMsgBlockAlloc::free(block);
    ///读取回复
    if (0 >= CwxSocket::read(g_stream.getHandle(), head, block)){
        fprintf(g_fdout, "failure to read export report reply, errno=%d\n", errno);
        return false;
    }
    if (UnistorPoco::MSG_TYPE_EXPORT_REPORT_REPLY == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseExportReportReply(g_tss.m_pReader,
            block,
            g_session,
            g_startSid,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack export report reply msg, err=%s\n", g_tss.m_szBuf2K);
            CwxMsgBlockAlloc::free(block);
            return false;
        }
        CwxMsgBlockAlloc::free(block);
        reCalcStartSid();
        fprintf(g_fdout, "Start sid:%s\n", CwxCommon::toString(g_startSid, g_tss.m_szBuf2K, 10));
        return true;
    }else if (UnistorPoco::MSG_TYPE_SYNC_ERR == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncErr(g_tss.m_pReader,
            block,
            iRet,
            szErrMsg,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack export report reply err msg, err=%s\n", g_tss.m_szBuf2K);
        }else{
            fprintf(g_fdout, "failure to export report ,err-code=%d, err-msg=%s\n", iRet, szErrMsg);
        }
    }else{
        fprintf(g_fdout, "recieve unknown export-reply, msg-type=%u\n", head.getMsgType());
    }
    CwxMsgBlockAlloc::free(block);
    return false;
}

///接受数据。0：完成；1：获取了一组数据；-1：失败；
int  recvExportData(){
    CwxMsgBlock* block=NULL;
    CwxMsgHead head;
    int iRet = 0;
    char const* szErrMsg = NULL;
    CWX_UINT64 ullSeq=0;
    CwxKeyValueItemEx const* key=NULL;
    CwxKeyValueItemEx const* data=NULL;
    CwxKeyValueItemEx const* extra =NULL;
    CWX_UINT32 uiVersion=0;
    CWX_UINT32 uiExpire=0;
    bool bReadCached = false;
    bool bWriteCached = false;

    static CWX_UINT32 uiNum = 0;
    ///读取export数据
    if (0 >= CwxSocket::read(g_stream.getHandle(), head, block)){
        fprintf(g_fdout, "failure to read export data, errno=%d\n", errno);
        return -1;
    }
    if (UnistorPoco::MSG_TYPE_EXPORT_DATA == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseMultiExportData(&g_reader1,
            block,
            ullSeq,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack export data msg, err=%s\n", g_tss.m_szBuf2K);
            CwxMsgBlockAlloc::free(block);
            return -1;
        }
        ///插入数据
        for (CWX_UINT32 i=0; i<g_reader1.getKeyNum(); i++){
            if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseExportDataItem(&g_reader2,
                g_reader1.getKey(i)->m_szData,
                g_reader1.getKey(i)->m_uiDataLen,
                key,
                data,
                extra,
                uiVersion,
                uiExpire,
                g_tss.m_szBuf2K))
            {
                fprintf(g_fdout, "Failure to unpack export data item, err=%s\n", g_tss.m_szBuf2K);
                CwxMsgBlockAlloc::free(block);
                return -1;
            }
            ///重新设置expire
            if (g_uiExpire){
                uiExpire = g_uiExpire;
            }
            iRet = g_store.syncImportKey(&g_tss,
                *key,
                extra,
                *data,
                uiVersion,
                false,
                uiExpire,
                g_startSid?g_startSid:1,
                bReadCached,
                bWriteCached,
                true);
            if (-1 == iRet){
                fprintf(g_fdout, "Failure to save export data, key=%s, err:%s\n", key->m_szData, g_tss.m_szBuf2K);
                CwxMsgBlockAlloc::free(block);
                return -1;
            }else if (0 == iRet){
                fprintf(g_fdout, "Not save export data, key=%s\n", key->m_szData);
                CwxMsgBlockAlloc::free(block);
                return -1;
            }
            uiNum++;
            if (uiNum%10000 == 0){
                g_store.checkpoint(&g_tss);
                time_t now =time(NULL);
                fprintf(g_fdout, "export key=%s,  num=%u, time=%s\n", key->m_szData, uiNum, ctime(&now));
            }
            fprintf(g_fdout, "export key=%s, extra=%s\n", key->m_szData, extra?extra->m_szData:"");
        }
        CwxMsgBlockAlloc::free(block);
        ///回复消息
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packExportDataReply(g_tss.m_pWriter, block, 0, ullSeq, g_tss.m_szBuf2K)){
            fprintf(g_fdout, "Failure to pack export data reply, err=%s\n", g_tss.m_szBuf2K);
            return -1;
        }
        if (block->length() != (CWX_UINT32)CwxSocket::write_n(g_stream.getHandle(),
            block->rd_ptr(),
            block->length()))
        {
            fprintf(g_fdout, "failure to send export data reply, errno=%d\n", errno);
            CwxMsgBlockAlloc::free(block);
            return -1;
        }
        CwxMsgBlockAlloc::free(block);
        return 1;
    }else if (UnistorPoco::MSG_TYPE_EXPORT_END == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseExportEnd(g_tss.m_pReader,
            block,
            g_endSid,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack export end msg, err=%s\n", g_tss.m_szBuf2K);
            CwxMsgBlockAlloc::free(block);
            return -1;
        }
        CwxMsgBlockAlloc::free(block);
        return 0;

    }else if (UnistorPoco::MSG_TYPE_SYNC_ERR == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncErr(g_tss.m_pReader,
            block,
            iRet,
            szErrMsg,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack export data  err msg, err=%s\n", g_tss.m_szBuf2K);
        }else{
            fprintf(g_fdout, "failure to export ,err-code=%d, err-msg=%s\n", iRet, szErrMsg);
        }
    }else{
        fprintf(g_fdout, "recieve unknown export-reply, msg-type=%u\n", head.getMsgType());
    }
    CwxMsgBlockAlloc::free(block);
    return -1;

}

///数据同步报告
bool reportSync(){
    CwxMsgBlock* block=NULL;
    CwxMsgHead head;
    int iRet = 0;
    char const* szErrMsg = NULL;
    ///report
    if (UNISTOR_ERR_SUCCESS != UnistorPoco::packReportData(g_tss.m_pWriter,
        block,
        0,
        g_startSid,
        false,
        g_uiChunkSize,
        g_subscribe.c_str(),
        g_user.c_str(),
        g_passwd.c_str(),
        NULL,
        false,
        g_tss.m_szBuf2K))
    {
        fprintf(g_fdout, "Failure to pack sync report package, err=%s\n", g_tss.m_szBuf2K);
        return false;
    }
    if (block->length() != (CWX_UINT32)CwxSocket::write_n(g_stream.getHandle(),
        block->rd_ptr(),
        block->length()))
    {
        fprintf(g_fdout, "failure to send sync report message, errno=%d\n", errno);
        CwxMsgBlockAlloc::free(block);
        return false;
    }
    CwxMsgBlockAlloc::free(block);
    ///读取回复
    if (0 >= CwxSocket::read(g_stream.getHandle(), head, block)){
        fprintf(g_fdout, "failure to read sync report reply, errno=%d\n", errno);
        return false;
    }
    if (UnistorPoco::MSG_TYPE_SYNC_REPORT_REPLY == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseReportDataReply(g_tss.m_pReader,
            block,
            g_session,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack sync report reply msg, err=%s\n", g_tss.m_szBuf2K);
            CwxMsgBlockAlloc::free(block);
            return false;
        }
        CwxMsgBlockAlloc::free(block);
        return true;
    }else if (UnistorPoco::MSG_TYPE_SYNC_ERR == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncErr(g_tss.m_pReader,
            block,
            iRet,
            szErrMsg,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack sync report reply err msg, err=%s\n", g_tss.m_szBuf2K);
        }else{
            fprintf(g_fdout, "failure to  report sync,err-code=%d, err-msg=%s\n", iRet, szErrMsg);
        }
    }else{
        fprintf(g_fdout, "recieve unknown sync-report-reply, msg-type=%u\n", head.getMsgType());
    }
    CwxMsgBlockAlloc::free(block);
    return false;
}
///同步binlog。0：完成；1：获取了一组数据；-1：失败；
int recvBinlog(){
    CwxMsgBlock* block=NULL;
    CwxMsgHead head;
    int iRet = 0;
    char const* szErrMsg = NULL;
    CWX_UINT64 ullSeq=0;
    CWX_UINT64 ullSid;
    CWX_UINT32 uiVersion;
    CWX_UINT32 ttTimestamp;
    CWX_UINT32 uiGroup;
    CWX_UINT32 uiType;
    CwxKeyValueItemEx const* data;
    CWX_UINT32 uiNum = 0;
    ///读取export数据
    if (0 >= CwxSocket::read(g_stream.getHandle(), head, block)){
        fprintf(g_fdout, "failure to read export data, errno=%d\n", errno);
        return -1;
    }
    if (UnistorPoco::MSG_TYPE_SYNC_DATA_CHUNK == head.getMsgType()){
        ullSeq =  UnistorPoco::getSeq(block->rd_ptr());
        if (!g_reader1.unpack(block->rd_ptr() + sizeof(ullSeq), block->length() - sizeof(ullSeq), false, true)){
            fprintf(g_fdout, "Failure to unpack sync-binlog, err:%s\n", g_reader1.getErrMsg());
            CwxMsgBlockAlloc::free(block);
            return -1;
        }
        for (CWX_UINT32 i=0; i<g_reader1.getKeyNum(); i++){
            if(0 != strcmp(g_reader1.getKey(i)->m_szKey, UNISTOR_KEY_M)) {
                fprintf(g_fdout, "binlog's key must be:%s, but:%s\n", UNISTOR_KEY_M, g_reader1.getKey(i)->m_szKey);
                CwxMsgBlockAlloc::free(block);
                return -1;
            }
            ///获取binlog的数据
            if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncData(&g_reader2,
                g_reader1.getKey(i)->m_szData,
                g_reader1.getKey(i)->m_uiDataLen,
                ullSid,
                ttTimestamp,
                data,
                uiGroup,
                uiType,
                uiVersion,
                g_tss.m_szBuf2K))
            {
                fprintf(g_fdout, "Failure to parse binlog from master, err=%s\n", g_tss.m_szBuf2K);
                CwxMsgBlockAlloc::free(block);
                return -1;
            }
            CwxKeyValueItemEx item = *data;
            if (0 != g_store.syncMasterBinlog(&g_tss,
                g_tss.m_pReader,
                g_bNewSid?0:ullSid,
                ttTimestamp,
                uiGroup,
                uiType,
                item,
                uiVersion,
                false))
            {
                fprintf(g_fdout, "Failure to sync master binlog , err=%s\n", g_tss.m_szBuf2K);
                CwxMsgBlockAlloc::free(block);
                return -1;
            }
            uiNum++;
            if (uiNum%10000 == 0){
                g_store.checkpoint(&g_tss);
                time_t now =time(NULL);
                fprintf(g_fdout, "export sid=%s,  num=%u, time=%s\n", CwxCommon::toString(ullSid, g_tss.m_szBuf2K, 10), uiNum, ctime(&now));
            }
            fprintf(g_fdout, "Sync sid=%s\n", CwxCommon::toString(ullSid, g_tss.m_szBuf2K, 10));
            if (g_endSid <= ullSid){
                CwxMsgBlockAlloc::free(block);
                return 0;
            }
        }
        CwxMsgBlockAlloc::free(block);

        ///回复消息
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packSyncDataReply(g_tss.m_pWriter, block, 0, ullSeq, UnistorPoco::MSG_TYPE_SYNC_DATA_CHUNK_REPLY, g_tss.m_szBuf2K)){
            fprintf(g_fdout, "Failure to pack sync binlog reply, err=%s\n", g_tss.m_szBuf2K);
            return -1;
        }
        if (block->length() != (CWX_UINT32)CwxSocket::write_n(g_stream.getHandle(),
            block->rd_ptr(),
            block->length()))
        {
            fprintf(g_fdout, "failure to send sync binlog reply, errno=%d\n", errno);
            CwxMsgBlockAlloc::free(block);
            return -1;
        }
        CwxMsgBlockAlloc::free(block);
        return 1;
    }else if (UnistorPoco::MSG_TYPE_SYNC_ERR == head.getMsgType()){
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseSyncErr(g_tss.m_pReader,
            block,
            iRet,
            szErrMsg,
            g_tss.m_szBuf2K))
        {
            fprintf(g_fdout, "failure to unpack export data  err msg, err=%s\n", g_tss.m_szBuf2K);
        }else{
            fprintf(g_fdout, "failure to export ,err-code=%d, err-msg=%s\n", iRet, szErrMsg);
        }
    }else{
        fprintf(g_fdout, "recieve unknown export-reply, msg-type=%u\n", head.getMsgType());
    }
    CwxMsgBlockAlloc::free(block);
    return -1;
}

void closeEnv(){
    if (g_output.length() && g_fdout) fclose(g_fdout);
    g_store.close();
}

int main(int argc ,char** argv){
    ///获取参数
    int iRet = parseArg(argc, argv);
    if (0 == iRet) return 0;
    if (-1 == iRet) return 1;
    ///初始化输出文件
    if (!initEnv()){
        closeEnv();
        return 1;
    }
    if (!g_lastEndSid){///需要导出数据
        ///初始化网路
        if (!connect()){
            closeEnv();
            return 1;
        }
        ///report export的数据
        if (!reportExport()){
            closeEnv();
            return 1;
        }
        ///同步数据
        ///设置sid
        g_store.setCurSid(g_startSid==0?1:g_startSid);
        while(1){
            iRet = recvExportData();
            if (1 != iRet) break;
        }
        if (-1 == iRet){
            closeEnv();
            return 1; ///失败
        }
    }else{
        reCalcStartSid();
    }
    ///开始导出binlog
    ///重新初始化网络
    if (!connect()){
        closeEnv();
        return 1;
    }
    ///report 同步binlog的信息
    if (!reportSync()){
        closeEnv();
        return 1;
    }
    ///同步binlog
    while(1){
        iRet = recvBinlog();
        if (1 != iRet) break;
    }
    if (-1 == iRet){
        closeEnv();
        return 1;
    }
    ///保存数据
    if (0 != g_store.commit(g_tss.m_szBuf2K)){
        closeEnv();
        return 1;
    }
    ///二次提交数据
    if (0 != g_store.commit(g_tss.m_szBuf2K)){
        closeEnv();
        return 1;
    }
    closeEnv();
    return 0;
}
