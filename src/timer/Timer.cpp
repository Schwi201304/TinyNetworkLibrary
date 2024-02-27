#include "timer/Timer.hpp"

namespace schwi
{
    void Timer::restart(Timestamp now)
    {
        if (_repeat)
        {
            // 如果是重复定时器，下一次超时时间 = 当前时间 + 时间间隔
            _expiration = addTime(now, _interval);
        }
        else
        {
            _expiration = Timestamp::invalid();
        }
    }
} // namespace schwi