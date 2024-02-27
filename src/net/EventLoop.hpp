#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

#include "base/noncopyable.hpp"
#include "base/Timestamp.hpp"
#include "base/CurrentThread.hpp"
#include "timer/TimerQueue.hpp"

namespace schwi
{
    class Channel;
    class Poller;

    class EventLoop : noncopyable
    {
    public:
        using Functor = std::function<void()>;

        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        Timestamp pollReturnTime() const { return _pollReturnTime; }

        void runInLoop(Functor cb);
        void queueInLoop(Functor cb);

        void wakeup();

        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        void hasChannel(Channel *channel);

        bool isInLoopThread() const { return _threadId == CurrentThread::tid(); }

        void runAt(const Timestamp &time, Functor cb);
        void runAfter(double delay, Functor cb);
        void runEvery(double interval, Functor cb);

    private:
        void handleRead();
        void doPendingFunctors();

        using ChannelList = std::vector<Channel *>;
        std::atomic_bool _looping;
        std::atomic_bool _quit;
        std::atomic_bool _callingPendingFunctors;
        const pid_t _threadId;
        Timestamp _pollReturnTime;
        std::unique_ptr<Poller> _poller;
        std::unique_ptr<TimerQueue> _timerQueue;

        int _wakeupFd;
        std::unique_ptr<Channel> _wakeupChannel;

        ChannelList _activeChannels;
        Channel *_currentActiveChannel;
        std::mutex _mutex;
        std::vector<Functor> _pendingFunctors;
    };
} // namespace schwi