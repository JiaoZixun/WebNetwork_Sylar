#ifndef __SYLAR_URI_H__
#define __SYLAR_URI_H__

#include <memory>
#include <string>
#include <stdint.h>
#include "address.h"

namespace sylar {

/*

    foo://user@example.com:8042/over/there?name=ferret#nose
    sceme     authority         path   query          fragment
                   |
         [userinfo, host, port]
*/

class Uri {
public:
    typedef std::shared_ptr<Uri> ptr;

    static Uri::ptr Create(const std::string& uri);
    Uri();

    const std::string& getScheme() const {return m_scheme;}
    const std::string& getUserinfo() const {return m_userinfo;}
    const std::string& getHost() const {return m_host;}
    const std::string& getPath() const;
    const std::string& getQuery() const {return m_query;}
    const std::string& getFragment() const {return m_fragment;}
    int32_t getPort() const;


    void setScheme(const std::string& v) {m_scheme = v;}
    void setUserinfo(const std::string& v) { m_userinfo = v;}
    void setHost(const std::string& v) { m_host = v;}
    void setPath(const std::string& v) { m_path = v;}
    void setQuery(const std::string& v) { m_query = v;}
    void setFragment(const std::string& v) { m_fragment = v;}
    void setPort(int32_t v) {m_port = v;}

    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;
    Address::ptr createAddress() const;
private:
    bool isDefaultPort() const;
private:
    std::string m_scheme;
    std::string m_userinfo;
    std::string m_host;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    int32_t m_port;
};
}


#endif