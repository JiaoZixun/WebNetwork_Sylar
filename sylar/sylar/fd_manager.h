#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include<memory>
#include<vector>

#include"thread.h"
#include"singleton.h"

namespace sylar {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx(int fd);
    ~FdCtx();

    bool init();
    bool isInit() const { return m_isInit; }
    bool isSocket() const {return m_isSocket;}
    bool isClose() const {return m_isClosed;}
    bool close();

    void setUserNonblock(bool v) {m_userNonblock = v;}
    bool getUserNonblock() const {return m_userNonblock;}

    void setSysNonblock(bool v) {m_sysNonblock = v;}
    bool getSysNonblock() const {return m_sysNonblock;}

    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);
  
private:
    bool m_isInit: 1;                       // 是否初始化
    bool m_isSocket: 1;                     // 是否是socket
    bool m_sysNonblock: 1;                  // 是否是系统调用
    bool m_userNonblock: 1;                 // 是否是用户态
    bool m_isClosed: 1;                     // 是否关闭
    int m_fd;                               // fd 文件描述符

    uint64_t m_recvTimeout;                 // 接收超时时间
    uint64_t m_sendTimeout;                 // 发送超时时间

};


class FdManager {
public:
    typedef RWMutex RWMutexType;
    FdManager();

    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);

private:
    RWMutexType m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

typedef Singleton<FdManager> FdMgr;
}

#endif