#ifndef __UNISTOR_STORE_BDB_H__
#define __UNISTOR_STORE_BDB_H__


#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxFile.h"
#include "CwxMutexLock.h"
#include "CwxLockGuard.h"
#include <db.h>
#include "UnistorStoreBase.h"



//1、bdb的key的结构是：就是字符串key。
//2、所有的key都是字符串类型
//3、系统采用写cache，当写cache满或超过指定的量后，则将写cache flush到bdb。
//    flush 到bdb的时候，采用写事务模式。
//4、在启动的时候，binglog会根据系统key中的sid值，从binlog恢复bdb。
//5、对于非系统key，存储的数据为[data][expire][version][sign]的结构，expire为32位整数，version为32位的整数，sign=1表示data为kv结构，否则不是kv结构

/********************写必须单线程*************************/

///引擎加载函数
extern "C" {
	UnistorStoreBase* unistor_create_engine();
}

///配置文件的bdb参数对象
class UnistorConfigBdb{
public:
    ///构造函数
	UnistorConfigBdb(){
		m_bZip = false;
		m_uiCacheMByte = 512;
		m_uiPageKSize = 0;
	}
public:
	string				m_strEnvPath; ///<env的路径
	string				m_strDbPath; ///<bdb数据文件路径
	bool				m_bZip;      ///<是否压缩索引
	CWX_UINT32          m_uiCacheMByte; ///<bdb索引的cache mbyte。
	CWX_UINT32			m_uiPageKSize; ///<页的大小
};

///bdb的cursor定义对象
class UnistorStoreBdbCursor{
public:
    ///构造函数
	UnistorStoreBdbCursor(){
		m_cursor = NULL;
		m_bFirst = true;
        m_unExportBeginKeyLen = 0;
        m_szStoreKey[0] = 0x00;
        m_unStoreKeyLen = 0;
        m_szStoreData[0] = 0x00;
        m_uiStoreDataLen = 0;
        m_bStoreValue = false;
        m_bStoreMore = true;
	}
    ///析构函数
	~UnistorStoreBdbCursor(){
	}
public:
	bool				  m_bFirst; ///<是否是第一个获取
	DBC*				  m_cursor;  ///<bdb的cursor handle
    char                  m_szExportBeginKey[UNISTOR_MAX_KEY_SIZE]; ///<export的开始key
    CWX_UINT16            m_unExportBeginKeyLen; ///<key的长度
    char			      m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<存储的key
    CWX_UINT16            m_unStoreKeyLen; ///<key的长度
    char                  m_szStoreData[UNISTOR_MAX_KV_SIZE]; ///<存储的data
    CWX_UINT32            m_uiStoreDataLen; ///<data的长度
    bool                  m_bStoreValue; ///<是否存储store的值。
    bool                  m_bStoreMore; ///<store中是否还有值

};


///bdb的存储引擎
class UnistorStoreBdb : public UnistorStoreBase{
public:
    ///构造函数
    UnistorStoreBdb(){
		m_bdbEnv = NULL;
		m_bdb = NULL;
        m_sysDb = NULL;
        m_expireDb = NULL;
		m_bZip = false;
        m_bdbTxn = NULL;
        m_exKey = NULL;
        m_unExKeyNum = 0;
        m_unExKeyPos = 0;
        UnistorStoreBase::m_fnKeyStoreGroup = UnistorStoreBdb::keyAsciiGroup;
        UnistorStoreBase::m_fnKeyAsciiGroup = UnistorStoreBdb::keyAsciiGroup;
        UnistorStoreBase::m_fnKeyAsciiLess = UnistorStoreBdb::keyAsciiCmpLess;
    }
    ///析构函数
    ~UnistorStoreBdb(){
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
        CWX_UINT8 ucKeyInfo=0 ///<是否获取key的information。0：获取key的data。1：获取key信息；2：获取系统key
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
		return "bdb";
	}
	
