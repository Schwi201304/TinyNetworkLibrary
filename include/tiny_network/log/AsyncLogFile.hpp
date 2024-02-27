#pragma once

#include "base/Thread.hpp"
#include "base/Timestamp.hpp"
#include "log/FixedBuffer.hpp"
#include "log/ILogStreamBase.hpp"

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <fstream>

namespace schwi
{
    class AsyncLogFile : public ILogStreamBase
    {
    public:
        AsyncLogFile(const char *filename,
                     int rollSize,
                     int flushInterval = 3,
                     int checkEveryN = 1024);
        ~AsyncLogFile();

        virtual void append(const char *logline, size_t len) override;

        void start();
        void stop();

        virtual void flush() override;
        void write(const char *buf, size_t len);

    private:
        void threadFunc();
        void rollFile();
        static std::string getLogFileName(const std::string &basename, Timestamp now);

    private:
        using Buffer = FixedBuffer<kLargeBuffer>;
        using BufferPtr = std::unique_ptr<Buffer>;
        using BufferVector = std::vector<BufferPtr>;

        const std::string _basename; // 日志文件名
        const int _flushInterval;    // 日志刷新间隔
        std::atomic_bool _running;   // 是否运行
        const int _rollSize;         // 日志文件滚动大小
        const int _checkEveryN;      // 检查次数
        Thread _thread;
        std::mutex _mutex;
        std::condition_variable _cond;

        int _count; // 计数

        Timestamp _startOfPeriod; // 开始时间
        Timestamp _lastRoll;      // 上次滚动时间
        Timestamp _lastFlush;     // 上次刷新时间

        constexpr static int kSecondsPerRoll = 60 * 60 * 24; // 滚动时间间隔

        BufferPtr _currentBuffer; // 当前缓冲区
        BufferPtr _nextBuffer;    // 下一个缓冲区
        BufferVector _buffers;    // 缓冲区数组

        std::ofstream _file; // 文件流
    };                       // class AsyncLogFile
} // namespace schwi