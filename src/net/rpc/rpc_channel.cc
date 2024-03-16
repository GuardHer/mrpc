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

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr)
{
}
RpcChannel::~RpcChannel()
{
    if (m_done) {
        delete m_done;
        m_done = nullptr;
    }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                            google::protobuf::Message *response, google::protobuf::Closure *done)
{
    std::shared_ptr<TinyPBProtocol> req_protocol = std::make_shared<TinyPBProtocol>();
    auto my_controller = dynamic_cast<RpcController *>(controller);
    if (my_controller == nullptr) {
        LOG_ERROR << "failed callmethod, RpcController convert error";
        return;
    }

    // 设置 m_msg_id 和 m_msg_id_len
    if (my_controller->GetMsgId().empty()) {
        req_protocol->m_msg_id = MsgIdUtil::GenMsgId();
        my_controller->SetMsgId(req_protocol->m_msg_id);
    } else {
        req_protocol->m_msg_id = my_controller->GetMsgId();
    }
    req_protocol->m_msg_id_len = static_cast<int32_t>(req_protocol->m_msg_id.length());

    // 设置 method_name 和 m_method_len
    req_protocol->m_method_name = method->full_name();
    req_protocol->m_method_len = static_cast<int32_t>(req_protocol->m_method_name.length());
    LOG_INFO << req_protocol->m_msg_id << " | call method name [" << req_protocol->m_method_name << "]";

    // 序列化
    if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
        std::string err_info = "failed to SerializeToString";
        my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
        LOG_ERROR << req_protocol->m_msg_id << " | failed to SerializeToString";
        return;
    }

    // 设置 req_protocol->m_pk_len
    int32_t pb_data_len = static_cast<int32_t>(req_protocol->m_pb_data.length());
    req_protocol->m_pk_len = 26 + pb_data_len + req_protocol->m_method_len + req_protocol->m_msg_id_len + req_protocol->m_error_info_len;

    // 完成回调
    if (done) {
        m_done = dynamic_cast<RpcClosure<void> *>(done);
    }

    // 创建 client
    m_client.reset(new TcpClient(m_peer_addr));
    m_client->setConnectionCallBack(std::bind(&RpcChannel::onConnection, this, std::placeholders::_1, req_protocol));
    m_client->setWriteCompleteCallBack(std::bind(&RpcChannel::onWriteComplete, this, std::placeholders::_1));
    m_client->setReadCallBack(std::bind(&RpcChannel::onRead, this, std::placeholders::_1));
    m_client->connect();
}

void RpcChannel::onConnection(const TcpConnectionPtr &conn, const AbstractProtocolPtr &message)
{
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
    LOG_INFO << rsp_protocol->m_msg_id << " | success get rpc response, method name [" << rsp_protocol->m_method_name << "]";
    if (m_done) {
        m_done->Run();
    }
}

}// namespace mrpc