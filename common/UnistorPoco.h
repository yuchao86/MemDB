#ifndef __UNISTOR_POCO_H__
#define __UNISTOR_POCO_H__

#include "UnistorMacro.h"
#include "CwxMsgBlock.h"
#include "CwxPackageReaderEx.h"
#include "CwxPackageWriterEx.h"
#include "CwxCrc32.h"
#include "CwxMd5.h"


//unistor��Э�鶨�����
class UnistorPoco
{
public:
    enum{///<��Ϣ��ͷ������λ
        MSG_HEAD_ATTR_MASTER = 0x01 ///<��master��ȡ��Ϣ
    };
    
    enum ///<��Ϣ���Ͷ���
    {
        ///RECV�������͵���Ϣ���Ͷ���
        MSG_TYPE_TIMESTAMP = 0, ///<ʱ����Ϣ����
        MSG_TYPE_RECV_ADD = 1, ///<ADD key/value
        MSG_TYPE_RECV_ADD_REPLY = 2, ///<set key/value�Ļظ�
        MSG_TYPE_RECV_SET = 3, ///<SET key
        MSG_TYPE_RECV_SET_REPLY = 4, ///<SET key�Ļظ�
		MSG_TYPE_RECV_UPDATE = 5, ///update key
		MSG_TYPE_RECV_UPDATE_REPLY = 6, ///<update key�Ļظ�
		MSG_TYPE_RECV_INC = 7, ///<inc key
		MSG_TYPE_RECV_INC_REPLY = 8, ///<inc key�Ļظ�
		MSG_TYPE_RECV_DEL = 9, ///<del key
		MSG_TYPE_RECV_DEL_REPLY = 10, ///<del key�Ļظ�
        MSG_TYPE_RECV_EXIST = 11, ///<Key�Ƿ����
        MSG_TYPE_RECV_EXIST_REPLY = 12, ///<Key�Ƿ���ڵĻظ�
		MSG_TYPE_RECV_GET = 13, ///<get key
		MSG_TYPE_RECV_GET_REPLY = 14, ///<get key�Ļظ�
		MSG_TYPE_RECV_GETS = 15, ///<get ���key
		MSG_TYPE_RECV_GETS_REPLY = 16, ///<get ���key�Ļظ�
		MSG_TYPE_RECV_LIST = 17, ///<��ȡ�б�
		MSG_TYPE_RECV_LIST_REPLY = 18, ///<�б�ظ�
        MSG_TYPE_RECV_IMPORT = 19, ///<��ȡ�б�
        MSG_TYPE_RECV_IMPORT_REPLY = 20, ///<�б�ظ�
        MSG_TYPE_RECV_AUTH = 21, ///<��֤
        MSG_TYPE_RECV_AUTH_REPLY = 22, ///<��֤�ظ�
        ///���ݵ�����������
        MSG_TYPE_EXPORT_REPORT = 51, ///<���ݵ���export
        MSG_TYPE_EXPORT_REPORT_REPLY = 52, ///<���ݵ�����reply
        MSG_TYPE_EXPORT_DATA = 53, ///<���ݵ���������
        MSG_TYPE_EXPORT_DATA_REPLY = 54, ///<���ݵ���������reply
        MSG_TYPE_EXPORT_END = 55, ///<���ݵ������
        ///�ַ�����Ϣ���Ͷ���
        MSG_TYPE_SYNC_REPORT = 101, ///<ͬ��SID�㱨����Ϣ����
        MSG_TYPE_SYNC_REPORT_REPLY = 102, ///<report����
        MSG_TYPE_SYNC_CONN = 103, ///<����ͨ��
        MSG_TYPE_SYNC_CONN_REPLY = 104, ///<����ͨ���ظ�
        MSG_TYPE_SYNC_DATA = 105,  ///<��������
        MSG_TYPE_SYNC_DATA_REPLY = 106, ///<���ݵĻظ�
        MSG_TYPE_SYNC_DATA_CHUNK = 107,  ///<chunkģʽ��������
        MSG_TYPE_SYNC_DATA_CHUNK_REPLY = 108, ///<chunkģʽ�������ݵĻظ�
        MSG_TYPE_SYNC_ERR = 109 ///<sync�Ĵ�����Ϣ
    };
    enum
    {
        MAX_CONTINUE_SEEK_NUM = 8192
    };
public:
    ///���ô�master��ȡ������λ
    inline static CWX_UINT8 setFromMaster(CWX_UINT8& ucAttr){
        CWX_SET_ATTR(ucAttr, MSG_HEAD_ATTR_MASTER);
        return ucAttr;
    }
    ///check�Ƿ������˴�master��ȡ������λ
    inline static bool isFromMaster(CWX_UINT8 ucAttr){
        return CWX_CHECK_ATTR(ucAttr, MSG_HEAD_ATTR_MASTER);
    }
    ///�����master��ȡ������λ
    inline static CWX_UINT8 clearFromMaster(CWX_UINT8& ucAttr){
        return CWX_CLR_ATTR(ucAttr, MSG_HEAD_ATTR_MASTER);
    }
    ///pack Add key�����ݡ� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvImport(CwxPackageWriterEx* writer, ///<����pack��writer������ͨ��writer����
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        bool       bCache=true, ///<�Ƿ�cache����Ϊtrue�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack Add key����Ϣ���� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvImport(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        bool       bCache=true, ///<�Ƿ�cache����Ϊtrue�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///����Add key�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvImport(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<����key�ֶ�
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        CwxKeyValueItemEx const*& data,  ///<����data�ֶ�
        CWX_UINT32& uiExpire,  ///<����expire����Ϊ0��ʾû��ָ��
        CWX_UINT32& uiVersion, ///<���ذ汾
        bool&       bCache,    ///<����cache
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
        char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
        );



    ///pack Add key�����ݡ� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvAdd(CwxPackageWriterEx* writer, ///<����pack��writer������ͨ��writer����
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiSign=0,    ///<��ǣ���Ϊ0�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        bool       bCache=true, ///<�Ƿ�cache����Ϊtrue�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
        );

	///pack Add key����Ϣ���� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
	static int packRecvAdd(CwxPackageWriterEx* writer, ///<����pack��writer
		CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
		CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiExpire=0,  ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiSign=0,    ///<��ǣ���Ϊ0�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        bool       bCache=true, ///<�Ƿ�cache����Ϊtrue�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
		);

	///����Add key�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
	static int parseRecvAdd(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
		CwxKeyValueItemEx const*& data,  ///<����data�ֶ�
		CWX_UINT32& uiExpire,  ///<����expire����Ϊ0��ʾû��ָ��
        CWX_UINT32& uiSign,    ///<����sign
        CWX_UINT32& uiVersion, ///<���ذ汾
        bool&       bCache,    ///<����cache
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
		char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack set�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvSet(CwxPackageWriterEx* writer,///<����pack��writer������ͨ��writer����
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<��ǣ���Ϊ0�����
        CWX_UINT32 uiExpire=0, ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiVersion=0,///<�汾����Ϊ0�����
        bool   bCache=true, ///<�Ƿ�cache����Ϊtrue�����
        char const* user=NULL, ///<�û�����ΪNULL�����
        char const* passwd=NULL,///<�û������ΪNULL�����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack set����Ϣ�塣����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvSet(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<��ǣ���Ϊ0�����
        CWX_UINT32 uiExpire=0, ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiVersion=0,///<�汾����Ϊ0�����
        bool   bCache=true, ///<�Ƿ�cache����Ϊtrue�����
        char const* user=NULL, ///<�û�����ΪNULL�����
        char const* passwd=NULL,///<�û������ΪNULL�����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse set�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvSet(CwxPackageReaderEx* reader,  ///<reader
        CwxKeyValueItemEx const*& key, ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        CwxKeyValueItemEx const*& data, ///<����data�ֶ�
        CWX_UINT32& uiSign, ///<����sign
		CWX_UINT32& uiExpire, ///<����expire
        CWX_UINT32& uiVersion, ///<���ذ汾
        bool&   bCache,  ///<����cache
        char const*& user, ///<�����û���NULL��ʾ������
        char const*& passwd, ///<���ؿ��NULL��ʾ������
        char* szErr2K=NULL  ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack update�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvUpdate(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<��ǣ���Ϊ0�����
        CWX_UINT32 uiExpire=0, ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        char const* user=NULL, ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

	///pack update����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
	static int packRecvUpdate(CwxPackageWriterEx* writer, ///<����pack��writer
		CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
		CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CwxKeyValueItemEx const& data, ///<data
        CWX_UINT32 uiSign=0, ///<��ǣ���Ϊ0�����
        CWX_UINT32 uiExpire=0, ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        char const* user=NULL, ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
		);

	///parse update�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
	static int parseRecvUpdate(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key, ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        CwxKeyValueItemEx const*& data, ///<����data�ֶ�
        CWX_UINT32& uiSign, ///<����sign
        CWX_UINT32& uiExpire, ///<����expire����Ϊ0��ʾû��ָ��
        CWX_UINT32& uiVersion, ///<���ذ汾
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
        char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
		);

    ///pack inc�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvInc(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CWX_INT64   num, ///<inc�����֣������ɸ�
        CWX_INT64   result, ///<����Ľ������Ϊ0�����,�˼�¼���յļ�������
        CWX_INT64   max=0, ///<��incΪ��ֵ����ͨ��max�޶����ֵ
        CWX_INT64   min=0, ///<��incΪ��ֵ����ͨ��min�޶���Сֵ
        CWX_UINT32  uiExpire=0, ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32  uiSign=0, ///<��ǣ���Ϊ0�����
        CWX_UINT32  uiVersion=0, ///<�汾����Ϊ0�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
        );

	///pack inc����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
	static int packRecvInc(CwxPackageWriterEx* writer, ///<����pack��writer
		CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
		CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CWX_INT64   num, ///<inc�����֣������ɸ�
        CWX_INT64   result, ///<����Ľ������Ϊ0�����,�˼�¼���յļ�������
        CWX_INT64   max=0, ///<��incΪ��ֵ����ͨ��max�޶����ֵ
        CWX_INT64   min=0, ///<��incΪ��ֵ����ͨ��min�޶���Сֵ
        CWX_UINT32  uiExpire=0, ///<��ʱʱ�䣬��Ϊ0�����
        CWX_UINT32  uiSign=0, ///<��ǣ���Ϊ0�����
        CWX_UINT32  uiVersion=0, ///<�汾����Ϊ0�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
		);

	///����inc�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
	static int parseRecvInc(CwxPackageReaderEx* reader,///<reader
        CwxKeyValueItemEx const*& key, ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        CWX_INT64&   num, ///<����inc��num
        CWX_INT64&   result, ///<��������ֵ
        CWX_INT64&   max, ///<����max
        CWX_INT64&   min, ///<����min
        CWX_UINT32& uiExpire, ///<����expire����Ϊ0��ʾû��ָ��
        CWX_UINT32&  uiSign, ///<����sign
        CWX_UINT32&  uiVersion, ///<����version
        char const*& user,  ///<����user
        char const*& passwd, ///<����password
		char* szErr2K=NULL  ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack delete�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvDel(CwxPackageWriterEx* writer,///<����pack��writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack delete����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvDel(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        CWX_UINT32 uiVersion=0, ///<�汾����Ϊ0�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL       ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse delete�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvDel(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        CWX_UINT32& uiVersion, ///<���ذ汾
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
        char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack��inc������ݸ��·�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvReply(CwxPackageWriterEx* writer,///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
		CWX_UINT16 unMsgType, ///<�ظ���Ϣ������Ϣ����
        int ret,  ///<���ص�ret����
        CWX_UINT32 uiVersion, ///<���صİ汾��
        CWX_UINT32 uiFieldNum, ///<���ص�field����
        char const* szErrMsg, ///<���صĴ�����Ϣ
        char* szErr2K=NULL    ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse��inc������ݸ��·�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ص����ݰ�
        int& ret,  ///<���ص�retֵ
        CWX_UINT32& uiVersion, ///<���ص�version
        CWX_UINT32& uiFieldNum,  ///<���ص�field number
        char const*& szErrMsg,  ///<���صĴ�����Ϣ
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack inc�ķ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvIncReply(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT16 unMsgType, ///<��Ϣ����
        int ret,  ///<ret����
        CWX_INT64 llNum, ///<��������ֵ
        CWX_UINT32 uiVersion, ///<�汾��
        char const* szErrMsg, ///<������Ϣ
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse inc���ص���Ϣ���� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvIncReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ص����ݰ�
        int& ret,  ///<���ص�retֵ
        CWX_UINT32& uiVersion, ///<���صİ汾
        CWX_INT64& llNum, ///<���صļ�������ֵ
        char const*& szErrMsg, ///<������Ϣ
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack get�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packGetKey(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        bool bVersion = false, ///<�Ƿ��ȡ�汾
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        CWX_UINT8 ucKeyInfo=0, ///<�Ƿ��ȡkey��infomation
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack get����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packGetKey(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        bool bVersion = false, ///<�Ƿ��ȡ�汾
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        bool bMaster = false, ///<�Ƿ��master��ȡ
        CWX_UINT8 ucKeyInfo=0, ///<�Ƿ��ȡkey��infomation
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse get�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseGetKey(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        bool&        bVersion, ///<�汾
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
        CWX_UINT8& ucKeyInfo, ///<�Ƿ��ȡkey��infomation
        char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack exist�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExistKey(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        bool bVersion = false, ///<�Ƿ��ȡ�汾
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack exist����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExistKey(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        bool bVersion = false, ///<�Ƿ��ȡ�汾
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        bool bMaster = false, ///<�Ƿ��master��ȡ
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse exist�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseExistKey(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& key,   ///<����key�ֶ�
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        bool&        bVersion, ///<�汾
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
        char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack multi-get���ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packGetKeys(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxPackageWriterEx* writer1, ///<����pack��writer1
        list<pair<char const*, CWX_UINT16> > const& keys, ///<key���б�
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        CWX_UINT8 ucKeyInfo=0, ///<�Ƿ��ȡkey��infomation
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack multi-get��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packGetKeys(CwxPackageWriterEx* writer,///<����pack��writer
        CwxPackageWriterEx* writer1, ///<����pack��writer1
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        list<pair<char const*, CWX_UINT16> > const& keys, ///<key���б�
        CwxKeyValueItemEx const* field, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra, ///<extra��Ϣ����ΪNULL�����
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        bool bMaster = false, ///<�Ƿ��master��ȡ
        CWX_UINT8 ucKeyInfo=0, ///<�Ƿ��ȡkey��infomation
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse multi-get�����ݰ��� ����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseGetKeys(CwxPackageReaderEx* reader,///<reader
        CwxPackageReaderEx* reader1,///<reader1
        list<pair<char const*, CWX_UINT16> >& keys,///<key���б�
        CWX_UINT32& uiKeyNum, ///<key������
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        char const*& user,     ///<�����û���NULL��ʾ������
        char const*& passwd,   ///<���ؿ��NULL��ʾ������
        CWX_UINT8&   ucKeyInfo, ///<�Ƿ��ȡkey��infomation
        char* szErr2K=NULL     ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack ��ȡkey�б�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packGetList(CwxPackageWriterEx* writer,///<����pack��writer
        CwxKeyValueItemEx const* begin=NULL, ///<��ʼ��key
        CwxKeyValueItemEx const* end=NULL,  ///<������key
        CWX_UINT16  num=0,  ///<���ص�����
        CwxKeyValueItemEx const* field=NULL, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra=NULL, ///<extra��Ϣ����ΪNULL�����
        bool        bAsc=true, ///<�Ƿ�����
        bool        bBegin=true, ///<�Ƿ��ȡbegin��ֵ
        bool        bKeyInfo=false, ///<�Ƿ񷵻�key��info
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack ��ȡkey�б����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packGetList(CwxPackageWriterEx* writer,
        CwxMsgBlock*& msg,
        CWX_UINT32 uiTaskId,
        CwxKeyValueItemEx const* begin=NULL, ///<��ʼ��key
        CwxKeyValueItemEx const* end=NULL,  ///<������key
        CWX_UINT16  num=0,  ///<���ص�����
        CwxKeyValueItemEx const* field=NULL, ///<field�ֶΣ���ΪNULL�Ĳ����
        CwxKeyValueItemEx const* extra=NULL, ///<extra��Ϣ����ΪNULL�����
        bool        bAsc=true, ///<�Ƿ�����
        bool        bBegin=true, ///<�Ƿ��ȡbegin��ֵ
        bool        bKeyInfo=false, ///<�Ƿ񷵻�key��info
        char const* user=NULL,  ///<�û�����ΪNULL�����
        char const* passwd=NULL, ///<�û������ΪNULL�����
        bool bMaster = false, ///<�Ƿ��master��ȡ
        char* szErr2K=NULL   ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse get list�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseGetList(CwxPackageReaderEx* reader, ///<reader
        CwxKeyValueItemEx const*& begin, ///<���ؿ�ʼ
        CwxKeyValueItemEx const*& end, ///<���ؼ���
        CWX_UINT16&  num, ///<��ȡ������
        CwxKeyValueItemEx const*& field, ///<field�ֶΣ���ΪNULL��ʾ������
        CwxKeyValueItemEx const*& extra, ///<extra��Ϣ����ΪNULL��ʾ������
        bool&        bAsc, ///<����
        bool&        bBegin, ///<�Ƿ��ȡ��ʼֵ
        bool&        bKeyInfo, ///<�Ƿ񷵻�key��info
        char const*& szUser, ///<�û�
        char const*& szPasswd, ///<����
        char*        szErr2K=NULL ///<����Ĵ�����Ϣ
        );

    ///pack��Ȩ��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvAuth(CwxPackageWriterEx* writer,///<����pack��writer
        CwxMsgBlock*& msg,///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId,///<��Ϣ����task id
        char const* szUser = NULL, ///<�û�����ΪNULL�����
        char const* szPasswd = NULL,///<�û������ΪNULL�����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse��Ȩ�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvAuth(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg,///<���ݰ�
        char const*& szUser,///<�����û���NULL��ʾ������
        char const*& szPasswd,///<���ؿ��NULL��ʾ������
        char*     szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack��Ȩ�ظ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packRecvAuthReply(CwxPackageWriterEx* writer,///<����pack��writer
        CwxMsgBlock*& msg,///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId,///<��Ϣ����task id
        CWX_UINT16 unMsgType, ///<��Ϣ����
        int ret, ///<��Ȩ���
        char const* szErrMsg, ///<������Ϣ
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse��Ȩ�ظ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseRecvAuthReply(CwxPackageReaderEx* reader,///<reader
        CwxMsgBlock const* msg,///<���ݰ�
        int& ret,///<��Ȩ���
        char const*& szErrMsg,///<������Ϣ
        char* szErr2K=NULL///<���ʱ�Ĵ�����Ϣ
        );

    ///pack export��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExportReport(CwxPackageWriterEx* writer,///<����pack��writer
        CwxMsgBlock*& msg,///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId,///<��Ϣ����task id
        CWX_UINT32  uiChunkSize, ///<���ݷ��͵�chunk��С
        char const* subscribe = NULL, ///<���ݶ�������
        char const* key = NULL, ///<��ʼ��key
        char const* extra = NULL, ///<extra��Ϣ����ΪNULL�����
        char const* user=NULL, ///<�û���
        char const* passwd=NULL, ///<����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse export��report���ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseExportReport(CwxPackageReaderEx* reader,///<reader
        CwxMsgBlock const* msg,///<���ݰ�
        CWX_UINT32&  uiChunkSize,///<���ݷ��͵�chunk��С
        char const*& subscribe,///<���ݶ����������ձ�ʾȫ������
        char const*& key,///<��ʼ��key���ձ�ʾû������
        char const*& extra, ///<extra��Ϣ����ΪNULL��ʾû��ָ��
        char const*& user,///<�û���
        char const*& passwd,///<����
        char* szErr2K=NULL///<���ʱ�Ĵ�����Ϣ
        );

    ///pack export��report�ظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExportReportReply(CwxPackageWriterEx* writer,///<����pack��writer
        CwxMsgBlock*& msg,///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId,///<��Ϣ����task id
        CWX_UINT64 ullSession, ///<session
        CWX_UINT64 ullSid,  ///<���ݿ�ʼ����ʱ��sid
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse export��report�ظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseExportReportReply(CwxPackageReaderEx* reader,///<reader
        CwxMsgBlock const* msg,///<���ݰ�
        CWX_UINT64& ullSession,///<session
        CWX_UINT64& ullSid,///<���ݿ�ʼ����ʱ��sid
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///packһ��export��key/value�����ݡ�����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExportDataItem(CwxPackageWriterEx* writer,///<����pack��writer
        CwxKeyValueItemEx const& key, ///<key
        CwxKeyValueItemEx const& data, ///<data
        CwxKeyValueItemEx const* extra, ///<extra
        CWX_UINT32 version, ///<�汾��
        CWX_UINT32 expire, ///<��ʱʱ��
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parseһ��export��key/value�����ݷ���ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseExportDataItem(CwxPackageReaderEx* reader,///<reader
        char const* szData, ///<key/value����
        CWX_UINT32  uiDataLen, ///<key/value���ݵĳ���
        CwxKeyValueItemEx const*& key, ///<���ݵ�key
        CwxKeyValueItemEx const*& data, ///<���ݵ�data
        CwxKeyValueItemEx const*& extra, ///<extra
        CWX_UINT32& version, ///<���ݵİ汾
        CWX_UINT32& expire, ///<���ݵĳ�ʱ
        char* szErr2K=NULL///<���ʱ�Ĵ�����Ϣ
        );

    ///pack��chunk��֯�Ķ���export��key/value����Ϣ������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packMultiExportData(
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        char const* szData,  ///<����key/value��ɵ�����package
        CWX_UINT32 uiDataLen, ///<���ݵĳ���
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT64 ullSeq, ///<���к�
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse��chunk��֯�Ķ���export��key/value�����ݡ�����ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseMultiExportData(
        CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg,///<���ݰ�
        CWX_UINT64& ullSeq, ///<���к�
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack export���ݵ�reply��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExportDataReply(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSeq, ///<���к�
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse export���ݵ�reply��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseExportDataReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSeq, ///<���к�
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack export��ɵ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packExportEnd(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSid, ///<���ʱ��sid
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse export��ɵ���Ϣ������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseExportEnd(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSid,///<���ʱ��sid
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack binlog sync��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packReportData(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSid, ///<��ʼ��sid
        bool      bNewly,  ///<�Ƿ������binlog��ʼͬ��
        CWX_UINT32  uiChunkSize, ///<ͬ����chunk��С
        char const* subscribe = NULL, ///<binlog���Ĺ��򣬿ձ�ʾȫ������
        char const* user=NULL, ///<�û���
        char const* passwd=NULL, ///<�û�����
        char const* sign=NULL, ///<ǩ����ʽ���ձ�ʾ��ǩ��
        bool        zip = false, ///<�Ƿ�ѹ��
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse binlog sync��report��Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseReportData(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSid, ///<��ʼ��sid
        bool&       bNewly, ///<�Ƿ������binlog��ʼͬ��
        CWX_UINT32&  uiChunkSize, ///<ͬ����chunk��С
        char const*& subscribe, ///<binlog���Ĺ��򣬿ձ�ʾȫ������
        char const*& user, ///<�û���
        char const*& passwd, ///<�û�����
        char const*& sign, ///<ǩ����ʽ���ձ�ʾ��ǩ��
        bool&        zip, ///<�Ƿ�ѹ��
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack report�Ļظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packReportDataReply(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSession, ///<session id
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse report�Ļظ����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseReportDataReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSession, ///<session id
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack sync��session���ӱ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packReportNewConn(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSession, ///<����������session
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse sync��session���ӱ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseReportNewConn(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSession, ///<����������session
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack report��sync�ĳ�����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packSyncErr(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        int ret, ///<�������
        char const* szErrMsg, ///<������Ϣ
        char* szErr2K=NULL///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse report��sync�ĳ������ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseSyncErr(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        int& ret,  ///<�������
        char const*& szErrMsg,  ///<������Ϣ
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );


    ///pack sync��һ��binlog����Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packSyncData(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSid, ///<binlog��sid
        CWX_UINT32 uiTimeStamp, ///<binlog��ʱ���
        CwxKeyValueItemEx const& data, ///<binlog��data
        CWX_UINT32 group,  ///<binlog�����ķ���
        CWX_UINT32 type,   ///<binlog�����ͣ�Ҳ������Ϣ����
        CWX_UINT32 version,  ///<��Ӧ��key�İ汾
        CWX_UINT64 ullSeq,  ///<��Ϣ�����к�
        char const* sign=NULL, ///<ǩ����ʽ
        bool       zip = false, ///<�Ƿ�ѹ��
        char* szErr2K=NULL///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack һ��binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packSyncDataItem(CwxPackageWriterEx* writer, ///<����pack��writer
        CWX_UINT64 ullSid, ///<binlog��sid
        CWX_UINT32 uiTimeStamp, ///<binlog��ʱ���
        CwxKeyValueItemEx const& data, ///<binlog��data
        CWX_UINT32 group,  ///<binlog�����ķ���
        CWX_UINT32 type,   ///<binlog�����ͣ�Ҳ������Ϣ����
        CWX_UINT32 version,  ///<��Ӧ��key�İ汾
        char const* sign=NULL, ///<ǩ����ʽ
        char* szErr2K=NULL///<pack����ʱ�Ĵ�����Ϣ
        );

    ///pack ����binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packMultiSyncData(
        CWX_UINT32 uiTaskId, ///<����id
        char const* szData, ///<������Ϣ������buf
        CWX_UINT32 uiDataLen, ///<�������ݵ�����buf����
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT64 ullSeq, ///<��Ϣ������Ϣ���к�
        bool  zip = false, ///<�Ƿ�ѹ��
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parseһ��binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseSyncData(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSid, ///<binlog��sid
        CWX_UINT32& uiTimeStamp, ///<binlog��ʱ���
        CwxKeyValueItemEx const*& data, ///<binlog������
        CWX_UINT32& group, ///<binlog������group
        CWX_UINT32& type, ///<binlog��Ӧ�����ݱ����Ϣ����
        CWX_UINT32& version, ///<binlog��Ӧ�����ݱ����key�İ汾
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///parseһ��binlog�����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseSyncData(CwxPackageReaderEx* reader,
        char const* szData,
        CWX_UINT32 uiDataLen,
        CWX_UINT64& ullSid, ///<binlog��sid
        CWX_UINT32& uiTimeStamp, ///<binlog��ʱ���
        CwxKeyValueItemEx const*& data, ///<binlog������
        CWX_UINT32& group, ///<binlog������group
        CWX_UINT32& type, ///<binlog��Ӧ�����ݱ����Ϣ����
        CWX_UINT32& version, ///<binlog��Ӧ�����ݱ����key�İ汾
        char* szErr2K=NULL ///<���ʱ�Ĵ�����Ϣ
        );

    ///pack sync binlog�Ļظ���Ϣ��������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int packSyncDataReply(CwxPackageWriterEx* writer, ///<����pack��writer
        CwxMsgBlock*& msg, ///<���ص���Ϣ�����������ڲ�����
        CWX_UINT32 uiTaskId, ///<��Ϣ����task id
        CWX_UINT64 ullSeq, ///<��Ϣ�����к�
        CWX_UINT16 unMsgType, ///<��Ϣ����
        char* szErr2K=NULL ///<pack����ʱ�Ĵ�����Ϣ
        );

    ///parse sync binlog�Ļظ����ݰ�������ֵ��UNISTOR_ERR_SUCCESS���ɹ�����������ʧ��
    static int parseSyncDataReply(CwxPackageReaderEx* reader, ///<reader
        CwxMsgBlock const* msg, ///<���ݰ�
        CWX_UINT64& ullSeq, ///<��Ϣ�����к�
        char* szErr2K=NULL  ///<���ʱ�Ĵ�����Ϣ
        );

    ///��������ͬ������seq��
    inline static void setSeq(char* szBuf, CWX_UINT64 ullSeq){
        CWX_UINT32 byte4 = (CWX_UINT32)(ullSeq>>32);
        byte4 = CWX_HTONL(byte4);
        memcpy(szBuf, &byte4, 4);
        byte4 = (CWX_UINT32)(ullSeq&0xFFFFFFFF);
        byte4 = CWX_HTONL(byte4);
        memcpy(szBuf + 4, &byte4, 4);

    }    
    ///��ȡ����ͬ������seq��
    inline static CWX_UINT64 getSeq(char const* szBuf) {
        CWX_UINT64 ullSeq = 0;
        CWX_UINT32 byte4;
        memcpy(&byte4, szBuf, 4);
        ullSeq = CWX_NTOHL(byte4);
        memcpy(&byte4, szBuf+4, 4);
        ullSeq <<=32;
        ullSeq += CWX_NTOHL(byte4);
        return ullSeq;
    }

    ///�Ƿ�������Ҷ��ĵ���Ϣ����
    inline static bool isContinueSeek(CWX_UINT32 uiSeekedNum){
        return MAX_CONTINUE_SEEK_NUM>uiSeekedNum;
    }
private:
    ///��ֹ��������ʵ��
    UnistorPoco()
    {
    }
    ///��������
    ~UnistorPoco();
};





#endif
