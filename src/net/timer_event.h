#ifndef MRPC_NET_TIMER_EVENT_H
#define MRPC_NET_TIMER_EVENT_H

#include <functional>
#include <memory>

namespace mrpc
{

class TimerEvent
{
public:
    typedef std::shared_ptr<TimerEvent> s_ptr;

    /// @brief 构造函数
    /// @param interval    事件间隔 ms
    /// @param is_repeated 是否是重复任务
    /// @param cb          具体任务
    TimerEvent(int interval, bool is_repeated, std::function<void()> cb);

private:
    int64_t m_arrive_time;       // 指定时间点 ms
    int64_t m_interval;          // 事件间隔 ms
    bool m_is_repeated{false};   // 是否是重复任务
    bool m_is_cancled{false};    // 是否取消
    std::function<void()> m_task;// 具体任务
};

}// namespace mrpc

#endif//MRPC_NET_TIMER_EVENT_H