#include "base/Timestamp.hpp"
#include <chrono>

namespace schwi
{
    /**
     * @brief 获取当前时间戳
     */
    Timestamp Timestamp::now()
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        return Timestamp(microseconds);
    }

    /**
     * @brief 将时间戳转换为字符串
     */
    std::string Timestamp::toString(bool showMicroseconds) const
    {
        if (showMicroseconds)
        {
            return fmt::format("{:%Y-%m-%d %H:%M:%S}.{:06}", fmt::localtime(seconds()), microseconds() % kMicroSecondsPerSecond);
        }
        else
        {
            return fmt::format("{:%Y-%m-%d %H:%M:%S}", fmt::localtime(seconds()));
        }
    }

    /**
     * @brief 获取时间戳的微秒数
     */
    int64_t Timestamp::microseconds() const
    {
        return _microseconds;
    }

    /**
     * @brief 获取时间戳的秒数
     */
    time_t Timestamp::seconds() const
    {
        return static_cast<time_t>(_microseconds / kMicroSecondsPerSecond);
    }

    /**
     * @brief 获取一个无效的时间戳
     */
    Timestamp Timestamp::invalid()
    {
        return Timestamp();
    }
} // namespace schwi