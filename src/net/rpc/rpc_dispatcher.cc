#include "src/net/rpc/rpc_dispatcher.h"
#include "src/common/error_code.h"
#include "src/common/log.h"
#include "src/net/rpc/rpc_closure.h"
#include "src/net/rpc/rpc_controller.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_connection.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

namespace mrpc
{


void RpcDispatcher::dispatch(const AbstractProtocol::s_ptr &request, AbstractProtocol::s_ptr response, const TcpConnection *conn)
{
    if (!request || !response) {
        // TODO
    }

    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    std::string full_name = req_protocol->m_method_name;
    std::string service_name;
    std::string method_name;

    // 初始化 rsp_protocol
    rsp_protocol->m_req_id = req_protocol->m_req_id;
    rsp_protocol->m_req_id_len = req_protocol->m_req_id_len;
    rsp_protocol->m_method_name = req_protocol->m_method_name;
    rsp_protocol->m_method_len = req_protocol->m_method_len;

    if (!parseServiceFullName(full_name, service_name, method_name)) {
        // 解析错误
        LOG_ERROR << "[" << req_protocol->m_req_id << "] parse service_name or method_name failed: " << full_name;
        setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service_name or method_name failed");
        return;
    }

    auto it = m_services.find(service_name);
    if (it == m_services.end()) {
        // service 不存在
        LOG_ERROR << "[" << req_protocol->m_req_id << "] service not found: " << service_name;
        setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }

    service_s_ptr service = it->second;
    auto method = service->GetDescriptor()->FindMethodByName(method_name);
    if (method == nullptr) {
        // method 不存在
        LOG_ERROR << "[" << req_protocol->m_req_id << "] method not found: " << method_name << ", in service: " << service_name;
        setTinyPBError(rsp_protocol, ERROR_METHOD_NOT_FOUND, "method not found");
        return;
    }

    auto req_message = service->GetRequestPrototype(method).New();

    // 反序列化, 将 Pb_data 反序列化为 req_message
    if (!req_message->ParseFromString(req_protocol->m_pb_data)) {
        // 反序列化失败
        LOG_ERROR << "[" << req_protocol->m_req_id << "] deserilize failed: " << service_name;
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize failed");
        if (req_message) {
            delete req_message;
            req_message = nullptr;
        }
        return;
    }

    auto rsp_message = service->GetResponsePrototype(method).New();
    RpcController rpcController;

    rpcController.SetLocalAddr(conn->getLocalAddr());
    rpcController.SetPeerAddr(conn->getPeerAddr());
    rpcController.SetReqId(req_protocol->m_req_id);

    std::function<void()> reply_package_func = []() {
        LOG_INFO << "reply_package_func";
    };
    RpcClosure<void> closure(reply_package_func);


    service->CallMethod(method, &rpcController, req_message, rsp_message, &closure);

    // 序列化到 rsp_protocol->m_pb_data
    if (!rsp_message->SerializeToString(&(rsp_protocol->m_pb_data))) {
        // 序列化失败
        LOG_ERROR << "[" << req_protocol->m_req_id << "] serilize failed: " << service_name;
        setTinyPBError(rsp_protocol, ERROR_FAILED_SERIALIZE, "serilize failed");
        if (req_message) {
            delete req_message;
            req_message = nullptr;
        }
        if (rsp_message) {
            delete rsp_message;
            rsp_message = nullptr;
        }
        return;
    }

    rsp_protocol->m_error_code = 0;
    LOG_INFO << "req id: " << req_protocol->m_req_id << ", get rpc request: " << req_message->ShortDebugString();

    delete req_message;
    delete rsp_message;
    req_message = nullptr;
    rsp_message = nullptr;
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

void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> message, int32_t error_code, const std::string &error_info)
{
    message->m_error_code = error_code;
    message->m_error_info = error_info;
    message->m_error_info_len = static_cast<int32_t>(error_info.length());
}


}// namespace mrpc