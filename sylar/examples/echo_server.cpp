#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/bytearray.h"
#include "sylar/address.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();


class EchoServer : public sylar::TcpServer {
public:
    EchoServer(int type);
    void handleClient(sylar::Socket::ptr client);
private:
    int m_type = 0;
};

EchoServer::EchoServer(int type) 
    :m_type(type) {
}

void EchoServer::handleClient(sylar::Socket::ptr client) {
    SYLAR_LOG_INFO(g_logger) << "handleClient " << *client;
    sylar::ByteArray::ptr ba(new sylar::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0) {
            SYLAR_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        }
        else if(rt < 0) {
            SYLAR_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" <<strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1) {   //text
            SYLAR_LOG_INFO(g_logger) << ba->toString();
        }
        else {
            SYLAR_LOG_INFO(g_logger) << ba->toHexString();
        }
    }
}

int type = 1;

void run() {
    EchoServer::ptr es(new EchoServer(type));
    auto addr = sylar::Address::LookAny("0.0.0.0:8020");
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

int main() {
    sylar::IOManager iom(2);
    iom.scheduler(run);
    return 0;
}