#ifndef MRPC_NET_TCP_TCP_CONNECTION_H
#define MRPC_NET_TCP_TCP_CONNECTION_H

#include "src/net/callback.h"
#include "src/net/eventloop.h"
#include "src/net/io_thread.h"
#include "src/net/tcp/abstract_coder.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_buffer.h"
#include <memory>
#include <queue>
#include <vector>

namespace mrpc
{

enum ConnState
{
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 3,
};

enum ConnType
{
    ConnByServer = 1,// server
    ConnByClient = 2,// client
};

class TcpConnection
{
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;
    typedef std::pair<AbstractProtocol::s_ptr, WriteCompleteCallback> write_callback_pair;

public:
    TcpConnection(EventLoop *event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, ConnType type = ConnType::ConnByServer);
    ~TcpConnection();

public:
    /// @brief 可读事件函数
    void onRead();

    /// @brief
    void excute();

    /// @brief 可写事件函数
    void onWrite();

    /// @brief 启动监听可写
    void listenWrite();

    /// @brief 启动监听可读
    void listenRead();

    /// @brief
    void clear();

    /// @brief 服务器主动关闭连接
    void shutdown();

    /// @brief 设置 m_state
    /// @param state
    void setState(const ConnState &state) { m_state = state; }

    /// @brief 获取 m_state
    /// @return m_state
    ConnState getState() const { return m_state; }

    /// @brief 设置 m_conn_type
    /// @param state
    void setConnType(const ConnType &type) { m_conn_type = type; }

    /// @brief 获取 m_conn_type
    /// @return m_conn_type
    ConnType getConnType() const { return m_conn_type; }

    void pushWriteMessage(write_callback_pair cb_pair) { m_write_callbask.push_back(cb_pair); }
    void pushReadMessage(const std::string &req_id, ReadCallback cb) { m_read_callbask.insert(std::make_pair(req_id, cb)); }


private:
    int m_fd{-1};                                // socket
    EventLoop *m_event_loop{nullptr};            //
    NetAddr::s_ptr m_addr;                       //
    NetAddr::s_ptr m_peer_addr;                  //
    TcpBuffer::s_ptr m_in_buffer;                // 接收缓冲区
    TcpBuffer::s_ptr m_out_buffer;               // 发送缓冲区
    FdEvent *m_fd_event{nullptr};                //
    ConnState m_state{ConnState::Connected};     //
    ConnType m_conn_type{ConnType::ConnByServer};//
    AbstractCoder *m_coder{nullptr};             // 编解码
    // std::pair<AbstractProtocol::s_ptr, std::function<AbstractProtocol::s_ptr>>;
    std::vector<write_callback_pair> m_write_callbask;
    std::map<std::string, ReadCallback> m_read_callbask;
};

}// namespace mrpc

#endif//MRPC_NET_TCP_TCP_CONNECTION_H