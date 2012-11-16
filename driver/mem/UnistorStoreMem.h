#ifndef __UNISTOR_STORE_MEM_H__
#define __UNISTOR_STORE_MEM_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxFile.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include "UnistorStoreBase.h"
#include "UnistorStoreMemCache.h"

///�����ļ�
/****
[mem]
expire_check_step=���г�ʱ���Ĳ���
****/

///������غ���
extern "C" {
	UnistorStoreBase* unistor_create_engine();
}


///�����������ö���
class UnistorConfigMem{
public:
    enum{
        UNISTOR_CONFIG_MIN_CHECK_NUM = 1000, ///<ÿ�γ�ʱ������С����
        UNISTOR_CONFIG_MAX_CHECK_NUM = 1000000, ///<ÿ�μ����������
        UNISTOR_CONFIG_DEF_CHECK_NUM = 100000,  ///<ȱʡÿ�εļ������
        UNISTOR_CONFIG_BATCH_REMOVE_NUM = 100  ///<ÿ����ɾ��������
    };
public:
    ///���캯��
	UnistorConfigMem(){
        m_uiExpireCheckNum = UNISTOR_CONFIG_DEF_CHECK_NUM;
	}
public:
    CWX_UINT32          m_uiExpireCheckNum; ///<expire�ļ������
};



//mem��cursor�������
class UnistorStoreMemCursor{
public:
    ///���캯��
    UnistorStoreMemCursor(){
        m_unExportBeginKeyLen = 0;
        m_szStoreKey[0] = 0x00;
        m_unStoreKeyLen = 0;
        m_szStoreData[0] = 0x00;
        m_uiStoreDataLen = 0;
        m_uiSlotIndex = 0;
        m_uiSlotPos = 0;
    }
    ///��������
    ~UnistorStoreMemCursor(){
    }
public:
    char                  m_szExportBeginKey[UNISTOR_MAX_KEY_SIZE]; ///<export�Ŀ�ʼkey
    CWX_UINT16            m_unExportBeginKeyLen; ///<key�ĳ���
    char			      m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<�洢��key
    CWX_UINT16            m_unStoreKeyLen; ///<key�ĳ���
    char                  m_szStoreData[UNISTOR_MAX_KV_SIZE]; ///<�洢��data
    CWX_UINT32            m_uiStoreDataLen; ///<data�ĳ���
    CWX_UINT32            m_uiSlotIndex; ///<cache��slot��index
    CWX_UINT32            m_uiSlotPos; ///<cache��slot��pos
};


