#include "src/net/tcp/tcp_connection.h"
#include "src/common/log.h"
#include "src/net/coder/tinypb_coder.h"
#include "src/net/fd_event_group.h"
#include <string.h>
#include <unistd.h>

namespace mrpc
{

TcpConnection::TcpConnection(EventLoop *event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, ConnType type)
    : m_fd(fd), m_event_loop(event_loop), m_local_addr(local_addr), m_peer_addr(peer_addr), m_state(ConnState::NotConnected), m_conn_type(type)
{

    m_coder = new TinyPBCoder();

    m_in_buffer  = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    // 获取 m_fd_event
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    // 设置非阻塞
    m_fd_event->setNonBlocking();

    if (m_conn_type == ConnType::ConnByServer) {
        listenRead();
    }
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "~TcpConnection, addr: " << this->m_peer_addr->toString();
    m_peer_addr.reset();
    m_local_addr.reset();
    m_in_buffer.reset();
    m_out_buffer.reset();
    if (m_coder) {
        delete m_coder;
        m_coder = nullptr;
    }
}

void TcpConnection::onRead()
{
    LOG_INFO << "onRead!";
    /// 从 socket 缓冲区, 调用系统的 read 到 in_buffer
    if (m_state != ConnState::Connected) {
        LOG_ERROR << "onRead error, client has alread disconnected, cliend_fd: " << m_fd << ", addr: " << m_peer_addr->toString();
        clear();
        return;
    }

    bool is_read_all = false;
    bool is_close    = false;
    while (!is_read_all) {
        if (m_in_buffer->writAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->getBufferSize());
        }
        int read_count = m_in_buffer->writAble();
        // char tmp[read_count + 10];
        // memset(tmp, 0, sizeof(tmp));
        std::string tmp;
        tmp.resize(read_count);
        auto rt = ::read(m_fd, tmp.data(), read_count);


        // 如果有数据读
        if (rt > 0) {
            tmp.resize(rt);// 缩小 tmp 的大小以匹配实际读取的字节数
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
        ::close(m_fd_event->getFd());
        if (m_close_cb) {
            m_close_cb(shared_from_this());
        }
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
    if (m_conn_type == ConnType::ConnByServer) {
        // 将 rpc 请求执行业务逻辑, 获取 rpc 响应, 再把 rpc 响应发送出去
        std::vector<AbstractProtocol::s_ptr> result;
        std::vector<AbstractProtocol::s_ptr> reply_result;

        m_coder->decode(result, m_in_buffer);
        LOG_DEBUG << "result size: " << result.size();
        for (auto re: result) {
            // 针对每一个请求, 调用 rpc 方法, 获取响应 message
            // 将响应 message 放入到发送缓冲区， 监听可写事件回包
            std::shared_ptr<TinyPBProtocol> message = std::dynamic_pointer_cast<TinyPBProtocol>(re);
            RpcDispatcher::GetRpcDispatcher()->dispatch(re, message, this);

            reply_result.push_back(message);
        }
        m_coder->encode(reply_result, m_out_buffer);
        listenWrite();
    } else {
        // 从buffer里 decode 得到message, 执行回调
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);

        for (auto re: result) {
            std::string msg_id = re->m_msg_id;
            auto it            = m_read_callbask.find(msg_id);
            if (it != m_read_callbask.end()) {
                if (it->second) it->second(re);
            }
        }
    }
}

void TcpConnection::onWrite()
{
    /// 将当前 out_buffer 里面的数据发送给client
    if (m_state != ConnState::Connected) {
        LOG_ERROR << "onWrite error, client has alread disconnected, cliend_fd: " << m_fd << ", addr: " << m_peer_addr->toString();
        clear();
        return;
    }

    if (m_conn_type == ConnType::ConnByClient) {
        // 将message encode 得到字节流
        // 将字节流写入到buffer
        std::vector<AbstractProtocol::s_ptr> messages;
        for (auto tmp: m_write_callbask) {
            messages.push_back(tmp.first);
        }
        m_coder->encode(messages, m_out_buffer);
    }

    bool is_write_all = false;
    while (!is_write_all) {
        if (m_out_buffer->readAble() == 0) {
            LOG_DEBUG << "no data need to send to client: " << m_peer_addr->toString();
            is_write_all = true;
            break;
        }
        int write_size  = m_out_buffer->readAble();
        std::string tmp = m_out_buffer->peekAsString(write_size);
        int rt          = ::write(m_fd, tmp.c_str(), write_size);
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

    if (m_conn_type == ConnType::ConnByClient) {
        for (auto tmp: m_write_callbask) {
            auto func = tmp.second;
            if (func) {
                func(tmp.first);
            }
        }
        m_write_callbask.clear();
    }
}

void TcpConnection::listenWrite()
{
    // 监听写事件
    m_fd_event->listen(FdEvent::EVENT_OUT, std::bind(&TcpConnection::onWrite, this));
    // 添加到epoll
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::listenRead()
{
    // 监听读事件
    m_fd_event->listen(FdEvent::EVENT_IN, std::bind(&TcpConnection::onRead, this));
    // 添加到epoll
    m_event_loop->addEpollEvent(m_fd_event);
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

    // ::close(m_fd_event->getFd());
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