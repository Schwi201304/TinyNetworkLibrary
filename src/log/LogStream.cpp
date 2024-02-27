#include "log/LogStream.hpp"
#include "base/Timestamp.hpp"

#include <iostream>

namespace schwi
{
    /**
     * @brief 构造函数，创建一个日志流
     */
    template <int SIZE>
    LogStreamBase<SIZE>::LogStreamBase()
    {
        _buffer.setFullCallback(
            [this]
            { this->flush(); });
    }
    /**
     * @brief 向缓冲区中添加数据
     *
     * @param buf 数据
     * @param len 数据长度
     */
    template <int SIZE>
    void LogStreamBase<SIZE>::append(const char *buf, size_t len)
    {
        _buffer.append(buf, len);
    }

    /**
     * @brief 重置缓冲区
     */
    template <int SIZE>
    void LogStreamBase<SIZE>::resetBuffer()
    {
        _buffer.reset();
    }

    /**
     * @brief 获取缓冲区
     */
    template <int SIZE>
    const LogStreamBase<SIZE>::Buffer &LogStreamBase<SIZE>::getBuffer() const
    {
        return _buffer;
    }

    /**
     * @brief 向控制台中写入缓冲区数据
     */
    void LogConsole::append(const char *buf, size_t len)
    {
        std::string_view view(buf, len);
        std::cout << view;
    }

    /**
     * @brief 向控制台中写入缓冲区数据
     */
    void LogConsole::flush()
    {
        std::cout << std::flush;
    }

    /**
     * @brief 构造函数，创建一个文件日志流
     */
    LogFile::LogFile(const char *filename)
    {
        _file.open(filename, std::ios::app);
    }

    /**
     * @brief 向文件中写入缓冲区数据
     */
    void LogFile::flush()
    {
        _file << getBuffer().data() << std::flush;
        resetBuffer();
    }

} // namespace schwi