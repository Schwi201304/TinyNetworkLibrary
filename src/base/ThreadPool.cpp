#include "base/ThreadPool.hpp"
#include <fmt/format.h>

namespace schwi
{
    /**
     * @brief 构造函数，创建一个线程池对象
     * @param numThreads 线程池中线程的数量
     * @param name 线程池名称
     */
    ThreadPool::ThreadPool(size_t numThreads, const std::string &name)
        : _threads(),
          _queue(),
          _mutex(),
          _condition(),
          _name(name),
          _threadInitCallback(nullptr),
          _stop(false),
          _numThreads(numThreads)
    {
    }

    /**
     * @brief 析构函数，停止线程池中的所有线程
     */
    ThreadPool::~ThreadPool()
    {
        stop();
        for (auto &thread : _threads)
        {
            thread->join();
        }
    }

    /**
     * @brief 启动线程池中的所有线程
     */
    void ThreadPool::start()
    {
        _stop = false;
        _threads.reserve(_numThreads);
        for (size_t i = 0; i < _numThreads; ++i)
        {
            std::string thread_name = fmt::format("{} {}", _name, i);
            _threads.emplace_back(new Thread(std::bind(&ThreadPool::run, this), thread_name));
            if (_threadInitCallback)
            {
                _threadInitCallback();
            }
            _threads[i]->start();
        }
        if (_numThreads == 0 && _threadInitCallback)
        {
            _threadInitCallback();
        }
    }

    /**
     * @brief 停止线程池中的所有线程
     */
    void ThreadPool::stop()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
        _condition.notify_all();
    }

    /**
     * @brief 设置线程池中线程的数量
     */
    void ThreadPool::setThreadSize(size_t numThreads)
    {
        _numThreads = numThreads;
    }

    /**
     * @brief 设置线程池中线程的初始化回调函数
     */
    void ThreadPool::setThreadInitCallback(const ThreadFunc &cb)
    {
        _threadInitCallback = cb;
    }

    /**
     * @brief 获取线程池名称
     */
    const std::string &ThreadPool::name() const
    {
        return _name;
    }

    /**
     * @brief 获取线程池中线程的数量
     */
    size_t ThreadPool::size() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _threads.size();
    }

    /**
     * @brief 添加一个任务到线程池中
     */
    void ThreadPool::addTask(const ThreadFunc &task)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(task);
        _condition.notify_one();
    }
    
    /**
     * @brief 判断线程池是否已满
     */
    bool ThreadPool::isFull() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size() >= _threads.size();
    }

    /**
     * @brief 线程池中线程的运行函数
     */
    void ThreadPool::run()
    {
        if (_threadInitCallback)
        {
            _threadInitCallback();
        }
        while (!_stop)
        {
            ThreadFunc task;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this]
                                { return _stop || !_queue.empty(); });
                if (_stop && _queue.empty())
                {
                    return;
                }
                task = _queue.front();
                _queue.pop();
            }
            if (task)
            {
                task();
            }
        }
    }
} // namespace schwi