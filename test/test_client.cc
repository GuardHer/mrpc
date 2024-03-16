#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/coder/abstract_protocol.h"
#include "src/net/coder/tinypb_coder.h"
#include "src/net/coder/tinypb_protocol.h"
#include "src/net/tcp/net_addr.h"
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


    auto conn_fun = [&cli](const TcpConnectionPtr &conn, const AbstractProtocolPtr &protocol) {
        LOG_INFO << conn->getState() << ", conn_fun conn success!";
        // std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
        auto message = std::dynamic_pointer_cast<TinyPBProtocol>(protocol);
        message->m_msg_id = "123456789";
        message->m_msg_id_len = static_cast<int32_t>(message->m_msg_id.length());
        message->m_method_name = "mrpc";
        message->m_method_len = static_cast<int32_t>(message->m_method_name.length());
        message->m_error_code = int32_t(123);
        message->m_error_info = "error in this func";
        message->m_error_info_len = static_cast<int32_t>(message->m_error_info.length());
        message->m_pb_data = "this is pb_data";
        message->m_check_sum = int32_t(456);
        message->m_pk_len = 26 + message->m_msg_id_len + message->m_method_len + message->m_error_info_len + message->m_pb_data.length();
        LOG_DEBUG << "m_pk_len: " << message->m_pk_len;

        cli.writeMessage(message);
        cli.writeMessage(message);

        cli.readMessage("123456789");
    };
    auto write_fun = [&cli](const AbstractProtocolPtr &message) {
        LOG_INFO << "write_fun!";
    };
    auto read_fun = [&cli](const AbstractProtocolPtr &message) {
        auto msg = std::dynamic_pointer_cast<TinyPBProtocol>(message);
        LOG_INFO << "read_fun, msg_id: " << msg->m_msg_id;
        LOG_INFO << "read_fun, info: " << msg->m_pb_data;
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