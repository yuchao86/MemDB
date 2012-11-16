#ifndef __UNISTOR_STORE_BDBC_H__
#define __UNISTOR_STORE_BDBC_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxFile.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include <db.h>
#include "UnistorStoreBase.h"

///�����ļ�
/****
[bdbc]
env_home=/data3/bdb_home      #bdb��homeĿ¼
db_path=/data4/bdb_data       #bdb������Ŀ¼
compress=no                   #�Ƿ�ѹ��
cache_msize=8192              #cache�Ĵ�С
page_ksize = 32               #bdb page�Ĵ�С
key_type=int32/int64/int128/int256/char #key�����ͣ������������ͣ�ȫ�����޷�������
int32_hex=yes/no              #�Ƿ񽫷���ֵĬ��10���Ƹ�Ϊ16����
int64_hex=yes/no              #�Ƿ񽫷���ֵĬ��16���Ƹ�Ϊ10����
hex_upper=yes/no              #���ڷ��ص�hex���������Ƿ�Ϊ��д��Ĭ��ΪСд
group_start_time_bit = 4      #����32λgroup�������ļ�����Ŀ�ʼλ����0��ʼ��ʾbit0��
group_end_time_bit =7         #����32λgroup�������ļ�����Ľ���λ��7��ʾ[4,7]��4λ��
counter_def_file =/data4/bdb_data/counter.def #counter�Ķ����ļ�
****/

/***********************************bdbc����˵��*******************************/
//1��key������Ϊint32��int64��int128��int256��char��
//        ���ڲ�ͬ��key���ͣ�group��Ӧֵ���£�
//        int32-->����groupֵ
//        int64-->��32λΪgroupֵ
//        int128-->��32λΪgroupֵ
//        int256-->��32λΪgroupֵ
//        char-->md5ǩ���ĵ�32λΪgroupֵ
//        ���ڲ�ͬ��key���ͣ�hash��Ӧֵ����
//        int32-->����hashֵ
//        int64-->��32λΪhashֵ
//        int128-->��32λΪhashֵ
//        int256-->��32λΪhashֵ
//        char-->md5ǩ���ĵ�32λΪhashֵ
//2��key�Ĵ���
//        int32-->����Ϊ10���ƣ�Ҳ����Ϊ16���ƣ�16������0x��ͷ������ʱĬ����10���ƣ���������Ϊ16���ơ�
//        int64-->����Ϊ10���ƣ�Ҳ����Ϊ16���ƣ�16������0x��ͷ������ʱĬ����16���ƣ���������Ϊ10���ơ�
//        int128-->16���Ƶ��ַ���������0x��ͷ������ʱ��16����
//        int256-->16���Ƶ��ַ���������0x��ͷ������ʱ��16����

//2���������ݣ�bdb�ڲ��洢��Ϊ[������1][������2]...[version].
//     �������ĸ�ʽΪ��[һ���ַ��ļ���������][32λ�ļ���ֵ]��5���ֽ�
//     version��Ϊ32λ��������
//3�����ڼ�����Ϊ0�ļ�������ϵͳ���Զ�ɾ����
//4������ȡkey��һ��ָ���ļ��������򷵻ؼ���������ֵ��Ϊ10���Ʊ�ʾ��
//5������ȡkey�Ķ�����������򷵻صĸ�ʽΪ��[������1����=��ֵ];[������2����=��ֵ].....
//     һ���������ڣ���������ֵͨ����=���ָ����Ϊ10���Ƶ��ַ���
//     �����������ͨ����;���ָ
//7����һ��ʵ���ڲ��������������2^[group_end_time_bit - group_start_time_bit + 1]��bdb�ļ��洢��
//   ÿ���ļ�������Ϊbdbc.��ʼbit-������-0.dat, bdbc.��ʼbit-������-1.dat.....���ļ��洢��
//   ÿ���ļ���ŵ��Ƕ�Ӧ��[2^group_start_time_bit]���ڵ����ݡ�group_start_time_bit�������ⲿ����ġ�
//   ����һ�����ò����޸�
//8��ϵͳ����һ��ϵͳ���ݿ�.bdbc.sys���䱣�����µ�key��
//   sid:��ǰ�Ѿ�commit��sid��ֵ
//   group_start_time_bit:��ʼ��ʱ��λ
//   group_end_time_bit:����ʱ��λ
/********************д���뵥�߳�*************************/
#define  BDBC_MIN_READ_CACHE_MSIZE   128            ///<��С��cache
#define  BDBC_MAX_READ_CACHE_MSIZE   (64 * 1024)   ///<����cache

#define  BDBC_MIN_WRITE_CACHE_MSIZE  8             ///<��Сдcache
#define  BDBC_MAX_WRITE_CACHE_MSIZE  128           ///<���дcache

