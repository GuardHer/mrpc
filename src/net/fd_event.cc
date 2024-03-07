#include "src/net/fd_event.h"
#include <string.h>

namespace mrpc
{
FdEvent::FdEvent(int fd) : m_fd(fd)
{
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::FdEvent()
{
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::~FdEvent()
{
}

void FdEvent::handler(TriggerEvent event_type)
{
    if (event_type == TriggerEvent::EVENT_IN) {
        if (m_read_callback)
            m_read_callback();
    } else {
        if (m_write_callback)
            m_write_callback();
    }
}
void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback)
{
    if (event_type == TriggerEvent::EVENT_IN) {
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    } else {
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }

    m_listen_events.data.ptr = this;
}


}// namespace mrpc