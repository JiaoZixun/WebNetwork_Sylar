#include "service_base.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

ServiceBase::ServiceBase(sylar::http::HttpRequest::ptr request
                    , sylar::http::HttpResponse::ptr response
                    , sylar::http::HttpSession::ptr session)
        : m_req(request)
        , m_res(response)
        , m_sess(session) {

}

bool ServiceBase::isParamOk() {
    return true;
}


}