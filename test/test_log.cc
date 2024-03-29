#include "src/common/config.h"
#include "src/common/log.h"
#include "src/common/log_stream.h"
#include <chrono>
#include <iostream>
#include <pthread.h>
#include <thread>
#include <vector>
//
void *func(void *)
{
    mrpc::Logger::InitGlobalLogger();

    LOG_INFO << "LOG_INFO";
    LOG_DEBUG << "LOG_DEBUG";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return nullptr;
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    std::vector<pthread_t> threads;
    for (int i = 0; i < 100; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pthread_t thread;
        pthread_create(&thread, nullptr, &func, nullptr);
        threads.push_back(thread);
    }

    // 等待线程结束
    for (auto thread: threads) {
        pthread_join(thread, nullptr);
    }
    return 0;
}