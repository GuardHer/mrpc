#ifndef MRPC_NET_TCP_TCP_CONNECTION_H
#define MRPC_NET_TCP_TCP_CONNECTION_H

#include "src/net/io_thread.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_buffer.h"

namespace mrpc
{
enum ConnState
{
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 3,
};

class TcpConnection
{
public:
public:
    TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
    ~TcpConnection();

public:
    /// @brief 可读事件函数
    void onRead();
    /// @brief
    void excute();
    /// @brief 可写事件函数
    void onWrite();

    /// @brief 设置 m_state
    /// @param state
    void setState(const ConnState &state) { m_state = state; }
    /// @brief 获取 m_state
    /// @return m_state
    ConnState getState() const { return m_state; }

private:
    NetAddr::s_ptr m_addr;         //
    NetAddr::s_ptr m_peer_addr;    //
    TcpBuffer::s_ptr m_in_buffer;  // 接收缓冲区
    TcpBuffer::s_ptr m_out_buffer; // 发送缓冲区
    IOThread *m_io_thread{nullptr};// 持有该连接的io线程
    FdEvent *m_fd_event{nullptr};  //
    ConnState m_state;             //
    int m_fd{-1};                  //
};

}// namespace mrpc

#endif//MRPC_NET_TCP_TCP_CONNECTION_H