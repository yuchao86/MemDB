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

///配置文件
/****
[bdbc]
env_home=/data3/bdb_home      #bdb的home目录
db_path=/data4/bdb_data       #bdb的数据目录
compress=no                   #是佛压缩
cache_msize=8192              #cache的大小
page_ksize = 32               #bdb page的大小
key_type=int32/int64/int128/int256/char #key的类型，对于数字类型，全部是无符号整数
int32_hex=yes/no              #是否将返回值默认10进制改为16进制
int64_hex=yes/no              #是否将返回值默认16进制改为10进制
hex_upper=yes/no              #对于返回的hex进制数字是否为大写，默认为小写
group_start_time_bit = 4      #基于32位group的数据文件分组的开始位，从0开始表示bit0。
group_end_time_bit =7         #基于32位group的数据文件分组的结束位，7表示[4,7]工4位。
counter_def_file =/data4/bdb_data/counter.def #counter的定义文件
****/

/***********************************bdbc引擎说明*******************************/
//1、key：可以为int32、int64、int128、int256、char。
//        对于不同的key类型，group对应值如下：
//        int32-->就是group值
//        int64-->低32位为group值
//        int128-->低32位为group值
//        int256-->低32位为group值
//        char-->md5签名的低32位为group值
//        对于不同的key类型，hash对应值如下
//        int32-->就是hash值
//        int64-->低32位为hash值
//        int128-->低32位为hash值
//        int256-->低32位为hash值
//        char-->md5签名的低32位为hash值
//2、key的传递
//        int32-->可以为10进制，也可以为16进制，16进制以0x开头。返回时默认是10进制，可以配置为16进制。
//        int64-->可以为10进制，也可以为16进制，16进制以0x开头。返回时默认是16进制，可以配置为10进制。
//        int128-->16进制的字符串，无需0x开头。返回时是16进制
//        int256-->16进制的字符串，无需0x开头。返回时是16进制

//2、对于数据，bdb内部存储的为[计数器1][计数器2]...[version].
//     计数器的格式为：[一个字符的计数器名字][32位的计数值]共5个字节
//     version：为32位的整数。
//3、对于计数器为0的计数器，系统会自动删除。
//4、若获取key的一个指定的计数器，则返回计数器的数值，为10进制表示。
//5、若获取key的多个计数器，则返回的格式为：[计数器1名字=数值];[计数器2名字=数值].....
//     一个计数器内，名字与数值通过【=】分割，数字为10进制的字符串
//     多个计数器间通过【;】分割。
//7、在一个实例内部，计数器会采用2^[group_end_time_bit - group_start_time_bit + 1]个bdb文件存储。
//   每个文件的名字为bdbc.开始bit-分组数-0.dat, bdbc.开始bit-分组数-1.dat.....的文件存储。
//   每个文件存放的是对应的[2^group_start_time_bit]秒内的数据。group_start_time_bit是用来外部分组的。
//   分组一旦启用不能修改
//8、系统存在一个系统数据库.bdbc.sys，其保护如下的key：
//   sid:当前已经commit的sid的值
//   group_start_time_bit:开始的时间位
//   group_end_time_bit:结束时间位
/********************写必须单线程*************************/
#define  BDBC_MIN_READ_CACHE_MSIZE   128            ///<最小读cache
#define  BDBC_MAX_READ_CACHE_MSIZE   (64 * 1024)   ///<最大读cache

#define  BDBC_MIN_WRITE_CACHE_MSIZE  8             ///<最小写cache
#define  BDBC_MAX_WRITE_CACHE_MSIZE  128           ///<最大写cache

#define  BDBC_MIN_READ_CACHE_KEY_NUM   100000     ///<读cache的最多key的数量
#define  BDBC_MAX_READ_CACHE_KEY_NUM   1600000000  ///<读cache的最多key的数量


#define  BDBC_KEY_GROUP_START_TIME_BIT  "bdbc_group_start_time_bit"
#define  BDBC_KEY_GROUP_END_TIME_BIT    "bdbc_group_end_time_bit"
#define  BDBC_KEY_COUNTERS              "bdbc_counters"
#define  BDBC_KEY_KEY_TYPE              "bdbc_key_type"

#define  BDBC_MAX_COUNTER_VALUE         0xFFFFFFFF 

#define  BDBC_KEY_COUNTER_SPLIT_CHR      ';'     ///<计数器的分割字符

#define  BDBC_DATA_FILE_FORMAT           "bdbc.%u-%u-%u.dat"  ///<数据文件的format字符串，三个%分别是分组的开始位、分组数量、所属分组
#define  BDBC_SYS_FILE_NAME              ".bdbc.sys"   ///<系统bdb文件名
#define  BDBC_KEY_VERSION                "version"  ///<版本号
#define  BDBC_SYS_COUNTER_VERSION        "counter_version"  ///<版本号

#define BDBC_ENGINE_KEY_TYPE_INT32       "int32"    ///<int32类型的key
#define BDBC_ENGINE_KEY_TYPE_INT64       "int64"    ///<int64类型的key
#define BDBC_ENGINE_KEY_TYPE_INT128      "int128"   ///<int128类型的key
#define BDBC_ENGINE_KEY_TYPE_INT256      "int256"   ///<int256类型的key
#define BDBC_ENGINE_KEY_TYPE_CHAR        "char"     ///<char类型的key

