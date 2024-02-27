#include "base/Thread.hpp"

#include <mutex>
#include <condition_variable>
#include <fmt/format.h>
#include "base/CurrentThread.hpp"

namespace schwi
{
    /**
     * @brief 线程编号
     */
    std::atomic_int32_t Thread::_numCreated(0);

    /**
     * @brief 构造函数，创建一个线程对象
     * @param func 线程函数
     * @param name 线程名称
     */
    Thread::Thread(ThreadFunc func, const std::string &name)
        : _started(false),
          _joined(false),
          _thread(nullptr),
          _tid(0),
          _func(std::move(func)),
          _name(name)
    {
        setDefaultName();
    }

    /**
     * @brief 析构函数，如果线程已启动但未加入，则等待线程结束
     */
    Thread::~Thread()
    {
        if (_started && !_joined)
        {
            _thread->join();
        }
    }

    /**
     * @brief 启动线程
     */
    void Thread::start()
    {
        _started = true;
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;
        _thread = std::make_shared<std::thread>([&]()
                                                {
            _tid = CurrentThread::tid();
            {
                std::lock_guard<std::mutex> lock(mtx);
                ready = true;
                cv.notify_one();
            }
            _func(); });
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&ready]
                    { return ready; });
        }
    }

    /**
     * @brief 等待线程结束
     */
    void Thread::join()
    {
        _joined = true;
        _thread->join();
    }

    /**
     * @brief 获取线程信息
     * @return 线程信息
     */
    std::string Thread::info() const
    {
        return fmt::format("Thread name: {}, tid: {}", _name, _tid);
    }

    /**
     * @brief 设置默认线程名称
     *
     * 如果当前线程名称为空，则将其设置为"Thread" + 线程编号。
     */
    void Thread::setDefaultName()
    {
        int num = ++_numCreated;
        if (_name.empty())
        {
            _name = fmt::format("Thread {}", num);
        }
    }
}
