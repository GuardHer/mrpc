#include "src/common/util.h"

#include <sys/syscall.h>

namespace mrpc
{
	static int32_t g_pid = 0;
	static thread_local int32_t g_thread_id = 0;

	int32_t getPid()
	{
		if (g_pid)
		{
			return g_pid;
		}

		return getpid();
	}

	int32_t getThreadId()
	{
		if (g_thread_id)
		{
			return g_thread_id;
		}

		return syscall(SYS_gettid);
	}
}
