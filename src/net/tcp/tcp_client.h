#ifndef MRPC_NET_TCP_TCP_CLIENT_H
#define MRPC_NET_TCP_TCP_CLIENT_H

#include "src/net/eventloop.h"
#include "src/net/tcp/abstract_protocol.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_connection.h"

namespace mrpc
{

class TcpClient
{
public:
    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();

    /// @brief 如果 connect 成功, done 会被执行
    void connect(std::function<void()> done);

    /// @brief 发送 message 成功, done 会被执行
    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    /// @brief 发送 message 成功, done 会被执行
    void readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);


private:
    NetAddr::s_ptr m_peer_addr;
    EventLoop *m_event_loop{nullptr};
    int m_fd{-1};
    FdEvent *m_fd_event{nullptr};
    TcpConnection::s_ptr m_conn;
};

}// namespace mrpc

#endif// MRPC_NET_TCP_TCP_CLIENT_H