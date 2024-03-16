#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/coder/abstract_protocol.h"
#include "src/net/coder/tinypb_coder.h"
#include "src/net/coder/tinypb_protocol.h"
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


    auto conn_fun = [&cli](const TcpConnectionPtr &conn) {
        LOG_INFO << conn->getState() << ", conn_fun conn success!";

        std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
        message->m_req_id = "99998888";
        message->m_req_id_len = static_cast<int32_t>(message->m_req_id.length());
        message->m_check_sum = int32_t(456);

        makeOrderRequest request;
        request.set_price(9);
        request.set_goods("apple");
        if (!request.SerializeToString(&(message->m_pb_data))) {
            LOG_ERROR << "SerializeToString failed";
            return;
        }
        message->m_method_name = "Order.makeOrder";
        message->m_method_len = static_cast<int32_t>(message->m_method_name.length());
        message->m_pk_len = 26 + message->m_req_id_len + message->m_method_len + message->m_error_info_len + message->m_pb_data.length();

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
        LOG_INFO << "read_fun, req_id: " << msg->m_req_id;
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

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();

    // tcp connection
    //test_tcp_connection();

    // tcp client
    test_tcp_client();

    return 0;
}