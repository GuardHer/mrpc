#ifndef MRPC_NET_TCP_NET_ADDR_H
#define MRPC_NET_TCP_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

namespace mrpc
{

class NetAddr
{
public:
    virtual sockaddr *getSockAddr() = 0;

    virtual socklen_t getSocketLen() = 0;

    virtual int getFamily() = 0;

    virtual std::string toString() = 0;
};

class IPNetAddr : public NetAddr
{
public:
    IPNetAddr(const std::string &ip, uint16_t port);
    explicit IPNetAddr(const std::string &addr);
    explicit IPNetAddr(sockaddr_in addr);


public:
    sockaddr *getSockAddr() override;

    socklen_t getSocketLen() override;

    int getFamily() override;

    std::string toString() override;

private:
    std::string m_ip;
    uint16_t m_port;

    sockaddr_in m_addr;
};

}// namespace mrpc

#endif//MRPC_NET_TCP_NET_ADDR_H