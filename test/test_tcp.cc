#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/tcp_server.h"

void test_net_addr()
{
    mrpc::IPNetAddr addr("127.0.0.1", 12345);
    mrpc::IPNetAddr addr2("127.0.0.1:12345");
    LOG_DEBUG << "create addr: " << addr.toString();
    LOG_DEBUG << "create addr: " << addr2.toString();
}

void test_tcp_server()
{
    mrpc::IPNetAddr::s_ptr addr = std::make_shared<mrpc::IPNetAddr>("127.0.0.1", 12345);

    mrpc::TcpServer server(addr);

    server.start();
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();

    // tcp server
    test_tcp_server();
    return 0;
}