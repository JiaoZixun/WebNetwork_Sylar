#include "sylar/log.h"
#include "sylar/http/http.h"
#include "sylar/service/service_event_type.h"
#include "sylar/service/service_demo.h"
#include "sylar/service/init_server.h"

sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();

void Listen() {
    // 事件组
    EventVec events;
    // 服务执行函数
    auto reuqest_func = [](sylar::http::HttpRequest::ptr request
                    , sylar::http::HttpResponse::ptr response
                    , sylar::http::HttpSession::ptr session){
        sylar::ServiceDemo::ptr service(new sylar::ServiceDemo(request, response, session));
        service->processRequest();
        return 0;
    };
    // 服务路由
    auto request_route = "/sylar/test1";
    events.push_back(std::make_pair(request_route, reuqest_func));
    // 开始执行函数
    sylar::Listen(events);
}

int main() {
    sylar::InitServer();
    sylar::IOManager iom(2);
    iom.scheduler(Listen);
    // Listen();
    return 0;
}