#define  BDBC_MIN_READ_CACHE_KEY_NUM   100000     ///<��cache�����key������
#define  BDBC_MAX_READ_CACHE_KEY_NUM   1600000000  ///<��cache�����key������


#define  BDBC_KEY_GROUP_START_TIME_BIT  "bdbc_group_start_time_bit"
#define  BDBC_KEY_GROUP_END_TIME_BIT    "bdbc_group_end_time_bit"
#define  BDBC_KEY_COUNTERS              "bdbc_counters"
#define  BDBC_KEY_KEY_TYPE              "bdbc_key_type"

#define  BDBC_MAX_COUNTER_VALUE         0xFFFFFFFF 

#define  BDBC_KEY_COUNTER_SPLIT_CHR      ';'     ///<�������ķָ��ַ�

#define  BDBC_DATA_FILE_FORMAT           "bdbc.%u-%u-%u.dat"  ///<�����ļ���format�ַ���������%�ֱ��Ƿ���Ŀ�ʼλ��������������������
#define  BDBC_SYS_FILE_NAME              ".bdbc.sys"   ///<ϵͳbdb�ļ���
#define  BDBC_KEY_VERSION                "version"  ///<�汾��
#define  BDBC_SYS_COUNTER_VERSION        "counter_version"  ///<�汾��

#define BDBC_ENGINE_KEY_TYPE_INT32       "int32"    ///<int32���͵�key
#define BDBC_ENGINE_KEY_TYPE_INT64       "int64"    ///<int64���͵�key
#define BDBC_ENGINE_KEY_TYPE_INT128      "int128"   ///<int128���͵�key
#define BDBC_ENGINE_KEY_TYPE_INT256      "int256"   ///<int256���͵�key
#define BDBC_ENGINE_KEY_TYPE_CHAR        "char"     ///<char���͵�key

#define  BDBC_HEX_64_ZERO                "000000000000000000000000000000000000000000000000000000000000000000"
///������غ���
extern "C" {
	UnistorStoreBase* unistor_create_engine();
}

///key��group������
typedef int (*BDBC_KEY_CMP_LESS_FN)(DB *, const DBT *a, const DBT *b);

