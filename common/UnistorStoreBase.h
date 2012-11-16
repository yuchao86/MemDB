#ifndef __UNISTOR_STORE_BASE_H__
#define __UNISTOR_STORE_BASE_H__

#include "UnistorMacro.h"
#include "UnistorConfig.h"
#include "UnistorTss.h"
#include "CwxBinLogMgr.h"
#include "CwxMd5.h"
#include "UnistorPoco.h"
#include "UnistorCache.h"
#include "UnistorSubscribe.h"

class UnistorStoreBase;
///驱动的消息通道函数
typedef int (*UNISTOR_MSG_CHANNEL_FN)(void* app, ///<app对象
                                   CwxMsgBlock* msg, ///<传递的消息
                                   bool bWriteThread, ///<是否传递给write线程，否则给checkpoint线程
                                   char* szErr2K ///<传递失败时的错误信息
                                   ); 

///key的field名字对象
class UnistorKeyField{
public:
    ///构造函数
	UnistorKeyField(){
		m_next = NULL;
	}
public:
	UnistorKeyField*   m_next; ///<下一个field
	string		       m_key;  ///<field的名字
};

///expire key定义对象
struct UnistorStoreExpireKey{
public:
    CWX_UINT32       m_ttExpire; ///<key的expire时间
    unsigned char    m_key[0]; ///<key的内容
}__attribute__ ((__packed__));


///写内存的cursor
class UnistorStoreCacheCursor{
public:
    UnistorStoreCacheCursor(){
        m_szCacheKey[0] = 0x00;
        m_unCacheKeyLen = 0;
        m_szCacheData[0] = 0x00;
        m_uiCacheDataLen = 0;
        m_bCacheValue = false;
        m_bCacheMore = true;
        m_bCacheFirst = true;
        m_bCacheDel = false;
    }
public:
    char			  m_szCacheKey[UNISTOR_MAX_KEY_SIZE]; ///<cache当前的key
    CWX_UINT16        m_unCacheKeyLen; ///<key的长度
    char              m_szCacheData[UNISTOR_MAX_KV_SIZE]; ///<存储的data
    CWX_UINT32        m_uiCacheDataLen; ///<data的长度
    bool              m_bCacheValue; ///<是否存储store的值。
    bool              m_bCacheMore; ///<store中是否还有值
    bool              m_bCacheFirst; ///<是否是cache的第一个个
    bool              m_bCacheDel; ///<cache是否已经删除
};

///cursor的定义
class UnistorStoreCursor{
public:
    UnistorStoreCursor():m_asc(true), m_bBegin(false){
        m_unBeginKeyLen = 0;
        m_unEndKeyLen = 0;
        m_cursorHandle = NULL;
        m_cacheCursor = NULL;
        m_field = NULL;
    }

    UnistorStoreCursor(bool bAsc, bool bBegin):m_asc(bAsc), m_bBegin(bBegin)
    {
        m_unBeginKeyLen = 0;
        m_unEndKeyLen = 0;
        m_cursorHandle = NULL;
        m_field = NULL;
        m_cacheCursor = NULL;
    }
    ~UnistorStoreCursor();
public:
    char              m_beginKey[UNISTOR_MAX_KEY_SIZE]; ///<开始位置,此为存储类型的key
    CWX_UINT16        m_unBeginKeyLen; ///<开始key的长度
    char              m_endKey[UNISTOR_MAX_KEY_SIZE]; ///<结束位置，此为存储类型的key
    CWX_UINT16        m_unEndKeyLen; ///<结束key的长度
    bool const        m_asc; ///<是否升序
    bool const        m_bBegin; ///<是否包含开始值
    void*             m_cursorHandle; ///<store的handle
    UnistorStoreCacheCursor* m_cacheCursor; ///<cache的遍历cursor
    UnistorKeyField*  m_field; ///<field信息 
    UnistorSubscribe  m_scribe; ///订阅规则
};

