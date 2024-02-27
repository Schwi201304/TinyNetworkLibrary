#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "base/noncopyable.hpp"
#include "base/Thread.hpp"

namespace schwi
{
    /**
     * @brief 线程池类
    */
    class ThreadPool : noncopyable
    {
    public:
        using ThreadFunc = std::function<void()>;

        explicit ThreadPool(size_t numThreads, const std::string &name = std::string("ThreadPool"));
        ~ThreadPool();

        void start();
        void stop();

        void setThreadSize(size_t numThreads);
        void setThreadInitCallback(const ThreadFunc &cb);

        const std::string &name() const;
        size_t size() const;

        void addTask(const ThreadFunc &task);

    private:
        bool isFull() const;
        void run();

        std::vector<std::unique_ptr<Thread>> _threads; // 线程池中的线程
        std::queue<ThreadFunc> _queue;                 // 任务队列

        mutable std::mutex _mutex;
        std::condition_variable _condition;
        std::string _name;              // 线程池名称
        ThreadFunc _threadInitCallback; // 线程初始化回调函数
        bool _stop;                     // 线程池是否停止
        size_t _numThreads;             // 线程池中线程的数量
    };
} // namespace schwi