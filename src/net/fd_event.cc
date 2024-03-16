#include "src/net/fd_event.h"
#include "src/common/log.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


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
    } else if (event_type == TriggerEvent::EVENT_OUT) {
        if (m_write_callback)
            m_write_callback();
    } else if (event_type == TriggerEvent::EVENT_ERR) {
        if (m_error_callback)
            m_error_callback();
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

void FdEvent::cancle(TriggerEvent event_type)
{
    if (event_type == TriggerEvent::EVENT_IN) {
        if (m_listen_events.events & EPOLLIN)
            m_listen_events.events &= (~EPOLLIN);
    } else {
        if (m_listen_events.events & EPOLLOUT)
            m_listen_events.events &= (~EPOLLOUT);
    }
}

void FdEvent::setNonBlocking()
{
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags & O_NONBLOCK) return;
    if (flags == -1) {
        // 获取套接字属性失败
        LOG_ERROR << "setNonBlocking get socket flag failed: " << strerror(errno);
        return;
    }

    flags |= O_NONBLOCK;// 添加非阻塞标志
    if (fcntl(m_fd, F_SETFL, flags) == -1) {
        // 设置套接字属性失败
        LOG_ERROR << "setNonBlocking set socket O_NONBLOCK failed: " << strerror(errno);
        return;
    }
}


}// namespace mrpc