///存储引擎接口类
class UnistorStoreBase{
public:
    ///构造函数
    UnistorStoreBase(){
        m_pMsgPipeFunc = NULL;
        m_pGetSysInfoFunc = NULL;
        m_pApp = NULL;
		m_binLogMgr = NULL; 
		m_config = NULL; 
		m_bValid = false; 
        m_bEnableExpire = false; 
        m_ttExpireClock = 0; 
        m_uiDefExpire = 0;
		strcpy(m_szErrMsg, "No init.");
		m_szDataBuf = new char[UNISTOR_DEF_KV_SIZE];
		m_uiDataBufLen = UNISTOR_DEF_KV_SIZE;
        m_cache = NULL;
        m_ullStoreSid = 0;
        m_uiUncommitBinlogNum = 0;
        m_uiLastCommitSecond = 0;
        m_fnKeyStoreGroup = NULL;
        m_fnKeyAsciiGroup = NULL;
        m_fnKeyAsciiLess = NULL;

    }
    ///析构函数
    virtual ~UnistorStoreBase(){
		if (m_szDataBuf) delete [] m_szDataBuf;
		m_szDataBuf = NULL;
        if (m_cache) delete m_cache;
        if (m_binLogMgr){
            delete m_binLogMgr;
            m_binLogMgr = NULL;
        }
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
        )=0;
	
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
        CWX_UINT32 uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        )=0;
	
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
        CWX_UINT32 uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        )=0;
    
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
        CWX_UINT32 uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        )=0;
	
    ///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；-3：超出边界
    virtual int incKey(UnistorTss* tss, ///<线程tss对象
        CwxKeyValueItemEx const& key,  ///<inc的key
        CwxKeyValueItemEx const* field, ///<若要inc一个field计数器，则指定对应的field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CWX_INT64  num, ///<增加或减少的数量
        CWX_INT64  llMax, ///<若是增加而且此值不为0，则inc后的值不能超过此值
        CWX_INT64  llMin, ///<若是减少而起此值不为0，则dec后的值不能超过此值
        CWX_UINT32  uiSign, ///<inc的标记
        CWX_INT64& llValue, ///<inc或dec后的新值
        CWX_UINT32& uiVersion, ///<若指定，则key的版本号必须等于此值，否则失败。返回新版本号。
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        CWX_UINT32  uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        )=0;
	
    ///del key，1：成功；0：不存在；-1：失败；-2:版本错误；
    virtual int delKey(UnistorTss* tss, ///<线程tss对象
        CwxKeyValueItemEx const& key, ///<要删除的key
        CwxKeyValueItemEx const* field, ///<若要删除field，则指定field的名字
        CwxKeyValueItemEx const* extra,///<存储引擎的extra 数据
        CWX_UINT32& uiVersion, ///<若指定版本号，则修改前的版本号必须与此值相等，否则失败。返回新版本号
        CWX_UINT32& uiFieldNum,  ///<key的字段数量
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached ///<数据是否在write cache中
        )=0;

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
        )=0;

    ///sync 添加key，1：成功；0：存在；-1：失败；
    virtual int syncAddKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<字段的名字
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<add的数据
        CWX_UINT32 uiSign, ///<add的sign
        CWX_UINT32 uiVersion, ///<变更后的版本号
        bool bCache, ///<是否cache数据
        CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
        CWX_UINT64 ullSid, ///<变更日志的sid值
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否是从binlog恢复的数据
        )=0;
    
    ///sync set key，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
    virtual int syncSetKey(UnistorTss* tss, ///<线程的tss数据
        CwxKeyValueItemEx const& key, ///<set的key
        CwxKeyValueItemEx const* field, ///<若是set field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<set的数据
        CWX_UINT32 uiSign,  ///<set的sign
        CWX_UINT32 uiVersion, ///<set的key 版本号
        bool bCache, ///<是否cache数据
        CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
        CWX_UINT64 ullSid, ///<set binlog的sid值
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否是从binlog恢复的数据
        )=0;
    
    ///sync update key，1：成功；0：不存在；-1：失败
    virtual int syncUpdateKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<update的key
        CwxKeyValueItemEx const* field, ///<若是update field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<update的新数据
        CWX_UINT32 uiSign, ///<update的标记
        CWX_UINT32 uiVersion, ///<update后的key的版本号
        CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
        CWX_UINT64 ullSid, ///<update变更binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog中恢复的数据
        )=0;
    
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
        CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
        CWX_UINT64 ullSid, ///<inc操作binlog的sid值
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
        )=0;
    
    ///sync del key，1：成功；0：不存在；-1：失败；
    virtual int syncDelKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<要删除的key
        CwxKeyValueItemEx const* field, ///<若是删除field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CWX_UINT32 uiVersion, ///<key进行delete后的版本号
        CWX_UINT64 ullSid, ///<操作对应的binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
        )=0;

    ///sync import key，1：成功；-1：错误。
    virtual int syncImportKey(UnistorTss* tss, ///<线程的tss数据
        CwxKeyValueItemEx const& key, ///<set的key
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<set的数据
        CWX_UINT32 uiVersion, ///<set的key 版本号
        bool bCache,    ///<是否将key放到读cache
        CWX_UINT32 uiExpire, ///<若创建key，而且指定了uiExpire则设置key的超时时间
        CWX_UINT64 ullSid, ///<delete操作对应的binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
        )=0;


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
        )=0;
    
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
        )=0;

    ///建立游标。-1：内部错误失败；0：成功
    virtual int createCursor(UnistorStoreCursor& cursor, ///<游标对象
        char const* szBeginKey, ///<开始的key，若为NULL表示没有指定
        char const* szEndKey, ///<结束的key，若为NULL表示没有指定
        CwxKeyValueItemEx const* field, ///<指定游标要返回的field。
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        char* szErr2K ///<若出错，返回错误信息
        ) = 0;

	///获取下一个key数据。-1：失败；0：结束；1：获取一个
    virtual int next(UnistorTss* tss, ///<线程的tss
        UnistorStoreCursor& cursor,  ///<Next的游标
        char const*& szKey,  ///<返回的key，内存由存储引擎分配
        CWX_UINT16& unKeyLen,  ///<返回key的字节数
        char const *& szData,  ///<返回key的data，内存由存储引擎分配
        CWX_UINT32& uiDataLen, ///<返回data的字节数
        bool& bKeyValue,  ///<data是否为keyValue结构
        CWX_UINT32& uiVersion,  ///<key的版本号
        bool bKeyInfo=false ///<是否返回key的information而不是其data
        )=0;
	
    ///关闭游标
	virtual void closeCursor(UnistorStoreCursor& cursor) = 0;
    
    ///开始导出数据。-1：内部错误失败；0：成功
    virtual int exportBegin(UnistorStoreCursor& cursor, ///<export的游标
        char const* szStartKey, ///<export的开始key，不包含此key
        char const* szExtra, ///<extra信息
        UnistorSubscribe const& scribe,  ///<导出数据的订阅规则
        CWX_UINT64& ullSid, ///<当前的sid值
        char* szErr2K  ///<若出错则返回错误信息
        ) = 0;

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
        )=0;
    
    ///结束导出数据
    virtual void exportEnd(UnistorStoreCursor& cursor)=0;
	
    ///commit数据；0：成功；-1：失败
	virtual int commit(char* szErr2K)=0;
	
    ///同步master的binlog.0：成功；-1：失败
    virtual int syncMasterBinlog(UnistorTss* tss, ///<线程tss
        CwxPackageReaderEx* reader,  ///<对data进行解包的外部CwxPackageReaderEx对象
        CWX_UINT64 ullSid, ///<binlog的sid值，若为0，是从新编号sid
        CWX_UINT32 ttTimestamp, ///<binlog的时间戳
        CWX_UINT32 uiGroup, ///<binlog所属的分组
        CWX_UINT32 uiType,  ///<binlog的消息类型
        CwxKeyValueItemEx const& data, ///<binlog的数据
        CWX_UINT32 uiVersion, ///<binlog对应的key的版本号
        bool  bRestore=false  ///<是否是从本地binlog恢复的数据
        );
    
    ///event处理函数，实现存储引擎与上层的交互；0：成功；-1：失败
    virtual int storeEvent(UnistorTss* tss, ///<线程的tss
        CwxMsgBlock*& msg  ///<消息
        )=0;
	
    ///存储引擎的checkpoint
	virtual void checkpoint(UnistorTss* tss)=0;
    ///失去同步通知，由引擎处理。返回值，0：没有变化；1：可以同步；-1：失败
    virtual int lostSync(){ return 0;}
	
    ///获取engine的名字
	virtual char const* getName() const = 0;
	
    ///获取engine的版本
	virtual char const* getVersion() const = 0;
	
    ///关闭存储引擎
	virtual int close();
    
    ///是否订阅指定的数据
    virtual bool isSubscribe(UnistorSubscribe const& subscribe, ///<订阅对象
        CWX_UINT32 uiGroup, ///<binlog所属的分组
        char const* szKey ///<binlog的key
        )
    {
        return subscribe.isSubscribe(uiGroup, szKey);
    }
    
    ///检查订阅格式是否合法
    virtual bool isValidSubscribe(UnistorSubscribe const& subscribe,///<订阅对象
        char* szErr2K ///<不合法时的错误消息
        )=0;
    ///获取cache是否正常
    virtual bool isCacheValid() const{
        return m_cache?m_cache->isValid():true;
    }
    ///获取cache的错误信息
    virtual char const* getCacheErrMsg() const{
        return m_cache?m_cache->getErrMsg():"";
    }
    ///获取写cache的key的数量
    virtual CWX_UINT32 getWriteCacheKeyNum() const{
        return m_cache?m_cache->getWriteCacheKeyNum():0;
    }
    ///获取write cache使用的尺寸
    virtual  CWX_UINT32 getWriteCacheUsedSize() const{
        return m_cache?m_cache->getWriteCacheUsedSize():0;
    }
    ///获取读cache的大小
    virtual unsigned long int getReadCacheMaxSize( void ) const{
        return m_cache?m_cache->maxSize():0;
    }
    ///获取cache key的最大数量
    virtual CWX_UINT32 getReadCacheMaxKeyNum() const{
        return m_cache?m_cache->getMaxCacheKeyNum():0;
    }
    ///获取read cache使用的内存
    virtual unsigned long int getReadCacheUsedSize() const{
        return m_cache?m_cache->getUsedSize():0;
    }
    ///获取read cache中数据的占用的容量
    virtual unsigned long int getReadCacheUsedCapacity() const{
        return m_cache?m_cache->getUsedCapacity():0;
    }
    ///获取read cache中cache的有效数据大小
    virtual unsigned long int getReadCacheUsedDataSize() const{
        return m_cache?m_cache->getUsedDataSize():0;
    }
    ///获取read cache中空闲大小
    virtual unsigned long int getReadCacheFreeSize() const{
        return m_cache?m_cache->getFreeSize():0;
    }
    ///获取read cache中空闲的容量
    virtual unsigned long int getReadCacheFreeCapacity() const{
        return m_cache?m_cache->getFreeCapacity():0;
    }
    ///获取cache的key的数量
    virtual CWX_UINT32 getReadCacheKeyCount() const{
        return m_cache?m_cache->getCachedKeyCount():0;
    }

