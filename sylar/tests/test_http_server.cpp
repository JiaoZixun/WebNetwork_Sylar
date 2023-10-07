#include "sylar/http/http_server.h"
#include "sylar/http/servlet.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();

void run() {
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    sylar::Address::ptr addr = sylar::Address::LookAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx", [](sylar::http::HttpRequest::ptr req
                    , sylar::http::HttpResponse::ptr rsp
                    , sylar::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });

    sd->addGlobServlet("/sylar/*", [](sylar::http::HttpRequest::ptr req
                ,sylar::http::HttpResponse::ptr rsp
                ,sylar::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });
    server->start();
}


int main() {
    sylar::IOManager iom(2);
    iom.scheduler(run);
    return 0;
}