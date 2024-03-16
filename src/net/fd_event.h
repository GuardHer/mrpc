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
        EVENT_ERR = EPOLLERR,// 不需要主动添加
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

    /// @brief 取消监听
    /// @param event_type
    void cancle(TriggerEvent event_type);

    /// @brief 取消此fd_event
    void unregisterFromLoop();

    /// @brief 获取fd
    /// @return  m_fd
    int getFd() const { return m_fd; }

    /// @brief 设置fd
    /// @param fd
    void setFd(int fd) { m_fd = fd; }

    /// @brief 设置非阻塞
    void setNonBlocking();

    /// @brief 获取epoll_event
    /// @return m_listen_events
    epoll_event getEpollEvent() const { return m_listen_events; }

    /// @brief 设置读回调函数
    /// @param cb
    void setReadCallback(std::function<void()> cb) { m_read_callback = std::move(cb); }

    /// @brief 设置写回调函数
    /// @param cb
    void setWriteCallback(std::function<void()> cb) { m_write_callback = std::move(cb); }

    /// @brief 设置错误回调函数
    /// @param cb
    void setErrorCallback(std::function<void()> cb) { m_error_callback = std::move(cb); };

    /// @brief 获取读回调函数
    /// @return m_read_callback
    std::function<void()> getReadCallback() const { return m_read_callback; }

    /// @brief 获取写回到函数
    /// @return m_write_callback
    std::function<void()> getWriteCallback() const { return m_write_callback; }

    /// @brief 获取错误回到函数
    /// @return m_error_callback
    std::function<void()> getErrorCallback() const { return m_error_callback; }

private:
    int m_fd{-1};
    epoll_event m_listen_events;
    std::function<void()> m_read_callback{nullptr};
    std::function<void()> m_write_callback{nullptr};
    std::function<void()> m_error_callback{nullptr};
};
}// namespace mrpc

#endif// MRPC_NET_FDEVENT_H