public:
    
    ///获取cache对象
    inline UnistorCache*  getCache() {
        return  m_cache;
    }
    
    ///启动cache
    int startCache(CWX_UINT32 uiWriteCacheMBtye, ///<write cache的大小，若为0表示没有write cache
        CWX_UINT32 uiReadCacheMByte, ///<读cache的大小，若为0表示没有read cache
        CWX_UINT32 uiReadMaxCacheKeyNum, ///<读cache的最大cache条目
        UNISTOR_WRITE_CACHE_WRITE_BEGIN_FN fnBeginWrite, ///<写的开始函数
        UNISTOR_WRITE_CACHE_WRITE_WRITE_FN   fnWrite, ///<写函数
        UNISTOR_WRITE_CACHE_WRITE_END_FN     fnEndWrite, ///<写结束函数
        void*     context, ///<函数的context
        UNISTOR_KEY_CMP_EQUAL_FN  fnEqual, ///<key的相等比较函数
        UNISTOR_KEY_CMP_LESS_FN   fnLess,  ///<key的less比较函数
        UNISTOR_KEY_HASH_FN       fnHash,   ///<key的hash值获取函数
        float     fBucketRate=1.2, ///<read cache's bucket rate
        char*     szErr2K=NULL
        );

    ///获取下一个cache数据。-1：失败；0：结束；1：获取一个
    int nextCache(UnistorStoreCursor& cursor, ///<cursor对象
        char* szErr2K ///<失败时的错误消息，一般是内存不足
        );

	///获取指定大小的buf，返回NULL表示失败
	inline char* getBuf(CWX_UINT32 uiDataLen){
		if (uiDataLen > m_uiDataBufLen){
			delete [] m_szDataBuf;
			m_szDataBuf = new char[uiDataLen];
			if (m_szDataBuf)
				m_uiDataBufLen = uiDataLen;
			else
				m_uiDataBufLen = 0;
		}
		return m_szDataBuf;
	}
	
    ///从binlog中恢复数据。0：成功；-1：失败
	int restore(CWX_UINT64 ullSid/*开始恢复的sid值*/);

    ///get binlog
	inline CwxBinLogMgr* getBinLogMgr() {
        return m_binLogMgr;
    }

    ///flush binlog；0：成功；-1：失败
	inline int flushBinlog(char* szErr2K=NULL){
		int ret = 0;
		CWX_INFO(("Begin flush binlog....................."));
		if (m_binLogMgr){
			ret = m_binLogMgr->commit(false, szErr2K);
		}
		CWX_INFO(("End flush binlog....................."));
		return ret;
	}

    ///添加Add key binlog.0：成功；-1：失败
    int appendTimeStampBinlog(CwxPackageWriterEx& writer, ///<writer对象
        CWX_UINT32      ttNow, ///<当前的时间
        char* szErr2K=NULL ///<若出错则返回错误信息
        );

    ///添加Add key binlog.0：成功；-1：失败
    int appendAddBinlog(CwxPackageWriterEx& writer, ///<write对象
        CwxPackageWriterEx& writer1, ///<write对象
        CWX_UINT32 uiGroup, ///<所属的分组
        CwxKeyValueItemEx const& key, ///<binlog的key
        CwxKeyValueItemEx const* field, ///<binlog的field
        CwxKeyValueItemEx const* extra, ///<binlog的extra数据
        CwxKeyValueItemEx const& data, ///<data数据
        CWX_UINT32    uiExpire, ///<失效时间
        CWX_UINT32    uiSign, ///<add key的标记
        CWX_UINT32    uiVersion, ///<key add后的版本号
        bool          bCache, ///<是否cache数据
        char*		  szErr2K=NULL ///<出错时的错误消息
        );

    ///添加set key binlog.0：成功；-1：失败
    int appendSetBinlog(CwxPackageWriterEx& writer, ///<write对象
        CwxPackageWriterEx& writer1, ///<write对象
        CWX_UINT32 uiGroup, ///<所属的分组
        CwxKeyValueItemEx const& key, ///<binlog的key
        CwxKeyValueItemEx const* field, ///<binlog的field
        CwxKeyValueItemEx const* extra, ///<binlog的extra数据
        CwxKeyValueItemEx const& data, ///<data数据
        CWX_UINT32    uiExpire, ///<失效时间
        CWX_UINT32    uiSign, ///<set key的标记
        CWX_UINT32    uiVersion, ///<key set后的版本号
        bool          bCache, ///<是否cache数据
        char*		  szErr2K=NULL ///<出错时的错误消息
        );

    ///添加update key binlog.0：成功；-1：失败
    int appendUpdateBinlog(CwxPackageWriterEx& writer, ///<write对象
        CwxPackageWriterEx& writer1, ///<write对象
        CWX_UINT32 uiGroup, ///<所属的分组
        CwxKeyValueItemEx const& key, ///<binlog的key
        CwxKeyValueItemEx const* field, ///<binlog的field
        CwxKeyValueItemEx const* extra, ///<binlog的extra数据
        CwxKeyValueItemEx const& data, ///<data数据
        CWX_UINT32    uiExpire, ///<失效时间
        CWX_UINT32    uiSign, ///<update key的标记
        CWX_UINT32    uiVersion, ///<key update后的版本号
        char*		  szErr2K=NULL ///<出错时的错误消息
        );

    ///添加inc key binlog.0：成功；-1：失败
    int appendIncBinlog(CwxPackageWriterEx& writer, ///<write对象
        CwxPackageWriterEx& writer1, ///<write对象
        CWX_UINT32 uiGroup, ///<所属的分组
        CwxKeyValueItemEx const& key, ///<binlog的key
        CwxKeyValueItemEx const* field, ///<binlog的field
        CwxKeyValueItemEx const* extra, ///<binlog的extra数据
        CWX_INT64       num, ///<inc的数值
        CWX_INT64       result, ///<结果的值
        CWX_INT64        llMax, ///<计数器的最大值
        CWX_INT64        llMin, ///<计数器的最小值
        CWX_UINT32        uiExpire, ///<失效时间
        CWX_UINT32       uiSign, ///<Inc的标记
        CWX_UINT32       uiVersion, ///<key inc后的版本号
        char*			 szErr2K  ///<出错时的错误消息
        );

    ///添加del key binlog.0：成功；-1：失败
    int appendDelBinlog(CwxPackageWriterEx& writer, ///<write对象
        CwxPackageWriterEx& writer1, ///<write对象
        CWX_UINT32 uiGroup, ///<所属的分组
        CwxKeyValueItemEx const& key, ///<binlog的key
        CwxKeyValueItemEx const* field, ///<binlog的field
        CwxKeyValueItemEx const* extra, ///<binlog的extra数据
        CWX_UINT32       uiVersion, ///<key删除后的版本号
        char*			 szErr2K ///<出错时的错误消息
        );

    ///添加import key binlog.0：成功；-1：失败
    int appendImportBinlog(CwxPackageWriterEx& writer, ///<write对象
        CwxPackageWriterEx& writer1, ///<write对象
        CWX_UINT32 uiGroup, ///<所属的分组
        CwxKeyValueItemEx const& key, ///<binlog的key
        CwxKeyValueItemEx const* extra, ///<binlog的extra数据
        CwxKeyValueItemEx const& data, ///<data数据
        CWX_UINT32    uiExpire, ///<失效时间
        CWX_UINT32    uiVersion, ///<key set后的版本号
        bool          bCache, ///<是否cache数据
        char*		  szErr2K=NULL ///<出错时的错误消息
        );


	///append binlog；0：成功；-1：失败
	inline int appendBinlog(CWX_UINT64& ullSid, ///<binlog的sid
		CWX_UINT32 ttTimestamp, ///<binlog的时间戳
		CWX_UINT32 uiGroup, ///<binlog所属的分组
		char const* szData, ///<binlog的data
		CWX_UINT32 uiDataLen, ///<binlog data的字节数
		char* szErr2K=NULL  ///<出错时的错误消息
        )
    {
		int ret = m_binLogMgr->append(ullSid,
            ttTimestamp,
            uiGroup,
            szData,
            uiDataLen,
            szErr2K);
		if (0 != ret) return -1;
		return 0;
	}

    //获取系统key。1：成功；0：不存在；-1：失败;
    int getSysKey(char const* key, ///<要获取的key
        CWX_UINT16 unKeyLen, ///<key的长度
        char* szData, ///<若存在，则返回数据。内存有存储引擎分配
        CWX_UINT32& uiLen  ///<szData数据的字节数
        );


    ///获取当前的sid值
	inline CWX_UINT64 getCurSid() const{
		return m_binLogMgr->getCurNextSid();
	}

    ///设置当前的sid值
    inline void setCurSid(CWX_UINT64 ullSid){
        m_binLogMgr->setNextSid(ullSid);
    }

    ///获取当前expire时间点
    inline CWX_UINT32 getExpireClock() const{
        return m_ttExpireClock;
    }

    ///设置当前的expire时间点
    inline void setExpireClock(CWX_UINT32 ttClock){
        m_ttExpireClock = ttClock;
    }

    ///获取配置信息
	inline UnistorConfig const* getConfig() const{
        return m_config;
    }

	///存储是否有效
	inline bool isValid() const {
        return m_bValid;
    }

	///存储无效时的错误信息
	char const* getErrMsg() const {
        return m_szErrMsg;
    }

    ///基于存储key获取key的分组
    inline CWX_UINT32 getKeyStoreGroup(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        )
    {
        return m_fnKeyStoreGroup(szKey, unKeyLen);
    }

    //获取key的分组
    inline CWX_UINT32 getKeyAsciiGroup(char const* szKey, ///<key
        CWX_UINT16 unKeyLen ///<key的长度
        )
    {
        return m_fnKeyAsciiGroup(szKey, unKeyLen);
    }
    
    ///key的ascii级别less函数。0：相等；-1：小于；1：大于
    inline int isKeyAsciiLess(char const* key1, ///<第一个key
        CWX_UINT16 unKey1Len, ///<第一个key的长度
        char const* key2, ///<第二个key
        CWX_UINT16 unKey2Len ///<第二个key的长度
        )
    {
        return m_fnKeyAsciiLess(key1, unKey1Len, key2, unKey2Len);
    }

