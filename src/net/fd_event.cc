#include "src/net/fd_event.h"

namespace mrpc
{
FdEvent::FdEvent(int fd) : m_fd(fd)
{
}

FdEvent::~FdEvent()
{
}

void FdEvent::handler(TriggerEvent event_type)
{
    if (event_type == TriggerEvent::EVENT_IN) {
        m_read_callback();
    } else {
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