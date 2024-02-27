#pragma once

#include "log/AsyncLogFile.hpp"
#include "log/Logger.hpp"
#include "base/Singleton.hpp"

#include <memory>

namespace schwi
{

    class GlobalLogger : public Singleton<GlobalLogger>
    {
    public:
        GlobalLogger() = default;
        ~GlobalLogger() = default;

        std::shared_ptr<Logger> getLogger()
        {
            return _logger;
        }

        void setLogger(std::shared_ptr<Logger> logger)
        {
            _logger = logger;
        }

    public:
        std::shared_ptr<Logger> _logger;

    }; // class GlobalLogger

#define LOG_TRACE(...) LOGGER_TRACE(GlobalLogger::Instance().getLogger(), __VA_ARGS__)
#define LOG_DEBUG(...) LOGGER_DEBUG(GlobalLogger::Instance().getLogger(), __VA_ARGS__)
#define LOG_INFO(...) LOGGER_INFO(GlobalLogger::Instance().getLogger(), __VA_ARGS__)
#define LOG_WARN(...) LOGGER_WARN(GlobalLogger::Instance().getLogger(), __VA_ARGS__)
#define LOG_ERROR(...) LOGGER_ERROR(GlobalLogger::Instance().getLogger(), __VA_ARGS__)
#define LOG_FATAL(...) LOGGER_FATAL(GlobalLogger::Instance().getLogger(), __VA_ARGS__)

#ifndef NDEBUG
#include <assert.h>
#include <stdio.h>

#define __DEBUG__LOG__(...) LOG_DEBUG(__VA_ARGS__)
#define __DEBUG__ASSERT__(...) assert(__VA_ARGS__) \
                                   ? (void)0       \
                                   : __DEBUG__LOG__("Assertion failed: {}, file {}, line {}\n", #__VA_ARGS__, __FILE__, __LINE__)
#else
#define __DEBUG__LOG__(...)
#define __DEBUG__ASSERT__(...)
#endif // DEBUG

} // namespace schwi