#ifndef MRPC_COMMON_MUTEX_H
#define MRPC_COMMON_MUTEX_H

#include <pthread.h>

namespace mrpc
{

template<class T>
class ScopeMutex
{
public:
    /// @brief 构造函数，默认加锁
    /// @param mutex
    ScopeMutex(T &mutex) : m_mutex(mutex)
    {
        m_mutex.lock();
        m_is_lock = true;
    }

    /// @brief 超出作用域自动解锁
    ~ScopeMutex()
    {
        if (m_is_lock) m_mutex.unlock();
        m_is_lock = false;
    }

    /// @brief 加锁
    void lock()
    {
        if (!m_is_lock) {
            m_mutex.lock();
            m_is_lock = true;
        }
    }

    /// @brief 解锁
    void unlock()
    {
        if (m_is_lock) {
            m_mutex.unlock();
            m_is_lock = false;
        }
    }

private:
    T &m_mutex;
    bool m_is_lock{false};
};

class Mutex
{
public:
    Mutex()
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }


private:
    pthread_mutex_t m_mutex;
};

}// namespace mrpc

#endif// MRPC_COMMON_MUTEX_H