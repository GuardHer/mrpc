#include "src/net/io_thread_pool.h"
#include "src/common/log.h"

namespace mrpc
{

IOThreadPool::IOThreadPool(int size) : m_size(size)
{
    m_io_threads.resize(size);

    // 创建IOThread
    for (int i = 0; i < size; i++) {
        m_io_threads[i] = new IOThread();
    }
}

IOThreadPool::~IOThreadPool()
{
    for (size_t i = 0; i < m_io_threads.size(); i++) {
        if (m_io_threads[i]) {
            delete m_io_threads[i];
            m_io_threads[i] = nullptr;
        }
    }
}

void IOThreadPool::start()
{
    // 开启所有线程
    for (size_t i = 0; i < m_io_threads.size(); i++) {
        m_io_threads[i]->start();
    }
}

void IOThreadPool::join()
{
    // 等待所有线程
    for (size_t i = 0; i < m_io_threads.size(); i++) {
        m_io_threads[i]->join();
    }
}

IOThread *IOThreadPool::getIOThread()
{
    if (m_index == m_io_threads.size()) {
        m_index = 0;
    }

    return m_io_threads[m_index++];
}

}// namespace mrpc