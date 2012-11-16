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

///存储引擎的引擎加载Function类型定义
typedef UnistorStoreBase* (*CWX_LOAD_ENGINE)(void);

///存储驱动
class UnistorStore{
public:
    ///构造函数
    UnistorStore();
    ///析构函数
    ~UnistorStore();
public:
	//加载配置文件.-1:failure, 0:success
	int init(UNISTOR_MSG_CHANNEL_FN msgPipeFunc, ///<与UnistorApp的线程通信的消息通道Func
        UNISTOR_GET_SYS_INFO_FN getSysInfoFunc, ///<获取系统信息的函数
        void* pApp, ///<UnistorApp对象
        UnistorConfig const* config, ///<配置文件对象的指针
        string const& strEnginePath, ///<存储引擎动态库的安装目录
        char* szErr2K ///<若初始化话失败，返回错误信息
        );

	///检测是否存在key；1：存在；0：不存在；-1：失败；
	inline int isExist(UnistorTss* tss, ///tss对象
        CwxKeyValueItemEx const& key, ///<检查的key
        CwxKeyValueItemEx const* field, ///<检查的field，若为空表示检查key
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra data
        CWX_UINT32& uiVersion, ///<返回key的版本号
        CWX_UINT32& uiFieldNum, ///<返回key的field的数量
        bool& bReadCached ///<数据是否在read cache中
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

    ///添加key，1：成功；0：存在；-1：失败；
    inline int addKey(UnistorTss* tss, ///<tss对象
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

	///set key，1：成功；-1：失败；
	inline int setKey(UnistorTss* tss,///tss
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

    ///update key，1：成功；0：不存在；-1：失败；-2：版本错误
	inline int updateKey(UnistorTss* tss, ///<tss对象
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

    ///inc key，1：成功；0：不存在；-1：失败；-2:版本错误；-3：超出边界
	inline int incKey(UnistorTss* tss, ///<线程tss对象
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

	///inc key，1：成功；0：不存在；-1：失败；
	inline int delKey(UnistorTss* tss, ///<线程tss对象
        CwxKeyValueItemEx const& key, ///<要删除的key
        CwxKeyValueItemEx const* field, ///<若要删除field，则指定field的名字
        CwxKeyValueItemEx const* extra,///<存储引擎的extra 数据
		CWX_UINT32& uiVersion, ///<若指定版本号，则修改前的版本号必须与此值相等，否则失败。返回新版本号
        CWX_UINT32& uiFieldNum,  ///<key的字段数量
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached ///<数据是否在write cache中
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

    ///import key，1：成功；-1：失败；
    int importKey(UnistorTss* tss, ///<tss对象
        CwxKeyValueItemEx const& key, ///<添加的key
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CwxKeyValueItemEx const& data, ///<添加key或field的数据
        CWX_UINT32& uiVersion, ///<若大于0，则设置修改后的key为此版本
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool bCache=true, ///<是否将key放到读cache
        CWX_UINT32 uiExpire=0 ///<若创建key，而且指定了uiExpire则设置key的超时时间
        )
    {
        return m_impl->importKey(tss, key, extra, data, uiVersion, bReadCached, bWriteCached, bCache, uiExpire);
    }

    ///同步add key的binlog数据，1：成功；0：存在；-1：失败；
    inline int syncAddKey(UnistorTss* tss, ///<线程的tss对象
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

    ///同步set key的binlog数据，1：成功；-1：错误；0：不存在，此是设置一个key的field时。
    inline int syncSetKey(UnistorTss* tss, ///<线程的tss数据
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

    ///同步update key的binlog数据。1：成功；0：不存在；-1：失败
    inline int syncUpdateKey(UnistorTss* tss, ///<线程的tss对象
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

    ///同步inc key的binlog数据。1：成功；0：不存在；-1：失败；
    inline int syncIncKey(UnistorTss* tss, ///<线程的tss数据
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

    ///同步delete key的binlog数据。返回值 1：成功；0：不存在；-1：失败；
    inline int syncDelKey(UnistorTss* tss, ///<线程的tss对象
        CwxKeyValueItemEx const& key, ///<要删除的key
        CwxKeyValueItemEx const* field, ///<若是删除field，则指定field
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        CWX_UINT32 uiVersion, ///<key进行delete后的版本号
        CWX_UINT64 ullSid, ///<delete操作对应的binlog的sid
        bool& bReadCached, ///<数据是否在read cache中
        bool& bWriteCached, ///<数据是否在write cache中
        bool  bRestore=false ///<是否从binlog恢复的数据
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

    ///sync import key，1：成功；-1：错误。
    inline int syncImportKey(UnistorTss* tss, ///<线程的tss数据
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

	///获取key, 1：成功；0：不存在；-1：失败;
	inline int get(UnistorTss* tss, ///<线程tss对象
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

    ///获取多个key。返回值 1：成功；-1：失败;
    inline int gets(UnistorTss* tss, ///<线程的tss对象
        list<pair<char const*, CWX_UINT16> > const& keys,  ///<要获取的key的列表。pair的first为key的名字，second为key的长度
        CwxKeyValueItemEx const* field, ///<若指定，则限定获取的field范围
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        char const*& szData, ///<获取的数据，内存由存储引擎分配
        CWX_UINT32& uiLen, ///<返回数据的长度
        CWX_UINT32& uiReadCacheNum, ///<在read cache中的数量
        CWX_UINT32& uiExistNum, ///<存在的key的数量
        CWX_UINT8 ucKeyInfo=0 ///<是否获取key的information。0：获取key的data。1：获取key信息；2：获取系统key
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

	///建立游标。-1：内部错误失败；0：成功
	inline int createCursor(UnistorStoreCursor& cursor, ///<游标对象
        char const* szBeginKey, ///<开始的key，若为NULL表示没有指定
        char const* szEndKey, ///<结束的key，若为NULL表示没有指定
        CwxKeyValueItemEx const* field, ///<指定游标要返回的field。
        CwxKeyValueItemEx const* extra, ///<存储引擎的extra数据
        char* szErr2K ///<若出错，返回错误信息
        )
    {
		return m_impl->createCursor(cursor, szBeginKey, szEndKey, field, extra, szErr2K);
	}

	///获取下一个数据。-1：失败；0：结束；1：获取一个
    inline int next(UnistorTss* tss, ///<线程的tss
        UnistorStoreCursor& cursor,  ///<Next的游标
		char const*& szKey,  ///<返回的key，内存由存储引擎分配
		CWX_UINT16& unKeyLen,  ///<返回key的字节数
		char const *& szData,  ///<返回key的data，内存由存储引擎分配
		CWX_UINT32& uiDataLen, ///<返回data的字节数
		bool& bKeyValue,  ///<data是否为keyValue结构
		CWX_UINT32& uiVersion,  ///<key的版本号
        bool bKeyInfo=false ///<是否返回key的information而不是其data
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

	//释放游标
	inline void closeCursor(UnistorStoreCursor& cursor){
		return m_impl->closeCursor(cursor);
	}

    ///开始导出数据。-1：内部错误失败；0：成功
    inline int exportBegin(UnistorStoreCursor& cursor, ///<export的游标
        char const* szStartKey, ///<export的开始key，不包含此key
        char const* szExtra, ///<extra信息
        UnistorSubscribe const& scribe,  ///<导出数据的订阅规则
        CWX_UINT64& ullSid, ///<当前的sid值
        char* szErr2K  ///<若出错则返回错误信息
        )
    {
        return m_impl->exportBegin(cursor,
            szStartKey,
            szExtra,
            scribe,
            ullSid,
            szErr2K);
    }

    ///获取export的下一条数据。-1：失败；0：结束；1：获取一个；2：skip数量为0
    inline int exportNext(UnistorTss* tss,  ///<线程的tss对象
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

    ///结束导出数据
    inline void exportEnd(UnistorStoreCursor& cursor){
        return m_impl->exportEnd(cursor);
    }

	///同步master的binlog.0：成功；-1：失败
	inline int syncMasterBinlog(UnistorTss* tss, ///<线程tss
        CwxPackageReaderEx* reader,  ///<对data进行解包的外部CwxPackageReaderEx对象
		CWX_UINT64 ullSid, ///<binlog的sid值，若为0，是从新编号sid。
		CWX_UINT32 ttTimestamp, ///<binlog的时间戳
		CWX_UINT32 uiGroup, ///<binlog所属的分组
		CWX_UINT32 uiType,  ///<binlog的消息类型
		CwxKeyValueItemEx const& data, ///<binlog的数据
        CWX_UINT32 uiVersion, ///<binlog对应的key的版本号
        bool  bRestore=false  ///<是否是从本地binlog恢复的数据
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

    ///commit存储引擎write cache的数据；0：成功；-1：失败
    inline int commit(char* szErr2K){
        if (!m_impl) return 0;
        return m_impl->commit(szErr2K);
    }
    
    ///关闭存储引擎
    inline int close(){
        if (!m_impl) return 0;
        return m_impl->close();
    }

    ///event处理函数，实现存储引擎与上层的交互；0：成功；-1：失败
    int storeEvent(UnistorTss* tss, ///<线程的tss
        CwxMsgBlock*& msg  ///<消息
        )
    {
        if (!m_impl) return 0;
        return m_impl->storeEvent(tss, msg);
    }

	///存储引擎的checkpoint
	inline void checkpoint(UnistorTss* tss){
		m_impl->checkpoint(tss);
	}

    ///失去同步通知，由引擎处理。返回值，0：没有变化；1：可以同步；-1：失败
    inline int lostSync(){
        return m_impl->lostSync();
    }

    ///添加时钟同步的记录，此用于存储引擎expire的控制。返回值：0：成功；-1：失败
    inline int appendTimeStampBinlog(CwxPackageWriterEx& writer, ///<writer对象
        CWX_UINT32      ttNow, ///<当前的时间
        char* szErr2K=NULL ///<若出错则返回错误信息
        )
    {
        return m_impl->appendTimeStampBinlog(writer, ttNow, szErr2K);
    }

	///获取engine的名字
	inline char const* getName() const{
        if (!m_impl) return "";
		return m_impl->getName();
	}

	///获取engine的版本
	inline char const* getVersion() const{
        if (!m_impl) return "";
		return m_impl->getVersion();
	}

    ///设置当前的sid
    inline void setCurSid(CWX_UINT64 ullSid){
        m_impl->setCurSid(ullSid);
    }

    ///是否需要commit
    inline bool isNeedCommit() const{
        return m_impl->isNeedCommit();
    }

    ///获取当前expire时间点
    inline CWX_UINT32 getExpireClock() const{
        return m_impl->getExpireClock();
    }

    ///设置当前的expire时间点
    inline void setExpireClock(CWX_UINT32 ttClock){
        m_impl->setExpireClock(ttClock);
    }

    ///是否订阅指定的数据
    inline bool isSubscribe(UnistorSubscribe const& subscribe, ///<订阅对象
        CWX_UINT32 uiGroup, ///<binlog所属的分组
        char const* szKey ///<binlog的key
        )
    {
        return m_impl->isSubscribe(subscribe, uiGroup, szKey);
    }

    ///订阅格式是否合法
    inline bool isValidSubscribe(UnistorSubscribe const& subscribe,///<订阅对象
        char* szErr2K ///<不合法时的错误消息
        )
    {
        return m_impl->isValidSubscribe(subscribe, szErr2K);
    }

    ///key的ascii级别less函数。0：相等；-1：小于；1：大于
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

	///get binlog 管理器
	inline CwxBinLogMgr* getBinLogMgr(){
        return m_impl->getBinLogMgr();
    }

	///存储是否有效
	inline bool isValid() const{
        return m_impl->isValid();
    }

	///获取存储无效时的错误信息
	inline char const* getErrMsg() const{
        if (!m_impl) return "";
        return m_impl->getErrMsg();
    }

	///获取存储引擎对象
	inline UnistorStoreBase* getStoreEngine(){
        return m_impl;
    }

private:
	UnistorStoreBase*	    m_impl; ///<存储引擎对象指针
	void*					m_dllHandle; ///<dll的dlopen返回的handle
};



#endif
