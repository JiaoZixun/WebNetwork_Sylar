#include "tcp_server.h"
#include "config.h"

namespace sylar {

static sylar::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = 
    sylar::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), 
            "tcp server read timeout");

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

TcpServer::TcpServer(sylar::IOManager* worker, sylar::IOManager* acceptWorker)
    : m_worker(worker)
    , m_acceptWorker(acceptWorker)
    , m_recvTimeout(g_tcp_server_read_timeout->getValue())
    , m_name("sylar/1.0.0")
    , m_stop(true) {

}

TcpServer::~TcpServer() {
    for(auto& i:m_socks) {
        i->close();
    }
    m_socks.clear();
}

bool TcpServer::bind(sylar::Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails) {
    for(auto& addr:addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            SYLAR_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            SYLAR_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }
    for(auto& i:m_socks) {
        SYLAR_LOG_INFO(g_logger) << "server bind success: " << *i;
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_stop) {
        Socket::ptr client = sock->accept();
        if(client) {
            client->setRecvTimeout(m_recvTimeout);
            m_worker->scheduler(std::bind(&TcpServer::handleClient,
                        shared_from_this(), client));
        }
        else {
            SYLAR_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
        }
    }
}

bool TcpServer::TcpServer::start() {
    if(!m_stop) {
        return true;
    }
    m_stop = false;
    for(auto& sock:m_socks) {
        m_acceptWorker->scheduler(std::bind(&TcpServer::startAccept,
                shared_from_this(), sock));
    }
    return true;
}

void TcpServer::TcpServer::stop() {
    m_stop = true;
    auto self = shared_from_this();
    m_acceptWorker->scheduler([this, self](){
        for(auto& sock:m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}

void TcpServer::TcpServer::handleClient(Socket::ptr client) {
    SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
}
}