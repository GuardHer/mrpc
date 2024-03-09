#ifndef MPRC_NET_IO_THREAD_H
#define MPRC_NET_IO_THREAD_H


#include "src/net/eventloop.h"
#include <pthread.h>
#include <semaphore.h>

namespace mrpc
{

class IOThread
{
public:
    IOThread();
    ~IOThread();

    /// @brief 获取当前IOThread的EventLoop对象
    /// @return m_event_loop
    EventLoop *getEventLoop();

    /// @brief 开启当前IOThread的 eventloop->loop()
    void start();

    /// @brief 阻塞等待线程结束
    void join();

private:
    /// @brief 线程函数
    /// @param arg 参数: 指针 (this)
    /// @return nullptr
    static void *Main(void *arg);

private:
    int32_t m_tid{-1};               // 线程号
    pthread_t m_thread{0};           // 线程句柄
    EventLoop *m_event_loop{nullptr};// 当前io线程的loop对象
    sem_t m_init_semaphore;          // 信号量
    sem_t m_start_semaphore;         // 信号量
};

}// namespace mrpc

#endif// MPRC_NET_IO_THREAD_H