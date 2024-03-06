#include "src/common/config.h"
#include "src/common/log.h"
#include "src/common/log_stream.h"
#include <iostream>
#include <pthread.h>
#include <queue>

void *func(void *)
{
    LOG_INFO << "LOG_INFO";
    LOG_DEBUG << "LOG_DEBUG";
    LOG_WARNING << "LOG_WARNING";
    LOG_ERROR << "LOG_ERROR";

    for (int i = 0; i < 1000; i++) {
        LOG_INFO << "LOG_INFO";
    }


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