public:
    ///解析以[\n]分隔的多个字段。返回空格分隔的字段的数量
	static int parseMultiField(char const* szFields, ///<以\n分割的多个字段字符串 
        UnistorKeyField*& field ///<分离后的字段。
        );
	
    ///释放field字段
	static void freeField(UnistorKeyField*& key);
	
    ///对add key的新、旧field进行归并。-1：失败；0：存在；1：成功
    static int mergeAddKeyField(CwxPackageWriterEx* writer1, ///<writer对象
        CwxPackageReaderEx* reader1, ///<reader 对象
        CwxPackageReaderEx* reader2, ///<reader 对象
		char const* key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<field的名字
		char const* szOldData, ///<key的原数据
		CWX_UINT32 uiOldDataLen, ///<key原数据的长度
		bool bOldKeyValue, ///<key原数据是否为key/value
		char const* szNewData, ///<key的新数据
		CWX_UINT32 uiNewDataLen, ///<key新数据的长度
		bool bNewKeyValue, ///<key新数据是否为key/value
        CWX_UINT32& uiFieldNum, ///<merge后的字段数
        char* szErr2K ///<merge失败的错误消息
        );
	
    ///对set key的新、旧field进行归并。-1：失败；1：成功
    static int mergeSetKeyField(CwxPackageWriterEx* writer1, ///<writer对象
        CwxPackageReaderEx* reader1, ///<reader 对象
        CwxPackageReaderEx* reader2, ///<reader 对象
        char const* key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<field的名字
        char const* szOldData, ///<key的原数据
        CWX_UINT32 uiOldDataLen, ///<key原数据的长度
        bool bOldKeyValue, ///<key原数据是否为key/value
        char const* szNewData, ///<key的新数据
        CWX_UINT32 uiNewDataLen, ///<key新数据的长度
        bool bNewKeyValue, ///<key新数据是否为key/value
        CWX_UINT32& uiFieldNum, ///<merge后的字段数
        char* szErr2K ///<merge失败的错误消息
        );
	
    ///对update key的新、旧field进行归并。-1：失败；0：不存在；1：成功
    static int mergeUpdateKeyField(CwxPackageWriterEx* writer1, ///<writer对象
        CwxPackageReaderEx* reader1, ///<reader 对象
        CwxPackageReaderEx* reader2, ///<reader 对象
        char const* key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<field的名字
        char const* szOldData, ///<key的原数据
        CWX_UINT32 uiOldDataLen, ///<key原数据的长度
        bool bOldKeyValue, ///<key原数据是否为key/value
        char const* szNewData, ///<key的新数据
        CWX_UINT32 uiNewDataLen, ///<key新数据的长度
        bool bNewKeyValue, ///<key新数据是否为key/value
        CWX_UINT32& uiFieldNum, ///<merge后的字段数
        bool bAppend, ///<update merge的时候，对于不存在的新字段是佛append添加
        char* szErr2K ///<merge失败的错误消息
        );

	///对int key的更新。-2：key超出边界；-1：失败；0：不存在；1：成功;
    static int mergeIncKeyField(CwxPackageWriterEx* writer1, ///<writer对象
        CwxPackageReaderEx* reader1, ///<reader 对象
        char const* key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<field的名字
        char const* szOldData, ///<key的原数据
        CWX_UINT32 uiOldDataLen, ///<key原数据的长度
        bool bOldKeyValue, ///<key原数据是否为key/value
		CWX_INT64  num,  ///<增加的值
        CWX_INT64* result, ///<若不为NULL，则设置为此值
		CWX_INT64  llMax, ///<计数器的最大值
		CWX_INT64  llMin, ///<计数器的最小值
		CWX_INT64& llValue, ///<计数器的新值
		char* szBuf,  ///<merge后内容的buf
		CWX_UINT32& uiBufLen, ///<传入buf大小，返回新内容的字节数
		bool& bKeyValue, ///<新内容是否为key/vaue
        CWX_UINT32 uiSign, ///<merge的标志
        char* szErr2K ///<merge失败的错误消息
        );
	
    ///对delete  key的field。-1：失败；1：成功
    static int mergeRemoveKeyField(CwxPackageWriterEx* writer1, ///<writer对象
        CwxPackageReaderEx* reader1, ///<reader 对象
        char const* key, ///<key的名字
        CwxKeyValueItemEx const* field, ///<field的名字
        char const* szOldData, ///<key的原数据
        CWX_UINT32 uiOldDataLen, ///<key原数据的长度
        bool bOldKeyValue, ///<key原数据是否为key/value
        CWX_UINT32& uiFieldNum, ///<返回字段的数量
        char* szErr2K ///<merge失败的错误消息
        );
    
    ///提取指定的field, UNISTOR_ERR_SUCCESS：成功；其他：错误代码
    static int pickField(CwxPackageReaderEx& reader, ///<reader对象
        CwxPackageWriterEx& write, ///<writer对象
        UnistorKeyField const* field, ///<要获取的field列表
        char const* szData, ///<key的field 数据
        CWX_UINT32 uiDataLen, ///<key的数据的长度
        char* szErr2K ///<获取失败的错误消息
        );


    ///是否需要commit
    inline bool isNeedCommit() const{
        if (m_uiUncommitBinlogNum){
            if (m_uiUncommitBinlogNum >= m_config->getCommon().m_uiWriteCacheFlushNum) return true;
            if ((m_ttExpireClock > m_uiLastCommitSecond + m_config->getCommon().m_uiWriteCacheFlushSecond)||
                (m_ttExpireClock < m_uiLastCommitSecond))
            {
                return true;
            }
        }
        return false;
    }
    

    
    ///获取store key的group函数
    static inline UNISTOR_KEY_GROUP_FN getKeyStoreGroupFn(){
        return m_fnKeyStoreGroup;
    }

    ///获取key的group函数
    static inline UNISTOR_KEY_GROUP_FN getKeyAsciiGroupFn(){
        return m_fnKeyAsciiGroup;
    }
    
    ///获取key的ascii的less比较
    static inline UNISTOR_KEY_CMP_LESS_FN getKeyAsciiLess(){
        return m_fnKeyAsciiLess;
    }
    

