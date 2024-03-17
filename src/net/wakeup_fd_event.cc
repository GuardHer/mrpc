#include "src/net/wakeup_fd_event.h"
#include "src/common/log.h"
#include <unistd.h>

namespace mrpc
{

WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd)
{
}

WakeUpFdEvent::~WakeUpFdEvent()
{
}

void WakeUpFdEvent::wakeup()
{
    char buf[8] = {'a'};
    int fd      = getFd();
    int rt      = write(fd, buf, 8);
    if (rt != 8) {
        LOG_ERROR << "write to wakeup fd less than 8 bytes, fd: " << fd;
    }
}

}// namespace mrpc
