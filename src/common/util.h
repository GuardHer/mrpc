#ifndef MRPC_COMMON_UTIL_H
#define MRPC_COMMON_UTIL_H
#include <sys/types.h>
#include <unistd.h>
namespace mrpc
{
	int32_t getPid();

	int32_t getThreadId();
}

#endif // MRPC_COMMON_UTIL_H