///��������key����
struct UnistorBdbcKey{
public:
    enum{
        BDBC_KEY_TYPE_UNKNOWN = 0, ///<δ֪key����
        BDBC_KEY_TYPE_INT32 = 1,  ///<32λ���ֵ�key����
        BDBC_KEY_TYPE_INT64 = 2,  ///<64λ���ֵ�key����
        BDBC_KEY_TYPE_INT128 = 3, ///<128λ���ֵ�key����
        BDBC_KEY_TYPE_INT256 = 5, ///<256λ���ֵ�key����
        BDBC_KEY_TYPE_CHAR   = 6  ///<�ַ�����key����
    };
public:
    ///���캯��
    UnistorBdbcKey():m_szKey(NULL), m_unKeyLen(0){
    }
    ///���캯��
    UnistorBdbcKey(char const* szKey, CWX_UINT16 unKeyLen)
        :m_szKey(szKey), m_unKeyLen(unKeyLen)
    {
    }
public:    
    ///�������
    void reset(){
        m_szKey = NULL;
        m_unKeyLen = 0;
    }
    ///�Ƿ�key��Ҫ����;����ֵ��true����Ҫ���룻false������Ҫ����
    inline static bool isNeedEncode(){ return BDBC_KEY_TYPE_CHAR != m_ucKeyType;}
    ///�Ƿ�key��Ҫ����;����ֵ��true����Ҫ���룻false������Ҫ����
    inline static bool isNeedDecode(){ return BDBC_KEY_TYPE_CHAR != m_ucKeyType;}
    ///��ascii�γ�һ��key������ֵ��true���ɹ���false��ʧ��
    inline static bool encode(char const* szAsciiKey, CWX_UINT16 unKeyLen,  char* szBuf, CWX_UINT16& unBufLen, char* szErr2K){
        CWX_UINT32 uiValue=0;
        CWX_UINT64 ullValue=0;
        CWX_UINT32 i=0;
        char const* pKey = NULL;
        char szKey[65];
        char szTmp[17];
        switch(m_ucKeyType){
            case BDBC_KEY_TYPE_INT32:
                uiValue = strtoul(szAsciiKey, NULL, 0);
                memcpy(szBuf, &uiValue, sizeof(uiValue));
                unBufLen = sizeof(uiValue);
                return true;
            case BDBC_KEY_TYPE_INT64:
                ullValue = strtoull(szAsciiKey, NULL, 0);
                memcpy(szBuf, &ullValue, sizeof(ullValue));
                unBufLen = sizeof(ullValue);
                return true;
            case BDBC_KEY_TYPE_INT128:
                if (32 > unKeyLen){
                    memcpy(szKey, BDBC_HEX_64_ZERO, 32 - unKeyLen);
                    memcpy(szKey + 32 - unKeyLen, szAsciiKey, unKeyLen);
                    pKey = szKey;
                }else{
                    pKey = szAsciiKey;
                }
                for (i=0; i<2; i++){
                    memcpy(szTmp, pKey + i * 16, 16);
                    szTmp[0] = 0x0;
                    ullValue = strtoull(szTmp, NULL, 16);
                    memcpy(szBuf + i * sizeof(ullValue), &ullValue, sizeof(ullValue));
                }
                unBufLen = sizeof(ullValue)*2;
                return true;
            case BDBC_KEY_TYPE_INT256:
                if (64 > unKeyLen){
                    memcpy(szKey, BDBC_HEX_64_ZERO, 64 - unKeyLen);
                    memcpy(szKey + 64 - unKeyLen, szAsciiKey, unKeyLen);
                    pKey = szKey;
                }else{
                    pKey = szAsciiKey;
                }
                for (i=0; i<4; i++){
                    memcpy(szTmp, pKey + i * 16, 16);
                    szTmp[0] = 0x0;
                    ullValue = strtoull(szTmp, NULL, 16);
                    memcpy(szBuf + i * sizeof(ullValue), &ullValue, sizeof(ullValue));
                }
                unBufLen = sizeof(ullValue)*4;
                return true;
        }
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Key type[%u] needn't encode.");
        return false;
    }
    ///��store key�γ�key��ascii������ֵ��true���ɹ���false��ʧ��
    inline static bool decode(char const* szStoreKey, CWX_UINT16 ,  char* szBuf, CWX_UINT16& unBufLen, char* szErr2K){
        CWX_UINT32 uiValue=0;
        CWX_UINT64 ullValue=0;
        CWX_UINT32 i=0;
        char const* szFormat=NULL;
        switch(m_ucKeyType){
            case BDBC_KEY_TYPE_INT32:
                memcpy(&uiValue, szStoreKey, sizeof(CWX_UINT32));
                szFormat = (16 == m_ucBase)?(m_bUpperHex?"%X":"%x"):"%u";
                unBufLen = sprintf(szBuf, szFormat, uiValue);
                return true;
            case BDBC_KEY_TYPE_INT64:
                memcpy(&ullValue, szStoreKey, sizeof(CWX_UINT64));
                szFormat = (16 == m_ucBase)?(m_bUpperHex?"%"PRIX64:"%"PRIx64):"%u";
                unBufLen = sprintf(szBuf, szFormat, ullValue);
                return true;
            case BDBC_KEY_TYPE_INT128:
                szFormat = m_bUpperHex?"%16.16"PRIX64:"%16.16"PRIx64;
                for (i=0; i<2; i++){
                    memcpy(&ullValue, szStoreKey + sizeof(CWX_UINT64) * i, sizeof(CWX_UINT64));
                    sprintf(szBuf + 16 * i, szFormat, ullValue);
                }
                unBufLen=32;
                return true;
            case BDBC_KEY_TYPE_INT256:
                szFormat = m_bUpperHex?"%16.16"PRIX64:"%16.16"PRIx64;
                for (i=0; i<4; i++){
                    memcpy(&ullValue, szStoreKey + sizeof(CWX_UINT64) * i, sizeof(CWX_UINT64));
                    sprintf(szBuf + 16 * i, szFormat, ullValue);
                }
                unBufLen=64;
                return true;
        }
        if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Key type[%u] needn't decode.");
        return false;
    }

public:
    char const*            m_szKey; ///<key���ڴ�ָ�룬����߸�ֵ
    CWX_UINT32             m_uiGroup; ///<key��group
    CWX_UINT16             m_unKeyLen; ///<key�ĳ���
    static  CWX_UINT8      m_ucKeyType; ///<key������
    static  CWX_UINT8      m_ucBase;   ///<����INT32��INT64����ֵ�Ľ��ơ�
    static  bool           m_bUpperHex; ///<���ڷ��ص�16�����Ƿ��д
    static  UNISTOR_KEY_CMP_EQUAL_FN   m_storeKeyEqualFn; ///<�洢����key����ȱȽϺ���
    static  UNISTOR_KEY_CMP_LESS_FN    m_storeKeyLessFn; ///<�洢����key��less�ȽϺ���
    static  UNISTOR_KEY_HASH_FN        m_storeKeyHashFn; ///<�洢����key��hash����
    static  UNISTOR_KEY_GROUP_FN       m_storeKeyGroupFn; ///<�洢����key��group����
    static  UNISTOR_KEY_CMP_EQUAL_FN   m_asciiKeyEqualFn; ///<ascii����key����ȱȽϺ���
    static  UNISTOR_KEY_CMP_LESS_FN    m_asciiKeyLessFn;  ///<ascii����key��less����
    static  UNISTOR_KEY_GROUP_FN       m_asciiKeyGroupFn; ///<ascii����key��group����
    static  BDBC_KEY_CMP_LESS_FN       m_bdbKeyLessFn; ///<bdb�ıȽϺ���
};


