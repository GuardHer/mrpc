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

/// @brief 日志级别转字符串
/// @param level 日志级别
/// @return string
std::string LogLevelToString(LogLevel level);

/// @brief 字符串转日志级别
/// @param string_level 日志字符串
/// @return loglevel
LogLevel StringToLogLevel(const std::string &string_level);

/// @brief 从绝对路径或者相对路径中获取文件名
/// @param absolutePath
/// @return string 文件名
std::string extractFileName(const std::string &absolutePath);

/// @brief 获取日志级别对应的color
/// @param level 日志级别
/// @return color对应的str
const char *levelToColor(LogLevel level);

class Logger
{
public:
    Logger(LogLevel level = LogLevel::DEBUG) : m_set_level(level) {}
    ~Logger();

public:
    typedef std::shared_ptr<Logger> s_ptr;

    /// @brief 将日志入队列, 写入日志到缓存
    /// @param msg
    void pushLog(const std::string &msg);

    /// @brief 处理队列中的日志(写入文件或者stdout)
    void log();

    /// @brief 获取g_logger对应的level
    /// @return m_set_level
    LogLevel getLogLevel() const { return m_set_level; }

public:
    /// @brief 获取全局logger对象
    /// @return g_logger
    static Logger *GetGlobalLogger();

    /// @brief 初始化全局日志对象: g_logger
    static void InitGlobalLogger();


private:
    LogLevel m_set_level;            // 全局日志级别
    mrpc::Mutex m_mutex;             // 互斥锁
    std::queue<std::string> m_buffer;// 日志缓存 (mutex)
};

class Logging
{
public:
    Logging(LogLevel level) : m_level(level) {}
    Logging(const char *file, int line, LogLevel level, const char *func)
        : m_file_name(extractFileName(std::string(file))), m_func_name(std::string(func)), m_file_line(line), m_level(level), m_thread_id(getThreadId()), m_pid(getPid()) {}

    ~Logging();

public:
    /// @brief  获取当前日志所在的文件
    /// @return string: m_file_name
    std::string getFileName() const { return m_file_name; }

    /// @brief 获取当前日志对应的级别
    /// @return LogLeval: m_level
    LogLevel getLogLevel() const { return m_level; }

    /// @brief 获取当前日志流对象, 注意要返回引用
    /// @return LogStream&: m_stream的引用
    LogStream &stream();

private:
    std::string m_file_name;// 文件名
    std::string m_func_name;// 函数名
    int m_file_line;        // 行号
    LogLevel m_level;       // 日志级别
    int32_t m_thread_id;    // 线程id
    int32_t m_pid;          // 进程id
    LogStream m_stream;     // 日志流
};

#define LOG_DEBUG                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::DEBUG) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::DEBUG, __func__).stream()
#define LOG_INFO                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::INFO) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::INFO, __func__).stream()
#define LOG_WARNING                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::WARNING) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::WARNING, __func__).stream()
#define LOG_ERROR                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::ERROR) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::ERROR, __func__).stream()
#define LOG_FATAL                                                                \
    if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::LogLevel::FATAL) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::FATAL, __func__).stream()

}// namespace mrpc

#endif// MRPC_COMMON_LOG_H