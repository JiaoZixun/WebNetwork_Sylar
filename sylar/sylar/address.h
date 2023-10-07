#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#include<memory>
#include<string>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<iostream>
#include<arpa/inet.h>
#include<unistd.h>
#include<vector>
#include<map>

namespace sylar{

class IPAddress;
class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
    // 通过域名找到地址
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static Address::ptr LookAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static std::shared_ptr<IPAddress> LookAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    static bool GetInterfaceAddresses(std::multimap<std::string
                            , std::pair<Address::ptr, uint32_t> >& result
                            , int family = AF_INET);
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >& result
                            , const std::string& iface, int family = AF_INET);
    virtual ~Address() {}

    // 获取socket类型
    int getFamily() const;
    // 获取socket地址
    virtual const sockaddr* getAddr() const = 0;
    virtual sockaddr* getAddr() = 0;
    // 获取socket长度
    virtual socklen_t getAddrLen() const = 0;
    // 输出流
    virtual std::ostream& insert(std::ostream& os) const = 0;

    std::string toString() const;

    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    static IPAddress::ptr Create(const char* address, uint16_t port = 0);

    // 根据ip获取广播地址
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    // 根据ip获取网络地址
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    // 根据ip获取子网掩码
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0; 

};

class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

    IPv4Address(const sockaddr_in& address);
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    // 基类虚函数
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override; 

private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv6Address> ptr;

    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);

    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    // 基类虚函数
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override; 
private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string& path);

    void setAddrLen(uint32_t v);

    // 基类虚函数
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

class UnKnownAddress : public Address {
public:
    typedef std::shared_ptr<UnKnownAddress> ptr;
    UnKnownAddress(int family);
    UnKnownAddress(const sockaddr& addr);

    // 基类虚函数
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);

}

#endif