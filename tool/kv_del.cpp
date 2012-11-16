#include "CwxSocket.h"
#include "CwxINetAddr.h"
#include "CwxSockStream.h"
#include "CwxSockConnector.h"
#include "CwxGetOpt.h"
#include "UnistorPoco.h"

using namespace cwinux;
string g_strHost;
CWX_UINT16 g_unPort = 0;
CWX_UINT32 g_version = 0;
string g_key;
string g_field;
string g_extra;
string g_user;
string g_passwd;
///-1£ºÊ§°Ü£»0£ºhelp£»1£º³É¹¦
int parseArg(int argc, char**argv)
{
    CwxGetOpt cmd_option(argc, argv, "H:P:k:f:X:v:u:p:h");
    int option;
    while( (option = cmd_option.next()) != -1)
    {
        switch (option)
        {
        case 'h':
            printf("delete a key or key's field.\n");
            printf("%s  -H host -P port -k key.....\n", argv[0]);
            printf("-H: server host\n");
            printf("-P: server port\n");
			printf("-k: key name\n");
            printf("-f: field name\n");
            printf("-X: engine extra data\n");
			printf("-v: key's version.\n");
            printf("-u: user name.\n");
            printf("-p: user password.\n");
            printf("-h: help\n");
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
        case 'k':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-k requires an argument.\n");
                return -1;
            }
            g_key = cmd_option.opt_arg();
            break;
        case 'f':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-F requires an argument.\n");
                return -1;
            }
            g_field = cmd_option.opt_arg();
            break;
        case 'X':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-X requires an argument.\n");
                return -1;
            }
            g_extra = cmd_option.opt_arg();
            break;
		case 'v':
			if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
			{
				printf("-s requires an argument.\n");
				return -1;
			}
			g_version = strtoul(cmd_option.opt_arg(), NULL, 10);
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
    if (!g_key.length())
    {
        printf("No key, set by -k\n");
        return -1;
    }
    return 1;
}

int main(int argc ,char** argv)
{
    int iRet = parseArg(argc, argv);

    if (0 == iRet) return 0;
    if (-1 == iRet) return 1;

    CwxSockStream  stream;
    CwxINetAddr  addr(g_unPort, g_strHost.c_str());
    CwxSockConnector conn;
    if (0 != conn.connect(stream, addr))
    {
        printf("Failure to connect ip:port: %s:%u, errno=%d\n", g_strHost.c_str(), g_unPort, errno);
        return 1;
    }
    CwxPackageWriterEx writer;
    CwxPackageReaderEx reader;
    CwxMsgHead head;
    CwxMsgBlock* block=NULL;
    char szErr2K[2048];
    char const* pErrMsg=NULL;
    CWX_UINT32 uiVersion=0;
    CWX_UINT32 uiFieldNum=0;
    CwxKeyValueItemEx  key;
    key.m_szData = g_key.c_str();
    key.m_uiDataLen = g_key.length();
    key.m_bKeyValue = false;
    CwxKeyValueItemEx  field;
    field.m_szData = g_field.c_str();
    field.m_uiDataLen = g_field.length();
    field.m_bKeyValue = false;
    CwxKeyValueItemEx  extra;
    extra.m_szData = g_extra.c_str();
    extra.m_uiDataLen = g_extra.length();
    extra.m_bKeyValue = false;

	do 
    {
		if (UNISTOR_ERR_SUCCESS != UnistorPoco::packRecvDel(
            &writer,
            block,
			0,
			key,
            g_field.length()?&field:NULL,
            g_extra.length()?&extra:NULL,
            g_version,
            g_user.c_str(),
            g_passwd.c_str(),
			szErr2K))
		{
            printf("failure to pack delete key package, err=%s\n", szErr2K);
            iRet = 1;
            break;
        }
        if (block->length() != (CWX_UINT32)CwxSocket::write_n(stream.getHandle(),
            block->rd_ptr(),
            block->length()))
        {
            printf("failure to send message, errno=%d\n", errno);
            iRet = 1;
            break;
        }
        CwxMsgBlockAlloc::free(block);
        block = NULL;
        //recv msg
        if (0 >= CwxSocket::read(stream.getHandle(), head, block))
        {
            printf("failure to read the reply, errno=%d\n", errno);
            iRet = 1;
            break;
        }
        if (UnistorPoco::MSG_TYPE_RECV_DEL_REPLY != head.getMsgType())
        {
            printf("recv a unknow msg type, msg_type=%u\n", head.getMsgType());
            iRet = 1;
            break;
        }
        if (UNISTOR_ERR_SUCCESS != UnistorPoco::parseRecvReply(&reader,
            block,
            iRet,
            uiVersion,
            uiFieldNum,
            pErrMsg,
            szErr2K))
        {
            printf("failure to unpack reply msg, err=%s\n", szErr2K);
            iRet = 1;
            break;
        }
        if (UNISTOR_ERR_SUCCESS != iRet)
        {
            printf("failure to delete key, err_code=%d, err=%s\n", iRet, pErrMsg);
            iRet = 1;
            break;
        }
        iRet = 0;
        printf("success to delete key[%s], version=%u, field_num=%u\n",
            g_key.c_str(),
            uiVersion,
            uiFieldNum);
    } while(0);
    if (block) CwxMsgBlockAlloc::free(block);
    stream.close();
    return iRet;
}
