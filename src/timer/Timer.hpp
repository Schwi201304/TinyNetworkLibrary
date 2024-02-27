#pragma once

#include <functional>
#include "base/noncopyable.hpp"
#include "base/Timestamp.hpp"

namespace schwi
{
    class Timer : noncopyable
    {
    public:
        using TimerCallback = std::function<void()>;
        Timer(TimerCallback cb, Timestamp when, double interval)
            : _callback(std::move(cb)),
              _expiration(when),
              _interval(interval),
              _repeat(interval > 0.0)
        {
        }

        void run() const
        {
            _callback();
        }

        Timestamp expiration() const { return _expiration; }
        bool repeat() const { return _repeat; }
        void restart(Timestamp now); // 重启定时器

    private:
        const TimerCallback _callback; // 定时器回调函数
        Timestamp _expiration;         // 下一次的超时时刻
        const double _interval;        // 超时时间间隔，如果是一次性定时器，该值为0
        const bool _repeat;            // 是否重复(false 表示是一次性定时器)
    };
}