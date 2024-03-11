#ifndef MRPC_NET_TCP_TCP_ACCEPTOR_H
#define MRPC_NET_TCP_TCP_ACCEPTOR_H

#include "src/net/tcp/net_addr.h"
#include <memory>

namespace mrpc
{

class TcpAcceptor
{
public:
    typedef std::shared_ptr<TcpAcceptor> s_ptr;

    TcpAcceptor(NetAddr::s_ptr addr);
    ~TcpAcceptor();

    std::pair<int, NetAddr::s_ptr> accept();
    int getListenFd();

private:
    NetAddr::s_ptr m_addr;// 服务器监听地址
    int m_listenfd{-1};   // 监听套接字
    int m_family{-1};     // 协议
};

}// namespace mrpc

#endif//MRPC_NET_TCP_TCP_ACCEPTOR_H