#define  BDBC_HEX_64_ZERO                "000000000000000000000000000000000000000000000000000000000000000000"
///引擎加载函数
extern "C" {
	UnistorStoreBase* unistor_create_engine();
}

///key的group函数。
typedef int (*BDBC_KEY_CMP_LESS_FN)(DB *, const DBT *a, const DBT *b);

///计数器的key定义
struct UnistorBdbcKey{
public:
    enum{
        BDBC_KEY_TYPE_UNKNOWN = 0, ///<未知key类型
        BDBC_KEY_TYPE_INT32 = 1,  ///<32位数字的key类型
        BDBC_KEY_TYPE_INT64 = 2,  ///<64位数字的key类型
        BDBC_KEY_TYPE_INT128 = 3, ///<128位数字的key类型
        BDBC_KEY_TYPE_INT256 = 5, ///<256位数字的key类型
        BDBC_KEY_TYPE_CHAR   = 6  ///<字符串的key类型
    };
public:
    ///构造函数
    UnistorBdbcKey():m_szKey(NULL), m_unKeyLen(0){
    }
    ///构造函数
    UnistorBdbcKey(char const* szKey, CWX_UINT16 unKeyLen)
        :m_szKey(szKey), m_unKeyLen(unKeyLen)
    {
    }
public:    
    ///清空数据
    void reset(){
        m_szKey = NULL;
        m_unKeyLen = 0;
    }
    ///是否key需要编码;返回值：true：需要编码；false：不需要编码
    inline static bool isNeedEncode(){ return BDBC_KEY_TYPE_CHAR != m_ucKeyType;}
    ///是否key需要解码;返回值：true：需要编码；false：不需要编码
    inline static bool isNeedDecode(){ return BDBC_KEY_TYPE_CHAR != m_ucKeyType;}
    ///从ascii形成一个key。返回值：true：成功；false：失败
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
    ///从store key形成key的ascii。返回值：true：成功；false：失败
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
    char const*            m_szKey; ///<key的内存指针，由外边赋值
    CWX_UINT32             m_uiGroup; ///<key的group
    CWX_UINT16             m_unKeyLen; ///<key的长度
    static  CWX_UINT8      m_ucKeyType; ///<key的类型
    static  CWX_UINT8      m_ucBase;   ///<对于INT32、INT64返回值的进制。
    static  bool           m_bUpperHex; ///<对于返回的16进制是否大写
    static  UNISTOR_KEY_CMP_EQUAL_FN   m_storeKeyEqualFn; ///<存储类型key的相等比较函数
    static  UNISTOR_KEY_CMP_LESS_FN    m_storeKeyLessFn; ///<存储类型key的less比较函数
    static  UNISTOR_KEY_HASH_FN        m_storeKeyHashFn; ///<存储类型key的hash函数
    static  UNISTOR_KEY_GROUP_FN       m_storeKeyGroupFn; ///<存储类型key的group函数
    static  UNISTOR_KEY_CMP_EQUAL_FN   m_asciiKeyEqualFn; ///<ascii类型key的相等比较函数
    static  UNISTOR_KEY_CMP_LESS_FN    m_asciiKeyLessFn;  ///<ascii类型key的less函数
    static  UNISTOR_KEY_GROUP_FN       m_asciiKeyGroupFn; ///<ascii类型key的group函数
    static  BDBC_KEY_CMP_LESS_FN       m_bdbKeyLessFn; ///<bdb的比较函数
};


///一个计数器对象定义
struct UnistorBdbcCounter{
public:
    ///构造函数
    UnistorBdbcCounter(){
        m_uiValue = 0;
        m_ucId = 0;
    }
public:
    CWX_UINT32  m_uiValue; ///<计数值
    CWX_UINT8   m_ucId; ///<计数器的ID
};

///计数器的data对象，在存入bdb时会进行编码，获取时会解码
struct UnistorBdbcData{
public:
    ///构造函数
    UnistorBdbcData(){
        memset(this, 0x00, sizeof(UnistorBdbcData));
    }
    ///构造函数
    UnistorBdbcData(CWX_UINT32 uiVersion){
        memset(this, 0x00, sizeof(UnistorBdbcData));
        m_uiVersion = uiVersion;
    }
public:
    void reset(){
        memset(this, 0x00, sizeof(UnistorBdbcData));
    }
    ///将数据空间解析为计数器
    bool decode(unsigned char const* szData, ///<计数器的encode数据
        CWX_UINT32 uiDataLen, ///<数据的长度
        char* szErr2k=NULL ///<若不为空，则在出错时返回出错信息
        );

    ///将计数器编码为数据空间，返回pack的有效计数器的数量。若失败为-1。
    int encode(unsigned char* szData, ///<encode的输出buf
        CWX_UINT32& uiDataLen, ///<输入buf的长度，返回encode内容的长度
        char* szErr2k=NULL ///<错误信息
        ) const;

