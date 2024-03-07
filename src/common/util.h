#ifndef MRPC_COMMON_UTIL_H
#define MRPC_COMMON_UTIL_H
#include <sys/types.h>
#include <unistd.h>
namespace mrpc
{

/// 获取当前进程id
int32_t getPid();
/// 获取当前线程id
int32_t getThreadId();
/// 获取当前时间 ms
int64_t getNowMs();


}// namespace mrpc

#endif// MRPC_COMMON_UTIL_H