#include "src/net/tcp/net_addr.h"
#include "src/common/log.h"

#include <string.h>

namespace mrpc
{

IPNetAddr::IPNetAddr(const std::string &ip, uint16_t port)
    : m_ip(ip), m_port(port)
{
    // 初始化
    memset(&m_addr, 0, sizeof(m_addr));

    m_addr.sin_family = AF_INET;                     // ipv4
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());// ip
    m_addr.sin_port = htons(m_port);                 // port: 主机字节序转为网络字节序
}

IPNetAddr::IPNetAddr(const std::string &addr)
{
    // 查找第一个 : 的位置
    size_t i = addr.find_first_not_of(":");
    if (i == addr.npos) {
        LOG_ERROR << "invalid ipv4 addr string: " << addr;
        return;
    }

    m_ip = addr.substr(0, i);
    m_port = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str());

    memset(&m_addr, 0, sizeof(m_addr));

    m_addr.sin_family = AF_INET;                     // ipv4
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());// ip
    m_addr.sin_port = htons(m_port);
}

IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr)
{
    m_ip = std::string(inet_ntoa(m_addr.sin_addr));
    m_port = ntohs(m_addr.sin_port);
}

sockaddr *IPNetAddr::getSockAddr()
{
    return reinterpret_cast<sockaddr *>(&m_addr);
}

socklen_t IPNetAddr::getSocketLen()
{
    return sizeof(m_addr);
}

int IPNetAddr::getFamily()
{
    return AF_INET;
}

std::string IPNetAddr::toString()
{
	std::string re;

	re = m_ip + ":" + std::to_string(m_port);
	return re;
}

}// namespace mrpc