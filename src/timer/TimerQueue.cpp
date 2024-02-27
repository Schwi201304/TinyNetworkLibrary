#include "timer/TimerQueue.hpp"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>

#include "base/base.hpp"
#include "net/Channel.hpp"
#include "net/EventLoop.hpp"
#include "timer/Timer.hpp"

namespace schwi
{
    int createTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0)
        {
            LOG_ERROR("Failed in timerfd_create");
        }
        return timerfd;
    }

    void ReadTimerfd(int timerfd)
    {
        uint64_t read_byte;
        ssize_t n = ::read(timerfd, &read_byte, sizeof(read_byte));
        if (n != sizeof(read_byte))
        {
            LOG_ERROR("TimerQueue::handleRead() reads {} bytes instead of 8", n);
        }
    }

    TimerQueue::TimerQueue(EventLoop *loop)
        : _loop(loop),
          _timerfd(createTimerfd()),
          _timerfdChannel(loop, _timerfd),
          _timers()
    {
        _timerfdChannel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
        _timerfdChannel.enableReading();
    }

    TimerQueue::~TimerQueue()
    {
        _timerfdChannel.disableAll();
        _timerfdChannel.remove();
        ::close(_timerfd);
        for (auto &timer : _timers)
        {
            delete timer.second;
        }
    }

    void TimerQueue::addTimer(const Timer::TimerCallback &cb, Timestamp when, double interval)
    {
        Timer *timer = new Timer(cb, when, interval);
        _loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    }

    void TimerQueue::addTimerInLoop(Timer *timer)
    {
        bool earliestChanged = insert(timer);
        if (earliestChanged)
        {
            resetTimerfd(_timerfd, timer->expiration());
        }
    }

    void TimerQueue::handleRead()
    {
        Timestamp now(Timestamp::now());
        ReadTimerfd(_timerfd);

        std::vector<Entry> expired = getExpired(now);

        _callingExpiredTimers = true;
        for (auto &entry : expired)
        {
            entry.second->run();
        }
        _callingExpiredTimers = false;
        reset(expired, now);
    }

    void TimerQueue::resetTimerfd(int timerfd, Timestamp expiration)
    {
        struct itimerspec new_value;
        struct itimerspec old_value;

        bzero(&new_value, sizeof(new_value));
        bzero(&old_value, sizeof(old_value));

        int64_t micro_seconds = expiration.microseconds() - Timestamp::now().microseconds();
        if (micro_seconds < 100)
        {
            micro_seconds = 100;
        }

        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(micro_seconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>((micro_seconds % Timestamp::kMicroSecondsPerSecond) * 1000);
        new_value.it_value = ts;

        if (::timerfd_settime(timerfd, 0, &new_value, &old_value) < 0)
        {
            LOG_ERROR("timerfd_settime()");
        }
    }

    std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
    {
        Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
        auto end = _timers.lower_bound(sentry);
        std::vector<Entry> expired(_timers.begin(), end);
        _timers.erase(_timers.begin(), end);

        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
    {
        for (auto &entry : expired)
        {
            if (entry.second->repeat())
            {
                entry.second->restart(now);
                insert(entry.second);
            }
            else
            {
                delete entry.second;
            }

            if (!_timers.empty())
            {
                resetTimerfd(_timerfd, _timers.begin()->second->expiration());
            }
        }
    }

    bool TimerQueue::insert(Timer *timer)
    {
        bool earliestChanged = false;
        Timestamp when = timer->expiration();
        auto iter = _timers.begin();
        if (iter == _timers.end() || when < iter->first)
        {
            earliestChanged = true;
        }
        _timers.insert(std::make_pair(when, timer));
        return earliestChanged;
    }
} // namespace schwi