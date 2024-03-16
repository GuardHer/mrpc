#ifndef MRPC_NET_TCP_TCP_CLIENT_H
#define MRPC_NET_TCP_TCP_CLIENT_H

#include "src/net/coder/abstract_protocol.h"
#include "src/net/eventloop.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_buffer.h"
#include "src/net/tcp/tcp_connection.h"

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

    /// @brief 如果 connect 成功, done 会被执行
    void connect();

    /// @brief 发送 message 成功, done 会被执行
    void writeMessage(AbstractProtocol::s_ptr message);

    /// @brief 发送 message 成功, done 会被执行
    void readMessage(const std::string &msg_id);

    /// @brief 退出 loop
    void quitLoop();

    void setConnectionCallBack(const ConnectionCallback &cb) { m_conn_callback = cb; }
    void setCloseCallBack(const CloseCallback &cb) { m_close_callback = cb; }
    void setWriteCompleteCallBack(const WriteCompleteCallback &cb) { m_write_complete_callback = cb; }
    void setReadCallBack(const ReadCallback &cb) { m_read_callback = cb; }

    addr_pair getSocketLocalAddr();

private:
    NetAddr::s_ptr m_peer_addr;
    EventLoop *m_event_loop{nullptr};
    int m_fd{-1};
    FdEvent *m_fd_event{nullptr};
    TcpConnection::s_ptr m_conn;

    ConnectionCallback m_conn_callback;
    CloseCallback m_close_callback;
    WriteCompleteCallback m_write_complete_callback;
    ReadCallback m_read_callback;
};

}// namespace mrpc

#endif// MRPC_NET_TCP_TCP_CLIENT_H