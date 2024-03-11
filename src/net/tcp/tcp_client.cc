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

    m_conn = std::make_shared<TcpConnection>(m_event_loop, m_fd, 1024, m_peer_addr);
    m_conn->setConnType(ConnType::ConnByClient);
}

TcpClient::~TcpClient()
{
    if (m_fd) {
        ::close(m_fd);
    }
}

void TcpClient::connect(std::function<void()> done)
{
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSocketLen());
    if (rt == 0) {
        LOG_DEBUG << "connect [" << m_peer_addr->toString() << "] success!";
        if (done) done();
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // epoll 监听可写事件, 判断错误码
            m_fd_event->listen(FdEvent::EVENT_OUT, [this, done]() {
                int error = 0;
                socklen_t error_len = sizeof(error);
                ::getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if (error == 0) {
                    LOG_DEBUG << "connect [" << m_peer_addr->toString() << "] success!";
                    if (done) done();
                } else {
                    LOG_ERROR << "connect error: " << strerror(errno) << ", fd: " << m_fd;
                }
                m_fd_event->cancle(FdEvent::EVENT_OUT);
                m_event_loop->addEpollEvent(m_fd_event);
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

void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
{
}

void TcpClient::readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
{
}

}// namespace mrpc