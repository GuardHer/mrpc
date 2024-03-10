#include "src/net/tcp/tcp_connection.h"
#include "src/common/log.h"
#include "src/net/fd_event_group.h"
#include <string.h>
#include <unistd.h>

namespace mrpc
{

TcpConnection::TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
    : m_io_thread(io_thread), m_peer_addr(peer_addr), m_state(ConnState::NotConnected), m_fd(fd)
{
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    // 获取 m_fd_event
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    // 设置非阻塞
    m_fd_event->setNonBlocking();
    // 监听读事件
    m_fd_event->listen(FdEvent::EVENT_IN, std::bind(&TcpConnection::onRead, this));
}

TcpConnection::~TcpConnection()
{
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
        char tmp[read_count + 10];
        auto rt = ::read(m_fd, tmp, read_count);
        LOG_DEBUG << "success read bytes: " << rt;

        // 如果有数据读
        if (rt > 0) {
            m_in_buffer->wirteToBuffer(&tmp[0], rt);
            // 如果没读完, 就继续读
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {// 读完了
                is_read_all = true;
                break;
            }
        } else {// 没数据读
            is_close = true;
        }
    }

    // 关闭连接
    if (is_close) {
        // TODO: 处理关闭连接
        LOG_DEBUG << "peer closed, peer addr: " << m_peer_addr->toString();
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


    LOG_INFO << "success get request: " << tmp << " , form client addr: " << m_peer_addr->toString();

    m_out_buffer->wirteToBuffer(tmp.c_str(), tmp.length());

    m_fd_event->listen(FdEvent::EVENT_OUT, std::bind(&TcpConnection::onWrite, this));
}

void TcpConnection::onWrite()
{
    /// 将当前 out_buffer 里面的数据发送给client
    if (m_state != ConnState::Connected) {
        LOG_ERROR << "onWrite error, client has alread disconnected, cliend_fd: " << m_fd << ", addr: " << m_peer_addr->toString();
        return;
    }

    while (true) {
        if (m_out_buffer->readAble() == 0) {
            LOG_DEBUG << "no data need to send to client: " << m_peer_addr->toString();
            break;
        }
        int write_size = m_out_buffer->readAble();
        std::string tmp = m_out_buffer->peekAsString(write_size);
        int rt = ::write(m_fd, tmp.c_str(), write_size);
        // 发送完了
        if (rt >= write_size) {
            LOG_DEBUG << "no data need to send to client: " << m_peer_addr->toString();
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // 发送缓存区已满, 不能再次发送
            // 等下次fd可写的时候再次发送数据
            LOG_ERROR << "write data error, erron info: " << strerror(errno);
            break;
        }
    }
}

}// namespace mrpc