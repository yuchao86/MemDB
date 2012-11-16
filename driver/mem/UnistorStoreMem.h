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

///配置文件
/****
[mem]
expire_check_step=进行超时检测的步长
****/

///引擎加载函数
extern "C" {
	UnistorStoreBase* unistor_create_engine();
}


///计数器的配置对象
class UnistorConfigMem{
public:
    enum{
        UNISTOR_CONFIG_MIN_CHECK_NUM = 1000, ///<每次超时检测的最小数量
        UNISTOR_CONFIG_MAX_CHECK_NUM = 1000000, ///<每次检测的最大数量
        UNISTOR_CONFIG_DEF_CHECK_NUM = 100000,  ///<缺省每次的检测数量
        UNISTOR_CONFIG_BATCH_REMOVE_NUM = 100  ///<每次锁删除的数量
    };
public:
    ///构造函数
	UnistorConfigMem(){
        m_uiExpireCheckNum = UNISTOR_CONFIG_DEF_CHECK_NUM;
	}
public:
    CWX_UINT32          m_uiExpireCheckNum; ///<expire的检测数量
};



//mem的cursor定义对象
class UnistorStoreMemCursor{
public:
    ///构造函数
    UnistorStoreMemCursor(){
        m_unExportBeginKeyLen = 0;
        m_szStoreKey[0] = 0x00;
        m_unStoreKeyLen = 0;
        m_szStoreData[0] = 0x00;
        m_uiStoreDataLen = 0;
        m_uiSlotIndex = 0;
        m_uiSlotPos = 0;
    }
    ///析构函数
    ~UnistorStoreMemCursor(){
    }
public:
    char                  m_szExportBeginKey[UNISTOR_MAX_KEY_SIZE]; ///<export的开始key
    CWX_UINT16            m_unExportBeginKeyLen; ///<key的长度
    char			      m_szStoreKey[UNISTOR_MAX_KEY_SIZE]; ///<存储的key
    CWX_UINT16            m_unStoreKeyLen; ///<key的长度
    char                  m_szStoreData[UNISTOR_MAX_KV_SIZE]; ///<存储的data
    CWX_UINT32            m_uiStoreDataLen; ///<data的长度
    CWX_UINT32            m_uiSlotIndex; ///<cache的slot的index
    CWX_UINT32            m_uiSlotPos; ///<cache的slot的pos
};


///mem的存储引擎
class UnistorStoreMem: public UnistorStoreBase{
public:
    ///构造函数
    UnistorStoreMem(){
        m_memCache = NULL;
        UnistorStoreBase::m_fnKeyStoreGroup = UnistorStoreMem::keyAsciiGroup;
        UnistorStoreBase::m_fnKeyAsciiGroup = UnistorStoreMem::keyAsciiGroup;
        UnistorStoreBase::m_fnKeyAsciiLess = UnistorStoreMem::keyAsciiCmpLess;
    }
    ///析构函数
    ~UnistorStoreMem(){
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

    ///失去同步通知，由引擎处理。返回值，0：没有变化；1：可以同步；-1：失败
    virtual int lostSync();

    ///获取engine的名字
    virtual char const* getName() const{
        return "mem";
    }

    ///获取engine的版本
    virtual char const* getVersion() const{
        return "1.0.0";
    }
    ///获取cache是否正常
    virtual bool isCacheValid() const{
        return true;
    }
    ///获取cache的错误信息
    virtual char const* getCacheErrMsg() const{
        return "";
    }
    ///获取写cache的key的数量
    virtual CWX_UINT32 getWriteCacheKeyNum() const{
        return 0;
    }
    ///获取write cache使用的尺寸
    virtual  CWX_UINT32 getWriteCacheUsedSize() const{
        return 0;
    }
    ///获取读cache的大小
    virtual unsigned long int getReadCacheMaxSize( void ) const{
        return m_memCache?m_memCache->maxSize():0;
    }
    ///获取cache key的最大数量
    virtual CWX_UINT32 getReadCacheMaxKeyNum() const{
        return m_memCache?m_memCache->getMaxCacheKeyNum():0;
    }
    ///获取read cache使用的内存
    virtual unsigned long int getReadCacheUsedSize() const{
        return m_memCache?m_memCache->getUsedSize():0;
    }
    ///获取read cache中数据的占用的容量
    virtual unsigned long int getReadCacheUsedCapacity() const{
        return m_memCache?m_memCache->getUsedCapacity():0;
    }
    ///获取read cache中cache的有效数据大小
    virtual unsigned long int getReadCacheUsedDataSize() const{
        return m_memCache?m_memCache->getUsedDataSize():0;
    }
    ///获取read cache中空闲大小
    virtual unsigned long int getReadCacheFreeSize() const{
        return m_memCache?m_memCache->getFreeSize():0;
    }
    ///获取read cache中空闲的容量
    virtual unsigned long int getReadCacheFreeCapacity() const{
        return m_memCache?m_memCache->getFreeCapacity():0;
    }
    ///获取cache的key的数量
    virtual CWX_UINT32 getReadCacheKeyCount() const{
        return m_memCache?m_memCache->getCachedKeyCount():0;
    }

private:

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
    ///commit。0：成功；-1：失败
    int _commit(char* szErr2K);

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
        CWX_UINT32* uiExpire, ///<expire时间值
        char* szErr2K=NULL ///<出错时返回错误描述
        );

