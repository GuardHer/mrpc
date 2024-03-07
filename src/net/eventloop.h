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

    void loop();
    void wakeup();
    void quit();

    void addTask(Task cb, bool is_wake_up = false);
    void addEpollEvent(FdEvent *event);
    void delEpollEvent(FdEvent *event);

private:
    int createEventfd();
    void initWakeUpFdEvent();

private:
    int32_t m_tid{0};
    int m_epoll_fd{0};
    int m_wakeup_fd{0};
    WakeUpFdEvent *m_wakeup_fd_event;
    bool m_stop_flag{false};

    std::set<int> m_listen_fds;
    std::queue<Task> m_pending_tasks;// mutex

    Mutex m_mutex;
};

}// namespace mrpc

#endif//MRPC_NET_EVENTLOOP_H