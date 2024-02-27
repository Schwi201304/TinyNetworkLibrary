#pragma once

#include "base/noncopyable.hpp"

namespace schwi
{
    /**
     * @brief 日志流基类
     */
    class ILogStreamBase : noncopyable
    {
    public:
        ILogStreamBase() = default;
        virtual ~ILogStreamBase() = default;
        virtual void append(const char *buf, size_t len) = 0;
        virtual void append(std::string_view str) { append(str.data(), str.size()); }
        virtual void flush() = 0;
    }; // class ILogStreamBase
} // namespace schwi