#include "src/net/timer.h"
#include "src/common/log.h"
#include "src/common/util.h"
#include <string.h>
#include <sys/timerfd.h>

namespace mrpc
{

Timer::Timer() : FdEvent()
{
    // 创建 timer_fd
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd < 0) {
        LOG_FATAL << "failed to create timerfd!";
    }

    // 设置事件的fd
    this->setFd(timer_fd);

    // 将onTimer绑定到 fd 的读事件
    this->listen(FdEvent::EVENT_IN, std::bind(&Timer::onTimer, this));
}

Timer::~Timer()
{
}

void Timer::resetArriveTime()
{
    ScopeMutex<Mutex> lock(m_mutex);
    auto tmp = m_pending_events;
    m_mutex.unlock();

    if (tmp.empty()) {
        return;
    }

    // 获取当前时间 ms
    int64_t now = getNowMs();
    // 时间间隔
    int64_t inteval = 0;

    auto it = tmp.begin();
    // 如果	最早的一个任务的执行时间 > 现在时间
    if (it->second->getArriveTime() > now) {
        // 设置时间间隔为: 最早的任务的执行时间 - 现在时间
        inteval = it->second->getArriveTime() - now;
    } else {
        // 如果	最早的一个任务的执行时间 < 现在时间, 设置为100ms后执行
        inteval = 100;// ms
    }

    itimerspec value;
    memset(&value, 0, sizeof(value));
    // 初始定时器值
    value.it_value.tv_sec = inteval / 1000;             // ms -> s
    value.it_value.tv_nsec = (inteval % 1000) * 1000000;// ms -> ns

    // 设置 timer_fd 的超时时间
    int ret = timerfd_settime(this->getFd(), 0, &value, nullptr);
    if (ret < 0) {
        LOG_ERROR << "timerfd_settime error info:  " << strerror(errno);
    }

    // LOG_DEBUG << "timer reset to: " << now + inteval;
}

void Timer::addTimerEvent(TimerEvent::s_ptr event)
{
    bool is_reset_timerfd = false;

    ScopeMutex<Mutex> lock(m_mutex);
    // 队列为空时,也就是第一次加入任务时，需要timerfd_settime
    if (m_pending_events.empty()) {
        is_reset_timerfd = true;
    } else {
        auto it = m_pending_events.begin();
        // 如果当前加入的定时任务的时间小于队列中的所有任务，需要timerfd_settime
        if (it->second->getArriveTime() > event->getArriveTime()) {
            is_reset_timerfd = true;
        }
    }
    // 添加任务进队列
    m_pending_events.emplace(event->getArriveTime(), event);
    lock.unlock();

    // 重新设置timerfd开启时间和间隔时间
    if (is_reset_timerfd) {
        resetArriveTime();
    }
}

void Timer::delTimerEvent(TimerEvent::s_ptr event)
{
    event->setCancle(true);
    ScopeMutex<Mutex> lock(m_mutex);
    // 因为是mutimap, 可能存在多个相同的键
    // 第一个
    auto begin = m_pending_events.lower_bound(event->getArriveTime());
    // 最后一个
    auto end = m_pending_events.upper_bound(event->getArriveTime());

    // 查找 event
    auto it = begin;
    for (it = begin; it != end; ++it) {
        if (it->second == event) break;
    }

    // 检查是否找到, 如果找到就删除
    if (it != end) {
        m_pending_events.erase(it);
    }
    lock.unlock();
    LOG_DEBUG << "success delete TimerEvent: " << event->getArriveTime();
}

void Timer::onTimer()
{
    char buf[8];
    while (read(this->getFd(), buf, 8) > 0 && errno != EAGAIN) {}

    // 执行定时任务
    int64_t now = getNowMs();
    // 临时缓存需要重新添加的重复执行的任务
    std::vector<TimerEvent::s_ptr> tmps;
    // task: std::function<void()>, 需要执行的任务
    std::vector<timer_task> tasks;
    ScopeMutex<Mutex> lock(m_mutex);

    // 找到所有在now及之前的任务
    auto it = m_pending_events.begin();
    for (; it != m_pending_events.end(); ++it) {
        if (it->second->getArriveTime() <= now) {
            // 如果这个任务没有被取消
            if (!it->second->isCancle()) {
                // 先添加进tmps, 后面再判断是否是重复任务: isRepeated()
                tmps.push_back(it->second);
                // 先添加进任务队列, 后面再执行
                tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallBack()));
            }
        } else {
            break;
        }
    }

    // 删除now之前的任务, 因为已经添加到执行队列里或者已经取消了
    m_pending_events.erase(m_pending_events.begin(), it);
    lock.unlock();

    // 需要把是重复任务的event事件再添加
    for (auto event: tmps) {
        // 如果是重复任务, 就再添加
        if (event->isRepeated()) {
            // 重新设置时间
            event->resetArriveTime();
            // 添加任务
            addTimerEvent(event);
        }
    }
    resetArriveTime();

    // 执行任务
    for (auto t: tasks) {
        if (t.second) t.second();
    }
}


}// namespace mrpc