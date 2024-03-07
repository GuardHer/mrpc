#include "src/common/util.h"

#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace mrpc
{

static int32_t g_pid = 0;
static thread_local int32_t t_thread_id = 0;

int32_t getPid()
{
    if (g_pid) {
        return g_pid;
    }
    g_pid = getpid();
    return g_pid;
}

int32_t getThreadId()
{
    if (t_thread_id) {
        return t_thread_id;
    }

    t_thread_id = syscall(SYS_gettid);
    return t_thread_id;
}

int64_t getNowMs()
{
    timeval val;
    gettimeofday(&val, nullptr);

    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

}// namespace mrpc