///һ��������������
struct UnistorBdbcCounter{
public:
    ///���캯��
    UnistorBdbcCounter(){
        m_uiValue = 0;
        m_ucId = 0;
    }
public:
    CWX_UINT32  m_uiValue; ///<����ֵ
    CWX_UINT8   m_ucId; ///<��������ID
};

///��������data�����ڴ���bdbʱ����б��룬��ȡʱ�����
struct UnistorBdbcData{
public:
    ///���캯��
    UnistorBdbcData(){
        memset(this, 0x00, sizeof(UnistorBdbcData));
    }
    ///���캯��
    UnistorBdbcData(CWX_UINT32 uiVersion){
        memset(this, 0x00, sizeof(UnistorBdbcData));
        m_uiVersion = uiVersion;
    }
public:
    void reset(){
        memset(this, 0x00, sizeof(UnistorBdbcData));
    }
    ///�����ݿռ����Ϊ������
    bool decode(unsigned char const* szData, ///<��������encode����
        CWX_UINT32 uiDataLen, ///<���ݵĳ���
        char* szErr2k=NULL ///<����Ϊ�գ����ڳ���ʱ���س�����Ϣ
        );

    ///������������Ϊ���ݿռ䣬����pack����Ч����������������ʧ��Ϊ-1��
    int encode(unsigned char* szData, ///<encode�����buf
        CWX_UINT32& uiDataLen, ///<����buf�ĳ��ȣ�����encode���ݵĳ���
        char* szErr2k=NULL ///<������Ϣ
        ) const;

    ///�Ƿ���ڼ�������0�������ڣ����򷵻�����ֵ
    inline int isExist(CWX_UINT8 ucId) const{
        if (!ucId) return 0; 
        return m_counterMap[ucId];
    }
    ///�Ƿ�Ϊ��
    inline bool isEmpty() const{
        return 0==m_ucCounterNum;
    }
public:
    UnistorBdbcCounter          m_counter[256]; ///<������
    CWX_UINT8                   m_ucCounterNum; ///<������������
    CWX_UINT32                  m_uiVersion; ///<�汾
    CWX_UINT8                   m_counterMap[256]; ///<��¼�������ļ�������λ����Ϣ����ֵΪm_counter�ġ��±�+1������Ϊ0��ʾ������
};

///�����������ö���
class UnistorConfigBdbc{
public:
    ///���캯��
	UnistorConfigBdbc(){
        m_ucKeyType = UnistorBdbcKey::BDBC_KEY_TYPE_UNKNOWN;
        m_bInt32Hex = false;
        m_bInt64Hex = true;
        m_bHexUpper = false;
		m_bZip = false;
		m_uiCacheMByte = 512;
		m_uiPageKSize = 0;
        m_ucGroupStartBit = 0;
        m_ucGroupEndBit = 0;
	}
public:
	string				m_strEnvPath; ///<env��·��
	string              m_strDbPath; ///<bdb�����ļ�·��
	bool				m_bZip;      ///<�Ƿ�ѹ������
	CWX_UINT32          m_uiCacheMByte; ///<bdb������cache mbyte��
	CWX_UINT32			m_uiPageKSize; ///<ҳ�Ĵ�С
    CWX_UINT8           m_ucKeyType; ///<Key������
    string              m_strKeyType; ///<Key���͵�����
    bool                m_bInt32Hex; ///<���ص�int32��key�Ƿ�16����
    bool                m_bInt64Hex; ///<���ص�int64��key�Ƿ�16����
    bool                m_bHexUpper; ///<���ص�16�����Ƿ��д
    CWX_UINT8           m_ucGroupStartBit; ///<����ʱ�����Ŀ�ʼλ
    CWX_UINT8           m_ucGroupEndBit;  ///<����ʱ�����Ľ���Ϊ
    string              m_strCounterDefFile; ///<counter�Ķ����ļ�
};

