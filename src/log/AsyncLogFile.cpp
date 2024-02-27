#include "log/AsyncLogFile.hpp"
#include "base/base.hpp"
#include "base/Timestamp.hpp"

namespace schwi
{
    /**
     * @brief 构造函数，用于初始化 AsyncLogFile 对象。
     *
     * @param filename 日志文件名
     * @param rollSize 日志文件滚动大小
     * @param flushInterval 刷新间隔时间
     */
    AsyncLogFile::AsyncLogFile(const char *filename,
                               int rollSize,
                               int flushInterval,
                               int checkEveryN)
        : _flushInterval(flushInterval),
          _running(false),
          _basename(filename),
          _rollSize(rollSize),
          _checkEveryN(checkEveryN),
          _thread(std::bind(&AsyncLogFile::threadFunc, this), "Logging"),
          _mutex(),
          _cond(),
          _currentBuffer(new Buffer),
          _nextBuffer(new Buffer),
          _buffers()
    {
        _currentBuffer->bzero();
        _nextBuffer->bzero();
        _buffers.reserve(16);
        rollFile();
    }

    /**
     * @brief 析构函数
     */
    AsyncLogFile::~AsyncLogFile()
    {
        if (_running)
        {
            stop();
        }
    }

    /**
     * @brief 向缓冲区中添加数据
     *
     * @param logline 数据
     * @param len 数据长度
     */
    void AsyncLogFile::append(const char *logline, size_t len)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_currentBuffer->avail() > len)
        {
            _currentBuffer->append(logline, len);
        }
        else
        {
            // 缓冲区已满，将当前缓冲区放入缓冲区队列中
            _buffers.push_back(std::move(_currentBuffer));
            if (_nextBuffer) [[likely]]
            {
                _currentBuffer = std::move(_nextBuffer);
            }
            else
            {
                _currentBuffer.reset(new Buffer);
            }
            _currentBuffer->append(logline, len);
            _cond.notify_one();
        }
    }

    /**
     * @brief 启动日志线程
     */
    void AsyncLogFile::start()
    {
        _running = true;
        _thread.start();
    }

    /**
     * @brief 停止日志线程
     */
    void AsyncLogFile::stop()
    {
        _running = false;
        flush();
        _thread.join();
    }

    /**
     * @brief 刷新日志
     */
    void AsyncLogFile::flush()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_currentBuffer->length() > 0)
        {
            _buffers.push_back(std::move(_currentBuffer));
            _currentBuffer.reset(new Buffer);
            _currentBuffer->bzero();
            _cond.notify_one();
        }
    }

    /**
     * @brief 日志线程函数
     */
    void AsyncLogFile::threadFunc()
    {
        // 创建两个缓冲区
        BufferPtr newBuffer1(new Buffer);
        BufferPtr newBuffer2(new Buffer);
        newBuffer1->bzero();
        newBuffer2->bzero();
        // 缓冲区队列
        BufferVector buffersToWrite;
        buffersToWrite.reserve(16);
        while (_running)
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_buffers.empty())
                {
                    // 等待缓冲区有数据
                    _cond.wait_for(lock, std::chrono::seconds(_flushInterval));
                }
                // 将当前缓冲区放入缓冲区队列中
                _buffers.push_back(std::move(_currentBuffer));
                // 交换缓冲区
                _currentBuffer = std::move(newBuffer1);
                buffersToWrite.swap(_buffers);
                // 如果下一个缓冲区为空，则创建一个新的缓冲区
                if (!_nextBuffer)
                {
                    _nextBuffer = std::move(newBuffer2);
                }
            }

            for (const auto &buffer : buffersToWrite)
            {
                write(buffer->data(), buffer->length());
            }

            if (buffersToWrite.size() > 2)
            {
                buffersToWrite.resize(2);
            }

            if (!newBuffer1)
            {
                newBuffer1 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
                newBuffer1->reset();
            }

            if (!newBuffer2)
            {
                newBuffer2 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
                newBuffer2->reset();
            }

            buffersToWrite.clear();
            _file.flush();
        }
        _file.flush();
    }

    /**
     * @brief 向文件中写入数据
     *
     * @param buf 数据
     * @param len 数据长度
     */
    void AsyncLogFile::write(const char *buf, size_t len)
    {
        _file.write(buf, len);
        if (_file.tellp() >= _rollSize)
        {
            __DEBUG__LOG__("RollFile: tellp() >= _rollSize\n");
            rollFile();
        }
        else
        {
            ++_count;
            if (_count >= _checkEveryN)
            {
                _count = 0;
                Timestamp now = Timestamp::now();
                Timestamp thisPeriod(now.seconds() / kSecondsPerRoll * kSecondsPerRoll, 0);
                if (thisPeriod != _startOfPeriod)
                {
                    __DEBUG__LOG__("RollFile: thisPeriod != _startOfPeriod\n");
                    rollFile();
                }
                else if (now > addTime(_lastFlush, _flushInterval))
                {
                    _lastFlush = now;
                    _file.flush();
                }
            }
        }
    }

    /**
     * @brief 滚动日志文件
     */
    void AsyncLogFile::rollFile()
    {
        Timestamp now = Timestamp::now();
        std::string filename = getLogFileName(_basename, now);
        Timestamp start(now.seconds() / kSecondsPerRoll * kSecondsPerRoll, 0);

        if (now > _lastRoll)
        {
            _lastRoll = now;
            _lastFlush = now;
            _startOfPeriod = start;
            _file.close();
            _file.open(filename, std::ios::app);
        }
    }

    /**
     * @brief 获取日志文件名
     *
     * @param basename 文件名
     * @param now 时间
     * @return std::string 文件名
     */
    std::string AsyncLogFile::getLogFileName(const std::string &basename, Timestamp now)
    {
        std::string filename = fmt::format("{}-{}.log", basename, now.toString(false));

        return filename;
    }
} // namespace schwi