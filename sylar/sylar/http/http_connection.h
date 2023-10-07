#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__

#include "sylar/socket_stream.h"
#include "http.h"
#include "sylar/uri.h"
#include "sylar/thread.h"
#include <list>

namespace sylar {
namespace http {

struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;
    enum class Error {
        // 成功
        OK = 0,
        // 非法URL
        INVALID_URL = 1,
        // 无法解析HOST
        INVALID_HOST = 2,
        // 连接失败
        CONNECT_FAIL = 3,
        // 连接对端被关闭
        SEND_CLOSE_BY_PEER = 4,
        // 发送请求产生socket错误
        SEND_SOCKET_ERROR = 5,
        // 超时
        TIMEOUT = 6,
        // 创建socket失败
        CREATE_SOCKET_ERROR = 7,
        // 从连接池中取连接失败
        POOL_GET_CONNECTION = 8,
        // 无效链接
        POOL_INVALID_CONNECTION = 9,
    };
    HttpResult(int _result, HttpResponse::ptr _response, const std::string& _error)
        : result(_result)
        , response(_response)
        , error(_error){}
    int result;
    HttpResponse::ptr response;
    std::string error;

    std::string toString() const;
};


class HttpConnectionPool;

// 客户端类
class HttpConnection : public SocketStream {
friend class HttpConnectionPool;
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    // 使用字符串url
    static HttpResult::ptr DoGet(const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    //使用uri解析类
    static HttpResult::ptr DoGet(Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    // 使用字符串url
    static HttpResult::ptr DoPost(const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    //使用uri解析类
    static HttpResult::ptr DoPost(Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    /*
    * 请求方法；url；超时时间；headers；消息
    */
    static HttpResult::ptr DoRequest(HttpMethod method
                        , const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method
                        , Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpRequest::ptr req
                        , Uri::ptr uri
                        , uint64_t timeout_ms);
    
    
    HttpConnection(Socket::ptr sock, bool owner = true);

    ~HttpConnection();

    // 接受HTTP响应
    HttpResponse::ptr recvResponse();
    // 发送HTTP请求
    int sendRequest(HttpRequest::ptr rsp);

private:
    uint64_t m_createTime = 0;
    uint64_t m_request = 0;
};

class HttpConnectionPool {
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;

    HttpConnectionPool(const std::string& host
                , const std::string& vhost
                , uint32_t port
                , uint32_t max_size
                , uint32_t max_alive_time
                , uint32_t max_request);

    HttpConnection::ptr getConnection();

    HttpResult::ptr doGet(const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    
    HttpResult::ptr doGet(Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    
    HttpResult::ptr doPost(const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");
    
    HttpResult::ptr doPost(Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");


    HttpResult::ptr doRequest(HttpMethod method
                        , const std::string& url
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    HttpResult::ptr doRequest(HttpMethod method
                        , Uri::ptr uri
                        , uint64_t timeout_ms
                        , const std::map<std::string, std::string>& headers = {}
                        , const std::string& body = "");

    HttpResult::ptr doRequest(HttpRequest::ptr req
                        , uint64_t timeout_ms);
private:
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
    
private:
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    // 连接池中存在的最大连接数，若用完则需要继续创建，当处理完后超出则释放
    uint32_t m_maxSize;
    // 连接最大存活时间     超出则关闭
    uint32_t m_maxAliveTime;
    // 连接最多处理数量     超出则关闭
    uint32_t m_maxRequest;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int> m_total = {0};
};



}
}


#endif