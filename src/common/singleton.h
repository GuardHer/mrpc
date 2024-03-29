#ifndef MRPC_COMMON_SINGLETON_H
#define MRPC_COMMON_SINGLETON_H

#include <cassert>
#include <iostream>
#include <memory>
#include <utility>

namespace mrpc
{

class singleton_module
{
private:
    bool &get_lock()
    {
        static bool lock = false;
        return lock;
    }

public:
    void lock()
    {
        get_lock() = true;
    }
    void unlock()
    {
        get_lock() = false;
    }
    bool is_locked()
    {
        return get_lock();
    }
};

static inline singleton_module &get_singleton_module()
{
    static singleton_module m;
    return m;
}


template<class T>
class singleton_wrapper : public T
{
    static bool &get_is_destroyed()
    {

        static bool is_destroyed_flag = false;
        return is_destroyed_flag;
    }

public:
    singleton_wrapper()
    {
        assert(!is_destroyed());
    }
    ~singleton_wrapper()
    {
        get_is_destroyed() = true;
    }
    static bool is_destroyed()
    {
        return get_is_destroyed();
    }
};

template<class T>
class singleton
{
private:
    static T *m_instance;
    static void use(T const &) {}
    static T &get_instance()
    {
        assert(!is_destroyed());

        static singleton_wrapper<T> t;


        if (m_instance) use(*m_instance);

        return static_cast<T &>(t);
    }

protected:
    singleton() {}
    singleton(const singleton &)            = delete;
    singleton &operator=(const singleton &) = delete;

public:
    static T &get_mutable_instance()
    {
        assert(!get_singleton_module().is_locked());
        return get_instance();
    }
    static const T &get_const_instance()
    {
        return get_instance();
    }
    static bool is_destroyed()
    {
        return singleton_wrapper<T>::is_destroyed();
    }
};


template<class T>
T *singleton<T>::m_instance = &singleton<T>::get_instance();

}// namespace mrpc

#endif// MRPC_COMMON_SINGLETON_HPP