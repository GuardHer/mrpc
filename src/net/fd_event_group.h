#ifndef MRPC_NET_FD_EVENT_GROUP_H
#define MRPC_NET_FD_EVENT_GROUP_H

#include "src/common/mutex.h"
#include "src/net/fd_event.h"
#include <vector>

namespace mrpc
{

class FdEventGroup
{
public:
    FdEventGroup(int size);
    ~FdEventGroup();

    FdEvent *getFdEvent(int fd);

public:
    static FdEventGroup *GetFdEventGroup();

private:
    int m_size{0};
    Mutex m_mutex;
    std::vector<FdEvent *> m_fd_groups;
};

}// namespace mrpc

#endif//MRPC_NET_FD_EVENT_GROUP_H