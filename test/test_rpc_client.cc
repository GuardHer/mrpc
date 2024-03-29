#include "src/common/config.h"
#include "src/common/log.h"
#include "src/common/msg_id_util.h"
#include "src/net/coder/abstract_protocol.h"
#include "src/net/coder/tinypb_coder.h"
#include "src/net/coder/tinypb_protocol.h"
#include "src/net/rpc/rpc_channel.h"
#include "src/net/rpc/rpc_closure.h"
#include "src/net/rpc/rpc_controller.h"
#include "src/net/rpc/rpc_dispatcher.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_client.h"
#include "src/net/tcp/tcp_connection.h"
#include "src/net/tcp/tcp_server.h"
#include "test/order.pb.h"

#include <arpa/inet.h>
#include <chrono>
#include <google/protobuf/service.h>
#include <memory>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <unistd.h>

void test_tcp_client()
{
    using namespace mrpc;

    IPNetAddr::s_ptr addr = std::make_shared<IPNetAddr>("127.0.0.1", 12345);

    TcpClient cli(addr);


    auto conn_fun = [&cli](const TcpConnectionPtr &conn, const AbstractProtocolPtr &protocol) {
        LOG_INFO << conn->getState() << ", conn_fun conn success!";
        // std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
        auto message          = std::dynamic_pointer_cast<TinyPBProtocol>(protocol);
        message->m_msg_id     = "99998888";
        message->m_msg_id_len = static_cast<int32_t>(message->m_msg_id.length());
        message->m_check_sum  = int32_t(456);

        makeOrderRequest request;
        request.set_price(9);
        request.set_goods("apple");
        if (!request.SerializeToString(&(message->m_pb_data))) {
            LOG_ERROR << "SerializeToString failed";
            return;
        }
        message->m_method_name = "Order.makeOrder";
        message->m_method_len  = static_cast<int32_t>(message->m_method_name.length());
        message->m_pk_len      = 26 + message->m_msg_id_len + message->m_method_len + message->m_error_info_len + message->m_pb_data.length();

        LOG_DEBUG << "m_pk_len: " << message->m_pk_len;

        cli.writeMessage(message);
        cli.writeMessage(message);
        cli.writeMessage(message);
        cli.writeMessage(message);
        cli.writeMessage(message);

        cli.readMessage("99998888");
    };
    auto write_fun = [&cli](const AbstractProtocolPtr &message) {
        LOG_INFO << "write_fun!";
    };
    auto read_fun = [&cli](const AbstractProtocolPtr &message) {
        auto msg = std::dynamic_pointer_cast<TinyPBProtocol>(message);
        LOG_INFO << "read_fun, msg_id: " << msg->m_msg_id;
        LOG_INFO << "read_fun, info: " << msg->m_pb_data;
        makeOrderResponse response;
        if (!response.ParseFromString(msg->m_pb_data)) {
            LOG_ERROR << "ParseFromString failed";
            return;
        }
        LOG_INFO << "response: " << response.DebugString();
    };

    cli.setWriteCompleteCallBack(write_fun);
    cli.setReadCallBack(read_fun);
    cli.setConnectionCallBack(conn_fun);

    cli.connect();
}

void test_msg_id()
{
    LOG_INFO << "msg_id: " << mrpc::MsgIdUtil::GenMsgId();
    LOG_INFO << "msg_id: " << mrpc::MsgIdUtil::GenMsgId();
}

void test_rpc_channel()
{
    using namespace mrpc;
    IPNetAddr::s_ptr addr               = std::make_shared<IPNetAddr>("127.0.0.1", 12345);
    std::shared_ptr<RpcChannel> channel = std::make_shared<RpcChannel>(addr);

    std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    request->set_price(100);
    request->set_goods("apple");
    std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();
    std::shared_ptr<RpcController> controller   = std::make_shared<RpcController>();
    controller->SetMsgId(mrpc::MsgIdUtil::GenMsgId());

    std::function<void()> reply_package_func = [controller, channel, request, response]() mutable {
        if (controller->GetErrorCode() == 0) {
            LOG_INFO << "call rpc success";
        } else {
            LOG_INFO << "call rpc failed, error info: " << controller->GetErrorInfo();
        }
        auto req_string = request->DebugString();
        auto rsp_string = response->DebugString();
        LOG_INFO << "request: " << req_string << ", len: " << req_string.length();
        LOG_INFO << "response: " << rsp_string << ", len: " << rsp_string.length();
        channel->getTcpClient()->quitLoop();
        channel.reset();
    };
    std::shared_ptr<RpcClosure<void>> done = std::make_shared<RpcClosure<void>>(reply_package_func);

    channel->Init(controller, request, response, done);

    Order_Stub stub(channel.get());

    controller->SetTimeout(1000);
    stub.makeOrder(controller.get(), request.get(), response.get(), done.get());
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    auto config = mrpc::Config::GetGlobalConfig();
    config->setLogFileName("rpc_client");
    mrpc::Logger::InitGlobalLogger();

    // tcp connection
    //test_tcp_connection();

    // tcp client
    // test_tcp_client();

    // tcp channel
    // test_msg_id();
    test_rpc_channel();

    return 0;
}