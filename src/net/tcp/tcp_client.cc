#include "src/net/tcp/tcp_client.h"
#include "src/common/log.h"
#include "src/net/fd_event_group.h"

#include <string.h>
#include <sys/socket.h>

namespace mrpc
{

TcpClient::TcpClient(NetAddr::s_ptr peer_addr)
    : m_peer_addr(peer_addr)
{
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = ::socket(peer_addr->getFamily(), SOCK_STREAM, 0);

    if (m_fd < 0) {
        LOG_ERROR << "";
        return;
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlocking();

    m_conn = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, m_peer_addr, nullptr, ConnType::ConnByClient);
}

TcpClient::~TcpClient()
{
    LOG_INFO << "~TcpClient";
    if (m_fd) {
        ::close(m_fd);
    }
}

void TcpClient::connect()
{
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSocketLen());
    if (rt == 0) {
        LOG_DEBUG << "connect [" << m_peer_addr->toString() << "] success!";
        m_conn->setState(ConnState::Connected);
        if (m_conn_callback) m_conn_callback(m_conn, nullptr);
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // epoll 监听可写事件, 判断错误码
            m_fd_event->listen(FdEvent::EVENT_OUT, [this]() {
                int error = 0;
                bool is_conned = false;
                socklen_t error_len = sizeof(error);
                ::getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if (error == 0) {
                    LOG_DEBUG << "connect [" << m_peer_addr->toString() << "] success!";
                    is_conned = true;
                    m_conn->setState(ConnState::Connected);
                    auto addr_pair = getSocketLocalAddr();
                    auto addr = std::make_shared<IPNetAddr>(addr_pair.first, addr_pair.second);
                    LOG_DEBUG << "local addr [" << addr_pair.first << ":" << addr_pair.second << "]";
                    m_conn->setLocalAddr(addr);
                } else {
                    m_fd_event->cancle(FdEvent::EVENT_IN);
                    LOG_ERROR << "connect error: " << strerror(errno) << ", fd: " << m_fd;
                }
                m_fd_event->cancle(FdEvent::EVENT_OUT);
                m_event_loop->addEpollEvent(m_fd_event);

                if (is_conned) {
                    if (m_conn_callback) m_conn_callback(m_conn, nullptr);
                }
            });
            m_event_loop->addEpollEvent(m_fd_event);

            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }

        } else {
            LOG_ERROR << "connect error: " << strerror(errno) << ", fd: " << m_fd;
        }
    }
}

void TcpClient::writeMessage(AbstractProtocol::s_ptr message)
{
    // 把message对象写入到connect的buffer中, 启动connect可写事件
    m_conn->pushWriteMessage(std::make_pair(message, m_write_complete_callback));
    m_conn->listenWrite();
}

void TcpClient::readMessage(const std::string &msg_id)
{
    // 监听可读事件
    // 从buffer里 decode 得到message
    m_conn->pushReadMessage(msg_id, m_read_callback);
    m_conn->listenRead();
}

void TcpClient::quitLoop()
{
    if (m_event_loop->isLooping()) {
        m_event_loop->quit();
        m_event_loop->wakeup();
    }
}

TcpClient::addr_pair TcpClient::getSocketLocalAddr()
{
    addr_pair result;
    auto local_addr_in = m_peer_addr->getSockAddrIn();
    socklen_t local_addr_len = m_peer_addr->getSocketLen();
    if (::getsockname(m_fd, m_peer_addr->getSockAddr(), &local_addr_len) == -1) {
        LOG_ERROR << "Error in getsockname(): " << strerror(errno);
        return result;
    }

    // 打印本地地址和端口号
    char local_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local_addr_in.sin_addr, local_ip, INET_ADDRSTRLEN);
    uint16_t local_port = ntohs(local_addr_in.sin_port);

    result.first = std::string(local_ip);
    result.second = local_port;

    return result;
}

}// namespace mrpc