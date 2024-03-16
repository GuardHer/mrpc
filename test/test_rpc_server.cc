#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/coder/abstract_protocol.h"
#include "src/net/coder/tinypb_coder.h"
#include "src/net/coder/tinypb_protocol.h"
#include "src/net/rpc/rpc_dispatcher.h"
#include "src/net/tcp/net_addr.h"
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

class OrderImpl : public Order
{
public:
    void makeOrder(google::protobuf::RpcController *controller,
                   const ::makeOrderRequest *request,
                   ::makeOrderResponse *response,
                   ::google::protobuf::Closure *done)
    {
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_ret_code(20240315);
        response->set_res_info("Hello world");
    }
};

void test_tcp_server()
{
    using namespace mrpc;

    IPNetAddr::s_ptr addr = std::make_shared<mrpc::IPNetAddr>("127.0.0.1", 12345);

    TcpServer server(addr);

    server.start();
}


int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    mrpc::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    // tcp server
    test_tcp_server();
    return 0;
}