#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();

void run() {
    auto addr = sylar::Address::LookAny("0.0.0.0:8033");
    //auto addr2 = sylar::UnixAddress::ptr(new sylar::UnixAddress("/tmp/unix_addr"));
    std::vector<sylar::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    sylar::TcpServer::ptr tcp_server(new sylar::TcpServer);
    std::vector<sylar::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    //SYLAR_LOG_INFO(g_logger) << *addr << " - " << *addr2;
}

int main() {
    sylar::IOManager iom(2);
    iom.scheduler(run);
    return 0;
}