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

    /// @brief get 指定时间点 ms
    /// @return m_arrive_time
    int64_t getArriveTime() const { return m_arrive_time; }

    /// @brief 设置 m_is_cancled
    /// @param value
    void setCancle(bool value) { m_is_cancled = value; }

    /// @brief 设置 m_is_repeated
    /// @param value
    void setRepeated(bool value) { m_is_repeated = value; }

    /// @brief 返回 m_is_cancled
    /// @return m_is_cancled
    bool isCancle() const { return m_is_cancled; }

    /// @brief 返回 m_is_cancled
    /// @return m_is_cancled
    bool isRepeated() const { return m_is_repeated; }

    /// @brief 获取定时任务
    /// @return
    std::function<void()> getCallBack() const { return m_task; }

    /// @brief 重新设置 m_arrive_time
    void resetArriveTime();

private:
    int64_t m_arrive_time;       // 指定时间点 ms
    int64_t m_interval;          // 事件间隔 ms
    bool m_is_repeated{false};   // 是否是重复任务
    bool m_is_cancled{false};    // 是否取消
    std::function<void()> m_task;// 具体任务
};

}// namespace mrpc

#endif//MRPC_NET_TIMER_EVENT_H