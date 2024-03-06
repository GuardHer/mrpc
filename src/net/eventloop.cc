#include "src/net/eventloop.h"
#include "src/common/log.h"
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>

#define ADD_TO_EPOLL()                                                                                         \
    auto it = m_listen_fds.find(event->getFd());                                                               \
    int op = EPOLL_CTL_ADD;                                                                                    \
    if (it != m_listen_fds.end()) {                                                                            \
        op = EPOLL_CTL_MOD;                                                                                    \
    }                                                                                                          \
    epoll_event tmp = event->getEpollEvent();                                                                  \
    int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                                 \
    if (ret < 0) {                                                                                             \
        LOG_ERROR << "failed epoll_ctl when add fd " << event->getFd() << ", error info: " << strerror(errno); \
    }                                                                                                          \
    LOG_DEBUG << "add event success! fd: " << event->getFd();

#define DEL_TO_EPOLL()                                                                                         \
    auto it = m_listen_fds.find(event->getFd());                                                               \
    if (it != m_listen_fds.end()) {                                                                            \
        return;                                                                                                \
    }                                                                                                          \
    int op = EPOLL_CTL_DEL;                                                                                    \
    epoll_event tmp = event->getEpollEvent();                                                                  \
    int ret = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                                 \
    if (ret < 0) {                                                                                             \
        LOG_ERROR << "failed epoll_ctl when del fd " << event->getFd() << ", error info: " << strerror(errno); \
    }                                                                                                          \
    LOG_DEBUG << "del event success! fd: " << event->getFd();


namespace mrpc
{

static thread_local EventLoop *t_current_eventloop = nullptr;
static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events = 10;

EventLoop::EventLoop() : m_wakeup_fd_event(nullptr)
{
    // 每个线程只能有一个 EventLoop
    if (t_current_eventloop != nullptr) {
        LOG_FATAL << "failed to create event loop, this thread has created event loop";
    }

    m_tid = getThreadId();
    m_epoll_fd = epoll_create(10);

    if (m_epoll_fd == -1) {
        LOG_FATAL << "failed to create epoll fd!, error info: " << strerror(errno);
    }

    // wakeup fd event
    initWakeUpFdEvent();


    LOG_DEBUG << "succ create event loop in thread: " << m_tid;

    t_current_eventloop = this;
}

EventLoop::~EventLoop()
{
    close(m_epoll_fd);
    if (m_wakeup_fd_event) {
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = nullptr;
    }
}

void EventLoop::loop()
{
    while (!m_stop_flag) {
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<Task> tmp_tasks;
        tmp_tasks.swap(m_pending_tasks);
        lock.unlock();

        while (!tmp_tasks.empty()) {
            tmp_tasks.front()();// 执行函数
            tmp_tasks.pop();
        }

        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];
        int ret = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
        if (ret < 0) {
            LOG_ERROR << "epoll_wait error, errno = " << strerror(errno);
        } else {
            for (int i = 0; i < ret; i++) {
                epoll_event trigger_event = result_events[i];
                FdEvent *fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);
                if (fd_event == nullptr) { continue; }
                if (trigger_event.events | EPOLLIN) {
                    addTask(fd_event->getReadCollback());
                }
                if (trigger_event.events | EPOLLOUT) {
                    addTask(fd_event->getWriteCollback());
                }
            }
        }
    }
}

// queue in loop
void EventLoop::addTask(Task cb, bool is_wake_up /* = false */)
{
    {
        ScopeMutex<Mutex> lock(m_mutex);
        m_pending_tasks.push(std::move(cb));
    }

    if (is_wake_up) {
        wakeup();
    }
}

// run in loop : FdEvent
void EventLoop::addEpollEvent(FdEvent *event)
{
    if (isInLoopThread()) {
        ADD_TO_EPOLL();
    } else {
        auto cb = [this, event]() { ADD_TO_EPOLL(); };
        addTask(cb, true);
    }
}

void EventLoop::delEpollEvent(FdEvent *event)
{
    if (isInLoopThread()) {
        DEL_TO_EPOLL();
    } else {
        auto cb = [this, event]() { DEL_TO_EPOLL(); };
        addTask(cb, true);
    }
}


void EventLoop::wakeup()
{
}

int EventLoop::createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL << "failed to create event fd (wakeup_fd), error info: " << strerror(errno);
    }
    return evtfd;
}

void EventLoop::initWakeUpFdEvent()
{
    m_wakeup_fd = createEventfd();
    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    m_wakeup_fd_event->listen(FdEvent::EVENT_IN, [this]() {
        char buf[8];
        while (read(m_wakeup_fd, buf, 8) > 0 && errno != EAGAIN) {}
        LOG_DEBUG << "read full bytes form wakeup fd: " << m_wakeup_fd;
    });
    addEpollEvent(m_wakeup_fd_event);
}


void EventLoop::stop()
{
}

bool EventLoop::isInLoopThread()
{
    return m_tid == getThreadId();
}

}// namespace mrpc