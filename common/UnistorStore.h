#ifndef __UNISTOR_STORE_H__
#define __UNISTOR_STORE_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxFile.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "UnistorStoreBase.h"
#include <dlfcn.h>

///�洢������������Function���Ͷ���
typedef UnistorStoreBase* (*CWX_LOAD_ENGINE)(void);

///�洢����
class UnistorStore{
public:
    ///���캯��
    UnistorStore();
    ///��������
    ~UnistorStore();
public:
	//���������ļ�.-1:failure, 0:success
	int init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc, ///<��UnistorApp���߳�ͨ�ŵ���Ϣͨ��Func
        UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<��ȡϵͳ��Ϣ�ĺ���
        void* pApp, ///<UnistorApp����
        UnistorConfig const* config, ///<�����ļ������ָ��
        string const& strEnginePath, ///<�洢���涯̬��İ�װĿ¼
        char* szErr2K ///<����ʼ����ʧ�ܣ����ش�����Ϣ
        );

	///����Ƿ����key��1�����ڣ�0�������ڣ�-1��ʧ�ܣ�
	inline int isExist(UnistorTss* tss, ///tss����
        CwxKeyValueItemEx const& key, ///<����key
        CwxKeyValueItemEx const* field, ///<����field����Ϊ�ձ�ʾ���key
        CwxKeyValueItemEx const* extra, ///<�洢�����extra data
        CWX_UINT32& uiVersion, ///<����key�İ汾��
        CWX_UINT32& uiFieldNum, ///<����key��field������
        bool& bReadCached ///<�����Ƿ���read cache��
        )
    {
        return m_impl->isExist(tss,
            key,
            field,
            extra,
            uiVersion,
            uiFieldNum,
            bReadCached);
	}

    ///���key��1���ɹ���0�����ڣ�-1��ʧ�ܣ�
    inline int addKey(UnistorTss* tss, ///<tss����
        CwxKeyValueItemEx const& key, ///<��ӵ�key
        CwxKeyValueItemEx const* field, ///<��ӵ�field����ָ���������signֵ�����Ƿ����field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<���key��field������
        CWX_UINT32    uiSign, ///<��ӵı�־
        CWX_UINT32& uiVersion, ///<������0���������޸ĺ��keyΪ�˰汾�����򷵻��°汾
        CWX_UINT32& uiFieldNum, ///����<key field������
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool bCache=true, ///<�Ƿ�key�ŵ���cache
        CWX_UINT32 uiExpire=0 ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        )
    {
		return m_impl->addKey(tss,
            key,
            field,
            extra,
            data,
            uiSign,
            uiVersion,
            uiFieldNum,
            bReadCached,
            bWriteCached,
            bCache,
            uiExpire);
	}

	///set key��1���ɹ���-1��ʧ�ܣ�
	inline int setKey(UnistorTss* tss,///tss
        CwxKeyValueItemEx const& key, ///<set��key
        CwxKeyValueItemEx const* field, ///<����set field����ָ��Ҫset��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra ����
        CwxKeyValueItemEx const& data, ///<set������
        CWX_UINT32 uiSign, ///<���õı��
        CWX_UINT32& uiVersion, ///<���õ�version��������0��������Ϊָ���İ汾�����򷵻�ָ���İ汾
        CWX_UINT32& uiFieldNum, ///<key�ֶε�����
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool bCache=true, ///<�Ƿ�����ݽ���cache
		CWX_UINT32 uiExpire=0 ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        )
    {
		return m_impl->setKey(tss,
            key,
            field,
            extra,
            data,
            uiSign,
            uiVersion,
            uiFieldNum,
            bReadCached,
            bWriteCached,
            bCache,
            uiExpire);
	}

    ///update key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2���汾����
	inline int updateKey(UnistorTss* tss, ///<tss����
        CwxKeyValueItemEx const& key, ///<update��key
        CwxKeyValueItemEx const* field,///<��update field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra ����
        CwxKeyValueItemEx const& data, ///<update������
        CWX_UINT32 uiSign, ///<update�ı��
        CWX_UINT32& uiVersion, ///<��ָ������key�İ汾�������ֵһ�£��������ʧ��
        CWX_UINT32& uiFieldNum, ///<����key field������
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
		CWX_UINT32 uiExpire=0 ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        )
    {
		return m_impl->updateKey(tss,
            key,
            field,
            extra,
            data,
            uiSign,
            uiVersion,
            uiFieldNum,
            bReadCached,
            bWriteCached,
            uiExpire);
	}

    ///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2:�汾����-3�������߽�
	inline int incKey(UnistorTss* tss, ///<�߳�tss����
        CwxKeyValueItemEx const& key,  ///<inc��key
        CwxKeyValueItemEx const* field, ///<��Ҫincһ��field����������ָ����Ӧ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
		CWX_INT64 num, ///<���ӻ���ٵ�����
		CWX_INT64  llMax, ///<�������Ӷ��Ҵ�ֵ��Ϊ0����inc���ֵ���ܳ�����ֵ
		CWX_INT64  llMin, ///<���Ǽ��ٶ����ֵ��Ϊ0����dec���ֵ���ܳ�����ֵ
        CWX_UINT32  uiSign, ///<inc�ı��
		CWX_INT64& llValue, ///<inc��dec�����ֵ
        CWX_UINT32& uiVersion, ///<��ָ������key�İ汾�ű�����ڴ�ֵ������ʧ�ܡ������°汾�š�
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        CWX_UINT32  uiExpire=0 ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        )
    {
		return m_impl->incKey(tss,
            key,
            field,
            extra,
            num,
            llMax,
            llMin,
            uiSign,
            llValue,
            uiVersion,
            bReadCached,
            bWriteCached,
            uiExpire);
	}

	///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�
	inline int delKey(UnistorTss* tss, ///<�߳�tss����
        CwxKeyValueItemEx const& key, ///<Ҫɾ����key
        CwxKeyValueItemEx const* field, ///<��Ҫɾ��field����ָ��field������
        CwxKeyValueItemEx const* extra,///<�洢�����extra ����
		CWX_UINT32& uiVersion, ///<��ָ���汾�ţ����޸�ǰ�İ汾�ű������ֵ��ȣ�����ʧ�ܡ������°汾��
        CWX_UINT32& uiFieldNum,  ///<key���ֶ�����
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached ///<�����Ƿ���write cache��
        ) 
    {
		return m_impl->delKey(tss,
            key,
            field,
            extra,
            uiVersion,
            uiFieldNum,
            bReadCached,
            bWriteCached);
	}

    ///import key��1���ɹ���-1��ʧ�ܣ�
    int importKey(UnistorTss* tss, ///<tss����
        CwxKeyValueItemEx const& key, ///<��ӵ�key
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<���key��field������
        CWX_UINT32& uiVersion, ///<������0���������޸ĺ��keyΪ�˰汾
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool bCache=true, ///<�Ƿ�key�ŵ���cache
        CWX_UINT32 uiExpire=0 ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        )
    {
        return m_impl->importKey(tss, key, extra, data, uiVersion, bReadCached, bWriteCached, bCache, uiExpire);
    }

    ///ͬ��add key��binlog���ݣ�1���ɹ���0�����ڣ�-1��ʧ�ܣ�
    inline int syncAddKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<key������
        CwxKeyValueItemEx const* field, ///<�ֶε�����
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<add������
        CWX_UINT32 uiSign, ///<add��sign
        CWX_UINT32 uiVersion, ///<�����İ汾��
        bool bCache, ///<�Ƿ�cache����
        CWX_UINT32 uiExpire, ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        CWX_UINT64 ullSid, ///<�����־��sidֵ
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��Ǵ�binlog�ָ�������
        )
    {
        return m_impl->syncAddKey(tss,
            key,
            field,
            extra,
            data,
            uiSign,
            uiVersion,
            bCache,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
    }

    ///ͬ��set key��binlog���ݣ�1���ɹ���-1������0�������ڣ���������һ��key��fieldʱ��
    inline int syncSetKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<set��key
        CwxKeyValueItemEx const* field, ///<����set field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<set������
        CWX_UINT32 uiSign,  ///<set��sign
        CWX_UINT32 uiVersion, ///<set��key �汾��
        bool bCache, ///<�Ƿ�cache����
        CWX_UINT32 uiExpire, ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        CWX_UINT64 ullSid, ///<set binlog��sidֵ
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��Ǵ�binlog�ָ�������
        )
    {
        return m_impl->syncSetKey(tss,
            key,
            field,
            extra,
            data,
            uiSign,
            uiVersion,
            bCache,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
    }

    ///ͬ��update key��binlog���ݡ�1���ɹ���0�������ڣ�-1��ʧ��
    inline int syncUpdateKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<update��key
        CwxKeyValueItemEx const* field, ///<����update field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<update��������
        CWX_UINT32 uiSign, ///<update�ı��
        CWX_UINT32 uiVersion, ///<update���key�İ汾��
        CWX_UINT32 uiExpire, ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        CWX_UINT64 ullSid, ///<update���binlog��sid
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�лָ�������
        )
    {
        return m_impl->syncUpdateKey(tss,
            key,
            field,
            extra,
            data,
            uiSign,
            uiVersion,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
    }

    ///ͬ��inc key��binlog���ݡ�1���ɹ���0�������ڣ�-1��ʧ�ܣ�
    inline int syncIncKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key,  ///<inc��key
        CwxKeyValueItemEx const* field, ///<���Ƕ�field����inc����ָ��field������
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CWX_INT64 num,  ///<inc����ֵ������Ϊ��ֵ
        CWX_INT64 result,  ///<inc����ֵ������Ϊ��ֵ
        CWX_INT64  llMax, ///<����inc��ֵ������ָ��llMax����inc���ֵ���ܳ�����ֵ
        CWX_INT64  llMin, ///<����������Сֵ
        CWX_UINT32 uiSign, ///<inc�ı��
        CWX_INT64& llValue, ///<inc�����ֵ
        CWX_UINT32 uiVersion, ///<inc���key�İ汾��
        CWX_UINT32 uiExpire, ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        CWX_UINT64 ullSid, ///<inc����binlog��sidֵ
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�ָ�������
        )
    {
        return m_impl->syncIncKey(tss,
            key,
            field,
            extra,
            num,
            result,
            llMax,
            llMin,
            uiSign,
            llValue,
            uiVersion,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
    }

    ///ͬ��delete key��binlog���ݡ�����ֵ 1���ɹ���0�������ڣ�-1��ʧ�ܣ�
    inline int syncDelKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<Ҫɾ����key
        CwxKeyValueItemEx const* field, ///<����ɾ��field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CWX_UINT32 uiVersion, ///<key����delete��İ汾��
        CWX_UINT64 ullSid, ///<delete������Ӧ��binlog��sid
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�ָ�������
        )
    {
        return m_impl->syncDelKey(tss,
            key,
            field,
            extra,
            uiVersion,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
    }

    ///sync import key��1���ɹ���-1������
    inline int syncImportKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<set��key
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<set������
        CWX_UINT32 uiVersion, ///<set��key �汾��
        bool bCache,    ///<�Ƿ�key�ŵ���cache
        CWX_UINT32 uiExpire, ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        CWX_UINT64 ullSid, ///<������Ӧ��binlog��sid
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�ָ�������
        )
    {
        return m_impl->syncImportKey(tss,
            key,
            extra,
            data,
            uiVersion,
            bCache,
            uiExpire,
            ullSid,
            bReadCached,
            bWriteCached,
            bRestore);
    }

	///��ȡkey, 1���ɹ���0�������ڣ�-1��ʧ��;
	inline int get(UnistorTss* tss, ///<�߳�tss����
        CwxKeyValueItemEx const& key, ///<Ҫ��ȡ��key
        CwxKeyValueItemEx const* field, ///<����Ϊ�գ����ȡָ����field�����field��\n�ָ�
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
		char const*& szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
		CWX_UINT32& uiLen,  ///<szData���ݵ��ֽ���
		bool& bKeyValue,  ///<���ص������Ƿ�Ϊkey/value�ṹ
        CWX_UINT32& uiVersion, ///<���Ե�ǰ�İ汾��
        CWX_UINT32& uiFieldNum, ///<key�ֶε�����
        bool& bReadCached, ///<�����Ƿ���read cache��
        CWX_UINT8 ucKeyInfo=0 ///<�Ƿ��ȡkey��information��0����ȡkey��data��1����ȡkey��Ϣ��2����ȡϵͳkey
        )
    {
		return m_impl->get(tss,
            key,
            field,
            extra,
            szData,
            uiLen,
            bKeyValue,
            uiVersion,
            uiFieldNum,
            bReadCached,
            ucKeyInfo);
	}

    ///��ȡ���key������ֵ 1���ɹ���-1��ʧ��;
    inline int gets(UnistorTss* tss, ///<�̵߳�tss����
        list<pair<char const*, CWX_UINT16> > const& keys,  ///<Ҫ��ȡ��key���б�pair��firstΪkey�����֣�secondΪkey�ĳ���
        CwxKeyValueItemEx const* field, ///<��ָ�������޶���ȡ��field��Χ
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        char const*& szData, ///<��ȡ�����ݣ��ڴ��ɴ洢�������
        CWX_UINT32& uiLen, ///<�������ݵĳ���
        CWX_UINT32& uiReadCacheNum, ///<��read cache�е�����
        CWX_UINT32& uiExistNum, ///<���ڵ�key������
        CWX_UINT8 ucKeyInfo=0 ///<�Ƿ��ȡkey��information��0����ȡkey��data��1����ȡkey��Ϣ��2����ȡϵͳkey
        )
    {
        return m_impl->gets(tss,
            keys,
            field,
            extra,
            szData,
            uiLen,
            uiReadCacheNum,
            uiExistNum,
            ucKeyInfo);
    };

	///�����αꡣ-1���ڲ�����ʧ�ܣ�0���ɹ�
	inline int createCursor(UnistorStoreCursor& cursor, ///<�α����
        char const* szBeginKey, ///<��ʼ��key����ΪNULL��ʾû��ָ��
        char const* szEndKey, ///<������key����ΪNULL��ʾû��ָ��
        CwxKeyValueItemEx const* field, ///<ָ���α�Ҫ���ص�field��
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        char* szErr2K ///<���������ش�����Ϣ
        )
    {
		return m_impl->createCursor(cursor, szBeginKey, szEndKey, field, extra, szErr2K);
	}

	///��ȡ��һ�����ݡ�-1��ʧ�ܣ�0��������1����ȡһ��
    inline int next(UnistorTss* tss, ///<�̵߳�tss
        UnistorStoreCursor& cursor,  ///<Next���α�
		char const*& szKey,  ///<���ص�key���ڴ��ɴ洢�������
		CWX_UINT16& unKeyLen,  ///<����key���ֽ���
		char const *& szData,  ///<����key��data���ڴ��ɴ洢�������
		CWX_UINT32& uiDataLen, ///<����data���ֽ���
		bool& bKeyValue,  ///<data�Ƿ�ΪkeyValue�ṹ
		CWX_UINT32& uiVersion,  ///<key�İ汾��
        bool bKeyInfo=false ///<�Ƿ񷵻�key��information��������data
        )
    {
		return m_impl->next(tss,
            cursor,
            szKey,
            unKeyLen,
            szData,
            uiDataLen,
            bKeyValue,
            uiVersion,
            bKeyInfo);
	}

	//�ͷ��α�
	inline void closeCursor(UnistorStoreCursor& cursor){
		return m_impl->closeCursor(cursor);
	}

    ///��ʼ�������ݡ�-1���ڲ�����ʧ�ܣ�0���ɹ�
    inline int exportBegin(UnistorStoreCursor& cursor, ///<export���α�
        char const* szStartKey, ///<export�Ŀ�ʼkey����������key
        char const* szExtra, ///<extra��Ϣ
        UnistorSubscribe const& scribe,  ///<�������ݵĶ��Ĺ���
        CWX_UINT64& ullSid, ///<��ǰ��sidֵ
        char* szErr2K  ///<�������򷵻ش�����Ϣ
        )
    {
        return m_impl->exportBegin(cursor,
            szStartKey,
            szExtra,
            scribe,
            ullSid,
            szErr2K);
    }

    ///��ȡexport����һ�����ݡ�-1��ʧ�ܣ�0��������1����ȡһ����2��skip����Ϊ0
    inline int exportNext(UnistorTss* tss,  ///<�̵߳�tss����
        UnistorStoreCursor& cursor,  ///<export���α�
        char const*& szKey,    ///<����key��ֵ
        CWX_UINT16& unKeyLen,   ///<key���ֽ���
        char const*& szData,    ///<����data��ֵ
        CWX_UINT32& uiDataLen,   ///<data���ֽ���
        bool& bKeyValue,   ///<data�Ƿ�ΪKeyValue�ṹ
        CWX_UINT32& uiVersion, ///<key�İ汾��
        CWX_UINT32& uiExpire, ///<key��expireʱ��
        CWX_UINT16& unSkipNum,  ///<��ǰ������skip��binlog����
        char const*& szExtra,  ///<extra����
        CWX_UINT32&  uiExtraLen ///<extra�ĳ���
        )
    {
        return m_impl->exportNext(tss,
            cursor,
            szKey,
            unKeyLen,
            szData,
            uiDataLen,
            bKeyValue,
            uiVersion,
            uiExpire,
            unSkipNum,
            szExtra,
            uiExtraLen);
    }

    ///������������
    inline void exportEnd(UnistorStoreCursor& cursor){
        return m_impl->exportEnd(cursor);
    }

	///ͬ��master��binlog.0���ɹ���-1��ʧ��
	inline int syncMasterBinlog(UnistorTss* tss, ///<�߳�tss
        CwxPackageReaderEx* reader,  ///<��data���н�����ⲿCwxPackageReaderEx����
		CWX_UINT64 ullSid, ///<binlog��sidֵ����Ϊ0���Ǵ��±��sid��
		CWX_UINT32 ttTimestamp, ///<binlog��ʱ���
		CWX_UINT32 uiGroup, ///<binlog�����ķ���
		CWX_UINT32 uiType,  ///<binlog����Ϣ����
		CwxKeyValueItemEx const& data, ///<binlog������
        CWX_UINT32 uiVersion, ///<binlog��Ӧ��key�İ汾��
        bool  bRestore=false  ///<�Ƿ��Ǵӱ���binlog�ָ�������
        )
    {
		return m_impl->syncMasterBinlog(tss,
            reader,
            ullSid,
            ttTimestamp,
            uiGroup,
            uiType,
            data,
            uiVersion,
            bRestore);
	}

    ///commit�洢����write cache�����ݣ�0���ɹ���-1��ʧ��
    inline int commit(char* szErr2K){
        if (!m_impl) return 0;
        return m_impl->commit(szErr2K);
    }
    
    ///�رմ洢����
    inline int close(){
        if (!m_impl) return 0;
        return m_impl->close();
    }

    ///event��������ʵ�ִ洢�������ϲ�Ľ�����0���ɹ���-1��ʧ��
    int storeEvent(UnistorTss* tss, ///<�̵߳�tss
        CwxMsgBlock*& msg  ///<��Ϣ
        )
    {
        if (!m_impl) return 0;
        return m_impl->storeEvent(tss, msg);
    }

	///�洢�����checkpoint
	inline void checkpoint(UnistorTss* tss){
		m_impl->checkpoint(tss);
	}

    ///ʧȥͬ��֪ͨ�������洦������ֵ��0��û�б仯��1������ͬ����-1��ʧ��
    inline int lostSync(){
        return m_impl->lostSync();
    }

    ///���ʱ��ͬ���ļ�¼�������ڴ洢����expire�Ŀ��ơ�����ֵ��0���ɹ���-1��ʧ��
    inline int appendTimeStampBinlog(CwxPackageWriterEx& writer, ///<writer����
        CWX_UINT32      ttNow, ///<��ǰ��ʱ��
        char* szErr2K=NULL ///<�������򷵻ش�����Ϣ
        )
    {
        return m_impl->appendTimeStampBinlog(writer, ttNow, szErr2K);
    }

	///��ȡengine������
	inline char const* getName() const{
        if (!m_impl) return "";
		return m_impl->getName();
	}

	///��ȡengine�İ汾
	inline char const* getVersion() const{
        if (!m_impl) return "";
		return m_impl->getVersion();
	}

    ///���õ�ǰ��sid
    inline void setCurSid(CWX_UINT64 ullSid){
        m_impl->setCurSid(ullSid);
    }

    ///�Ƿ���Ҫcommit
    inline bool isNeedCommit() const{
        return m_impl->isNeedCommit();
    }

    ///��ȡ��ǰexpireʱ���
    inline CWX_UINT32 getExpireClock() const{
        return m_impl->getExpireClock();
    }

    ///���õ�ǰ��expireʱ���
    inline void setExpireClock(CWX_UINT32 ttClock){
        m_impl->setExpireClock(ttClock);
    }

    ///�Ƿ���ָ��������
    inline bool isSubscribe(UnistorSubscribe const& subscribe, ///<���Ķ���
        CWX_UINT32 uiGroup, ///<binlog�����ķ���
        char const* szKey ///<binlog��key
        )
    {
        return m_impl->isSubscribe(subscribe, uiGroup, szKey);
    }

    ///���ĸ�ʽ�Ƿ�Ϸ�
    inline bool isValidSubscribe(UnistorSubscribe const& subscribe,///<���Ķ���
        char* szErr2K ///<���Ϸ�ʱ�Ĵ�����Ϣ
        )
    {
        return m_impl->isValidSubscribe(subscribe, szErr2K);
    }

    ///key��ascii����less������0����ȣ�-1��С�ڣ�1������
    inline int isKeyAsciiLess(char const* key1,
        CWX_UINT16 unKey1Len,
        char const* key2,
        CWX_UINT16 unKey2Len){
        return m_impl->isKeyAsciiLess(key1, unKey1Len, key2, unKey2Len);
    }
public:
	///flush binlog
    inline void flushBinlog(){
		if (m_impl) m_impl->flushBinlog();
	}

	///get binlog ������
	inline CwxBinLogMgr* getBinLogMgr(){
        return m_impl->getBinLogMgr();
    }

	///�洢�Ƿ���Ч
	inline bool isValid() const{
        return m_impl->isValid();
    }

	///��ȡ�洢��Чʱ�Ĵ�����Ϣ
	inline char const* getErrMsg() const{
        if (!m_impl) return "";
        return m_impl->getErrMsg();
    }

	///��ȡ�洢�������
	inline UnistorStoreBase* getStoreEngine(){
        return m_impl;
    }

private:
	UnistorStoreBase*	    m_impl; ///<�洢�������ָ��
	void*					m_dllHandle; ///<dll��dlopen���ص�handle
};



#endif