///�����������cursor�������
class UnistorStoreBdcCursor{
public:
    ///���캯��
	UnistorStoreBdcCursor(){
		m_cursor = NULL;
        m_uiDbIndex = 0;
		m_bFirst = true;
        m_unStoreExportBeginKeyLen = 0;
        m_unStoreKeyLen = 0;
        m_uiStoreDataLen = 0;
        m_bStoreValue = false;
        m_bStoreMore = true;
	}
    ///��������
	~UnistorStoreBdcCursor(){
	}
public:
	bool				  m_bFirst; ///<�Ƿ��ǵ�һ����ȡ
    CWX_UINT32            m_uiDbIndex; ///<cursor��Ӧ��db������
	DBC*				  m_cursor;  ///<bdb��cursor handle
    UnistorBdbcData       m_bdbcData;  ///<����������
    char                  m_szStoreExportBeginKey[UNISTOR_MAX_KEY_SIZE]; ///<export�Ŀ�ʼkey
    CWX_UINT16            m_unStoreExportBeginKeyLen; ///<key�ĳ���
    char			      m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<�洢��key
    CWX_UINT16            m_unStoreKeyLen; ///<key�ĳ���
    char                  m_szStoreData[UNISTOR_MAX_KV_SIZE]; ///<�洢��data
    CWX_UINT32            m_uiStoreDataLen; ///<data�ĳ���
    bool                  m_bStoreValue; ///<�Ƿ�洢store��ֵ��
    bool                  m_bStoreMore; ///<store���Ƿ���ֵ
};


///��������ż��������ֵ�ӳ���
class UnistorStoreBdbcCounterMap: public UnistorTssEngineObj
{
public:
    ///���캯��
    UnistorStoreBdbcCounterMap(){
        CWX_UINT16 i=0;
        m_ullSeq = 0;
        for (i=0; i<256; i++){
            m_idNameMap[i] = NULL;
        }
    }
    ///��������
    virtual ~UnistorStoreBdbcCounterMap(){
        CWX_UINT16 i=0;
        m_nameIdMap.clear();
        for (i=0; i<256; i++){
            if (m_idNameMap[i]) delete [] m_idNameMap[i];
        }
    }
public:
    ///����������Ϣ��������ʱ����Ҫ����������ֵ��true���ɹ���false��ʧ�ܡ�
    ///�ļ���ʽΪ��
    ///version=�汾��
    ///���1=����������
    ///���2=����������
    ///���3=����������
    ///...... ���Ϊ0~255��������\n�ָ�
    bool parse(char const* szDef, CWX_UINT32 uiLen, CWX_UINT32 ttTimestamp, char* szErr2K);
    ///clone��һ���¶��干��������߳�ʹ�ã�����Ҫ������
    void clone(UnistorStoreBdbcCounterMap& def);

    /////���µĲ������и��Ե��̵߳��ã��������
    ///�б�����������Ƿ�ı䡣����ֵ��true���ı䣻false��û�иı�
    inline bool isDiff(CWX_UINT64 ullSeq) const{
        return m_ullSeq != ullSeq;
    }
    ///��ȡ�����ļ���ʱ���
    inline CWX_UINT32 getTimestamp() const { return m_ttTimestamp;}
    ///<��ȡ���к�
    inline CWX_UINT64 getSeq() const{ return m_ullSeq;}
    ///<��ȡ�����ļ�
    inline string const& getDef() const{ return m_strDef;}
    ///<��ȡ�汾
    inline string const& getVersion() const{ return m_strVersion;}
    ///<���ݼ�����id����ȡ�����������֣�NULL��ʾ������
    inline char const* getName(CWX_UINT8 ucId) const{
        return m_idNameMap[ucId];
    }
    ///<���ݼ����������ֻ�ȡ��������id��false��ʾ������
    inline bool getId(char const* szName, CWX_UINT8& ucId) const{
        map<char const*, CWX_UINT8, CwxCharLess>::const_iterator iter=m_nameIdMap.find(szName);
        if (iter == m_nameIdMap.end()) return false;
        ucId = iter->second;
        return true;
    }
    ///��ȡ������������
    inline CWX_UINT8 getCount() const{ return m_nameIdMap.size();}
public:
    ///Ϊ����ṩ�ı���
    UnistorBdbcData               m_bdbcOldData; ///<�ɵ�΢����������
    UnistorBdbcData               m_bdbcInputData; ///<�µ�΢����������
    UnistorBdbcKey                m_bdbcKey;     ///<΢��������key
    friend class UnistorStoreBdbc; ///<����
private:
    volatile CWX_UINT32      m_ttTimestamp; ///<�����ļ���ʱ���
    volatile CWX_UINT64      m_ullSeq;     ///<�ڲ�������
    string                   m_strDef;     ///<ӳ��Ķ�������
    string                   m_strVersion; ///<�汾��
    char*                    m_idNameMap[256]; ///<�������������������ֵ�ӳ���
    map<char const*, CWX_UINT8, CwxCharLess>   m_nameIdMap; ///<�������������������ӳ��map
    CwxMutexLock             m_lock;
};


