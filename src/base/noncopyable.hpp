#pragma once

namespace schwi
{
    /**
     * @brief 禁止拷贝的类
     */
    class noncopyable
    {
    protected:
        noncopyable() = default;
        ~noncopyable() = default;

    private:
        noncopyable(const noncopyable &) = delete;
        noncopyable(noncopyable &&) = delete;
        noncopyable &operator=(const noncopyable &) = delete;
    };
}