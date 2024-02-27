#pragma once

#include "base/noncopyable.hpp"
#include "base/Timestamp.hpp"
#include "log/FixedBuffer.hpp"
#include "log/ILogStreamBase.hpp"

#include <string_view>
#include <fstream>

namespace schwi
{
    /**
     * @brief 日志流
     */
    template <int SIZE>
    class LogStreamBase : public ILogStreamBase
    {
    public:
        LogStreamBase();
        virtual ~LogStreamBase() = default;

        using Buffer = FixedBuffer<SIZE>;

        virtual void append(const char *buf, size_t len) override;
        const Buffer &getBuffer() const;

    protected:
        void resetBuffer();
        Buffer _buffer;

    }; // class LogStream

    /**
     * @brief 控制台日志流
     */
    class LogConsole : public ILogStreamBase
    {
    public:
        LogConsole() = default;
        ~LogConsole() { flush(); }

        virtual void append(const char *buf, size_t len) override;
        virtual void flush() override;
    }; // class LogConsole

    /**
     * @brief 文件日志流
     */
    class LogFile : public LogStreamBase<kSmallBuffer>
    {
    public:
        LogFile(const char *filename);
        ~LogFile() { flush(); }

        virtual void flush() override;

    private:
        std::ofstream _file; // 文件流

    }; // class LogFile
} // namespace schwi