///mem�Ĵ洢����
class UnistorStoreMem: public UnistorStoreBase{
public:
    ///���캯��
    UnistorStoreMem(){
        m_memCache = NULL;
        UnistorStoreBase::m_fnKeyStoreGroup = UnistorStoreMem::keyAsciiGroup;
        UnistorStoreBase::m_fnKeyAsciiGroup = UnistorStoreMem::keyAsciiGroup;
        UnistorStoreBase::m_fnKeyAsciiLess = UnistorStoreMem::keyAsciiCmpLess;
    }
    ///��������
    ~UnistorStoreMem(){
    }
public:
    //���������ļ�.-1:failure, 0:success
    virtual int init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc, ///<�洢�������ϲ����Ϣͨ������
        UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<��ȡϵͳ��Ϣ��function
        void* pApp, ///<UnistorApp����
        UnistorConfig const* config ///<�����ļ�
        );

    ///����Ƿ����key��1�����ڣ�0�������ڣ�-1��ʧ��
    virtual int isExist(UnistorTss* tss, ///tss����
        CwxKeyValueItemEx const& key, ///<����key
        CwxKeyValueItemEx const* field, ///<����field����Ϊ�ձ�ʾ���key
        CwxKeyValueItemEx const* extra, ///<�洢�����extra data
        CWX_UINT32& uiVersion, ///<����key�İ汾��
        CWX_UINT32& uiFieldNum, ///<����key��field������
        bool& bReadCached ///<�����Ƿ���read cache��
        );

    ///���key��1���ɹ���0�����ڣ�-1��ʧ�ܣ�
    virtual int addKey(UnistorTss* tss, ///<tss����
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
        CWX_UINT32 uiExpire=0 ///<����Ϊ0����ָ��key��expireʱ�䡣����Ҫ�洢����֧��
        );


    ///set key��1���ɹ���-1������0�������ڣ���������һ��key��fieldʱ��
    virtual int setKey(UnistorTss* tss,///tss
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
        CWX_UINT32 uiExpire=0 ///<��ָ�������޸�key��expireʱ�䡣����Ҫ�洢����֧��
        );    

    ///update key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2���汾����
    virtual int updateKey(UnistorTss* tss, ///<tss����
        CwxKeyValueItemEx const& key, ///<update��key
        CwxKeyValueItemEx const* field,///<��update field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra ����
        CwxKeyValueItemEx const& data, ///<update������
        CWX_UINT32 uiSign, ///<update�ı��
        CWX_UINT32& uiVersion, ///<��ָ������key�İ汾�������ֵһ�£��������ʧ��
        CWX_UINT32& uiFieldNum, ///<����key field������
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        CWX_UINT32 uiExpire=0 ///<��ָ�������޸�key��expireʱ��
        );


    ///inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2:�汾����-3�������߽�
    virtual int incKey(UnistorTss* tss, ///<�߳�tss����
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
        );


    ///delete key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�-2:�汾����
    virtual int delKey(UnistorTss* tss, ///<�߳�tss����
        CwxKeyValueItemEx const& key, ///<Ҫɾ����key
        CwxKeyValueItemEx const* field, ///<��Ҫɾ��field����ָ��field������
        CwxKeyValueItemEx const* extra,///<�洢�����extra ����
        CWX_UINT32& uiVersion, ///<��ָ���汾�ţ����޸�ǰ�İ汾�ű������ֵ��ȣ�����ʧ�ܡ������°汾��
        CWX_UINT32& uiFieldNum,  ///<key���ֶ�����
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached ///<�����Ƿ���write cache��
        );


    ///sync ���key��1���ɹ���0�����ڣ�-1��ʧ�ܣ�
    virtual int syncAddKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<key������
        CwxKeyValueItemEx const* field, ///<�ֶε�����
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<add������
        CWX_UINT32 uiSign, ///<add��sign
        CWX_UINT32 uiVersion, ///<�����İ汾��
        bool bCache, ///<�Ƿ�cache����
        CWX_UINT32 uiExpire, ///<key��expireʱ��
        CWX_UINT64 ullSid, ///<�����־��sidֵ
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��Ǵ�binlog�ָ�������
        );

    ///sync set key��1���ɹ���-1������0�������ڣ���������һ��key��fieldʱ��
    virtual int syncSetKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<set��key
        CwxKeyValueItemEx const* field, ///<����set field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<set������
        CWX_UINT32 uiSign,  ///<set��sign
        CWX_UINT32 uiVersion, ///<set��key �汾��
        bool bCache, ///<�Ƿ�cache����
        CWX_UINT32 uiExpire, ///<expireʱ��
        CWX_UINT64 ullSid, ///<set binlog��sidֵ
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��Ǵ�binlog�ָ�������
        );

    ///sync update key��1���ɹ���0�������ڣ�-1��ʧ��
    virtual int syncUpdateKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<update��key
        CwxKeyValueItemEx const* field, ///<����update field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<update��������
        CWX_UINT32 uiSign, ///<update�ı��
        CWX_UINT32 uiVersion, ///<update���key�İ汾��
        CWX_UINT32 uiExpire, ///<update��expireʱ��
        CWX_UINT64 ullSid, ///<update���binlog��sid
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�лָ�������
        );

    ///sync inc key��1���ɹ���0�������ڣ�-1��ʧ�ܣ�
    virtual int syncIncKey(UnistorTss* tss, ///<�̵߳�tss����
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
        CWX_UINT32 uiExpire, ///<update��expireʱ��
        CWX_UINT64 ullSid, ///<inc����binlog��sidֵ
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�ָ�������
        );

    ///sync delete key��1���ɹ���0�������ڣ�-1��ʧ��
    virtual int syncDelKey(UnistorTss* tss, ///<�̵߳�tss����
        CwxKeyValueItemEx const& key, ///<Ҫɾ����key
        CwxKeyValueItemEx const* field, ///<����ɾ��field����ָ��field
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CWX_UINT32 uiVersion, ///<key����delete��İ汾��
        CWX_UINT64 ullSid, ///<delete������Ӧ��binlog��sid
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool  bRestore=false ///<�Ƿ��binlog�ָ�������
        );

    ///import key��1���ɹ���-1��ʧ�ܣ�
    virtual int importKey(UnistorTss* tss, ///<tss����
        CwxKeyValueItemEx const& key, ///<��ӵ�key
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        CwxKeyValueItemEx const& data, ///<���key��field������
        CWX_UINT32& uiVersion, ///<������0���������޸ĺ��keyΪ�˰汾
        bool& bReadCached, ///<�����Ƿ���read cache��
        bool& bWriteCached, ///<�����Ƿ���write cache��
        bool bCache=true, ///<�Ƿ�key�ŵ���cache
        CWX_UINT32 uiExpire=0 ///<������key������ָ����uiExpire������key�ĳ�ʱʱ��
        );

    ///sync import key��1���ɹ���-1������
    virtual int syncImportKey(UnistorTss* tss, ///<�̵߳�tss����
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
        );


    ///��ȡkey, 1���ɹ���0�������ڣ�-1��ʧ��;
    virtual int get(UnistorTss* tss, ///<�߳�tss����
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
        );

    ///��ȡ���key, 1���ɹ���-1��ʧ��;
    virtual int gets(UnistorTss* tss, ///<�̵߳�tss����
        list<pair<char const*, CWX_UINT16> > const& keys,  ///<Ҫ��ȡ��key���б�pair��firstΪkey�����֣�secondΪkey�ĳ���
        CwxKeyValueItemEx const* field, ///<��ָ�������޶���ȡ��field��Χ
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        char const*& szData, ///<��ȡ�����ݣ��ڴ��ɴ洢�������
        CWX_UINT32& uiLen, ///<�������ݵĳ���
        CWX_UINT32& uiReadCacheNum, ///<��read cache�е�����
        CWX_UINT32& uiExistNum, ///<���ڵ�key������
        CWX_UINT8 ucKeyInfo=0 ///<�Ƿ��ȡkey��information��0����ȡkey��data��1����ȡkey��Ϣ��2����ȡϵͳkey
        );

    ///�����αꡣ-1���ڲ�����ʧ�ܣ�0����֧�֣�1���ɹ�
    virtual int createCursor(UnistorStoreCursor& cursor, ///<�α����
        char const* szBeginKey, ///<��ʼ��key����ΪNULL��ʾû��ָ��
        char const* szEndKey, ///<������key����ΪNULL��ʾû��ָ��
        CwxKeyValueItemEx const* field, ///<ָ���α�Ҫ���ص�field��
        CwxKeyValueItemEx const* extra, ///<�洢�����extra����
        char* szErr2K ///<���������ش�����Ϣ
        );

    ///��ȡ���ݡ�-1��ʧ�ܣ�0��������1����ȡһ��
    virtual int next(UnistorTss* tss, ///<�̵߳�tss
        UnistorStoreCursor& cursor,  ///<Next���α�
        char const*& szKey,  ///<���ص�key���ڴ��ɴ洢�������
        CWX_UINT16& unKeyLen,  ///<����key���ֽ���
        char const *& szData,  ///<����key��data���ڴ��ɴ洢�������
        CWX_UINT32& uiDataLen, ///<����data���ֽ���
        bool& bKeyValue,  ///<data�Ƿ�ΪkeyValue�ṹ
        CWX_UINT32& uiVersion,  ///<key�İ汾��
        bool bKeyInfo=false ///<�Ƿ񷵻�key��information��������data
        );

    ///�ر��α�
    virtual void closeCursor(UnistorStoreCursor& cursor);

    ///��ʼ�������ݡ�-1���ڲ�����ʧ�ܣ�0���ɹ�
    virtual int exportBegin(UnistorStoreCursor& cursor, ///<export���α�
        char const* szStartKey, ///<export�Ŀ�ʼkey����������key
        char const* szExtra, ///<extra��Ϣ
        UnistorSubscribe const& scribe,  ///<�������ݵĶ��Ĺ���
        CWX_UINT64& ullSid, ///<��ǰ��sidֵ
        char* szErr2K  ///<�������򷵻ش�����Ϣ
        );

    ///��ȡ���ݡ�-1��ʧ�ܣ�0��������1����ȡһ����2��skip����Ϊ0
    virtual int exportNext(UnistorTss* tss,  ///<�̵߳�tss����
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
        );

    ///������������
    virtual void exportEnd(UnistorStoreCursor& cursor);

    ///��鶩�ĸ�ʽ�Ƿ�Ϸ�
    virtual bool isValidSubscribe(UnistorSubscribe const& subscribe,///<���Ķ���
        char* szErr2K ///<���Ϸ�ʱ�Ĵ�����Ϣ
        );

    ///commit��0���ɹ���-1��ʧ��
    virtual int commit(char* szErr2K);

    ///�ر�bdb����
    virtual int close();

    ///event��������ʵ�ִ洢�������ϲ�Ľ�����0���ɹ���-1��ʧ��
    virtual int storeEvent(UnistorTss* tss, CwxMsgBlock*& msg);

    ///bdb����checkpoint
    virtual void checkpoint(UnistorTss* tss);

    ///ʧȥͬ��֪ͨ�������洦������ֵ��0��û�б仯��1������ͬ����-1��ʧ��
    virtual int lostSync();

    ///��ȡengine������
    virtual char const* getName() const{
        return "mem";
    }

    ///��ȡengine�İ汾
    virtual char const* getVersion() const{
        return "1.0.0";
    }
    ///��ȡcache�Ƿ�����
    virtual bool isCacheValid() const{
        return true;
    }
    ///��ȡcache�Ĵ�����Ϣ
    virtual char const* getCacheErrMsg() const{
        return "";
    }
    ///��ȡдcache��key������
    virtual CWX_UINT32 getWriteCacheKeyNum() const{
        return 0;
    }
    ///��ȡwrite cacheʹ�õĳߴ�
    virtual  CWX_UINT32 getWriteCacheUsedSize() const{
        return 0;
    }
    ///��ȡ��cache�Ĵ�С
    virtual unsigned long int getReadCacheMaxSize( void ) const{
        return m_memCache?m_memCache->maxSize():0;
    }
    ///��ȡcache key���������
    virtual CWX_UINT32 getReadCacheMaxKeyNum() const{
        return m_memCache?m_memCache->getMaxCacheKeyNum():0;
    }
    ///��ȡread cacheʹ�õ��ڴ�
    virtual unsigned long int getReadCacheUsedSize() const{
        return m_memCache?m_memCache->getUsedSize():0;
    }
    ///��ȡread cache�����ݵ�ռ�õ�����
    virtual unsigned long int getReadCacheUsedCapacity() const{
        return m_memCache?m_memCache->getUsedCapacity():0;
    }
    ///��ȡread cache��cache����Ч���ݴ�С
    virtual unsigned long int getReadCacheUsedDataSize() const{
        return m_memCache?m_memCache->getUsedDataSize():0;
    }
    ///��ȡread cache�п��д�С
    virtual unsigned long int getReadCacheFreeSize() const{
        return m_memCache?m_memCache->getFreeSize():0;
    }
    ///��ȡread cache�п��е�����
    virtual unsigned long int getReadCacheFreeCapacity() const{
        return m_memCache?m_memCache->getFreeCapacity():0;
    }
    ///��ȡcache��key������
    virtual CWX_UINT32 getReadCacheKeyCount() const{
        return m_memCache?m_memCache->getCachedKeyCount():0;
    }

