#ifndef __SYLAR_HTTP_SESSION_H__
#define __SYLAR_HTTP_SESSION_H__

#include "sylar/socket_stream.h"
#include "http.h"

namespace sylar {
namespace http {

// 服务端使用
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);

    // 接收请求
    HttpRequest::ptr recvRequest();
    // 发送反馈
    int sendResponse(HttpResponse::ptr rsp);

};
}


}


#endif