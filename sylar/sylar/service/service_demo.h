#ifndef __SYLAR_SERVICE_DEMOH__
#define __SYLAR_SERVICE_DEMOH__

#include "sylar/log.h"
#include "sylar/http/http.h"
#include "sylar/http/http_session.h"
#include "sylar/service/service_check.h"


namespace sylar{

class ServiceDemo : public ServiceCheck {
public:
    typedef std::shared_ptr<ServiceDemo> ptr;
    ServiceDemo(sylar::http::HttpRequest::ptr request
                    , sylar::http::HttpResponse::ptr response
                    , sylar::http::HttpSession::ptr session);

    virtual ~ServiceDemo() {}
    void processRequest();
    void Handle() override;


private:
    std::string m_image_url;
};
}


#endif