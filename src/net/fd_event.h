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
    FdEvent();
    ~FdEvent();

    /// @brief
    /// @param event_type 事件类型 (IN || OUT)
    void handler(TriggerEvent event_type);

    /// @brief 设置监听事件类型和回调函数
    /// @param event_type
    /// @param callback
    void listen(TriggerEvent event_type, std::function<void()> callback);

    /// @brief 获取fd
    /// @return  m_fd
    int getFd() const { return m_fd; }

    /// @brief 设置fd
    /// @param fd
    void setFd(int fd) { m_fd = fd; }

    void setNonBlocking();


    /// @brief 获取epoll_event
    /// @return m_listen_events
    epoll_event getEpollEvent() const { return m_listen_events; }

    /// @brief 设置读回调函数
    /// @param cb
    void setReadCollback(std::function<void()> cb) { m_read_callback = std::move(cb); }

    /// @brief 设置写回调函数
    /// @param cb
    void setWriteCollback(std::function<void()> cb) { m_write_callback = std::move(cb); }

    /// @brief 获取读回调函数
    /// @return m_read_callback
    std::function<void()> getReadCollback() { return m_read_callback; }

    /// @brief 获取写回到函数
    /// @return m_write_callback
    std::function<void()> getWriteCollback() { return m_write_callback; }


private:
    int m_fd{-1};
    epoll_event m_listen_events;
    std::function<void()> m_read_callback;
    std::function<void()> m_write_callback;
};
}// namespace mrpc

#endif// MRPC_NET_FDEVENT_H