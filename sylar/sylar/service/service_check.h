#ifndef __SYLAR_SERVICE_CHECK_H__
#define __SYLAR_SERVICE_CHECK_H__

#include "sylar/http/http.h"
#include "sylar/service/service_base.h"


namespace sylar {
class ServiceCheck : public ServiceBase {
public:
    ServiceCheck(sylar::http::HttpRequest::ptr request
            , sylar::http::HttpResponse::ptr response
            , sylar::http::HttpSession::ptr session);
    virtual ~ServiceCheck() {}
    // 虚函数，实际处理逻辑
    virtual void Handle() = 0;
    void HandleRequest();
};
}


#endif