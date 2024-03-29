#include "src/net/tcp/tcp_acceptor.h"
#include "src/common/log.h"
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace mrpc
{

TcpAcceptor::TcpAcceptor(NetAddr::s_ptr addr)
    : m_addr(addr)
{
    // 检查addr是否合法
    if (!addr->checkValid()) {
        LOG_FATAL << "invalid addr: " << addr->toString();
    }

    // 获取协议
    m_family = m_addr->getFamily();

    // 创建套接字
    m_listenfd = socket(m_family, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        LOG_FATAL << "invalid listenfd: " << m_listenfd;
    }

    int val = 1;
    // SO_REUSEADDR: 地址复用;
    if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        LOG_ERROR << "setsockopt error, error info: " << strerror(errno);
    }

    // bind
    socklen_t len = m_addr->getSocketLen();
    if (bind(m_listenfd, m_addr->getSockAddr(), len) != 0) {
        LOG_FATAL << "bind error: error info: " << strerror(errno);
    }

    // listen
    if (listen(m_listenfd, 1000) != 0) {
        LOG_FATAL << "listen error: error info: " << strerror(errno);
    }
}

TcpAcceptor::~TcpAcceptor()
{
}

int TcpAcceptor::getListenFd()
{
    return m_listenfd;
}

std::pair<int, NetAddr::s_ptr> TcpAcceptor::accept()
{
    std::pair<int, NetAddr::s_ptr> res;
    int client_fd = 0;
    // ipv4
    if (m_family == AF_INET) {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);

        client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len);
        if (client_fd < 0) {
            LOG_ERROR << "accept error: error info: " << strerror(errno);
        }

        IPNetAddr::s_ptr peer_addr = std::make_shared<IPNetAddr>(client_addr);
        res.second                 = peer_addr;

        LOG_INFO << "a client have accepted success, peer addr: " << peer_addr->toString();
    } else {
        // TODO: 其他协议
    }
    res.first = client_fd;
    return res;
}


}// namespace mrpc