///����bdb�ļ����洢����
class UnistorStoreBdbc : public UnistorStoreBase{
public:
    ///���캯��
    UnistorStoreBdbc():m_emptyData(0){
		m_bdbEnv = NULL;
		m_bdbs = NULL;
        m_sysDb = NULL;
        m_bdbTxn = NULL;
        m_uiGroupMask = 0;
        m_uiGroupNum = 0;
        m_pCounterDef = NULL;
        ///check expire�ĺ���
        UnistorStoreBase::m_fnKeyStoreGroup = NULL;
        UnistorStoreBase::m_fnKeyAsciiGroup = NULL;
        UnistorStoreBase::m_fnKeyAsciiLess = NULL;
    }
    ///��������
    ~UnistorStoreBdbc(){
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
        CWX_UINT8 ucKeyInfo=0 ///<�Ƿ��ȡkey��information
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
	
    ///��ȡengine������
	virtual char const* getName() const{
		return "bdbc";
	}
	
    ///��ȡengine�İ汾
	virtual char const* getVersion() const{
		return "1.0.1";
	}

private:
    ///dirty flush�߳�֪ͨ��ʼflush dirty���ݡ�����ֵ��0���ɹ���-1��ʧ��
    static int cacheWriteBegin(void* context, ///<��������Ϊbdb�������
        char* szErr2K ///<ʧ��ʱ�Ĵ�����Ϣ
        );

    ///dirty flush�߳�дdirty���ݣ�����ֵ��0���ɹ���-1��ʧ��
    static int cacheWrite(void* context, ///<��������Ϊbdb�������
        char const* szKey, ///<д���key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char const* szData, ///<д���data
        CWX_UINT32 uiDataLen, ///<data�ĳ���
        bool bDel, ///<�Ƿ�key��ɾ��
        CWX_UINT32 ttOldExpire, ///<key�ڴ洢�е�expireʱ��ֵ
        char* szStoreKeyBuf, ///<key��buf
        CWX_UINT16 unKeyBufLen, ///<key buf�Ĵ�С
        char* szErr2K ///<ʧ��ʱ�Ĵ�����Ϣ
        );
    
    ///dirty flush�߳����dirty���ݵ�д�롣����ֵ����ֵ��0���ɹ���-1��ʧ��
    static int cacheWriteEnd(void* context, ///<��������Ϊbdb�������
        CWX_UINT64 ullSid, ///<д�����ݵ�sidֵ
        void* userData, ///<�û�����������
        char* szErr2K ///<ʧ��ʱ�Ĵ�����Ϣ
        );
    
    
private:
    ///����couter=value;couter=value��data������ֵ��true���ɹ���false��ʧ��
    bool _parseDataCounter(UnistorBdbcData& bdbcData,///<���صļ�������Ϣ
        char const* szData, ///<�������ַ���
        CWX_UINT32 uiDataLen, ///<���ݳ���
        UnistorStoreBdbcCounterMap* pCounterMap, ///<�̵߳�counter map����
        bool bSync, ///<�Ƿ���ͬ��������
        char* szErr2K=NULL ///<�������򷵻ش���ԭ��
        );
    ///�γ� couter_id=value;couter_id=value��data������ֵ��true���ɹ���false��ʧ��
    bool _outputSyncCounter(UnistorBdbcData const& bdbcData,///<���صļ�������Ϣ
        char* szData, ///<�������ַ���
        CWX_UINT32& uiDataLen, ///<���ݳ���
        char* szErr2K=NULL ///<�������򷵻ش���ԭ��
        );

    ///merge��������true���ɹ���false��ʧ��
    bool _mergeAddCouter(UnistorBdbcData& oldBdbcData, ///<��ǰ��ֵ
        UnistorBdbcData const& newBdbcData, ///<��ֵ
        UnistorStoreBdbcCounterMap* pCounterMap, ///<�̵߳�counter map����
        char* szErr2K ///<ʧ��ʱ�Ĵ�����Ϣ
        );

    ///merge��������true���ɹ���false��ʧ��
    bool _mergeSetCouter(UnistorBdbcData& oldBdbcData, ///<��ǰ��ֵ
        UnistorBdbcData const& newBdbcData, ///<��ֵ
        char* szErr2K ///<ʧ��ʱ�Ĵ�����Ϣ
        );

    ///�γ�inc�ļ�������1���ɹ���-1��ʧ�ܣ�0��������Χ��
    int _mergeIncCouter(UnistorBdbcData& bdbcData, ///<��ǰ��ֵ
        CWX_INT32 iValue, ///<inc��ֵ������Ϊ����
        CWX_INT64 iMax, ///<���ֵ
        CWX_INT64 iMin, ///<��Сֵ
        CWX_UINT8 ucId, ///<��������id
        CWX_INT64& llValue, ///<�ۼ�ֵ
        char* szErr2K ///<ʧ��ʱ�Ĵ�����Ϣ
        );

    ///������������ݣ������µ�λ��
    char* _outputUiCounter(UnistorBdbcData& bdbcData, ///<����������
        char const* szField, ///<Ҫ����ļ�������NULL��ʾȫ�����
        char* szBuf, ///<�����buf
        CWX_UINT32& uiLen, ///<����buf�Ĵ�С������ʣ��buf�Ĵ�С
        UnistorStoreBdbcCounterMap* pCounterMap, ///<�̵߳�counter map����
        char* szErr2K ///<����ʱ�Ĵ�����Ϣ
        );

    ///��ȡkey�����ݿ�����
    inline CWX_UINT32 _getBdbIndex(UnistorBdbcKey const& key) const{
        CWX_UINT32 uiIndex = UnistorBdbcKey::m_storeKeyGroupFn(key.m_szKey, key.m_unKeyLen);
        uiIndex &= m_uiGroupMask;
        uiIndex >>=m_bdbcConf.m_ucGroupStartBit;
        return uiIndex;
    }

    ///commit��0���ɹ���-1��ʧ��
	int _commit(char* szErr2K);

    ///���ϵͳ��Ϣ��0���ɹ���-1��ʧ��
    int _checkSysInfo();

    ///��ȡ��������������Ϣ��0���ɹ���-1��ʧ��
    int _checkCounterDef(char* szErr2K);
	
    //����ϵͳ��Ϣ������ֵ��0:�ɹ���-1��ʧ��
	int _updateSysInfo(DB_TXN* tid, CWX_UINT64 ullSid, char* szErr2K);
	
    //����ϵͳ��Ϣ������ֵ��0:�ɹ���-1���ɹ�
	int _loadSysInfo(DB_TXN* tid, char* szErr2K);
	
    //����bdb��������Ϣ������ֵ0:�ɹ���-1��ʧ��
	int _parseConf();

    //��װ����Ĳ�������������ֵ0���ɹ���-1��ʧ��
    int _installFunc();

    //��ȡϵͳkey��1���ɹ���0�������ڣ�-1��ʧ��;
    int _getSysKey(UnistorTss* tss, ///<�߳�tss����
        char const* key, ///<Ҫ��ȡ��key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData, ///<�����ڣ��򷵻����ݡ��ڴ��д洢�������
        CWX_UINT32& uiLen  ///<szData���ݵ��ֽ���
        );

    //set key��0:�ɹ���-1��ʧ��
    int _setKey(UnistorBdbcKey const* key,///<������key
        char const* szData, ///<key��data
        CWX_UINT32 uiLen, ///<data�ĳ���
        bool& bWriteCache, ///<�Ƿ�key��write cache�д���
        bool bCache=true, ///<�Ƿ����cache�������������ɾ��cache
        char* szErr2K=NULL ///<����ʱ���ش�������
        );
    
    //��ȡkey��data������ֵ��1����ȡ��-1��ʧ��
    int _getKey(UnistorBdbcKey const* key,///<������key
        char* szData, ///<key��data
        CWX_UINT32& uiLen,///<����data��buf size������data�ĳ���
        char* szStoreKeyBuf, ///<key��buf
        CWX_UINT16 unKeyBufLen, ///<key��buf��С
        bool& isCached, ///<�����Ƿ���cache�С�
        bool bCache, ///<�Ƿ�ʹ��cache
        bool bAddEmpty, ///<�Ƿ����emtpy
        char* szErr2K ///<����ʱ���ش�������
        );
    
    //ɾ��key��ͬʱ��cache��ɾ��������ֵ��0:�ɹ���-1��ʧ��
    int _delKey(UnistorBdbcKey const* key, ///<������key
        bool& bWriteCache, ///<�Ƿ�key��write cache�д���
        char* szErr2K=NULL ///<����ʱ���ش�������
        );

	//����bdb��key��data������ֵ��0:�ɹ���-1��ʧ��
	int _setBdbKey(DB* db, ///<bdb��db handle
        DB_TXN* tid, ///<bdb��transaction
        char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        CWX_UINT16 unKeyBufLen, ///<key��buf�ռ��С
        char const* szData, ///<key��data
        CWX_UINT32 uiDataLen, ///<data�Ĵ�С
        CWX_UINT32 flags, ///<bdb��set flags
        char* szErr2K=NULL ///<����ʱ���ش�������
        );

    //��bdb�л�ȡkey������ֵ��0:�����ڣ�1����ȡ��-1��ʧ��
	int _getBdbKey(DB* db, ///<bdb��db handle
        DB_TXN* tid, ///<bdb��transaction
        char const* szKey, ///<get��key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        char* szData,  ///<key��data���ռ���߱�֤
        CWX_UINT32& uiLen, ///<����szData�Ŀռ��С������data�Ĵ�С
        char* szStoreKeyBuf, ///<�洢key�Ŀռ�
        CWX_UINT16 unKeyBufLen, ///<�ռ��С
        CWX_UINT32 flags, ///<bdb��get flags
        char* szErr2K=NULL  ///<����ʱ���ش�������
        );

    //��bdb��ɾ�����ݡ�����ֵ��0:�ɹ���-1��ʧ��
	int _delBdbKey(DB* db,  ///<bdb��db handle
        DB_TXN* tid, ///<bdb��transaction
        char const* szKey, ///<ɾ����key
        CWX_UINT16 unKeyLen, ///<key�ĳ���
        CWX_UINT16 unKeyBufLen, ///<szKey�Ŀռ��С
        CWX_UINT32 flags,  ///<bdbɾ�����
        char* szErr2K=NULL  ///<����ʱ���ش�������
        );

    //����modģʽ�Ķ��ġ�-1��ʧ�ܣ�0��������1����ȡһ����2��skip����Ϊ0
    int _exportNext(UnistorTss* tss,///<�߳�tss
        UnistorStoreCursor& cursor, ///<cursor����
        char const*& szKey, ///<��һ����key
        CWX_UINT16& unKeyLen, ///<key�ĳ���
        char const*& szData, ///<��һ��key��data
        CWX_UINT32& uiDataLen, ///<data�ĳ���
        bool& bKeyValue, ///<�Ƿ�dataΪkey/value
        CWX_UINT32& uiVersion, ///<key�İ汾��
        CWX_UINT32& uiExpire, ///<key�ĳ�ʱʱ��
        CWX_UINT16& unSkipNum, ///<��ǰʣ���skip��
        char const*& szExtra,   ///<extra�����ݣ�Ϊdb������
        CWX_UINT32& uiExtraLen  ///<extra���ݵĳ���
        );

    //export keyģʽ�Ķ��ġ�-1��ʧ�ܣ�0��������1����ȡһ����2��skip����Ϊ0
    int _exportKeyNext(UnistorTss* tss,///<�߳�tss
        UnistorStoreCursor& cursor, ///<cursor����
        char const*& szKey, ///<��һ����key
        CWX_UINT16& unKeyLen, ///<key�ĳ���
        char const*& szData, ///<��һ��key��data
        CWX_UINT32& uiDataLen, ///<data�ĳ���
        bool& bKeyValue, ///<�Ƿ�dataΪkey/value
        CWX_UINT32& uiVersion, ///<key�İ汾��
        CWX_UINT32& uiExpire, ///<key�ĳ�ʱʱ��
        CWX_UINT16& unSkipNum, ///<��ǰʣ���skip��
        char const*& szExtra,   ///<extra�����ݣ�Ϊdb������
        CWX_UINT32& uiExtraLen  ///<extra���ݵĳ���
        );

    //��ȡ��������һ��key�ķ�Χ��true���ɹ���false�����
    bool _exportKeyInit(string const& strKeyBegin, ///<��ǰexport���key
        string& strBegin, ///<��һ��key��Χ�Ŀ�ʼλ��
        string& strEnd, ///<��һ��key��Χ�Ľ���λ��
        UnistorSubscribeKey const& keys ///<key���Ĺ���
        );

    //����cursor������ֵ��0����ɣ�1���ɹ���-1��ʧ�ܡ�
    inline int _createCursor(CWX_UINT32 uiIndex, DBC*& cursor, char* szErr2K)
    {
        if (uiIndex >= m_uiGroupNum) return 0;
        int ret = m_bdbs[uiIndex]->cursor(m_bdbs[uiIndex],
            0,
            &cursor,
            DB_READ_UNCOMMITTED|DB_CURSOR_BULK);
        if (0 != ret){
            if (szErr2K) CwxCommon::snprintf(szErr2K, 2047, "Failure to create cursor. err-code:%d, err-msg=%s",
                ret,
                db_strerror(ret));
            return -1;
        }
        cursor->set_priority(cursor, DB_PRIORITY_VERY_LOW);
        return 1;
    }

private:
    UnistorBdbcData const         m_emptyData;       ///<�ռ�����
    char                          m_szEmptyData[1024]; ///<������
    CWX_UINT32                    m_uiEmptyDataLen; ///<�����ݵĳ���
	UnistorConfigBdbc			  m_bdbcConf; ///<bdb�������ļ�
	DB_ENV*					      m_bdbEnv;  ///<bdb��env
	DB**                          m_bdbs;    ///<bdb��handle������
    DB*                           m_sysDb;   ///<bdb��ϵͳ��
    DB_TXN*	                      m_bdbTxn;  ///<bdb������handle
    CWX_UINT32                    m_uiGroupMask; ///<���������
    CWX_UINT32                    m_uiGroupNum; ///<���������
    UnistorStoreBdbcCounterMap*   m_pCounterDef; ///<�������Ķ���
};

#endif
