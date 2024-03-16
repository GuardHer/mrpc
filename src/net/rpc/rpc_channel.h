#ifndef MRPC_NET_RPC_RPC_CHANNEL_H
#define MRPC_NET_RPC_RPC_CHANNEL_H

#include "src/net/callback.h"
#include "src/net/rpc/rpc_closure.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_client.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include <memory>

namespace mrpc
{

class RpcChannel : public google::protobuf::RpcChannel
{
public:
    RpcChannel(NetAddr::s_ptr peer_addr);
    ~RpcChannel();

public:
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                    google::protobuf::Message *response, google::protobuf::Closure *done) override;

private:
    void onConnection(const TcpConnectionPtr &conn, const AbstractProtocolPtr &message);

    void onWriteComplete(const AbstractProtocolPtr &message);

    void onRead(const AbstractProtocolPtr &message);

private:
    NetAddr::s_ptr m_peer_addr{nullptr};
    NetAddr::s_ptr m_local_addr{nullptr};

    std::shared_ptr<TcpClient> m_client;
    RpcClosure<void> *m_done{nullptr};
};

}// namespace mrpc

#endif