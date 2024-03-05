#ifndef MRPC_COMMON_LOG_H
#define MRPC_COMMON_LOG_H

#include <memory>
#include <queue>
#include <string>

namespace mrpc
{

template <typename... Args> std::string formatString(const char *str, Args &&...args)
{
    int size = snprintf(nullptr, 0, str, args...);

    std::string result{};
    if (size > 0)
    {
        result.resize(size);
        snprintf(&result[0], size + 1, str, args...);
    }

    return result;
}

enum LogLevel
{
    DEBUG = 1,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

std::string LogLevelToString(LogLevel level);

class Logger
{
  public:
    typedef std::shared_ptr<Logger> s_ptr;
    void pushLog(const std::string &msg);
    void log();

  public:
    static Logger *GetGlobalLogger();

  private:
    LogLevel m_set_level;

    std::queue<std::string> m_buffer;
};

class LogEvent
{
  public:
    LogEvent(LogLevel level) : m_level(level)
    {
    }

  public:
    std::string getFileName() const
    {
        return m_file_name;
    }
    LogLevel getLogLevel() const
    {
        return m_level;
    }
    std::string toString();

  private:
    std::string m_file_name; // 文件名
    int32_t m_file_line;     // 行号
    int32_t m_pid;           // 进程id
    int32_t m_thread_id;     // 线程id

    LogLevel m_level; // 日志级别
    Logger::s_ptr m_logger;
};

#define LOG_DEBUG(str, ...)                                                                                            \
    std::string msg =                                                                                                  \
        (new mrpc::LogEvent(mrpc::LogLevel::DEBUG)->toString()) + mrpc::formatString(str, ##__VA_ARGS__);              \
    mrpc::Logger::GetGlobalLogger()->pushLog(msg);                                                                     \
    mrpc::Logger::GetGlobalLogger()->log();

} // namespace mrpc

#endif // MRPC_COMMON_LOG_H