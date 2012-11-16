#ifndef __ZK_ADAPTOR_H__
#define __ZK_ADAPTOR_H__

/**
*@file  ZkAdaptor.h
*@brief  Zookeeper��c++�ӿڣ�����Zookeeper�Ķ��߳�ģʽ
*@author cwinux@gmail.com
*@version 1.0
*@date    2011-11-10
*@warning ��
*@bug   
*/

#include <string>
#include <vector>
#include <list>
#include <map>
#include <inttypes.h>

using namespace std;

extern "C" {
#include "zookeeper.h"
}

class ZkAdaptor
{
public:
	enum{
		ZK_DEF_RECV_TIMEOUT_MILISECOND = 5000  ///<ͨ�ų�ʱʱ�䣬5s
	};
	enum{///<add auth״̬
		AUTH_STATE_WAITING = 0,  ///<����add auth
		AUTH_STATE_FAIL = 1,     ///<add authʧ��
		AUTH_STATE_SUCCESS = 2   ///<add auth�ɹ�
	};
public:

	///���캯��
	ZkAdaptor(string const& strHost, ///<zookeeper���ӣ�Ϊhost:port�ṹ
		uint32_t uiRecvTimeout=ZK_DEF_RECV_TIMEOUT_MILISECOND);

	///��������
	virtual ~ZkAdaptor(); 

	/**
	*@brief  �����ʼ��.
	*@param [in] level ������־���𣬿���Ϊ
	             ZOO_LOG_LEVEL_ERROR,ZOO_LOG_LEVEL_WARN,ZOO_LOG_LEVEL_INFO,ZOO_LOG_LEVEL_DEBUG
	*@return 0:success; -1:failure.
	*/
	int init(ZooLogLevel level=ZOO_LOG_LEVEL_WARN);

	/**
	*@brief  �������ӣ����ӽ�����ᴥ��onConnect()���ײ����zookeeper_init()
	         Ҳ����ͨ��isConnected()ȥ��ѯ�����Ƿ�����
	*@param [in] clientid_t ���ӵ�session
	*@param [in] flags zookeeper_init��flags��������ǰΪ0.
    *@param [in] watch zk���¼�watch����������ָ�������ϵͳĬ�ϵ�watch����Ҫ����on�ĺ��������¼���
    *@param [in] context ���趨��watch������Ҫָ��watch��context��
	*@return 0:success; -1:failure.
	*/
	virtual int connect(const clientid_t *clientid=NULL, int flags=0, watcher_fn watch=NULL, void *context=NULL);

	///�ر�����
	void disconnect();


	///��������Ƿ�����true��������false��δ������
	bool isConnected()
	{
		if (m_zkHandle){
			int state = zoo_state (m_zkHandle);
			if (state == ZOO_CONNECTED_STATE) return true;
		} 
		return false;
	}

	/**
	*@brief  ��������Ȩ���ײ����zoo_add_auth()��
	*@param [in] scheme auth��scheme����ǰֻ֧��digest���͵���Ȩ
	*@param [in] cert auth��֤�顣����digestģʽ��Ϊuser:passwd�ĸ�ʽ
	*@param [in] certLen cert�ĳ��ȡ�
	*@param [in] timeout ��Ȩ��ʱ��ʱ�䣬��λΪms����Ϊ0�򲻵ȴ�����Ҫ��������getAuthState()��ȡ��Ȩ�Ľ����
	*@return true:��Ȩ�ɹ���false����Ȩʧ��.
	*/
	bool addAuth(const char* scheme, const char* cert, int certLen, uint32_t timeout=0, void_completion_t completion=NULL, const void *data=NULL);

	///��ȡ��Ȩ״̬��ΪAUTH_STATE_WAITING��AUTH_STATE_FAIL��AUTH_STATE_SUCCESS֮һ
	int getAuthState() const { return m_iAuthState;}

	/**
	*@brief  ����node���ײ����zoo_create()��
	*@param [in] path �ڵ�·��
	*@param [in] data �ڵ������
	*@param [in] dataLen ���ݵĳ��ȡ�
	*@param [in] acl �ڵ�ķ���Ȩ�ޡ�
	*@param [in] flags �ڵ��flag��0��ʾ�����ڵ㣬����ΪZOO_SEQUENCE��ZOO_EPHEMERAL����ϡ�
    *@param [in] recursive  �Ƿ�ݹ鴴�����нڵ㡣
	*@param [out] pathBuf ��ΪZOO_SEQUENCE���͵Ľڵ㣬���������Ľڵ����֡�
	*@param [in] pathBufLen  pathBuf��buf�ռ䡣
	*@return 1:�ɹ���0:�ڵ���ڣ�-1��ʧ��
	*/
	int createNode(const string &path, 
		char const* data,
		uint32_t dataLen,
		const struct ACL_vector *acl=&ZOO_OPEN_ACL_UNSAFE,
		int flags=0,
        bool recursive=false,
		char* pathBuf=NULL,
		uint32_t pathBufLen=0);

