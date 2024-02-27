#pragma once

#ifdef __unix__
#include <unistd.h>
#include <sys/syscall.h>
#else
#error "This code can only be compiled in a Unix environment."
#endif

namespace schwi
{
    /**
     * @brief 当前线程相关的功能
     */
    namespace CurrentThread
    {
        extern thread_local int t_cachedTid; // 当前线程的线程ID

        /**
         * @brief 获取当前线程的线程ID
         * @return 当前线程的线程ID
         */
        inline int tid()
        {
            if (t_cachedTid == 0) [[unlikely]]
            {
                t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
            }
            return t_cachedTid;
        }

    } // namespace CurrentThread
} // namespace schwi