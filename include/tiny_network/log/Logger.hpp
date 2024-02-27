#pragma once

#include <type_traits>
#include <string_view>
#include <vector>

#include "base/noncopyable.hpp"
#include "base/Timestamp.hpp"
#include "log/ILogStreamBase.hpp"

namespace schwi
{
    /**
     * @brief 文件工具类
     */
    class FileUtil
    {
    public:
        /**
         * @brief 获取文件名
         *
         * @param _filePath 文件路径
         */
        static std::string_view getFileName(const char *_filePath)
        {
            std::string_view path(_filePath);
            auto pos = path.find_last_of("/\\");
            if (pos != std::string_view::npos)
            {
                return path.substr(pos + 1);
            }
            return path;
        }
    };

    /**
     * @brief 日志类
     */
    class Logger : public noncopyable
    {
    public:
        /**
         * @brief 日志级别
         */
        enum LogLevel
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            LOG_COUNT
        };

        /**
         * @brief 日志级别名称
         */
        const char *getLevelName[Logger::LogLevel::LOG_COUNT] = {
            "TRACE",
            "DEBUG",
            "INFO ",
            "WARN ",
            "ERROR",
            "FATAL"};

        using LogStreamPtr = std::shared_ptr<ILogStreamBase>; // 日志流指针

        /**
         * @brief 构造函数
         *
         * @param level 日志级别
         * @param args 日志流
         */
        template <typename... Args>
        Logger(LogLevel level, Args &&...args)
            : _level(level), _streams{std::forward<Args>(args)...} {}

        /**
         * @brief 析构函数
         */
        virtual ~Logger() = default;

        template <typename T>
        using is_convertible_to_string =
            std::disjunction<
                std::is_convertible<std::decay_t<T>, std::string>,
                std::is_convertible<std::decay_t<T>, std::string_view>>;

        /**
         * @brief 记录日志
         *
         * @tparam T 参数类型
         * @param file 文件名
         * @param line 行号
         * @param level 日志级别
         * @param msg 消息
         */
        template <typename T>
            requires(is_convertible_to_string<T>::value)
        void log(const char *file, int line, LogLevel level, const T &msg)
        {
            log_(file, line, level, msg);
        }

        /**
         * @brief 记录日志
         *
         * @tparam T 参数类型
         * @param file 文件名
         * @param line 行号
         * @param level 日志级别
         * @param msg 消息
         */
        template <typename T>
            requires(!is_convertible_to_string<T>::value)
        void log(const char *file, int line, LogLevel level, const T &msg)
        {
            log_(file, line, level, "{}", msg);
        }

        /**
         * @brief 记录日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param level 日志级别
         * @param fmt 格式字符串
         * @param args 参数
         */
        template <typename... Args>
        void log(const char *file, int line, LogLevel level, std::string_view fmt, Args &&...args)
        {
            log_(file, line, level, fmt, std::forward<Args>(args)...);
        }

        /**
         * @brief 跟踪日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param args 参数
         */
        template <typename... Args>
        void trace(const char *file, int line, Args &&...args)
        {
            log(file, line, LogLevel::TRACE, std::forward<Args>(args)...);
        }

        /**
         * @brief 调试日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param args 参数
         */
        template <typename... Args>
        void debug(const char *file, int line, Args &&...args)
        {
            log(file, line, LogLevel::DEBUG, std::forward<Args>(args)...);
        }

        /**
         * @brief 信息日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param args 参数
         */
        template <typename... Args>
        void info(const char *file, int line, Args &&...args)
        {
            log(file, line, LogLevel::INFO, std::forward<Args>(args)...);
        }

        /**
         * @brief 警告日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param args 参数
         */
        template <typename... Args>
        void warn(const char *file, int line, Args &&...args)
        {
            log(file, line, LogLevel::WARN, std::forward<Args>(args)...);
        }

        /**
         * @brief 错误日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param args 参数
         */
        template <typename... Args>
        void error(const char *file, int line, Args &&...args)
        {
            log(file, line, LogLevel::ERROR, std::forward<Args>(args)...);
        }

        /**
         * @brief 致命错误日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param args 参数
         */
        template <typename... Args>
        void fatal(const char *file, int line, Args &&...args)
        {
            log(file, line, LogLevel::FATAL, std::forward<Args>(args)...);
        }

/**
 * @brief 日志宏
 * @param logger 日志对象
 * @param ... 参数
 */
#define LOGGER_TRACE(logger, ...) logger->trace(__FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_DEBUG(logger, ...) logger->debug(__FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_INFO(logger, ...) logger->info(__FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_WARN(logger, ...) logger->warn(__FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_ERROR(logger, ...) logger->error(__FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_FATAL(logger, ...) logger->fatal(__FILE__, __LINE__, ##__VA_ARGS__)

    private:
        /**
         * @brief 是否应该记录日志
         *
         * @param level 日志级别
         * @return true 如果应该记录
         */
        bool shouldLog(LogLevel level) const
        {
            return level >= _level;
        }

        template <typename T>
        auto format_arg_pointer(T &&arg)
        {
            if constexpr (std::is_pointer_v<std::remove_reference_t<T>> &&
                          !std::is_same_v<std::remove_reference_t<T>, const char *>)
            {
                return fmt::ptr(arg);
            }
            else
            {
                return arg;
            }
        }

        /**
         * @brief 记录日志
         *
         * @tparam Args 参数类型
         * @param file 文件名
         * @param line 行号
         * @param level 日志级别
         * @param fmt 格式字符串
         * @param args 参数
         */
        template <typename... Args>
        void log_(const char *file, int line, LogLevel level, std::string_view fmt, Args &&...args)
        {
            if (!shouldLog(level))
                return;

            std::string message =
                fmt::format(
                    fmt,
                    format_arg_pointer(std::forward<Args>(args))...);
            std::string prefixStr = fmt::format(
                "[{}] [{} {}:{}] ",
                getLevelName[level],
                Timestamp::now().toString(),
                FileUtil::getFileName(file),
                line);
            std::string logMessage = prefixStr + message + "\n";

            for (auto &stream : _streams)
            {
                stream->append(logMessage);
            }
        }

        /**
         * @brief 记录日志
         *
         * @tparam T 参数类型
         * @param file 文件名
         * @param line 行号
         * @param level 日志级别
         * @param msg 消息
         */
        void log_(const char *file, int line, LogLevel level, std::string_view msg)
        {

            if (!shouldLog(level))
                return;

            std::string prefixStr = fmt::format(
                "[{}] [{} {}:{}] ",
                getLevelName[level],
                Timestamp::now().toString(),
                FileUtil::getFileName(file),
                line);
            std::string logMessage = prefixStr + std::string(msg) + "\n";

            for (auto &stream : _streams)
            {
                stream->append(logMessage);
            }
        }

    private:
        LogLevel _level;                    // 日志级别
        std::vector<LogStreamPtr> _streams; // 日志流

    }; // class Logger
} // namespace schwi