private:

    ///key����ȱȽϺ���������ֵ��true����ȣ�false�������
    static bool keyStoreCmpEqual(char const* key1, ///<��һ��key
        CWX_UINT16 unKey1Len, ///<key�ĳ���
        char const* key2, ///<�ڶ���key
        CWX_UINT16 unKey2Len ///<�ڶ���key�ĳ���
        )
    {
        return (unKey1Len == unKey2Len) && (memcmp(key1, key2, unKey1Len)==0);
    }

    ///key��С�ڱȽϺ���������ֵ��0��key1==key2��1��key1>key2��-1��key1<key2
    static int keyStoreCmpLess(char const* key1, ///<��һ��key
        CWX_UINT16 unKey1Len, ///<��һ��key�ĳ���
        char const* key2, ///<�ڶ���key
        CWX_UINT16 unKey2Len ///<�ڶ���key�ĳ���
        )
    {
        int ret = memcmp(key1, key2, unKey1Len<unKey2Len?unKey1Len:unKey2Len);
        if (0 != ret) return ret;
        return unKey1Len==unKey2Len?0:(unKey1Len<unKey2Len?-1:1);
    }

    ///key��hash����
    static size_t keyStoreHash(char const* key, ///<key
        CWX_UINT16 unKeyLen ///<key�ĳ���
        )
    {
        size_t h = 216613626UL;
        for (CWX_UINT16 i = 0; i < unKeyLen; ++i) {
            h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
            h ^= key[i];
        }
        return h;
    }
    ///key��group����
    static CWX_UINT32 keyStoreGroup(char const* key, ///<key������
        CWX_UINT16 unKeyLen ///<key�ĳ���
        )
    {
        return keyAsciiGroup(key, unKeyLen);
    }

    ///key��ascii��group����
    static CWX_UINT32 keyAsciiGroup(char const* key, ///<key������
        CWX_UINT16 unKeyLen ///<key�ĳ���
        )
    {
        CWX_UINT32 uiGroup = 0;
        CwxMd5 md5;
        unsigned char szMd5[16];
        md5.update((unsigned char const*)key, unKeyLen);
        md5.final(szMd5);
        memcpy(&uiGroup, szMd5, 4);
        return uiGroup;
    }

    ///key��ascii���͵�С�ڱȽϺ���������ֵ��0��key1==key2��1��key1>key2��-1��key1<key2
    static int keyAsciiCmpLess(char const* key1, ///<��һ��key
        CWX_UINT16 unKey1Len, ///<��һ��key�ĳ���
        char const* key2, ///<�ڶ���key
        CWX_UINT16 unKey2Len ///<�ڶ���key�ĳ���
        )
    {
        int ret = memcmp(key1, key2, unKey1Len<unKey2Len?unKey1Len:unKey2Len);
        if (0 != ret) return ret;
        return unKey1Len==unKey2Len?0:(unKey1Len<unKey2Len?-1:1);
    }