    ///获取engine的版本
	virtual char const* getVersion() const{
		return "1.0.0";
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
    
    ///key的相等比较函数。返回值：true，相等；false：不相等
    static bool keyStoreCmpEqual(char const* key1, ///<第一个key
        CWX_UINT16 unKey1Len, ///<key的长度
        char const* key2, ///<第二个key
        CWX_UINT16 unKey2Len ///<第二个key的长度
        )
    {
        return (unKey1Len == unKey2Len) && (memcmp(key1, key2, unKey1Len)==0);
    }
    
    ///key的小于比较函数。返回值：0，key1==key2；1，key1>key2；-1：key1<key2
    static int keyStoreCmpLess(char const* key1, ///<第一个key
        CWX_UINT16 unKey1Len, ///<第一个key的长度
        char const* key2, ///<第二个key
        CWX_UINT16 unKey2Len ///<第二个key的长度
        )
    {
        int ret = memcmp(key1, key2, unKey1Len<unKey2Len?unKey1Len:unKey2Len);
        if (0 != ret) return ret;
        return unKey1Len==unKey2Len?0:(unKey1Len<unKey2Len?-1:1);
    }
    
    ///key的hash函数
    static size_t keyStoreHash(char const* key, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        )
    {
        size_t h = 216613626UL;
        for (CWX_UINT16 i = 0; i < unKeyLen; ++i) {
            h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
            h ^= key[i];
        }
        return h;
    }
    ///key的group函数
    static CWX_UINT32 keyStoreGroup(char const* key, ///<key的名字
        CWX_UINT16 unKeyLen ///<key的长度
        )
    {
        return keyAsciiGroup(key, unKeyLen);
    }
    
    ///key的ascii的group函数
    static CWX_UINT32 keyAsciiGroup(char const* key, ///<key的名字
        CWX_UINT16 unKeyLen ///<key的长度
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
    
    ///key的ascii类型的小于比较函数。返回值：0，key1==key2；1，key1>key2；-1：key1<key2
    static int keyAsciiCmpLess(char const* key1, ///<第一个key
        CWX_UINT16 unKey1Len, ///<第一个key的长度
        char const* key2, ///<第二个key
        CWX_UINT16 unKey2Len ///<第二个key的长度
        )
    {
        int ret = memcmp(key1, key2, unKey1Len<unKey2Len?unKey1Len:unKey2Len);
        if (0 != ret) return ret;
        return unKey1Len==unKey2Len?0:(unKey1Len<unKey2Len?-1:1);
    }
    
private:
    ///获取数据。-1：失败；0：结束；1：获取一个
    int _nextBdb(UnistorStoreCursor& cursor, char* szErr2K);

    ///commit。0：成功；-1：失败
	int _commit(char* szErr2K);
	
    //更新系统信息。返回值：0:成功；-1：失败
	int _updateSysInfo(DB_TXN* tid, CWX_UINT64 ullSid, char* szErr2K);
	
    //加载系统信息。返回值：0:成功；-1：成功
	int _loadSysInfo(DB_TXN* tid, char* szErr2K);
	
    //解析bdb的配置信息。返回值0:成功；-1：失败
	int parseConf();

    //获取系统key。1：成功；0：不存在；-1：失败;
    int _getSysKey(UnistorTss* tss, ///<线程tss对象
        char const* key, ///<要获取的key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<若存在，则返回数据。内存有存储引擎分配
        CWX_UINT32& uiLen  ///<szData数据的字节数
        );

    //set key。0:成功；-1：失败
    int _setKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen,///<key的长度
        char const* szData, ///<key的data
        CWX_UINT32 uiLen, ///<data的长度
        CWX_UINT32 ttOldExpire, ///<先前的expire时间值
        bool& bWriteCache, ///<是否key在write cache中存在
        bool bCache=true, ///<是否更新cache，若不更新则会删除cache
        char* szErr2K=NULL ///<出错时返回错误描述
        );
    
    //获取key的data。返回值：0:不存在；1：获取；-1：失败
    int _getKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<key的data
        CWX_UINT32& uiLen,///<传入data的buf size，返回data的长度
        char* szStoreKeyBuf, ///<key的buf
        CWX_UINT16 unKeyBufLen, ///<key的buf大小
        bool& isCached, ///<数据是否在cache中。
        bool bCache=true, ///<是否使用cache
        char* szErr2K=NULL ///<出错时返回错误描述
        );
    
    //删除key，同时从cache中删除。返回值：0:成功；-1：失败
    int _delKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        CWX_UINT32 ttOldExpire, ///<key的当前expire时间
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

    //处理commit事件。0：成功；-1：失败
    int _dealCommitEvent(UnistorTss* tss, ///<线程tss
        CwxMsgBlock*& msg ///<消息
        );

    //加载超时的数据。0：没有新数据；1：获取了数据；-1：失败
    int _loadExpireData(UnistorTss* tss, ///<线程tss
        bool bJustContinue ///<当遍历完所有key时，是否重新加载expire key
        );

    //发送超时数据。0：成功；-1：失败
    int _sendExpireData(UnistorTss* tss);

    //处理expire事件。0：成功；-1：失败
    int _dealExpireEvent(UnistorTss* tss, ///<线程tss
        CwxMsgBlock*& msg ///<回复的消息
        );

