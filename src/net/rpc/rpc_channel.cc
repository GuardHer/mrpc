#include "src/net/rpc/rpc_channel.h"
#include "src/common/error_code.h"
#include "src/common/log.h"
#include "src/common/msg_id_util.h"
#include "src/net/coder/tinypb_protocol.h"
#include "src/net/rpc/rpc_controller.h"
#include "src/net/tcp/tcp_connection.h"
#include <google/protobuf/message.h>

namespace mrpc
{

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr)
{
    m_client = std::make_shared<TcpClient>(peer_addr);
}

RpcChannel::~RpcChannel()
{
    LOG_INFO << "~RpcChannel";
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                            google::protobuf::Message *response, google::protobuf::Closure *done)
{
    std::shared_ptr<TinyPBProtocol> req_protocol = std::make_shared<TinyPBProtocol>();
    auto my_controller                           = dynamic_cast<RpcController *>(controller);
    if (my_controller == nullptr) {
        LOG_ERROR << "failed callmethod, RpcController convert error";
        return;
    }

    /// 设置 m_msg_id 和 m_msg_id_len
    if (my_controller->GetMsgId().empty()) {
        req_protocol->m_msg_id = MsgIdUtil::GenMsgId();
        my_controller->SetMsgId(req_protocol->m_msg_id);
    } else {
        req_protocol->m_msg_id = my_controller->GetMsgId();
    }
    req_protocol->m_msg_id_len = static_cast<int32_t>(req_protocol->m_msg_id.length());

    /// 设置 method_name 和 m_method_len
    req_protocol->m_method_name = method->full_name();
    req_protocol->m_method_len  = static_cast<int32_t>(req_protocol->m_method_name.length());
    LOG_INFO << req_protocol->m_msg_id << " | call method name [" << req_protocol->m_method_name << "]";

    /// 序列化
    if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
        std::string err_info = "failed to SerializeToString";
        my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
        LOG_ERROR << req_protocol->m_msg_id << " | failed to SerializeToString";
        return;
    }

    /// 设置 req_protocol->m_pk_len
    int32_t pb_data_len    = static_cast<int32_t>(req_protocol->m_pb_data.length());
    req_protocol->m_pk_len = 26 + pb_data_len + req_protocol->m_method_len + req_protocol->m_msg_id_len + req_protocol->m_error_info_len;

    /// 定时任务
    m_timer_event = std::make_shared<TimerEvent>(my_controller->GetTimeout(), false, std::bind(&RpcChannel::onTimeout, this));
    m_client->addTimerEvent(m_timer_event);

    /// 创建 client
    m_client->setConnectionCallBack(std::bind(&RpcChannel::onConnection, this, std::placeholders::_1, req_protocol));
    m_client->setWriteCompleteCallBack(std::bind(&RpcChannel::onWriteComplete, this, std::placeholders::_1));
    m_client->setReadCallBack(std::bind(&RpcChannel::onRead, this, std::placeholders::_1));
    m_client->connect();
}

void RpcChannel::Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done)
{
    if (m_is_init) {
        return;
    }

    m_controller = controller;
    m_request    = req;
    m_response   = res;
    m_closure    = done;

    m_is_init = true;
}

void RpcChannel::onConnection(const TcpConnectionPtr &conn, const AbstractProtocolPtr &message)
{
    auto controller = std::dynamic_pointer_cast<RpcController>(m_controller);

    if (m_client->getConnectErrorCode() != 0) {
        LOG_INFO << message->m_msg_id << " | connect to ["
                 << m_client->getPeerAddr()->toString() << "] failed, erroe code ["
                 << m_client->getConnectErrorCode()
                 << "], error info [" << m_client->getConnectErrorInfo() << "]";
        controller->SetError(m_client->getConnectErrorCode(), m_client->getConnectErrorInfo());
        // m_client->quitLoop();
        return;
    }

    m_client->writeMessage(message);
    m_client->readMessage(message->m_msg_id);
}

void RpcChannel::onWriteComplete(const AbstractProtocolPtr &message)
{
    auto req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(message);
    LOG_INFO << req_protocol->m_msg_id << " | send rpc request success, call method name [" << req_protocol->m_method_name << "]";
}

void RpcChannel::onRead(const AbstractProtocolPtr &message)
{
    auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(message);
    auto controller   = std::dynamic_pointer_cast<RpcController>(m_controller);

    LOG_INFO << rsp_protocol->m_msg_id << " | success get rpc response, method name [" << rsp_protocol->m_method_name << "]";

    // rpc调用成功, 取消超时定时任务
    m_timer_event->setCancle(true);

    if (!m_response->ParseFromString(rsp_protocol->m_pb_data)) {
        LOG_ERROR << rsp_protocol->m_msg_id << "| ParseFromString error";
        controller->SetError(ERROR_FAILED_SERIALIZE, "ParseFromString error");
        return;
    }

    if (rsp_protocol->m_error_code != 0) {
        LOG_ERROR << rsp_protocol->m_msg_id << "| call rpc method ["
                  << rsp_protocol->m_method_name << "] failed, error code ["
                  << rsp_protocol->m_error_code << "], error info ["
                  << rsp_protocol->m_error_info << "]";
        controller->SetError(rsp_protocol->m_error_code, rsp_protocol->m_error_info);
        return;
    }


    // 如果没有被取消
    if (!controller->IsCanceled() && m_closure) {
        m_closure->Run();
    }

    auto channel = shared_from_this();
    channel.reset();
}

void RpcChannel::onTimeout()
{
    auto my_controller   = std::dynamic_pointer_cast<RpcController>(m_controller);
    int32_t err_code     = ERROR_RPC_CALL_TIMEOUT;
    std::string err_info = "rpc call timeout " + std::to_string(my_controller->GetTimeout());
    my_controller->StartCancel();
    my_controller->SetError(err_code, err_info);

    if (m_closure) {
        m_closure->Run();
    }

    auto channel = shared_from_this();
    channel.reset();
}

TcpClient *RpcChannel::getTcpClient() const
{
    return m_client.get();
}

}// namespace mrpc