    ///是否存在计数器。0：不存在，否则返回索引值
    inline int isExist(CWX_UINT8 ucId) const{
        if (!ucId) return 0; 
        return m_counterMap[ucId];
    }
    ///是否为空
    inline bool isEmpty() const{
        return 0==m_ucCounterNum;
    }
public:
    UnistorBdbcCounter          m_counter[256]; ///<计数器
    CWX_UINT8                   m_ucCounterNum; ///<计数器的数量
    CWX_UINT32                  m_uiVersion; ///<版本
    CWX_UINT8                   m_counterMap[256]; ///<记录解析出的计数器的位置信息，其值为m_counter的【下标+1】。若为0表示不存在
};

///计数器的配置对象
class UnistorConfigBdbc{
public:
    ///构造函数
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
	string				m_strEnvPath; ///<env的路径
	string              m_strDbPath; ///<bdb数据文件路径
	bool				m_bZip;      ///<是否压缩索引
	CWX_UINT32          m_uiCacheMByte; ///<bdb索引的cache mbyte。
	CWX_UINT32			m_uiPageKSize; ///<页的大小
    CWX_UINT8           m_ucKeyType; ///<Key的类型
    string              m_strKeyType; ///<Key类型的名字
    bool                m_bInt32Hex; ///<返回的int32的key是否16进制
    bool                m_bInt64Hex; ///<返回的int64的key是否16进制
    bool                m_bHexUpper; ///<返回的16进制是否大写
    CWX_UINT8           m_ucGroupStartBit; ///<基于时间分组的开始位
    CWX_UINT8           m_ucGroupEndBit;  ///<基于时间分组的结束为
    string              m_strCounterDefFile; ///<counter的定义文件
};

///计数器引擎的cursor定义对象
class UnistorStoreBdcCursor{
public:
    ///构造函数
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
    ///析构函数
	~UnistorStoreBdcCursor(){
	}
public:
	bool				  m_bFirst; ///<是否是第一个获取
    CWX_UINT32            m_uiDbIndex; ///<cursor对应的db的索引
	DBC*				  m_cursor;  ///<bdb的cursor handle
    UnistorBdbcData       m_bdbcData;  ///<计数的数据
    char                  m_szStoreExportBeginKey[UNISTOR_MAX_KEY_SIZE]; ///<export的开始key
    CWX_UINT16            m_unStoreExportBeginKeyLen; ///<key的长度
    char			      m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<存储的key
    CWX_UINT16            m_unStoreKeyLen; ///<key的长度
    char                  m_szStoreData[UNISTOR_MAX_KV_SIZE]; ///<存储的data
    CWX_UINT32            m_uiStoreDataLen; ///<data的长度
    bool                  m_bStoreValue; ///<是否存储store的值。
    bool                  m_bStoreMore; ///<store中是否还有值
};


///计数器序号及计数名字的映射表
class UnistorStoreBdbcCounterMap: public UnistorTssEngineObj
{
public:
    ///构造函数
    UnistorStoreBdbcCounterMap(){
        CWX_UINT16 i=0;
        m_ullSeq = 0;
        for (i=0; i<256; i++){
            m_idNameMap[i] = NULL;
        }
    }
    ///析构函数
    virtual ~UnistorStoreBdbcCounterMap(){
        CWX_UINT16 i=0;
        m_nameIdMap.clear();
        for (i=0; i<256; i++){
            if (m_idNameMap[i]) delete [] m_idNameMap[i];
        }
    }
public:
    ///解析配置信息，操作的时候需要加锁。返回值：true，成功；false：失败。
    ///文件格式为：
    ///version=版本号
    ///序号1=计数器名字
    ///序号2=计数器名字
    ///序号3=计数器名字
    ///...... 序号为0~255，各行以\n分割
    bool parse(char const* szDef, CWX_UINT32 uiLen, CWX_UINT32 ttTimestamp, char* szErr2K);
    ///clone出一个新定义共各个相关线程使用，此需要加锁。
    void clone(UnistorStoreBdbcCounterMap& def);

    /////以下的操作都有各自的线程调用，无需加锁
    ///判别计数器定义是否改变。返回值：true，改变；false：没有改变
    inline bool isDiff(CWX_UINT64 ullSeq) const{
        return m_ullSeq != ullSeq;
    }
    ///获取配置文件的时间戳
    inline CWX_UINT32 getTimestamp() const { return m_ttTimestamp;}
    ///<获取序列号
    inline CWX_UINT64 getSeq() const{ return m_ullSeq;}
    ///<获取定义文件
    inline string const& getDef() const{ return m_strDef;}
    ///<获取版本
    inline string const& getVersion() const{ return m_strVersion;}
    ///<根据计数器id，获取计数器的名字，NULL表示不存在
    inline char const* getName(CWX_UINT8 ucId) const{
        return m_idNameMap[ucId];
    }
    ///<根据计数器的名字获取计数器的id，false表示不存在
    inline bool getId(char const* szName, CWX_UINT8& ucId) const{
        map<char const*, CWX_UINT8, CwxCharLess>::const_iterator iter=m_nameIdMap.find(szName);
        if (iter == m_nameIdMap.end()) return false;
        ucId = iter->second;
        return true;
    }
    ///获取计数器的数量
    inline CWX_UINT8 getCount() const{ return m_nameIdMap.size();}
public:
    ///为外界提供的变量
    UnistorBdbcData               m_bdbcOldData; ///<旧的微博计数对象
    UnistorBdbcData               m_bdbcInputData; ///<新的微博计数对象
    UnistorBdbcKey                m_bdbcKey;     ///<微博计数的key
    friend class UnistorStoreBdbc; ///<友类
private:
    volatile CWX_UINT32      m_ttTimestamp; ///<配置文件的时间戳
    volatile CWX_UINT64      m_ullSeq;     ///<内部变更序号
    string                   m_strDef;     ///<映射的定义内容
    string                   m_strVersion; ///<版本号
    char*                    m_idNameMap[256]; ///<计数器序号与计数器名字的映射表
    map<char const*, CWX_UINT8, CwxCharLess>   m_nameIdMap; ///<计数器名字与计数器的映射map
    CwxMutexLock             m_lock;
};


