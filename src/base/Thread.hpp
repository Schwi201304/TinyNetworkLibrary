#pragma once

#include <thread>
#include <functional>
#include <memory>

#include "base/noncopyable.hpp"

namespace schwi
{
    /**
     * @brief 线程类
    */
    class Thread : public noncopyable
    {
    public:
        using ThreadFunc = std::function<void()>;

        explicit Thread(ThreadFunc func, const std::string &name = std::string());
        ~Thread();

        void start();
        void join();

        bool started() const { return _started; }
        pid_t tid() const { return _tid; }
        const std::string &name() const { return _name; }
        std::string info() const;

        static int numCreated() { return _numCreated; }

    private:
        void setDefaultName();

        bool _started;
        bool _joined;
        std::shared_ptr<std::thread> _thread;
        pid_t _tid;

        ThreadFunc _func;
        std::string _name;
        static std::atomic_int32_t _numCreated;
    };
} // namespace schwi
