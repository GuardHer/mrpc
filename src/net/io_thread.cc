#include "src/net/io_thread.h"
#include "src/common/log.h"
#include "src/common/util.h"

#include <assert.h>
#include <string.h>

namespace mrpc
{

IOThread::IOThread()
{
    // 初始化信号量
    int ret = sem_init(&m_init_semaphore, 0, 0);
    assert(ret == 0);
    ret = sem_init(&m_start_semaphore, 0, 0);
    assert(ret == 0);

    // 开启线程
    pthread_create(&m_thread, nullptr, &IOThread::Main, this);

    // wait, 直到新线程执行完 Main 函数的前置
    ret = sem_wait(&m_init_semaphore);
    if (ret < 0) {
        LOG_ERROR << "sem_wait error info: " << strerror(errno);
    }

    LOG_DEBUG << "IOThread create success, tid: " << m_tid;
}

IOThread::~IOThread()
{
    // 停止loop
    m_event_loop->quit();
    /// 销毁信号量
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);
    // 等待线程结束
    pthread_join(m_thread, nullptr);

    // delete eveltloop ptr
    if (m_event_loop) {
        delete m_event_loop;
        m_event_loop = nullptr;
    }
}

void *IOThread::Main(void *arg)
{
    // 获取指针
    IOThread *thread = static_cast<IOThread *>(arg);

    thread->m_event_loop = new EventLoop();
    thread->m_tid = getThreadId();

    // 唤醒等待的线程
    sem_post(&thread->m_init_semaphore);

    LOG_DEBUG << "IOThread created, wait start semaphore!";

    // 当线程等待, 直到主动开启loop循环
    sem_wait(&thread->m_start_semaphore);

    // 开启loop循环
    thread->m_event_loop->loop();

    LOG_DEBUG << "end loop!";

    return nullptr;
}

EventLoop *IOThread::getEventLoop()
{
    return m_event_loop;
}

void IOThread::start()
{
    sem_post(&m_start_semaphore);
}

void IOThread::join()
{
    pthread_join(m_thread, nullptr);
}

}// namespace mrpc