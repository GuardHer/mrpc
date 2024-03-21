#include "src/common/util.h"

#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace mrpc
{

static int32_t g_pid                    = 0;
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

std::string getCurrentTimeMillisecondsString()
{
    auto now            = std::chrono::system_clock::now();
    auto now_ms         = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto ms_since_epoch = now_ms.time_since_epoch();
    auto value          = std::chrono::duration_cast<std::chrono::milliseconds>(ms_since_epoch);
    auto ms             = value % 1000;
    auto in_time_t      = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string getCurrentTimeDayString()
{
    auto now       = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    return ss.str();
}

uint32_t networkToHost32(const char *net32)
{
    uint32_t result;

    std::memcpy(&result, net32, sizeof(uint32_t));
    return be32toh(result);
}


}// namespace mrpc
