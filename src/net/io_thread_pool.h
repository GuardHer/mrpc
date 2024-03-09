#ifndef MRPC_NET_IO_THREAD_POOL_H
#define MRPC_NET_IO_THREAD_POOL_H

#include "src/net/io_thread.h"
#include <vector>

namespace mrpc
{
class IOThreadPool
{
public:
    IOThreadPool(int size);
    ~IOThreadPool();

    /// @brief 开启threadloop, 即开启所有IOThread的loop()
    void start();

    /// @brief 等待所有线程结束
    void join();

    /// @brief 获取pool中的一个IOThread
    /// @return IOThread
    IOThread *getIOThread();// no thread safe

private:
    int m_size{0};
    size_t m_index{0};
    std::vector<IOThread *> m_io_threads;
};
}// namespace mrpc

#endif//MRPC_NET_IO_THREAD_POOL_H