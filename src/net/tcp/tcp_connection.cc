#include "src/net/tcp/tcp_connection.h"
#include "src/common/log.h"
#include "src/net/fd_event_group.h"
#include <string.h>
#include <unistd.h>

namespace mrpc
{

TcpConnection::TcpConnection(EventLoop *event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
    : m_event_loop(event_loop), m_peer_addr(peer_addr), m_state(ConnState::NotConnected), m_fd(fd)
{
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    // 获取 m_fd_event
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    // 设置非阻塞
    m_fd_event->setNonBlocking();
    // 监听读事件
    m_fd_event->listen(FdEvent::EVENT_IN, std::bind(&TcpConnection::onRead, this));
    //
    m_event_loop->addEpollEvent(m_fd_event);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "~TcpConnectio";
}

void TcpConnection::onRead()
{
    /// 从 socket 缓冲区, 调用系统的 read 到 in_buffer
    if (m_state != ConnState::Connected) {
        LOG_ERROR << "onRead error, client has alread disconnected, cliend_fd: " << m_fd << ", addr: " << m_peer_addr->toString();
        return;
    }

    bool is_read_all = false;
    bool is_close = false;
    while (!is_read_all) {
        if (m_in_buffer->wirteAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->getBufferSize());
        }
        int read_count = m_in_buffer->wirteAble();
        // char tmp[read_count + 10];
        // memset(tmp, 0, sizeof(tmp));
        std::string tmp;
        tmp.resize(read_count);
        auto rt = ::read(m_fd, tmp.data(), read_count);


        // 如果有数据读
        if (rt > 0) {
            tmp.resize(rt);     // 缩小 tmp 的大小以匹配实际读取的字节数
            tmp.push_back('\0');// 添加 null 终止字符
            LOG_DEBUG << "success read bytes: " << rt << ", wirteAble: " << read_count << ", tmp: " << tmp;
            m_in_buffer->wirteToBuffer(tmp);
            // 如果没读完, 就继续读
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {// 读完了
                is_read_all = true;
                break;
            }
        } else if (rt == 0) {
            // 对端关闭了连接, 也就是四次挥手的 FIN 报文
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // 读完了
            is_read_all = true;
            break;
        }
    }

    // 关闭连接
    if (is_close) {
        // TODO: 处理关闭连接
        clear();
        LOG_DEBUG << "peer closed, peer addr: " << m_peer_addr->toString();
        return;
    }

    // 没有读完
    if (!is_read_all) {
        LOG_DEBUG << "not read all data!";
    }

    // TODO: 简单的 echo, 后面补充 rpc 解析
    excute();
}

void TcpConnection::excute()
{
    // 将 rpc 请求执行业务逻辑, 获取 rpc 响应, 再把 rpc 响应发送出去
    std::string tmp;
    int size = m_in_buffer->readAble();
    tmp = m_in_buffer->readAsString(size);

    m_out_buffer->wirteToBuffer(tmp);
    m_fd_event->listen(FdEvent::EVENT_OUT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::onWrite()
{
    /// 将当前 out_buffer 里面的数据发送给client
    if (m_state != ConnState::Connected) {
        LOG_ERROR << "onWrite error, client has alread disconnected, cliend_fd: " << m_fd << ", addr: " << m_peer_addr->toString();
        return;
    }

    bool is_write_all = false;
    while (!is_write_all) {
        if (m_out_buffer->readAble() == 0) {
            LOG_DEBUG << "no data need to send to client: " << m_peer_addr->toString();
            is_write_all = true;
            break;
        }
        int write_size = m_out_buffer->readAble();
        std::string tmp = m_out_buffer->peekAsString(write_size);
        int rt = ::write(m_fd, tmp.c_str(), write_size);
        // 发送完了
        if (rt >= write_size) {
            LOG_DEBUG << "no data need to send to client: " << m_peer_addr->toString();
            is_write_all = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // 发送缓存区已满, 不能再次发送
            // 等下次fd可写的时候再次发送数据
            LOG_ERROR << "write data error, erron info: " << strerror(errno);
            break;
        }
    }

    if (is_write_all) {
        // 删除事件(水平触发)
        m_fd_event->cancle(FdEvent::EVENT_OUT);
        m_event_loop->addEpollEvent(m_fd_event);
    }
}

void TcpConnection::clear()
{
    // 处理一些关闭连接后的清理工作
    if (m_state == ConnState::Closed) {
        return;
    }
    m_fd_event->cancle(FdEvent::EVENT_IN);
    m_fd_event->cancle(FdEvent::EVENT_OUT);
    m_event_loop->delEpollEvent(m_fd_event);
    m_state = ConnState::Closed;
}

void TcpConnection::shutdown()
{
    // 服务器主动关闭连接
    if (m_state == ConnState::Closed || m_state == NotConnected) {
        return;
    }

    // 处于半关闭
    m_state = ConnState::HalfClosing;

    // 调用 shutdown 关闭读写, 意味着服务器不会再对这个fd进行读写操作了
    // 发送了 FIN 报文, 出发了四次挥手的第一个阶段
    // 当 fd 发送可读事件, 但是可读的数据为 0, 即对端发送了 FIN
    ::shutdown(m_fd, SHUT_RDWR);
}

}// namespace mrpc