private:
    ///commit��0���ɹ���-1��ʧ��
    int _commit(char* szErr2K);

    //����bdb��������Ϣ������ֵ0:�ɹ���-1��ʧ��
    int parseConf();

    //��ȡϵͳkey��1���ɹ���0�������ڣ�-1��ʧ��;
    int _getSysKey(UnistorTss* tss, ///<�߳�tss����
        char const* key, ///<Ҫ��ȡ��key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
        CWX_UINT32& uiLen  ///<szData���ݵ��ֽ���
        );

    //set key��0:�ɹ���-1��ʧ��
    int _setKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen,///<key�ĳ���
        char const* szData, ///<key��data
        CWX_UINT32 uiLen, ///<data�ĳ���
        CWX_UINT32* uiExpire, ///<expireʱ��ֵ
        char* szErr2K=NULL ///<����ʱ���ش�������
        );

    //��ȡkey��data������ֵ��0:�����ڣ�1����ȡ��-1��ʧ��
    int _getKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData, ///<key��data
        CWX_UINT32& uiLen,///<����data��buf size������data�ĳ���
        CWX_UINT32& uiExpire, ///<key�ĳ�ʱʱ��
        char* szErr2K=NULL ///<����ʱ���ش�������
        );

    //ɾ��key��ͬʱ��cache��ɾ��������ֵ��0:�ɹ���-1��ʧ��
    int _delKey(char const* szKey, ///<key������
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szErr2K=NULL ///<����ʱ���ش�������
        );

    //key��dataΪfield�ṹ��true���ǣ�false������
    inline bool isKvData(char const* szData, CWX_UINT32 uiDataLen){
        if (!szData || (uiDataLen<5)) return false;
        return szData[uiDataLen-1] == 0?false:true;
    }

    //��ȡdata��version
    inline void getKvVersion(char const* szData, ///<data����
        CWX_UINT32 uiDataLen, ///<data�ĳ���
        CWX_UINT32& uiVersion ///<key�İ汾��
        )
    {
        CWX_ASSERT(uiDataLen >= 5);
        memcpy(&uiVersion, szData + (uiDataLen-5), 4);
    }

    ///����data��extra����
    inline void setKvDataSign(char* szData, ///<key��data
        CWX_UINT32& uiDataLen, ///<���뵱ǰdata�ĳ��ȣ������³���
        CWX_UINT32 uiVersion, ///<key�İ汾��
        bool bKeyValue ///<data��key/value���
        )
    {
        memcpy(szData + uiDataLen, &uiVersion, sizeof(uiVersion));
        uiDataLen += sizeof(uiVersion);
        szData[uiDataLen] = bKeyValue?1:0;
        uiDataLen++;
    }

    ///��ȡdata��extra���ݳ���
    inline CWX_UINT32 getKvDataSignLen() const {
        return sizeof(CWX_UINT32) + 1;
    }
    //unpack��data��field��-1��ʧ�ܣ�0������kv�ṹ��1���ɹ�
    inline int unpackFields(CwxPackageReaderEx& reader, ///<reader����
        char const* szData, ///<fields��key/value����
        CWX_UINT32 uiDataLen, ///<fields��key/value���ݵĳ���
        CWX_UINT32& uiVersion ///<���ݵİ汾��
        )
    {
        if (!isKvData(szData, uiDataLen)) return 0;
        getKvVersion(szData, uiDataLen, uiVersion);
        if (!reader.unpack(szData, uiDataLen-5, false, true)){
            return -1;
        }
        return 1;
    }

    ///��ȡ��ʱʱ��
    inline CWX_UINT32 getNewExpire(CWX_UINT32 uiExpire ///<��ʱʱ��
        )
    {
        if (uiExpire > 3600 * 24 * 365){
            return uiExpire;
        }else if (uiExpire){
            return uiExpire + m_ttExpireClock;
        }
        return 0;
    }
private:
    UnistorConfigMem			m_memConf; ///<������Ϣ
    UnistorStoreMemCache*       m_memCache; ///<�ڴ�cache
};

#endif
