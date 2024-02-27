#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <compare>

namespace schwi
{
    class Timestamp
    {
    public:
        Timestamp() : _microseconds(0) {}
        explicit Timestamp(int64_t microseconds)
            : _microseconds(microseconds) {}
        explicit Timestamp(time_t seconds, int microseconds)
            : _microseconds(static_cast<int64_t>(seconds) * kMicroSecondsPerSecond + microseconds) {}

        auto operator<=>(const Timestamp &other) const = default;

        static Timestamp now();

        std::string toString(bool showMicroseconds = true) const;

        int64_t microseconds() const;
        time_t seconds() const;

        static Timestamp invalid();

        constexpr static int kMicroSecondsPerSecond = 1000 * 1000;

    private:
        int64_t _microseconds;
    };

    /**
     * @brief 将时间戳增加指定的秒数
     */
    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microseconds() + delta);
    }
} // namespace schwi
