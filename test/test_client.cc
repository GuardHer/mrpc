#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/tcp/net_addr.h"
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

    std::string msg = "hello mrpc!";
    rt = ::write(fd, msg.c_str(), msg.length());

    LOG_DEBUG << "success write rt:" << rt;

    std::string buf;
    buf.resize(100);
    rt = read(fd, buf.data(), 100);
    buf.resize(rt);

    close(fd);

    LOG_DEBUG << "read: " << buf << ", size: " << rt;
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();

    // tcp server
    for (int i = 0; i < 10000; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));// ms
        test_tcp_connection();
    }

    return 0;
}