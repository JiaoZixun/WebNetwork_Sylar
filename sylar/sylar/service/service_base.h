#ifndef __SYLAR_SERVICE_BASE_H__
#define __SYLAR_SERVICE_BASE_H__

#include "sylar/log.h"
#include "sylar/http/http.h"
#include "sylar/http/http_session.h"

namespace sylar {

class ServiceBase {
public:
    typedef std::shared_ptr<ServiceBase> ptr;
    ServiceBase(sylar::http::HttpRequest::ptr request
                    , sylar::http::HttpResponse::ptr response
                    , sylar::http::HttpSession::ptr session);
    virtual ~ServiceBase() {}

    bool isParamOk();
    // 虚函数，实际处理逻辑
    virtual void Handle() = 0;
    
protected:
    sylar::http::HttpRequest::ptr m_req;
    sylar::http::HttpResponse::ptr m_res;
    sylar::http::HttpSession::ptr m_sess;

};
}


#endif