	/**
	*@brief  ɾ���ڵ㼰��child�ڵ㡣
	*@param [in] path �ڵ�·��
	*@param [in] recursive �Ƿ�ɾ��child node��
	*@param [in] version ���ݰ汾��-1��ʾ����֤�汾�š�
	*@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
	*/
	int deleteNode(const string &path,
		bool recursive = false,
		int version = -1);

	/**
	*@brief  ��ȡһ��node�ĺ��ӡ�
	*@param [in] path �ڵ�·��
	*@param [out] childs �ڵ�ĺ����б�
	*@param [in] watch �Ƿ�watch�ڵ�ı仯��
	*@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
	*/
	int getNodeChildren( const string &path, list<string>& childs, int watch=0);

	/**
	*@brief  ���һ���ڵ��Ƿ���ڡ�
	*@param [in] path �ڵ�·��
	*@param [out] stat �ڵ����Ϣ
	*@param [in] watch �Ƿ�watch�ڵ�ı仯��
	*@return 1:���ڣ�0���ڵ㲻���ڣ�-1��ʧ��.
	*/
	int nodeExists(const string &path, struct Stat& stat, int watch=0);

	/**
	*@brief  ��ȡһ���ڵ�����ݼ���Ϣ��
	*@param [in] path �ڵ�·��
	*@param [out] data ���ݿռ䡣
	*@param [in out]  dataLen �������ݿռ��С������data����ʵ��С��
	*@param [out] stat �ڵ���Ϣ��
	*@param [in] watch �Ƿ�watch�ڵ�ı仯��
	*@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
	*/
	int getNodeData(const string &path, char* data, uint32_t& dataLen, struct Stat& stat, int watch=0);

    /**
    *@brief  ��ȡһ��node�ĺ��ӣ���ע���¼�������
    *@param [in] path �ڵ�·��
    *@param [out] childs �ڵ�ĺ����б�
    *@param [in] watcher watch����������ָ����watch��
    *@param [in] watcherCtx watch������context��
    *@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
    */
    int wgetNodeChildren( const string &path, list<string>& childs, watcher_fn watcher=NULL, void* watcherCtx=NULL);
    /**
    *@brief  ���һ���ڵ��Ƿ���ڡ�
    *@param [in] path �ڵ�·��
    *@param [out] stat �ڵ����Ϣ
    *@param [in] watcher watch����������ָ����watch��
    *@param [in] watcherCtx watch������context��
    *@return 1:���ڣ�0���ڵ㲻���ڣ�-1��ʧ��.
    */
    int wnodeExists(const string &path, struct Stat& stat, watcher_fn watcher=NULL, void* watcherCtx=NULL);
    /**
    *@brief  ��ȡһ���ڵ�����ݼ���Ϣ��
    *@param [in] path �ڵ�·��
    *@param [out] data ���ݿռ䡣
    *@param [in out]  dataLen �������ݿռ��С������data����ʵ��С��
    *@param [out] stat �ڵ���Ϣ��
    *@param [in] watcher watch����������ָ����watch��
    *@param [in] watcherCtx watch������context��
    *@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
    */
    int wgetNodeData(const string &path, char* data, uint32_t& dataLen, struct Stat& stat, watcher_fn watcher=NULL, void* watcherCtx=NULL);

	/**
	*@brief  ��ȡһ���ڵ�����ݼ���Ϣ��
	*@param [in] path �ڵ�·��
	*@param [in] data ���ݡ�
	*@param [in]  dataLen ���ݴ�С��
	*@param [in] version ���ݰ汾����Ϊ-1��ʾ���޶���
	*@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
	*/
	int setNodeData(const string &path, char const* data, uint32_t dataLen, int version = -1);

	/**
	*@brief  ��ȡһ���ڵ��ACL��Ϣ��
	*@param [in] path �ڵ�·��
	*@param [out] acl �ڵ��acl��Ϣ��
	*@param [out] stat �ڵ��ͳ����Ϣ��
	*@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
	*/
	int getAcl(const char *path, struct ACL_vector& acl, struct Stat& stat);

	/**
	*@brief  ����һ���ڵ��ACL��Ϣ��
	*@param [in] path �ڵ�·��
	*@param [int] acl �ڵ��acl��Ϣ��
	*@param [in] version ���ݰ汾����Ϊ-1��ʾ���޶���
	*@return 1:�ɹ���0���ڵ㲻���ڣ�-1��ʧ��.
	*/
    int setAcl(const char *path, const struct ACL_vector *acl=&ZOO_OPEN_ACL_UNSAFE, bool recursive=false, int version=-1);
	///��ȡzookeeper������
	string const& getHost() const { return m_strHost;}

	///��ȡzookeeper������handle
	zhandle_t* getZkHandle() { return m_zkHandle;}

	///��ȡzookeeper���ӵ�client session
	const clientid_t * getClientId() { return  isConnected()?zoo_client_id(m_zkHandle):NULL;}

	///��ȡ����Ĵ������
	int  getErrCode() const { return m_iErrCode;}

	///��ȡ������Ϣ
	char const* getErrMsg() const { return m_szErr2K;}
public:
	
	///sleep miliSecond����
	static void sleep(uint32_t miliSecond);
	///split the src
	static int split(string const& src, list<string>& value, char ch);

