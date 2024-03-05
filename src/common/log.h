#ifndef MRPC_COMMON_LOG_H
#define MRPC_COMMON_LOG_H

#include "mutex.h"
#include <memory>
#include <queue>
#include <string>

namespace mrpc
{

template<typename... Args>
std::string formatString(const char *str, Args &&...args)
{
    int size = snprintf(nullptr, 0, str, args...);
    std::string result{};
    if (size > 0) {
        result.resize(size);
        snprintf(&result[0], size + 1, str, args...);
    }

    return result;
}


enum LogLevel
{
    UNKNOWN,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string &string_level);

class Logger
{
public:
    Logger(LogLevel level = LogLevel::DEBUG) : m_set_level(level) {}

public:
    typedef std::shared_ptr<Logger> s_ptr;
    void pushLog(const std::string &msg);
    void log();
    LogLevel getLogLevel() const { return m_set_level; }

public:
    static Logger *GetGlobalLogger();
    static void InitGlobalLogger();

private:
    LogLevel m_set_level;

    std::queue<std::string> m_buffer;

    mrpc::Mutex m_mutex;
};

class LogEvent
{
public:
    LogEvent(LogLevel level) : m_level(level) {}

public:
    std::string getFileName() const { return m_file_name; }
    LogLevel getLogLevel() const { return m_level; }
    std::string toString();

private:
    std::string m_file_name;// 文件名
    int32_t m_file_line;    // 行号
    int32_t m_pid;          // 进程id
    int32_t m_thread_id;    // 线程id

    LogLevel m_level;// 日志级别
    Logger::s_ptr m_logger;
};

#define LOG_DEBUG(str, ...)                                                                                                                                                                                               \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::DEBUG) {                                                                                                                                        \
        mrpc::Logger::GetGlobalLogger()->pushLog((new mrpc::LogEvent(mrpc::LogLevel::DEBUG))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]-" + mrpc::formatString(str, ##__VA_ARGS__)); \
        mrpc::Logger::GetGlobalLogger()->log();                                                                                                                                                                           \
    }

#define LOG_INFO(str, ...)                                                                                                                                                                                               \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::INFO) {                                                                                                                                        \
        mrpc::Logger::GetGlobalLogger()->pushLog((new mrpc::LogEvent(mrpc::LogLevel::INFO))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]-" + mrpc::formatString(str, ##__VA_ARGS__)); \
        mrpc::Logger::GetGlobalLogger()->log();                                                                                                                                                                          \
    }

}// namespace mrpc

#endif// MRPC_COMMON_LOG_H