    //处理expire事件的回复。0：成功；-1：失败
    int _dealExpireReplyEvent(UnistorTss* tss,  ///<线程tss
        CwxMsgBlock*& msg ///<回复的消息
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
        CWX_UINT16& unSkipNum ///<当前剩余的skip量
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
        CWX_UINT16& unSkipNum ///<当前剩余的skip量
        );

    //获取导出的下一个key的范围。true：成功；false：完成
    bool _exportKeyInit(string const& strKeyBegin, ///<当前export点的key
        string& strBegin, ///<下一个key范围的开始位置
        string& strEnd, ///<下一个key范围的结束位置
        UnistorSubscribeKey const& keys ///<key订阅规则
        );
    //key的data为field结构；true：是；false：不是
    inline bool isKvData(char const* szData, CWX_UINT32 uiDataLen){
        if (!szData || (uiDataLen<9)) return false;
        return szData[uiDataLen-1] == 0?false:true;
    }

    //获取data的version
    inline void getKvVersion(char const* szData, ///<data数据
        CWX_UINT32 uiDataLen, ///<data的长度
        CWX_UINT32& uiExpire, ///<key的失效时间
        CWX_UINT32& uiVersion ///<key的版本号
        )
    {
        CWX_ASSERT(uiDataLen >= 9);
        memcpy(&uiVersion, szData + (uiDataLen-5), 4);
        memcpy(&uiExpire, szData + (uiDataLen - 9), 4);
    }

    ///设置data的extra数据
    inline void setKvDataSign(char* szData, ///<key的data
        CWX_UINT32& uiDataLen, ///<传入当前data的长度，返回新长度
        CWX_UINT32 uiExpire, ///<失效时间
        CWX_UINT32 uiVersion, ///<key的版本号
        bool bKeyValue ///<data的key/value标记
        )
    {
        memcpy(szData + uiDataLen, &uiExpire, sizeof(uiExpire));
        uiDataLen += sizeof(uiExpire);
        memcpy(szData + uiDataLen, &uiVersion, sizeof(uiVersion));
        uiDataLen += sizeof(uiVersion);
        szData[uiDataLen] = bKeyValue?1:0;
        uiDataLen++;
    }

    ///获取data的extra数据长度
    inline CWX_UINT32 getKvDataSignLen() const {
        return sizeof(CWX_UINT32) + sizeof(CWX_UINT32) + 1;
    }
    //unpack的data的field；-1：失败；0：不是kv结构；1：成功
    inline int unpackFields(CwxPackageReaderEx& reader, ///<reader对象
        char const* szData, ///<fields的key/value数据
        CWX_UINT32 uiDataLen, ///<fields的key/value数据的长度
        CWX_UINT32& uiExpire, ///<数据的失效时间
        CWX_UINT32& uiVersion ///<数据的版本号
        )
    {
        if (!isKvData(szData, uiDataLen)) return 0;
        getKvVersion(szData, uiDataLen, uiExpire, uiVersion);
        if (!reader.unpack(szData, uiDataLen-5, false, true)){
            return -1;
        }
        return 1;
    }

    ///获取超时时间
    inline CWX_UINT32 getNewExpire(CWX_UINT32 uiExpire ///<超时时间
        )
    {
        if (uiExpire > 3600 * 24 * 365){
            return uiExpire;
        }else if (uiExpire){
            return uiExpire + m_ttExpireClock;
        }
        return m_ttExpireClock + m_config->getCommon().m_uiDefExpire;
    }
private:
	UnistorConfigBdb			m_bdbConf; ///<bdb的配置文件
	DB_ENV*					    m_bdbEnv;  ///<bdb的env
	DB*					        m_bdb;     ///<bdb的handle
    DB*                         m_sysDb;   ///<bdb的系统库
    DB*                         m_expireDb; ///<bdb的超时索引库
	bool					    m_bZip;    ///<是否压缩
    DB_TXN*	                    m_bdbTxn;  ///<bdb的事务handle
    ///check expire的信息
    pair<CWX_UINT32, UnistorStoreExpireKey*>*      m_exKey; ///<超时的key cache
    CWX_UINT16                  m_unExKeyNum;     ///<取的key的数量
    CWX_UINT16                  m_unExKeyPos;     ///<下一个发送的key的位置
    list<CwxMsgBlock*>          m_exFreeMsg;      ///<空闲的message
    char			            m_exStoreKey[sizeof(UnistorStoreExpireKey) + UNISTOR_MAX_KEY_SIZE]; ///<存储的key
};

#endif
