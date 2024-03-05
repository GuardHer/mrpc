#include "src/common/log.h"
#include <pthread.h>

void *func(void *)
{
    LOG_DEBUG("this is thread func in %s", "func");
    return nullptr;
}

int main()
{

    pthread_t thread;
    pthread_create(&thread, nullptr, &func, nullptr);
    LOG_DEBUG("This is an debug message");
    return 0;
}