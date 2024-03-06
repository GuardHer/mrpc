#ifndef MRPC_NET_FDEVENT_H
#define MRPC_NET_FDEVENT_H

#include <functional>
#include <sys/epoll.h>

namespace mrpc
{
class FdEvent
{
public:
    enum TriggerEvent
    {
        EVENT_IN = EPOLLIN,
        EVENT_OUT = EPOLLOUT,
    };

public:
    FdEvent(int fd);
    ~FdEvent();

    void handler(TriggerEvent event_type);
    void listen(TriggerEvent event_type, std::function<void()> callback);
    int getFd() { return m_fd; }
    epoll_event getEpollEvent() { return m_listen_events; }
    void setReadCollback(std::function<void()> cb) { m_read_callback = std::move(cb); }
    void setWriteCollback(std::function<void()> cb) { m_write_callback = std::move(cb); }
    std::function<void()> getReadCollback() { return m_read_callback; }
    std::function<void()> getWriteCollback() { return m_write_callback; }


private:
    int m_fd{-1};
    epoll_event m_listen_events;
    std::function<void()> m_read_callback;
    std::function<void()> m_write_callback;
};
}// namespace mrpc

#endif// MRPC_NET_FDEVENT_H