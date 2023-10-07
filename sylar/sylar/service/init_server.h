#ifndef __SYLAR_INIT_SERVER_H__
#define __SYLAR_INIT_SERVER_H__

#include "sylar/http/http_server.h"
#include "sylar/http/servlet.h"
#include "sylar/log.h"
#include "sylar/service/service_event_type.h"

#include <iostream>


namespace sylar {

// const std::string& uri, FunctionServlet::callback cb
// void Listen(events)
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

void InitServer() {
    std::cout << "InitServer()" << std::endl;
    
    // SYLAR_LOG_INFO(g_logger) << "Server init complete";
}

void Listen(EventVec &events) {
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    sylar::Address::ptr addr = sylar::Address::LookAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "CONTINUE BIND";
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    for(auto& ev:events) {
        sd->addServlet(ev.first, ev.second);
    }
    server->start();
}

}


#endif