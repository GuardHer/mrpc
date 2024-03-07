#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/eventloop.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <queue>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
netstat -tln | grep 12345
telnet 127.0.0.1 12345
*/

void *func(void *)
{
    mrpc::EventLoop *eventloop = new mrpc::EventLoop();

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        LOG_FATAL << "listenfd: " << listenfd;
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;

    inet_aton("127.0.0.1", &addr.sin_addr);

    int ret = bind(listenfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (ret < 0) {
        LOG_FATAL << "bind err: " << strerror(errno);
    }

    ret = listen(listenfd, 100);
    if (ret < 0) {
        LOG_FATAL << "listen err: " << strerror(errno);
    }

    mrpc::FdEvent event(listenfd);
    event.listen(mrpc::FdEvent::EVENT_IN, [listenfd]() {
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr *>(&peer_addr), &addr_len);
        LOG_INFO << "success get client [ " << inet_ntoa(peer_addr.sin_addr) << ":" << ntohs(peer_addr.sin_port) << "]";
    });
    eventloop->addEpollEvent(&event);

    eventloop->loop();

	delete eventloop;

    return nullptr;
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();
    pthread_t thread;
    pthread_create(&thread, nullptr, &func, nullptr);
    // 等待线程结束
    pthread_join(thread, nullptr);
    return 0;
}