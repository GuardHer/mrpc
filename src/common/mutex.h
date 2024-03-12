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

// 互斥锁
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

// 读写锁
class RWLock
{
public:
    RWLock()
    {
        pthread_rwlock_init(&m_lock, nullptr);
    }

    ~RWLock()
    {
        pthread_rwlock_destroy(&m_lock);
    }

    void readLock()
    {
        pthread_rwlock_rdlock(&m_lock);
    }

    void writeLock()
    {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock()
    {
        pthread_rwlock_unlock(&m_lock);
    }

private:
    pthread_rwlock_t m_lock;
};

}// namespace mrpc

#endif// MRPC_COMMON_MUTEX_H