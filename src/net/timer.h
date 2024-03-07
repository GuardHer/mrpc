#ifndef MRPC_NET_TIMER_H
#define MRPC_NET_TIMER_H

#include "src/net/fd_event.h"
#include "src/net/timer_event.h"
#include <map>

namespace mrpc
{

class Timer : public FdEvent
{
public:
    Timer(int fd);
    ~Timer();

public:
    /// @brief 添加定时任务
    /// @param event: 定时任务
    void addTimerEvent(TimerEvent::s_ptr event);

    /// @brief 删除定时任务
    /// @param event: 定时任务
    void delTimerEvent(TimerEvent::s_ptr event);

private:
    /// @brief multimap: 可重复的map
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
};

}// namespace mrpc

#endif//MRPC_NET_TIMER_H