///基于bdb的计数存储引擎
class UnistorStoreBdbc : public UnistorStoreBase{
public:
    ///构造函数
    UnistorStoreBdbc():m_emptyData(0){
		m_bdbEnv = NULL;
		m_bdbs = NULL;
        m_sysDb = NULL;
        m_bdbTxn = NULL;
        m_uiGroupMask = 0;
        m_uiGroupNum = 0;
        m_pCounterDef = NULL;
        ///check expire的函数
        UnistorStoreBase::m_fnKeyStoreGroup = NULL;
        UnistorStoreBase::m_fnKeyAsciiGroup = NULL;
        UnistorStoreBase::m_fnKeyAsciiLess = NULL;
    }
    ///析构函数
    ~UnistorStoreBdbc(){
    }
public:
	//加载配置文件.-1:failure, 0:success
    virtual int init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc, ///<存储引擎与上层的消息通道函数
        UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<获取系统信息的function
        void* pApp, ///<UnistorApp对象
        UnistorConfig const* config ///<配置文件
        );

    ///检测是否存在key；1：存在；0：不存在；-1：失败
    virtual int isExist(UnistorTss* tss, ///tss对象
        CwxKeyValueItemEx const& key, ///<检查的key
        CwxKeyValueItemEx const* field, ///<检查的field，若为空表示检查key
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra data
        CWX_UINT32& uiVersion, ///<返回key的版本号
        CWX_UINT32& uiFieldNum, ///<返回key的field的数量
        bool& bReadCached ///<数据是否在read cache中
        );
    
    ///添加key，1：成功；0：存在；-1：失败；
    virtual int addKey(UnistorTss* tss, ///<tss对象
        CwxKeyValueItemEx const& key, ///<添加的key
        CwxKeyValueItemEx const* field, ///<添加的field，若指定，则根据sign值决定是否添加field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<添加key或field的数据
        CWX_UINT32    uiSign, ///<添加的标志
        CWX_UINT32& uiVersion, ///<若大于0，则设置修改后的key为此版本，否则返回新版本
        CWX_UINT32& uiFieldNum, ///返回<key field的数量
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool bCache=true, ///<是否将key放到读cache
        CWX_UINT32 uiExpire=0 ///<若不为0，则指定key的expire时间。此需要存储引擎支持
        );


    ///set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
    virtual int setKey(UnistorTss* tss,///tss
        CwxKeyValueItemEx const& key, ///<set的key
        CwxKeyValueItemEx const* field, ///<若是set field，则指定要set的field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra 数据
        CwxKeyValueItemEx const& data, ///<set的数据
        CWX_UINT32 uiSign, ///<设置的标记
        CWX_UINT32& uiVersion, ///<设置的version。若大于0，则设置为指定的版本，否则返回指定的版本
        CWX_UINT32& uiFieldNum, ///<key字段的数量
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool bCache=true, ///<是否对数据进行cache
        CWX_UINT32 uiExpire=0 ///<若指定，则修改key的expire时间。此需要存储引擎支持
        );    

    ///update key，1：成功；0：不存在；-1：失败；-2：版本错误
    virtual int updateKey(UnistorTss* tss, ///<tss对象
        CwxKeyValueItemEx const& key, ///<update的key
        CwxKeyValueItemEx const* field,///<若update field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra 数据
        CwxKeyValueItemEx const& data, ///<update的数据
        CWX_UINT32 uiSign, ///<update的标记
        CWX_UINT32& uiVersion, ///<若指定，则key的版本必须与此值一致，否则更新失败
        CWX_UINT32& uiFieldNum, ///<返回key field的数量
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        CWX_UINT32 uiExpire=0 ///<若指定，则修改key的expire时间
        );

    
    ///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；-3：超出边界
    virtual int incKey(UnistorTss* tss, ///<线程tss对象
        CwxKeyValueItemEx const& key,  ///<inc的key
        CwxKeyValueItemEx const* field, ///<若要inc一个field计数器，则指定对应的field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CWX_INT64 num, ///<增加或减少的数量
        CWX_INT64  llMax, ///<若是增加而且此值不为0，则inc后的值不能超过此值
        CWX_INT64  llMin, ///<若是减少而起此值不为0，则dec后的值不能超过此值
        CWX_UINT32  uiSign, ///<inc的标记
        CWX_INT64& llValue, ///<inc或dec后的新值
        CWX_UINT32& uiVersion, ///<若指定，则key的版本号必须等于此值，否则失败。返回新版本号。
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        CWX_UINT32  uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        );


    ///delete key，1：成功；0：不存在；-1：失败；-2:版本错误；
    virtual int delKey(UnistorTss* tss, ///<线程tss对象
        CwxKeyValueItemEx const& key, ///<要删除的key
        CwxKeyValueItemEx const* field, ///<若要删除field，则指定field的名字
        CwxKeyValueItemEx const* extra,///<存储引擎的extra 数据
        CWX_UINT32& uiVersion, ///<若指定版本号，则修改前的版本号必须与此值相等，否则失败。返回新版本号
        CWX_UINT32& uiFieldNum,  ///<key的字段数量
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached ///<数据是否在write cache中
        );


    ///sync 添加key，1：成功；0：存在；-1：失败；
    virtual int syncAddKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<字段的名字
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<add的数据
        CWX_UINT32 uiSign, ///<add的sign
        CWX_UINT32 uiVersion, ///<变更后的版本号
        bool bCache, ///<是否cache数据
        CWX_UINT32 uiExpire, ///<key的expire时间
        CWX_UINT64 ullSid, ///<变更日志的sid值
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否是从binlog恢复的数据
        );

    ///sync set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
    virtual int syncSetKey(UnistorTss* tss, ///<线程的tss数据
        CwxKeyValueItemEx const& key, ///<set的key
        CwxKeyValueItemEx const* field, ///<若是set field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<set的数据
        CWX_UINT32 uiSign,  ///<set的sign
        CWX_UINT32 uiVersion, ///<set的key 版本号
        bool bCache, ///<是否cache数据
        CWX_UINT32 uiExpire, ///<expire时间
        CWX_UINT64 ullSid, ///<set binlog的sid值
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否是从binlog恢复的数据
        );

    ///sync update key，1：成功；0：不存在；-1：失败
    virtual int syncUpdateKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<update的key
        CwxKeyValueItemEx const* field, ///<若是update field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<update的新数据
        CWX_UINT32 uiSign, ///<update的标记
        CWX_UINT32 uiVersion, ///<update后的key的版本号
        CWX_UINT32 uiExpire, ///<update的expire时间
        CWX_UINT64 ullSid, ///<update变更binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog中恢复的数据
        );

    ///sync inc key，1：成功；0：不存在；-1：失败；
    virtual int syncIncKey(UnistorTss* tss, ///<线程的tss数据
        CwxKeyValueItemEx const& key,  ///<inc的key
        CwxKeyValueItemEx const* field, ///<若是对field进行inc，则指定field的名字
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CWX_INT64 num,  ///<inc的数值，可以为负值
        CWX_INT64 result,  ///<inc的数值，可以为负值
        CWX_INT64  llMax, ///<若是inc正值，而且指定llMax，则inc后的值不能超过此值
        CWX_INT64  llMin, ///<计数器的最小值
        CWX_UINT32 uiSign, ///<inc的标记
        CWX_INT64& llValue, ///<inc后的数值
        CWX_UINT32 uiVersion, ///<inc后的key的版本号
        CWX_UINT32 uiExpire, ///<update的expire时间
        CWX_UINT64 ullSid, ///<inc操作binlog的sid值
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
        );

    ///sync delete key，1：成功；0：不存在；-1：失败
    virtual int syncDelKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<要删除的key
        CwxKeyValueItemEx const* field, ///<若是删除field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CWX_UINT32 uiVersion, ///<key进行delete后的版本号
        CWX_UINT64 ullSid, ///<delete操作对应的binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
        );

    ///import key，1：成功；-1：失败；
    virtual int importKey(UnistorTss* tss, ///<tss对象
        CwxKeyValueItemEx const& key, ///<添加的key
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<添加key或field的数据
        CWX_UINT32& uiVersion, ///<若大于0，则设置修改后的key为此版本
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool bCache=true, ///<是否将key放到读cache
        CWX_UINT32 uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        );

    ///sync import key，1：成功；-1：错误。
    virtual int syncImportKey(UnistorTss* tss, ///<线程的tss数据
        CwxKeyValueItemEx const& key, ///<set的key
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<set的数据
        CWX_UINT32 uiVersion, ///<set的key 版本号
        bool bCache,    ///<是否将key放到读cache
        CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
        CWX_UINT64 ullSid, ///<操作对应的binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
        );


    ///获取key, 1：成功；0：不存在；-1：失败;
    virtual int get(UnistorTss* tss, ///<线程tss对象
        CwxKeyValueItemEx const& key, ///<要获取的key
        CwxKeyValueItemEx const* field, ///<若不为空，则获取指定的field，多个field以\n分割
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        char const*& szData, ///<若存在，则返回数据。内存有存储引擎分配
        CWX_UINT32& uiLen,  ///<szData数据的字节数
        bool& bKeyValue,  ///<返回的数据是否为key/value结构
        CWX_UINT32& uiVersion, ///<可以当前的版本号
        CWX_UINT32& uiFieldNum, ///<key字段的数量
        bool& bReadCached, ///<数据是否在read cache中
        CWX_UINT8 ucKeyInfo=0 ///<是否获取key的information
        );

    ///获取多个key, 1：成功；-1：失败;
    virtual int gets(UnistorTss* tss, ///<线程的tss对象
        list<pair<char const*, CWX_UINT16> > const& keys,  ///<要获取的key的列表。pair的first为key的名字，second为key的长度
        CwxKeyValueItemEx const* field, ///<若指定，则限定获取的field范围
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        char const*& szData, ///<获取的数据，内存由存储引擎分配
        CWX_UINT32& uiLen, ///<返回数据的长度
        CWX_UINT32& uiReadCacheNum, ///<在read cache中的数量
        CWX_UINT32& uiExistNum, ///<存在的key的数量
        CWX_UINT8 ucKeyInfo=0 ///<是否获取key的information。0：获取key的data。1：获取key信息；2：获取系统key
        );

	///建立游标。-1：内部错误失败；0：不支持；1：成功
    virtual int createCursor(UnistorStoreCursor& cursor, ///<游标对象
        char const* szBeginKey, ///<开始的key，若为NULL表示没有指定
        char const* szEndKey, ///<结束的key，若为NULL表示没有指定
        CwxKeyValueItemEx const* field, ///<指定游标要返回的field。
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        char* szErr2K ///<若出错，返回错误信息
        );

	///获取数据。-1：失败；0：结束；1：获取一个
    virtual int next(UnistorTss* tss, ///<线程的tss
        UnistorStoreCursor& cursor,  ///<Next的游标
        char const*& szKey,  ///<返回的key，内存由存储引擎分配
        CWX_UINT16& unKeyLen,  ///<返回key的字节数
        char const *& szData,  ///<返回key的data，内存由存储引擎分配
        CWX_UINT32& uiDataLen, ///<返回data的字节数
        bool& bKeyValue,  ///<data是否为keyValue结构
        CWX_UINT32& uiVersion,  ///<key的版本号
        bool bKeyInfo=false ///<是否返回key的information而不是其data
        );
    	
    ///关闭游标
	virtual void closeCursor(UnistorStoreCursor& cursor);
    
    ///开始导出数据。-1：内部错误失败；0：成功
    virtual int exportBegin(UnistorStoreCursor& cursor, ///<export的游标
        char const* szStartKey, ///<export的开始key，不包含此key
        char const* szExtra, ///<extra信息
        UnistorSubscribe const& scribe,  ///<导出数据的订阅规则
        CWX_UINT64& ullSid, ///<当前的sid值
        char* szErr2K  ///<若出错则返回错误信息
        );
    
    ///获取数据。-1：失败；0：结束；1：获取一个；2：skip数量为0
    virtual int exportNext(UnistorTss* tss,  ///<线程的tss对象
        UnistorStoreCursor& cursor,  ///<export的游标
        char const*& szKey,    ///<返回key的值
        CWX_UINT16& unKeyLen,   ///<key的字节数
        char const*& szData,    ///<返回data的值
        CWX_UINT32& uiDataLen,   ///<data的字节数
        bool& bKeyValue,   ///<data是否为KeyValue结构
        CWX_UINT32& uiVersion, ///<key的版本号
        CWX_UINT32& uiExpire, ///<key的expire时间
        CWX_UINT16& unSkipNum,  ///<当前最多可以skip的binlog数量
        char const*& szExtra,  ///<extra数据
        CWX_UINT32&  uiExtraLen ///<extra的长度
        );

    ///结束导出数据
    virtual void exportEnd(UnistorStoreCursor& cursor);

    ///检查订阅格式是否合法
    virtual bool isValidSubscribe(UnistorSubscribe const& subscribe,///<订阅对象
        char* szErr2K ///<不合法时的错误消息
        );

	///commit。0：成功；-1：失败
	virtual int commit(char* szErr2K);
	
    ///关闭bdb引擎
	virtual int close();

    ///event处理函数，实现存储引擎与上层的交互；0：成功；-1：失败
    virtual int storeEvent(UnistorTss* tss, CwxMsgBlock*& msg);
	
    ///bdb进行checkpoint
	virtual void checkpoint(UnistorTss* tss);
	
    ///获取engine的名字
	virtual char const* getName() const{
		return "bdbc";
	}
	
    ///获取engine的版本
	virtual char const* getVersion() const{
		return "1.0.1";
	}

