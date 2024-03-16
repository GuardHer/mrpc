#ifndef MRPC_NET_TCP_TCP_CLIENT_H
#define MRPC_NET_TCP_TCP_CLIENT_H

#include "src/net/coder/abstract_protocol.h"
#include "src/net/eventloop.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_buffer.h"
#include "src/net/tcp/tcp_connection.h"
#include "src/net/timer_event.h"

#include "src/net/callback.h"

namespace mrpc
{

class TcpClient
{
public:
    typedef std::pair<std::string, uint16_t> addr_pair;
    typedef std::shared_ptr<TcpClient> s_ptr;

public:
    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();

    /// @brief EPOLLOUT 可写事件回调
    void writeCb();

    /// @brief EPOLLOUT 可写事件回调 写法2
    void writeCb2();

    /// @brief
    void reset();

    /// @brief EPOLLERR 错误事件回调
    void errorCb();

    /// @brief 如果 connect 完成, m_conn_callback 会被执行
    void connect();

    /// @brief 发送 message 成功, m_write_complete_callback 会被执行
    void writeMessage(AbstractProtocol::s_ptr message);

    /// @brief 接收 message 成功, m_read_callback 会被执行
    void readMessage(const std::string &msg_id);

    /// @brief 退出 loop
    void quitLoop();

    /// @brief 添加定时任务
    /// @param timer_event
    void addTimerEvent(TimerEvent::s_ptr timer_event);

    std::string getConnectErrorInfo() const { return m_connect_error_info; }
    int getConnectErrorCode() const { return m_connect_error_code; }

    void setConnectionCallBack(const ConnectionCallback &cb) { m_conn_callback = cb; }
    void setCloseCallBack(const CloseCallback &cb) { m_close_callback = cb; }
    void setWriteCompleteCallBack(const WriteCompleteCallback &cb) { m_write_complete_callback = cb; }
    void setReadCallBack(const ReadCallback &cb) { m_read_callback = cb; }

    void initSocketLocalAddr();
    NetAddr::s_ptr getPeerAddr() const { return m_peer_addr; }
    NetAddr::s_ptr getLocalAddr() const { return m_local_addr; }

private:
    NetAddr::s_ptr m_peer_addr;
    NetAddr::s_ptr m_local_addr;
    EventLoop *m_event_loop{nullptr};
    int m_fd{-1};
    FdEvent *m_fd_event{nullptr};
    TcpConnection::s_ptr m_conn;

    ConnectionCallback m_conn_callback;
    CloseCallback m_close_callback;
    WriteCompleteCallback m_write_complete_callback;
    ReadCallback m_read_callback;

    int m_connect_error_code{0};
    std::string m_connect_error_info;
};

}// namespace mrpc

#endif// MRPC_NET_TCP_TCP_CLIENT_H