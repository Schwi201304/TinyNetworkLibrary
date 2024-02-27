#pragma once

#include "base/noncopyable.hpp"
#include <string>
#include <cstring>
#include <string_view>
#include <functional>

namespace schwi
{
    constexpr int kSmallBuffer = 4 * 1024;    // 缓冲区大小
    constexpr int kLargeBuffer = 1024 * 1024; // 缓冲区大小

    /**
     * @brief 固定大小的缓冲区
     */
    template <int SIZE>
    class FixedBuffer : noncopyable
    {
    public:
        using FullCallback = std::function<void()>;
        FixedBuffer(FullCallback cb = nullptr)
            : _cur(_data),
              _fullCallback(cb) {}
        ~FixedBuffer() = default;

        /**
         * @brief 向缓冲区中添加数据
         *
         * @param buf 数据
         * @param len 数据长度
         */
        void append(const char *buf, size_t len)
        {
            if (avail() <= len)
            {
                if (_fullCallback)
                    _fullCallback();
                else
                    reset();
            }
            std::memcpy(_cur, buf, len);
            _cur += len;
            *_cur = '\0';
        }

        const char *data() const
        {
            return _data;
        }                                                             // 返回缓冲区的起始地址
        int length() const { return static_cast<int>(_cur - _data); } // 返回缓冲区中数据的长度

        char *current() { return _cur; }                             // 返回缓冲区中数据的结束地址
        int avail() const { return static_cast<int>(end() - _cur); } // 返回缓冲区中剩余空间的长度
        void add(size_t len) { _cur += len; }                        // 移动_cur指针

        void reset() { _cur = _data; }                    // 重置缓冲区
        void bzero() { memset(_data, 0, sizeof(_data)); } // 清空缓冲区

        void setFullCallback(FullCallback cb) { _fullCallback = cb; } // 设置缓冲区满时的回调函数

        std::string toString() const { return std::string(_data, length()); }               // 返回缓冲区中的数据
        std::string_view toStringView() const { return std::string_view(_data, length()); } // 返回缓冲区中的数据

    private:
        const char *end() const { return _data + sizeof(_data); } // 返回缓冲区的结束地址

        char _data[SIZE];           // 缓冲区
        char *_cur;                 // 指向缓冲区中数据的结束地址
        FullCallback _fullCallback; // 缓冲区满时的回调函数
    };
} // namespace schwi