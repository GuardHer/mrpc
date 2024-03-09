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

    setNonBlocking(m_listenfd);

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

int TcpAcceptor::accept()
{
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

        IPNetAddr peer_addr(client_addr);

        LOG_INFO << "a client have accepted success, peer addr: " << peer_addr.toString();
    } else {
        // TODO: 其他协议
    }
    return client_fd;
}

int TcpAcceptor::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        // 获取套接字属性失败
        LOG_ERROR << "setNonBlocking get socket flag failed: " << strerror(errno);
        return -1;
    }

    flags |= O_NONBLOCK;// 添加非阻塞标志
    if (fcntl(fd, F_SETFL, flags) == -1) {
        // 设置套接字属性失败
        LOG_ERROR << "setNonBlocking set socket O_NONBLOCK failed: " << strerror(errno);
        return -1;
    }

    return 0;
}

}// namespace mrpc