#ifndef MRPC_NET_TCP_TCP_SERVER_H
#define MRPC_NET_TCP_TCP_SERVER_H

#include "src/net/eventloop.h"
#include "src/net/io_thread_pool.h"
#include "src/net/tcp/tcp_acceptor.h"
#include "src/net/tcp/tcp_connection.h"

#include <set>

namespace mrpc
{

class TcpServer
{
public:
    TcpServer(NetAddr::s_ptr addr);
    ~TcpServer();

    void start();

private:
    void init();

    void onAccept();

private:
    TcpAcceptor::s_ptr m_acceptor;
    NetAddr::s_ptr m_addr;// listen addr
    FdEvent *listen_fd_event;
    EventLoop *m_main_event_loop{nullptr};// main reactor
    IOThreadPool *m_io_threads{nullptr};  // subReactors
    int m_client_counts{0};

    std::set<TcpConnection::s_ptr> m_clients;
};


}// namespace mrpc

#endif//MRPC_NET_TCP_TCP_SERVER_H