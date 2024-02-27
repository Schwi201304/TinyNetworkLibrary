#pragma once

#include <vector>
#include <set>

#include "base/Timestamp.hpp"
#include "net/Channel.hpp"

namespace schwi
{
    class EventLoop;
    class Timer;

    class TimerQueue
    {
    public:
        using TimerCallback = std::function<void()>;

        TimerQueue(EventLoop *loop);
        ~TimerQueue();

        void addTimer(const TimerCallback &cb, Timestamp when, double interval);

    private:
        using Entry = std::pair<Timestamp, Timer *>;
        using TimerList = std::set<Entry>;

        void addTimerInLoop(Timer *timer);

        void handleRead();
        void resetTimerfd(int timerfd, Timestamp expiration);

        std::vector<Entry> getExpired(Timestamp now);
        void reset(const std::vector<Entry> &expired, Timestamp now);

        bool insert(Timer *timer);

        EventLoop *_loop;
        const int _timerfd;
        Channel _timerfdChannel;
        TimerList _timers;

        bool _callingExpiredTimers;
    };
} // namespace schwi