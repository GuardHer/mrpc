#include "src/common/config.h"
#include "src/common/log.h"
#include <pthread.h>

void *func(void *)
{
    LOG_DEBUG("debug this is thread func in %s", "func");
    LOG_INFO("info this is thread func in %s", "func");
    return nullptr;
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();
    pthread_t thread;
    pthread_create(&thread, nullptr, &func, nullptr);
    LOG_DEBUG("This is an debug message");
    LOG_INFO("This is an info message");
    // 等待线程结束
    pthread_join(thread, nullptr);
    return 0;
}