    //获取key的data。返回值：0:不存在；1：获取；-1：失败
    int _getKey(char const* szKey, ///<key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<key的data
        CWX_UINT32& uiLen,///<传入data的buf size，返回data的长度
        CWX_UINT32& uiExpire, ///<key的超时时间
        char* szErr2K=NULL ///<出错时返回错误描述
        );

    //删除key，同时从cache中删除。返回值：0:成功；-1：失败
    int _delKey(char const* szKey, ///<key的名字
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szErr2K=NULL ///<出错时返回错误描述
        );

    //key的data为field结构；true：是；false：不是
    inline bool isKvData(char const* szData, CWX_UINT32 uiDataLen){
        if (!szData || (uiDataLen<5)) return false;
        return szData[uiDataLen-1] == 0?false:true;
    }

    //获取data的version
    inline void getKvVersion(char const* szData, ///<data数据
        CWX_UINT32 uiDataLen, ///<data的长度
        CWX_UINT32& uiVersion ///<key的版本号
        )
    {
        CWX_ASSERT(uiDataLen >= 5);
        memcpy(&uiVersion, szData + (uiDataLen-5), 4);
    }

    ///设置data的extra数据
    inline void setKvDataSign(char* szData, ///<key的data
        CWX_UINT32& uiDataLen, ///<传入当前data的长度，返回新长度
        CWX_UINT32 uiVersion, ///<key的版本号
        bool bKeyValue ///<data的key/value标记
        )
    {
        memcpy(szData + uiDataLen, &uiVersion, sizeof(uiVersion));
        uiDataLen += sizeof(uiVersion);
        szData[uiDataLen] = bKeyValue?1:0;
        uiDataLen++;
    }

    ///获取data的extra数据长度
    inline CWX_UINT32 getKvDataSignLen() const {
        return sizeof(CWX_UINT32) + 1;
    }
    //unpack的data的field；-1：失败；0：不是kv结构；1：成功
    inline int unpackFields(CwxPackageReaderEx& reader, ///<reader对象
        char const* szData, ///<fields的key/value数据
        CWX_UINT32 uiDataLen, ///<fields的key/value数据的长度
        CWX_UINT32& uiVersion ///<数据的版本号
        )
    {
        if (!isKvData(szData, uiDataLen)) return 0;
        getKvVersion(szData, uiDataLen, uiVersion);
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
        return 0;
    }
private:
    UnistorConfigMem			m_memConf; ///<配置信息
    UnistorStoreMemCache*       m_memCache; ///<内存cache
};

#endif
