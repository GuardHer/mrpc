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

class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel>
{
public:
    typedef std::shared_ptr<mrpc::RpcChannel> s_ptr;
    typedef std::shared_ptr<google::protobuf::RpcController> controller_s_ptr;
    typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
    typedef std::shared_ptr<google::protobuf::Closure> closure_s_ptr;

public:
    RpcChannel(NetAddr::s_ptr peer_addr);
    ~RpcChannel();

public:
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                    google::protobuf::Message *response, google::protobuf::Closure *done) override;

    void Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done);

    TcpClient *getTcpClient() const;

private:
    void onConnection(const TcpConnectionPtr &conn, const AbstractProtocolPtr &message);

    void onWriteComplete(const AbstractProtocolPtr &message);

    void onRead(const AbstractProtocolPtr &message);

private:
    bool m_is_init{false};
    NetAddr::s_ptr m_peer_addr{nullptr};         // 对端地址
    NetAddr::s_ptr m_local_addr{nullptr};        // 本地地址
    std::shared_ptr<TcpClient> m_client{nullptr};// tcp client
    controller_s_ptr m_controller{nullptr};      // controller
    message_s_ptr m_request{nullptr};            // request
    message_s_ptr m_response{nullptr};           // response
    closure_s_ptr m_closure{nullptr};            // done
};

}// namespace mrpc

#endif