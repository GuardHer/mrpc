#include "src/net/tcp/tcp_client.h"
#include "src/common/error_code.h"
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

void TcpClient::reset()
{
    m_fd = ::socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlocking();
    m_conn.reset(new TcpConnection(m_event_loop, m_fd, 128, m_peer_addr, nullptr, ConnType::ConnByClient));
}

void TcpClient::connect()
{
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSocketLen());
    if (rt == 0) {
        LOG_DEBUG << "connect [" << m_peer_addr->toString() << "] success!";
        m_conn->setState(ConnState::Connected);
        initSocketLocalAddr();
        if (m_conn_callback) m_conn_callback(m_conn, nullptr);
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // epoll 监听可写事件, 判断错误码
            m_fd_event->listen(FdEvent::EVENT_OUT, std::bind(&TcpClient::writeCb2, this));
            m_fd_event->setErrorCallback(std::bind(&TcpClient::errorCb, this));
            m_event_loop->addEpollEvent(m_fd_event);

            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }

        } else {
            LOG_ERROR << "connect error: " << strerror(errno) << ", fd: " << m_fd;
            m_connect_error_code = ERROR_FAILED_CONNECT;
            m_connect_error_info = "connect error, sys errno info = " + std::string(strerror(errno));
            if (m_conn_callback) m_conn_callback(m_conn, nullptr);
        }
    }
}

void TcpClient::writeCb()
{
    int error = 0;
    socklen_t error_len = sizeof(error);
    ::getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
    if (error == 0) {
        LOG_DEBUG << "connect to [" << m_peer_addr->toString() << "] success!";
        m_conn->setState(ConnState::Connected);
        initSocketLocalAddr();
        LOG_DEBUG << "peer addr [" << m_peer_addr->toString() << "]";
        LOG_DEBUG << "local addr [" << m_local_addr->toString() << "]";
        m_conn->setLocalAddr(m_local_addr);
    } else {
        m_connect_error_code = ERROR_FAILED_CONNECT;
        m_connect_error_info = "connect error, sys errno info = " + std::string(strerror(errno));
        m_fd_event->cancle(FdEvent::EVENT_IN);
        LOG_ERROR << "connect error: " << strerror(errno) << ", fd: " << m_fd;
        // ::close(m_fd);
        // m_fd = ::socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
        // this->reset();
    }
    // m_fd_event->cancle(FdEvent::EVENT_OUT);
    // m_event_loop->addEpollEvent(m_fd_event);

    // 链接成功后先删除epoll事件
    m_event_loop->delEpollEvent(m_fd_event);
    // 执行连接完成回调
    if (m_conn_callback) m_conn_callback(m_conn, nullptr);
}

void TcpClient::writeCb2()
{
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSocketLen());
    if ((rt < 0 && errno == EISCONN) || (rt == 0)) {
        LOG_DEBUG << "connect to [" << m_peer_addr->toString() << "] success!";
        m_conn->setState(ConnState::Connected);
        initSocketLocalAddr();
        LOG_DEBUG << "peer addr [" << m_peer_addr->toString() << "]";
        LOG_DEBUG << "local addr [" << m_local_addr->toString() << "]";
        m_conn->setLocalAddr(m_local_addr);
    } else {
        if (errno == ECONNREFUSED) {
            m_connect_error_code = ERROR_PEER_CLOSE;
            m_connect_error_info = "connect refused, errno info = " + std::string(strerror(errno));
        } else {
            m_connect_error_code = ERROR_FAILED_CONNECT;
            m_connect_error_info = "connect error, sys errno info = " + std::string(strerror(errno));
        }
        // ::close(m_fd);
        // m_fd = ::socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
        LOG_ERROR << "connect error: " << strerror(errno) << ", fd: " << m_fd;
    }


    // 链接成功后先删除epoll事件
    m_event_loop->delEpollEvent(m_fd_event);
    // 执行连接完成回调
    if (m_conn_callback) m_conn_callback(m_conn, nullptr);
}

void TcpClient::errorCb()
{
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

void TcpClient::addTimerEvent(TimerEvent::s_ptr timer_event)
{
    m_event_loop->addTimerEvent(timer_event);
}


void TcpClient::initSocketLocalAddr()
{
    sockaddr_in local_addr_in;
    socklen_t local_addr_len = sizeof(local_addr_in);
    if (::getsockname(m_fd, reinterpret_cast<sockaddr *>(&local_addr_in), &local_addr_len) == -1) {
        LOG_ERROR << "Error in getsockname : " << strerror(errno) << ", fd: " << m_fd;

        return;
    }

    // 打印本地地址和端口号
    char local_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local_addr_in.sin_addr, local_ip, INET_ADDRSTRLEN);
    uint16_t local_port = ntohs(local_addr_in.sin_port);

    m_local_addr = std::make_shared<IPNetAddr>(std::string(local_ip), local_port);
}

}// namespace mrpc