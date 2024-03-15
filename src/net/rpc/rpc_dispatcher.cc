#include "src/net/rpc/rpc_dispatcher.h"
#include "src/common/log.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace mrpc
{


void RpcDispatcher::dispatch(const AbstractProtocol::s_ptr &request, AbstractProtocol::s_ptr response)
{
    if (!request || !response) {
        // TODO
    }

    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    std::string full_name = req_protocol->m_method_name;
    std::string service_name;
    std::string method_name;
    if (!parseServiceFullName(full_name, service_name, method_name)) {
        // TODO: 错误
    }

    auto it = m_services.find(service_name);
    if (it == m_services.end()) {
        // TODO:
    }

    service_s_ptr service = it->second;
    auto method = service->GetDescriptor()->FindMethodByName(method_name);
    if (method == nullptr) {
        // TODO:
    }

    auto req_message = service->GetRequestPrototype(method).New();

    // 反序列化, 将 Pb_data 反序列化为 req_message
    if (!req_message->ParseFromString(req_protocol->m_pb_data)) {
        // TODO:
    }

    LOG_INFO << "req id: " << req_protocol->m_req_id << ", get rpc request: " << req_message->ShortDebugString();

    auto rsp_message = service->GetResponsePrototype(method).New();
    service->CallMethod(method, nullptr, req_message, rsp_message, nullptr);

    rsp_protocol->m_req_id = req_protocol->m_req_id;
    rsp_protocol->m_req_id_len = req_protocol->m_req_id_len;
    rsp_protocol->m_method_name = req_protocol->m_method_name;
    rsp_protocol->m_method_len = req_protocol->m_method_len;
    rsp_protocol->m_error_code = 0;

    // 序列化到 rsp_protocol->m_pb_data
    rsp_message->SerializeToString(&(rsp_protocol->m_pb_data));
}

void RpcDispatcher::registerService(const service_s_ptr &service)
{
    // return service name; if have pack, return pack.service_name
    std::string service_name = service->GetDescriptor()->full_name();
    m_services[service_name] = service;
}

bool RpcDispatcher::parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name)
{
    if (full_name.empty()) {
        LOG_ERROR << "full_name is empty!";
        return false;
    }
    auto index = full_name.find_last_of(".");
    if (index == full_name.npos) {
        LOG_ERROR << "not find . in full_name!";
        return false;
    }
    service_name = full_name.substr(0, index);
    method_name = full_name.substr(index + 1, full_name.length() - index - 1);

    LOG_INFO << "parse service_name: " << service_name << ", method_name: " << method_name;
    return true;
}


}// namespace mrpc