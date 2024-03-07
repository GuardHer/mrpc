#ifndef MRPC_NET_TIMER_H
#define MRPC_NET_TIMER_H

#include "src/common/mutex.h"
#include "src/net/fd_event.h"
#include "src/net/timer_event.h"
#include <map>

namespace mrpc
{

class Timer : public FdEvent
{
public:
    typedef std::pair<int64_t, std::function<void()>> timer_task;
    Timer();
    ~Timer();

public:
    /// @brief 添加定时任务
    /// @param event: 定时任务
    void addTimerEvent(TimerEvent::s_ptr event);

    /// @brief 删除定时任务
    /// @param event: 定时任务
    void delTimerEvent(TimerEvent::s_ptr event);

    /// @brief 当发生了IO事件后, eventloop 会执行这个函数
    void onTimer();

private:
    /// @brief
    void resetArriveTime();

private:
    /// @brief multimap: 可重复的map, 有序
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
    Mutex m_mutex;// 互斥锁
};

}// namespace mrpc

#endif//MRPC_NET_TIMER_H