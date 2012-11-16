#include "CwxSocket.h"
#include "CwxINetAddr.h"
#include "CwxSockStream.h"
#include "CwxSockConnector.h"
#include "CwxGetOpt.h"
#include "UnistorPoco.h"

using namespace cwinux;
string g_strHost;
CWX_UINT16 g_unPort = 0;
string g_start;
string g_end;
CWX_UINT16 g_num=0;
list<string> g_field;
string g_extra;
CWX_UINT16  g_asc=1;
string  g_user;
string  g_passwd;
bool    g_bMaster = false;
bool    g_bBegin = true;
bool    g_bKeyInfo=false;
///-1£ºÊ§°Ü£»0£ºhelp£»1£º³É¹¦
int parseArg(int argc, char**argv)
{
    CwxGetOpt cmd_option(argc, argv, "H:P:s:e:n:f:X:u:p:oimbh");
    int option;
    while( (option = cmd_option.next()) != -1)
    {
        switch (option)
        {
        case 'h':
            printf("get a key or key's field.\n");
            printf("%s  -H host -P port ......\n", argv[0]);
            printf("-H: server host\n");
            printf("-P: server port\n");
			printf("-s: start key name, from start if emtpy.\n");
			printf("-e: end key to fetch,  no limit if empty\n");
			printf("-n: number to fetch.\n");
			printf("-f: field to fetch. empty for fetching all field.\n");
            printf("-X: engine extra data.\n");
			printf("-o: order for key by desc.\n");
            printf("-u: user name.\n");
            printf("-p: user password.\n");
            printf("-m: get from master.\n");
            printf("-b: not fetch begin key.\n");
            printf("-i:  just fetch key's information.\n");
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
        case 's':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-s requires an argument.\n");
                return -1;
            }
			g_start = cmd_option.opt_arg();
            break;
		case 'e':
			if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
			{
				printf("-e requires an argument.\n");
				return -1;
			}
			g_end = cmd_option.opt_arg();
			break;
		case 'n':
			if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
			{
				printf("-n requires an argument.\n");
				return -1;
			}
			g_num = strtoul(cmd_option.opt_arg(), NULL, 10);
			break;
		case 'f':
			if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
			{
				printf("-f requires an argument.\n");
				return -1;
			}
			g_field.push_back(string(cmd_option.opt_arg()));
			break;
        case 'X':
            if (!cmd_option.opt_arg() || (*cmd_option.opt_arg() == '-'))
            {
                printf("-X requires an argument.\n");
                return -1;
            }
            g_extra = cmd_option.opt_arg();
            break;
		case 'o':
			g_asc = false;
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
        case 'i':
            g_bKeyInfo = true;
            break;
        case 'm':
            g_bMaster = true;
            break;
        case 'b':
            g_bBegin = false;
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
	CWX_UINT32  output_buf_len = UNISTOR_MAX_KVS_SIZE;
    char szErr2K[2048];
	char* output_buf = (char*)malloc(output_buf_len);
	do 
    {
		string strField;
		list<string>::iterator iter = g_field.begin();
		while(iter != g_field.end())
		{
			if (!strField.length()){
				strField = *iter;
			}else
			{
				strField += "\n";
				strField += *iter;
			}
			iter++;
		}
        CwxKeyValueItemEx  begin;
        CwxKeyValueItemEx  end;
        begin.m_szData = g_start.c_str();
        begin.m_uiDataLen = g_start.length();
        begin.m_bKeyValue = false;
        end.m_szData = g_end.c_str();
        end.m_uiDataLen = g_end.length();
        end.m_bKeyValue = false;
        CwxKeyValueItemEx  field;
        field.m_szData = strField.c_str();
        field.m_uiDataLen = strField.length();
        field.m_bKeyValue = false;
        CwxKeyValueItemEx  extra;
        extra.m_szData = g_extra.c_str();
        extra.m_uiDataLen = g_extra.length();
        extra.m_bKeyValue = false;

        if (UNISTOR_ERR_SUCCESS != UnistorPoco::packGetList(&writer,
            block,
            0,
            g_start.length()?&begin:NULL,
            g_end.length()?&end:NULL,
            g_num,
            strField.length()?&field:NULL,
            g_extra.length()?&extra:NULL,
            g_asc,
            g_bBegin,
            g_bKeyInfo,
            g_user.c_str(),
            g_passwd.c_str(),
            g_bMaster,
            szErr2K))
        {
            printf("failure to pack gets key package, err=%s\n", szErr2K);
            iRet = 1;
            break;
        }
		//send
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

		if (UnistorPoco::MSG_TYPE_RECV_LIST_REPLY != head.getMsgType())
        {
            printf("recv a unknow msg type, msg_type=%u\n", head.getMsgType());
            iRet = 1;
            break;
        }
		printf("query result:\n");
		memset(output_buf, 0x00, output_buf_len);
		CwxPackageEx::dump(block->rd_ptr(),
			block->length(),
			output_buf,
			output_buf_len,
			"  ");
		output_buf[output_buf_len] = 0x00;
        for (CWX_UINT32 i=0; i<output_buf_len; i++){
            if (0 == output_buf[i]) output_buf[i]=' ';
        }
		printf("%s\n", output_buf);
    } while(0);
    if (block) CwxMsgBlockAlloc::free(block);
	if (output_buf) free(output_buf);
    stream.close();
    return iRet;
}
