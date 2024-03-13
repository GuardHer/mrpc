#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/tcp/abstract_protocol.h"
#include "src/net/tcp/net_addr.h"
#include "src/net/tcp/string_coder.h"
#include "src/net/tcp/tcp_client.h"
#include "src/net/tcp/tcp_connection.h"
#include "src/net/tcp/tcp_server.h"

#include <arpa/inet.h>
#include <chrono>
#include <memory>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <unistd.h>


void test_tcp_connection()
{
    // 调用connection连接server
    // write一个字符串
    // 等待read返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_DEBUG << "";
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = ::connect(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));
    if (rt < 0) {
        LOG_ERROR << "connect, error";
    }

    std::string msg = "hello mrpc!";
    rt = ::write(fd, msg.c_str(), msg.length());
    if (rt < 0) {
        LOG_ERROR << "write, error";
    }

    LOG_DEBUG << "success write rt:" << rt;

    std::string buf;
    buf.resize(100);
    rt = read(fd, buf.data(), 100);
    buf.resize(rt);

    close(fd);

    LOG_DEBUG << "read: " << buf << ", size: " << rt;
}

void test_tcp_client()
{
    using namespace mrpc;

    IPNetAddr::s_ptr addr = std::make_shared<IPNetAddr>("127.0.0.1", 12345);

    TcpClient cli(addr);


    auto conn_fun = [&cli](const TcpConnectionPtr &conn) {
        LOG_INFO << conn->getState() << ", conn_fun conn success!";

        std::shared_ptr<StringProtocol> message = std::make_shared<StringProtocol>();
        message->info = "Hello mrpc!";
        message->setReqId("123456");

        cli.writeMessage(message);
        cli.writeMessage(message);
        cli.writeMessage(message);
        cli.writeMessage(message);

        cli.readMessage("123456");
    };
    auto write_fun = [&cli](const AbstractProtocolPtr &message) {
        LOG_INFO << "write_fun!";
    };
    auto read_fun = [&cli](const AbstractProtocolPtr &message) {
        auto msg = std::dynamic_pointer_cast<StringProtocol>(message);
        LOG_INFO << "read_fun, req_id: " << msg->getReqId();
        LOG_INFO << "read_fun, info: " << msg->info;
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