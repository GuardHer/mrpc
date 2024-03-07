#ifndef MRPC_NET_EVENTLOOP_H
#define MRPC_NET_EVENTLOOP_H

#include "src/common/mutex.h"
#include "src/common/util.h"
#include "src/net/fd_event.h"
#include "src/net/wakeup_fd_event.h"
#include <functional>
#include <queue>
#include <set>

namespace mrpc
{
class EventLoop
{
public:
    EventLoop();
    ~EventLoop();

    typedef std::function<void()> Task;

public:
    bool isInLoopThread();

    /// @brief 开启事件循环
    void loop();

    /// @brief 唤醒 epoll_wait
    void wakeup();

    /// @brief 停止事件循环
    void quit();

    /// @brief             添加任务
    /// @param cb:         任务func
    /// @param is_wake_up: 是否立即唤醒epoll_wait
    void addTask(Task cb, bool is_wake_up = false);

    /// @brief 添加事件
    /// @param event
    void addEpollEvent(FdEvent *event);

    /// @brief 删除事件
    /// @param event
    void delEpollEvent(FdEvent *event);

private:
    /// @brief 创建eventfd
    /// @return eventfd
    int createEventfd();

    /// @brief 初始化 wakeup_fd
    void initWakeUpFdEvent();

private:
    int32_t m_tid{0};                // 进程id
    int m_epoll_fd{0};               // epoll_fd
    int m_wakeup_fd{0};              // wake_fd
    WakeUpFdEvent *m_wakeup_fd_event;// wake_fd_event
    bool m_stop_flag{false};         // 是否停止

    std::set<int> m_listen_fds;      // 监听套接字集合 (wake_fd, client_fd, timer_fd)
    std::queue<Task> m_pending_tasks;// 任务队列 (需要mutex)

    Mutex m_mutex;
};

}// namespace mrpc

#endif//MRPC_NET_EVENTLOOP_H