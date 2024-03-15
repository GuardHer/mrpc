#ifndef MRPC_NET_RPC_RPC_CLOSURE_H
#define MRPC_NET_RPC_RPC_CLOSURE_H

#include <functional>
#include <google/protobuf/stubs/callback.h>
#include <memory>

namespace mrpc
{

template<typename ReturnType, typename... Args>
class RpcClosure : public google::protobuf::Closure
{
public:
    typedef std::shared_ptr<RpcClosure> s_ptr;
    typedef std::function<ReturnType(Args...)> Callback;

    explicit RpcClosure(Callback cb, Args... args) : m_cb(std::move(cb)), m_args(args...) {}

    ~RpcClosure() = default;

    void Run() override
    {
        if (m_cb) {
            std::apply(m_cb, m_args);
        }
    }

private:
    Callback m_cb;
    std::tuple<Args...> m_args;
};

}// namespace mrpc

#endif//MRPC_NET_RPC_RPC_CLOSURE_H