#ifndef __SYLAR_SERVICE_EVENT_TYPE_H__
#define __SYLAR_SERVICE_EVENT_TYPE_H__

#include "sylar/http/http.h"
#include "sylar/http/http_session.h"

typedef std::function<int32_t (sylar::http::HttpRequest::ptr request
                    , sylar::http::HttpResponse::ptr response
                    , sylar::http::HttpSession::ptr session) > event_cb;
typedef std::string service_url;

typedef std::vector<std::pair<service_url, event_cb> > EventVec;


#endif