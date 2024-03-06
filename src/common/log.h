#ifndef MRPC_COMMON_LOG_H
#define MRPC_COMMON_LOG_H

#include "src/common/log_stream.h"
#include "src/common/mutex.h"
#include "src/common/util.h"
#include <memory>
#include <queue>
#include <string>

namespace mrpc
{

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
std::string extractFileName(const std::string &absolutePath);
const char *levelToColor(LogLevel level);

class Logger
{
public:
    Logger(LogLevel level = LogLevel::DEBUG) : m_set_level(level) {}
    ~Logger();

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
    LogEvent(const char *file, int line, LogLevel level, const char *func)
        : m_file_name(extractFileName(std::string(file))), m_func_name(std::string(func)), m_file_line(line), m_level(level), m_thread_id(getThreadId()), m_pid(getPid()) {}

    ~LogEvent();

public:
    std::string getFileName() const { return m_file_name; }
    LogLevel getLogLevel() const { return m_level; }
    /// 返回LogStream对象, 注意要返回引用
    LogStream &stream();

private:
    std::string m_file_name;// 文件名
    std::string m_func_name;// 函数名
    int m_file_line;        // 行号
    LogLevel m_level;       // 日志级别
    int32_t m_thread_id;    // 线程id
    int32_t m_pid;          // 进程id


    LogStream m_stream;
};

#define LOG_DEBUG                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::DEBUG) \
    mrpc::LogEvent(__FILE__, __LINE__, mrpc::LogLevel::DEBUG, __func__).stream()
#define LOG_INFO                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::INFO) \
    mrpc::LogEvent(__FILE__, __LINE__, mrpc::LogLevel::INFO, __func__).stream()
#define LOG_WARNING                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::WARNING) \
    mrpc::LogEvent(__FILE__, __LINE__, mrpc::LogLevel::WARNING, __func__).stream()
#define LOG_ERROR                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::ERROR) \
    mrpc::LogEvent(__FILE__, __LINE__, mrpc::LogLevel::ERROR, __func__).stream()
#define LOG_FATAL                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::FATAL) \
    mrpc::LogEvent(__FILE__, __LINE__, mrpc::LogLevel::FATAL, __func__).stream()

}// namespace mrpc

#endif// MRPC_COMMON_LOG_H