#ifndef __ZK_LOCKER_H__
#define __ZK_LOCKER_H__
#include "ZkAdaptor.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

class ZkMutex{
public:
    ZkMutex(pthread_mutex_t* mutex){
        m_mutex = mutex;
        if (m_mutex) pthread_mutex_lock(m_mutex);
    }
    ~ZkMutex(){
        if (m_mutex) pthread_mutex_unlock(m_mutex);
    }
private:
    pthread_mutex_t*     m_mutex;
};
typedef void (* ZK_LOCK_COMPLETION) (bool bLock, void* context);


class ZkLocker{
public:
    enum{
        MAX_SEQ_NODE_SIZE = 2048
    };
    enum{
        ZK_WATCH_TYPE_MASTER=1,
        ZK_WATCH_TYPE_PREV=2,
        ZK_WATCH_TYPE_ROOT=3
    };
public:
    ZkLocker();
    ~ZkLocker();
public:
    ///��ʼ��
    int init(ZkAdaptor* zk,
        string const& path,
        string const& prex,
        ZK_LOCK_COMPLETION completion,
        void* context=NULL,
        ACL_vector* acl=&ZOO_OPEN_ACL_UNSAFE,
        watcher_fn lock_func=NULL,
        void* lock_func_context=NULL);
    ///������0�������ɹ���-1������ʧ��
    int lock(unsigned int uiWatchType=ZK_WATCH_TYPE_PREV);
    ///�ͷ�����0�������ɹ���-1������ʧ��
    int unlock();
public:
    ///�Ƿ�watch master
    inline unsigned int watchType() const{
        return m_uiWatchType;
    }
    ///�Ƿ�����ǰ�
    inline bool isLockFail(){
        ZkMutex locker(&m_mutex);
        return m_bLockFail;
    }
    ///��ȡ���е�locker
    inline void getLocker(list<string>& lockers){
        ZkMutex locker(&m_mutex);
        _getLocker(lockers);
    }
    ///�Ƿ����
    bool isLocked(){
        ZkMutex locker(&m_mutex);
        return _isLocked();
    }
    ///��ȡ�Լ�����node
    void getSelfNode(string& strNode){
        ZkMutex locker(&m_mutex);
        _getSelfNode(strNode);
    }
    ///��ȡ�Լ�����node��path name
    void getSelfPathNode(string& strPathNode){
        ZkMutex locker(&m_mutex);
        _getSelfPathNode(strPathNode);
    }
    ///��ȡowner����node
    void getOwnerNode(string& strNode){
        ZkMutex locker(&m_mutex);
        _getOwnerNode(strNode);
    }
    ///��ȡowner����node��path name
    void getOwnerPathNode(string& strPathNode){
        ZkMutex locker(&m_mutex);
        _getOwnerPathNode(strPathNode);
    }
    ///��ȡprev����node
    void getPrevNode(string& strNode){
        ZkMutex locker(&m_mutex);
        _getPrevNode(strNode);
    }
    ///��ȡPrev����node��path name
    void getPrevPathNode(string& strPathNode){
        ZkMutex locker(&m_mutex);
        _getPrevPathNode(strPathNode);
    }
    ///��ȡ���е�locker
    inline void _getLocker(list<string>& locers){
        locers.clear();
        map<uint64_t, string>::iterator iter = m_strSeqMap.begin();
        while(iter != m_strSeqMap.end()){
            locers.push_back(iter->second);
            iter++;
        }
        return;
    }
    ///�Ƿ����
    bool _isLocked(){
        if (m_strOwnerNode.length()){
            return m_strOwnerNode == m_strSelfNode;
        }
        return false;
    }
    ///��ȡ�Լ�����node
    void _getSelfNode(string& strNode){
        strNode = m_strSelfNode;
    }
    ///��ȡ�Լ�����node��path name
    void _getSelfPathNode(string& strPathNode){
        strPathNode = m_strSelfPathNode;
    }
    ///��ȡowner����node
    void _getOwnerNode(string& strNode){
        strNode = m_strOwnerNode;
    }
    ///��ȡowner����node��path name
    void _getOwnerPathNode(string& strPathNode){
        strPathNode = m_strOwnerPathNode;
    }
    ///��ȡprev����node
    void _getPrevNode(string& strNode){
        strNode = m_strPrevNode;
    }
    ///��ȡPrev����node��path name
    void _getPrevPathNode(string& strPathNode){
        strPathNode = m_strPrevPathNode;
    }

public:
    static void lock_watcher_fn(zhandle_t* , int , int ,
        const char* , void *watcherCtx);
    static bool splitSeqNode(string const& strSeqNode, uint64_t& seq, string& strNode);
    ///�Ի�ȡ�����߸�������Ϣ
    static bool splitSeqNode(string const& strSeqNode, uint64_t& seq, string& strPrev, string& strSession);
    ///�Ի�ȡ�����߸�������Ϣ
    static bool splitNode(string const& strNode, string& strPrev, string& strSession);
    ///��ȡnode��
    static void getNode(string const& strPrev, uint64_t clientid, string& strNode);
    ///��ȡ·�����һ���ڵ������
    static char const* getLastName(char* str);
    ///��ȡ�ڵ��·��
    static string const& getNodePath(string const& strPath, string const& strNode, string& strNodePath);

private:
    ///������0�����ڵ㲻���ڣ�1�������ɹ���-1������ʧ��
    int _retryLock();
    ///�ͷ�����0�������ɹ���-1������ʧ��
    int _unlock();

    inline void _clearLockInfo(){
        m_strSelfNode = "";
        m_strSelfPathNode = "";
        m_strOwnerNode = "";
        m_strOwnerPathNode = "";
        m_strPrevNode = "";
        m_strPrevPathNode = "";
        m_strSeqMap.clear();
        m_bLockFail = false;
    }

private:
    ZkAdaptor*          m_zkHandler; ///<zk��handler
    string              m_strPath;  ///<����·��
    string              m_strPrex; ///<����ǰ׺��nodeΪ[prex]-[session-id]-[seq]�ĸ�ʽ
    struct ACL_vector*  m_acl;    ///<����Ȩ��
    void *              m_context;  ///<������Ϣ
    ZK_LOCK_COMPLETION  m_completion;  ///<���仯��֪ͨ����
    pthread_mutex_t     m_mutex;     ///<������
    string              m_strSelfNode;   ///<�Լ��Ľڵ�����
    string              m_strSelfPathNode; ///<�ڵ��ȫ·��path
    string              m_strOwnerNode;   ///<��ӵ���ߵĽڵ�����
    string              m_strOwnerPathNode; ///<��ӵ���ߵ�ȫ·��path
    string              m_strPrevNode;    ///<�Լ�ǰһ���ڵ������
    string              m_strPrevPathNode; ///<�Լ�ǰһ���ڵ��ȫ·��path
    char                m_szBuf[MAX_SEQ_NODE_SIZE];    ///<seq�ڵ��ֵ
    map<uint64_t, string> m_strSeqMap;  ///<����session��seq��map
    unsigned int          m_uiWatchType; ///<�Ƿ�watch master��ȱʡwathe��ǰһ����ֹ��Ⱥ
    watcher_fn          m_lock_func; ///<���仯�Ļص�function
    void*               m_lock_func_context; ///<���仯�Ļص���context
    bool                m_bLockFail; ///<�Ƿ����ʧ��
};




#endif  