	/**
	*@brief  ��input���ַ�������base64��ǩ�����û���Ҫ�ͷŷ��ص��ַ��ռ䡣
	*@param [in] input Ҫbase64ǩ�����ַ���
	*@param [in] length input�ĳ��ȡ�
	*@return NULL:ʧ�ܣ�����Ϊinput��base64ǩ��
	*/
	static char* base64(const unsigned char *input, int length);

	/**
	*@brief  ��input���ַ�������sha1��ǩ����
	*@param [in] input Ҫsha1ǩ�����ַ���
	*@param [in] length input�ĳ��ȡ�
	*@param [out] output 20byte��sha1ǩ��ֵ��
	*@return void
	*/
	static void sha1(char const* input, int length, unsigned char *output);

	/**
	*@brief  ��input���ַ�������sha1ǩ�����ٽ���base64�任���û���Ҫ�ͷŷ��صĿռ�
	*@param [in] input Ҫǩ�����ַ���
	*@param [in] length input�ĳ��ȡ�
	*@return NULL:ʧ�ܣ�����Ϊinput��base64ǩ��
	*/
	static char* digest(char const* input, int length);

	/**
	*@brief  ����priv�γ�acl��priv����Ϊall,self,read����user:passwd:acrwd
	*@param [in] priv Ҫǩ�����ַ���,����Ϊall,self,read����user:passwd:acrwd
	*@param [in] acl  Ȩ��
	*@return false:ʧ�ܣ�true:�ɹ�
	*/
	static bool fillAcl(char const* priv, struct ACL& acl);


	///���Ȩ����Ϣ��һ��Ȩ��һ��list��Ԫ��
	static void dumpAcl(ACL_vector const& acl, list<string>& info);

	///����ڵ����Ϣ,һ��һ����Ϣ��
	static void dumpStat(struct Stat const& stat, string& info);
public:
    ///�¼�֪ͨ����ʱ�����¼��ĸ�api
    virtual void onEvent(zhandle_t *t, int type, int state, const char *path);

    ///���ӽ����Ļص��������еײ��zk�̵߳���
    virtual void onConnect(){}

    ///���ڽ�����ϵ�Ļص��������еײ��zk�̵߳���
    virtual void onAssociating(){}

    ///���ڽ������ӵĻص��������еײ��zk�̵߳���
    virtual void onConnecting(){}

    ///��Ȩʧ�ܵĻص��������еײ��zk�̵߳���
    virtual void onFailAuth(){}

    ///SessionʧЧ�Ļص��������еײ��zk�̵߳���
    virtual void onExpired(){}

    /**
    *@brief  watch��node�����¼��Ļص��������еײ��zk�̵߳��á�Ӧ����zoo_exists��watch��
    *@param [in] zk��watcher��state
    *@param [in] path watch��path.
    *@return void.
    */
    virtual void onNodeCreated(int state, char const* path);

    /**
    *@brief  watch��nodeɾ���¼��Ļص��������еײ��zk�̵߳��á�Ӧ���� zoo_exists��zoo_get��watch��
    *@param [in] zk��watcher��state
    *@param [in] path watch��path.
    *@return void.
    */
    virtual void onNodeDeleted(int state, char const* path);

    /**
    *@brief  watch��node�޸��¼��Ļص��������еײ��zk�̵߳��á�Ӧ���� zoo_exists��zoo_get��watch��
    *@param [in] zk��watcher��state
    *@param [in] path watch��path.
    *@return void.
    */
    virtual void onNodeChanged(int state, char const* path);

    /**
    *@brief  watch��node���ӱ���¼��Ļص��������еײ��zk�̵߳��á�Ӧ����zoo_get_children��watch��
    *@param [in] zk��watcher��state
    *@param [in] path watch��path.
    *@return void.
    */
    virtual void onNodeChildChanged(int state, char const* path);

    /**
    *@brief  zkȡ��ĳ��wathc��֪ͨ�¼��Ļص��������еײ��zk�̵߳��á�
    *@param [in] zk��watcher��state
    *@param [in] path watch��path.
    *@return void.
    */
    virtual void onNoWatching(int state, char const* path);

    /**
    *@brief  ����zookeeper���¼�֪ͨ�ص��������еײ��zk�̵߳��á�
    *@param [in] type��watcher���¼�type
    *@param [in] zk��watcher��state
    *@param [in] path watch��path.
    *@return void.
    */
    virtual void onOtherEvent(int type, int state, const char *path);

private:
	///�ڲ���wacher function
	static void watcher(zhandle_t *zzh, int type, int state, const char *path,
		void* context);

	///�ڲ�add auth��function
	static void authCompletion(int rc, const void *data);
private:
	string       m_strHost; ///<The host addresses of ZK nodes.
	uint32_t   m_uiRecvTimeout; ///<��Ϣ���ճ�ʱ
	int  		 m_iAuthState; ///<add auth�����״̬
	zhandle_t*   m_zkHandle; 	///<The current ZK session.
	int           m_iErrCode;  	///<Err code
	char          m_szErr2K[2048]; 	///<Err msg
};

#endif /* __ZK_ADAPTER_H__ */
