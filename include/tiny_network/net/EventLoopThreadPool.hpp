#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <string>

namespace schwi
{
    class EventLoop;
    class EventLoopThread;

    class EventLoopThreadPool
    {
    public:
        using ThreadInitCallback = std::function<void(EventLoop *)>;

        EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads) { _numThreads = numThreads; }
        void start(const ThreadInitCallback &cb = ThreadInitCallback());

        EventLoop *getNextLoop();

        std::vector<EventLoop *> getAllLoops();

        bool started() const { return _started; }
        const std::string &name() const { return _name; }

    private:
        EventLoop *_baseLoop;
        std::string _name;
        bool _started;
        int _numThreads;
        size_t _next;
        std::vector<std::unique_ptr<EventLoopThread>> _threads;
        std::vector<EventLoop *> _loops;
    };
} // namespace schwi
