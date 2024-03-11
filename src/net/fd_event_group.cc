#include "src/net/fd_event_group.h"
#include "src/common/log.h"

namespace mrpc
{

static FdEventGroup *g_fd_event_group = nullptr;

FdEventGroup::FdEventGroup(int size) : m_size(size)
{
    for (int i = 0; i < m_size; i++) {
        m_fd_groups.push_back(new FdEvent(i));
    }
}

FdEventGroup::~FdEventGroup()
{
    for (int i = 0; i < m_size; i++) {
        if (m_fd_groups[i]) {
            delete m_fd_groups[i];
            m_fd_groups[i] = nullptr;
        }
    }
}

FdEvent *FdEventGroup::getFdEvent(int fd)
{
    ScopeMutex<Mutex> lock(m_mutex);
    if (fd < int(m_fd_groups.size())) {
        return m_fd_groups[fd];
    }

    // 扩容
    int new_size = int(fd * 1.5);
    for (int i = m_fd_groups.size(); i < new_size; i++) {
        m_fd_groups.push_back(new FdEvent(i));
    }

    return m_fd_groups[fd];
}

FdEventGroup *FdEventGroup::GetFdEventGroup()
{
    if (g_fd_event_group) {
        return g_fd_event_group;
    }

    g_fd_event_group = new FdEventGroup(128);

    return g_fd_event_group;
}

}// namespace mrpc