protected:
    UNISTOR_MSG_CHANNEL_FN     m_pMsgPipeFunc; ///<消息通道的function
    UNISTOR_GET_SYS_INFO_FN    m_pGetSysInfoFunc; ///<获取信息信息的function
    void*                   m_pApp; ///<app对象
	CwxBinLogMgr*			m_binLogMgr; ///<binlog
	UnistorConfig const*    m_config; ///<系统配置
	string				    m_strEngine; ///<engine的类型
	bool					m_bValid; ///<驱动是否有效
    bool                    m_bEnableExpire; ///<是否支持超时
    volatile CWX_UINT32     m_ttExpireClock; ///<当前失效的时间点
    CWX_UINT32              m_uiDefExpire; ///<缺省的超时时间
	char					m_szErrMsg[2048]; ///<驱动无效时的错误信息
	char*					m_szDataBuf;  ///<临时buf
	CWX_UINT32				m_uiDataBufLen; ///<临时buf的大小
    CWX_UINT64              m_ullStoreSid;  ///<存储的sid
    volatile CWX_UINT32	    m_uiUncommitBinlogNum; ///<未提交的binlog数量
    volatile CWX_UINT32     m_uiLastCommitSecond; ///<上一次commit的时间
    UnistorCache*           m_cache;               ///<cache对象
    CWX_UINT32              m_uiWriteCacheMSize;   ///<写cache的MBYTE
    CWX_UINT32              m_uiReadCacheMSize;     ///<读cache的MBYTE
    CWX_UINT32              m_uiReadCacheItemNum;   ///<读cache的最多cache的key的数量
    static UNISTOR_KEY_GROUP_FN    m_fnKeyStoreGroup;   ///<获取基于store key的group
    static UNISTOR_KEY_GROUP_FN    m_fnKeyAsciiGroup;    ///<key的ascii group分组获取函数
    static UNISTOR_KEY_CMP_LESS_FN  m_fnKeyAsciiLess;///<key的ascii less比较函数
};

#endif