private:
    ///dirty flush线程通知开始flush dirty数据。返回值：0，成功；-1：失败
    static int cacheWriteBegin(void* context, ///<环境，此为bdb引擎对象
        char* szErr2K ///<失败时的错误信息
        );

    ///dirty flush线程写dirty数据，返回值：0，成功；-1：失败
    static int cacheWrite(void* context, ///<环境，此为bdb引擎对象
        char const* szKey, ///<写入的key
        CWX_UINT16 unKeyLen, ///<key的长度
        char const* szData, ///<写入的data
        CWX_UINT32 uiDataLen, ///<data的长度
        bool bDel, ///<是否key被删除
        CWX_UINT32 ttOldExpire, ///<key在存储中的expire时间值
        char* szStoreKeyBuf, ///<key的buf
        CWX_UINT16 unKeyBufLen, ///<key buf的大小
        char* szErr2K ///<失败时的错误消息
        );
    
    ///dirty flush线程完成dirty数据的写入。返回值返回值：0，成功；-1：失败
    static int cacheWriteEnd(void* context, ///<环境，此为bdb引擎对象
        CWX_UINT64 ullSid, ///<写入数据的sid值
        void* userData, ///<用户的其他数据
        char* szErr2K ///<失败时的错误消息
        );
    
    
private:
    ///解析couter=value;couter=value的data。返回值：true，成功；false，失败
    bool _parseDataCounter(UnistorBdbcData& bdbcData,///<返回的计数机信息
        char const* szData, ///<计数的字符串
        CWX_UINT32 uiDataLen, ///<数据长度
        UnistorStoreBdbcCounterMap* pCounterMap, ///<线程的counter map对象
        bool bSync, ///<是否是同步的数据
        char* szErr2K=NULL ///<若出错，则返回错误原因
        );
    ///形成 couter_id=value;couter_id=value的data。返回值：true，成功；false，失败
    bool _outputSyncCounter(UnistorBdbcData const& bdbcData,///<返回的计数机信息
        char* szData, ///<计数的字符串
        CWX_UINT32& uiDataLen, ///<数据长度
        char* szErr2K=NULL ///<若出错，则返回错误原因
        );

    ///merge计数器，true：成功；false：失败
    bool _mergeAddCouter(UnistorBdbcData& oldBdbcData, ///<当前的值
        UnistorBdbcData const& newBdbcData, ///<新值
        UnistorStoreBdbcCounterMap* pCounterMap, ///<线程的counter map对象
        char* szErr2K ///<失败时的错误消息
        );

    ///merge计数器，true：成功；false：失败
    bool _mergeSetCouter(UnistorBdbcData& oldBdbcData, ///<当前的值
        UnistorBdbcData const& newBdbcData, ///<新值
        char* szErr2K ///<失败时的错误消息
        );

    ///形成inc的计数器，1：成功；-1：失败；0：超过范围。
    int _mergeIncCouter(UnistorBdbcData& bdbcData, ///<当前的值
        CWX_INT32 iValue, ///<inc的值，可以为负数
        CWX_INT64 iMax, ///<最大值
        CWX_INT64 iMin, ///<最小值
        CWX_UINT8 ucId, ///<计数器的id
        CWX_INT64& llValue, ///<累计值
        char* szErr2K ///<失败时的错误消息
        );

    ///输出计数的内容，返回新的位置
    char* _outputUiCounter(UnistorBdbcData& bdbcData, ///<计数器对象
        char const* szField, ///<要输出的计数器，NULL表示全部输出
        char* szBuf, ///<输出的buf
        CWX_UINT32& uiLen, ///<传入buf的大小，返回剩余buf的大小
        UnistorStoreBdbcCounterMap* pCounterMap, ///<线程的counter map对象
        char* szErr2K ///<出错时的错误信息
        );

    ///获取key的数据库索引
    inline CWX_UINT32 _getBdbIndex(UnistorBdbcKey const& key) const{
        CWX_UINT32 uiIndex = UnistorBdbcKey::m_storeKeyGroupFn(key.m_szKey, key.m_unKeyLen);
        uiIndex &= m_uiGroupMask;
        uiIndex >>=m_bdbcConf.m_ucGroupStartBit;
        return uiIndex;
    }

    ///commit。0：成功；-1：失败
	int _commit(char* szErr2K);

    ///检查系统信息。0：成功；-1：失败
    int _checkSysInfo();

    ///获取计数器的配置信息。0：成功；-1：失败
    int _checkCounterDef(char* szErr2K);
	
    //更新系统信息。返回值：0:成功；-1：失败
	int _updateSysInfo(DB_TXN* tid, CWX_UINT64 ullSid, char* szErr2K);
	
    //加载系统信息。返回值：0:成功；-1：成功
	int _loadSysInfo(DB_TXN* tid, char* szErr2K);
	
    //解析bdb的配置信息。返回值0:成功；-1：失败
	int _parseConf();

    //安装引擎的操作方法。返回值0：成功；-1：失败
    int _installFunc();

    //获取系统key。1：成功；0：不存在；-1：失败;
    int _getSysKey(UnistorTss* tss, ///<线程tss对象
        char const* key, ///<要获取的key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<若存在，则返回数据。内存有存储引擎分配
        CWX_UINT32& uiLen  ///<szData数据的字节数
        );

    //set key。0:成功；-1：失败
    int _setKey(UnistorBdbcKey const* key,///<计数的key
        char const* szData, ///<key的data
        CWX_UINT32 uiLen, ///<data的长度
        bool& bWriteCache, ///<是否key在write cache中存在
        bool bCache=true, ///<是否更新cache，若不更新则会删除cache
        char* szErr2K=NULL ///<出错时返回错误描述
        );
    
    //获取key的data。返回值：1：获取；-1：失败
    int _getKey(UnistorBdbcKey const* key,///<计数的key
        char* szData, ///<key的data
        CWX_UINT32& uiLen,///<传入data的buf size，返回data的长度
        char* szStoreKeyBuf, ///<key的buf
        CWX_UINT16 unKeyBufLen, ///<key的buf大小
        bool& isCached, ///<数据是否在cache中。
        bool bCache, ///<是否使用cache
        bool bAddEmpty, ///<是否添加emtpy
        char* szErr2K ///<出错时返回错误描述
        );
    
    //删除key，同时从cache中删除。返回值：0:成功；-1：失败
    int _delKey(UnistorBdbcKey const* key, ///<计数的key
        bool& bWriteCache, ///<是否key在write cache中存在
        char* szErr2K=NULL ///<出错时返回错误描述
        );

	//设置bdb的key的data。返回值：0:成功；-1：失败
	int _setBdbKey(DB* db, ///<bdb的db handle
        DB_TXN* tid, ///<bdb的transaction
        char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key的长度
        CWX_UINT16 unKeyBufLen, ///<key的buf空间大小
        char const* szData, ///<key的data
        CWX_UINT32 uiDataLen, ///<data的大小
        CWX_UINT32 flags, ///<bdb的set flags
        char* szErr2K=NULL ///<出错时返回错误描述
        );

    //从bdb中获取key。返回值：0:不存在；1：获取；-1：失败
	int _getBdbKey(DB* db, ///<bdb的db handle
        DB_TXN* tid, ///<bdb的transaction
        char const* szKey, ///<get的key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData,  ///<key的data，空间外边保证
        CWX_UINT32& uiLen, ///<传入szData的空间大小，传出data的大小
        char* szStoreKeyBuf, ///<存储key的空间
        CWX_UINT16 unKeyBufLen, ///<空间大小
        CWX_UINT32 flags, ///<bdb的get flags
        char* szErr2K=NULL  ///<出错时返回错误描述
        );

    //从bdb中删除数据。返回值：0:成功；-1：失败
	int _delBdbKey(DB* db,  ///<bdb的db handle
        DB_TXN* tid, ///<bdb的transaction
        char const* szKey, ///<删除的key
        CWX_UINT16 unKeyLen, ///<key的长度
        CWX_UINT16 unKeyBufLen, ///<szKey的空间大小
        CWX_UINT32 flags,  ///<bdb删除标记
        char* szErr2K=NULL  ///<出错时返回错误描述
        );

    //导出mod模式的订阅。-1：失败；0：结束；1：获取一个；2：skip数量为0
    int _exportNext(UnistorTss* tss,///<线程tss
        UnistorStoreCursor& cursor, ///<cursor对象
        char const*& szKey, ///<下一个的key
        CWX_UINT16& unKeyLen, ///<key的长度
        char const*& szData, ///<下一个key的data
        CWX_UINT32& uiDataLen, ///<data的长度
        bool& bKeyValue, ///<是否data为key/value
        CWX_UINT32& uiVersion, ///<key的版本号
        CWX_UINT32& uiExpire, ///<key的超时时间
        CWX_UINT16& unSkipNum, ///<当前剩余的skip量
        char const*& szExtra,   ///<extra的数据，为db的索引
        CWX_UINT32& uiExtraLen  ///<extra数据的长度
        );

    //export key模式的订阅。-1：失败；0：结束；1：获取一个；2：skip数量为0
    int _exportKeyNext(UnistorTss* tss,///<线程tss
        UnistorStoreCursor& cursor, ///<cursor对象
        char const*& szKey, ///<下一个的key
        CWX_UINT16& unKeyLen, ///<key的长度
        char const*& szData, ///<下一个key的data
        CWX_UINT32& uiDataLen, ///<data的长度
        bool& bKeyValue, ///<是否data为key/value
        CWX_UINT32& uiVersion, ///<key的版本号
        CWX_UINT32& uiExpire, ///<key的超时时间
        CWX_UINT16& unSkipNum, ///<当前剩余的skip量
        char const*& szExtra,   ///<extra的数据，为db的索引
        CWX_UINT32& uiExtraLen  ///<extra数据的长度
        );

    //获取导出的下一个key的范围。true：成功；false：完成
    bool _exportKeyInit(string const& strKeyBegin, ///<当前export点的key
        string& strBegin, ///<下一个key范围的开始位置
        string& strEnd, ///<下一个key范围的结束位置
        UnistorSubscribeKey const& keys ///<key订阅规则
        );

    //创建cursor。返回值：0，完成；1，成功；-1，失败。
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
    UnistorBdbcData const         m_emptyData;       ///<空计数器
    char                          m_szEmptyData[1024]; ///<空数据
    CWX_UINT32                    m_uiEmptyDataLen; ///<空数据的长度
	UnistorConfigBdbc			  m_bdbcConf; ///<bdb的配置文件
	DB_ENV*					      m_bdbEnv;  ///<bdb的env
	DB**                          m_bdbs;    ///<bdb的handle的数组
    DB*                           m_sysDb;   ///<bdb的系统库
    DB_TXN*	                      m_bdbTxn;  ///<bdb的事务handle
    CWX_UINT32                    m_uiGroupMask; ///<分组的掩码
    CWX_UINT32                    m_uiGroupNum; ///<分组的数量
    UnistorStoreBdbcCounterMap*   m_pCounterDef; ///<计数器的定义
};

#endif
