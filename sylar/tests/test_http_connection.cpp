#include <iostream>
#include "sylar/http/http_connection.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();

void test_pool() {
    sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool("www.sylar.top", "", 80, 10, 1000 * 30, 5));
    sylar::IOManager::GetThis()->addTimer(1000, [pool](){
        auto t = pool->doGet("/", 300);
        SYLAR_LOG_INFO(g_logger) << t->toString();
    }, true);
}

void run() {
    sylar::Address::ptr addr = sylar::Address::LookAnyIPAddress("www.sylar.top:80");
    if(!addr) {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return ;
    }
    //SYLAR_LOG_INFO(g_logger) << "get addr success";
    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    //SYLAR_LOG_INFO(g_logger) << "get TCP success";
    bool rt = sock->connect(addr);
    if(!rt) {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << "failed";
        return ;
    }
    //SYLAR_LOG_INFO(g_logger) << "connect success";
    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    SYLAR_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    if(!rsp) {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return ;
    }
    SYLAR_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;
    
    SYLAR_LOG_INFO(g_logger) << "-------------------------------------------------------";

    auto rt2 = sylar::http::HttpConnection::DoGet("http://sylar.top/", 300);
    SYLAR_LOG_INFO(g_logger) << "result=" << rt2->result
        << "error=" << rt2->error
        << "rsp=" << (rsp ? rsp->toString() : "");
    SYLAR_LOG_INFO(g_logger) << "-------------------------------------------------------";
    test_pool();
}

int main() {
    sylar::IOManager iom(2);
    iom.scheduler(&run);
    return 0;
}