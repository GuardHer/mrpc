#ifndef MRPC_NET_RPC_RPC_DISPATCHER_H
#define MRPC_NET_RPC_RPC_DISPATCHER_H

#include "src/net/coder/tinypb_protocol.h"
#include <map>
#include <memory>

#include <google/protobuf/service.h>

namespace mrpc
{

class RpcDispatcher
{
public:
    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;

public:
    /// @brief
    /// @param request  请求
    /// @param response 响应
    void dispatch(const AbstractProtocol::s_ptr &request, AbstractProtocol::s_ptr response);

    /// @brief 注册一个service
    /// @param service
    void registerService(const service_s_ptr &service);

private:
    /// @brief 从 full_name, 解析出 service 名和 method 名
    /// @param full_name: 全名 (eg: order.getorder)
    /// @param service_name: service 名 (eg: order), 传出参数
    /// @param method_name: method 名 (eg: getorder), 传出参数
    bool parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name);

private:
    std::map<std::string, service_s_ptr> m_services;
};

}// namespace mrpc

#endif//MRPC_NET_RPC_RPC_DISPATCHER_H