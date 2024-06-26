#ifndef MRPC_COMMON_LOG_H
#define MRPC_COMMON_LOG_H

#include "src/common/log_stream.h"
#include "src/common/mutex.h"
#include "src/common/singleton.h"
#include "src/common/util.h"
#include "src/net/timer_event.h"
#include <memory>
#include <queue>
#include <semaphore.h>
#if __cplusplus >= 202002L
#include <source_location>// std::source_location
#endif
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

class AsyncLogger;

class Logger
{
public:
    Logger(LogLevel level = LogLevel::DEBUG);
    ~Logger();

public:
    typedef std::shared_ptr<Logger> s_ptr;

    /// @brief 将日志入队列, 写入日志到缓存
    /// @param msg
    void pushLog(const std::string &msg);

    /// @brief 处理队列中的日志(写入文件或者stdout)
    void initLog();

    /// @brief 获取g_logger对应的level
    /// @return m_set_level
    LogLevel getLogLevel() const { return m_set_level; }

    /// @brief
    void syncLoop();

    /// @brief 设置日志级别
    /// @param level
    void setLevel(LogLevel level) { m_set_level = level; }

public:
    /// @brief 获取全局logger对象
    /// @return g_logger
    static Logger *GetGlobalLogger();

    /// @brief 初始化全局日志对象: g_logger
    static void InitGlobalLogger();


private:
    LogLevel m_set_level;                       // 全局日志级别
    mrpc::Mutex m_mutex;                        // 互斥锁
    std::vector<std::string> m_buffer;          // 日志缓存 (mutex)
    std::string m_file_name;                    // 日志输出文件名
    std::string m_file_path;                    // 日志输出路径
    int32_t m_max_file_size { 0 };              // 日志单个文件最大大小
    std::shared_ptr<AsyncLogger> m_async_logger;//
    std::shared_ptr<TimerEvent> m_timer_event;  //
};

class AsyncLogger
{
    /// {m_file_path}/{m_file_name}_yyyymmdd.{m_no}.log
public:
    typedef std::queue<std::vector<std::string>> LogQueue;

    AsyncLogger(const std::string &file_name, const std::string &file_path, int32_t max_size);
    ~AsyncLogger();

public:
    /// @brief 将 buffer 里面的数据输出到文件
    static void *Loop(void *arg);

    /// @brief
    void push(const std::vector<std::string> &buffer);

    /// @brief 关闭异步日志
    void stop();

    /// @brief 刷新到磁盘
    void flush();


private:
    LogQueue m_buffer;               // 日志缓存 (mutex)
    std::string m_file_name;         // 日志输出文件名
    std::string m_file_path;         // 日志输出路径
    int32_t m_max_file_size { 0 };   // 日志单个文件最大大小
    sem_t m_semaphore;               // 信号量
    pthread_t m_thread;              // 线程
    pthread_cond_t m_condtion;       // 条件变量
    mrpc::Mutex m_mutex;             // 互斥锁
    std::string m_date;              // 当前打印日志的文件日期
    FILE *m_file_hanlder { nullptr };// 当前打开的日志文件句柄
    bool m_reopen_flag { false };    // 是否需要重新打开日志文件句柄
    int32_t m_no { 0 };              // 日志文件序号
    bool m_stop_flag { false };      // 退出循环
};


class Logging
{
public:
#if __cplusplus >= 202002L
    Logging(LogLevel level, const std::source_location &loc = std::source_location::current())
        : m_level(level), m_file_name(extractFileName(loc.file_name())), m_func_name(loc.function_name()), m_file_line(loc.line()), m_thread_id(getThreadId()), m_pid(getPid()) {};
#endif
    Logging(const char *file, int line, LogLevel level, const char *func)
        : m_level(level), m_file_name(extractFileName(std::string(file))), m_func_name(std::string(func)), m_file_line(line), m_thread_id(getThreadId()), m_pid(getPid()) {}
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
    LogLevel m_level { LogLevel::UNKNOWN };// 日志级别
    std::string m_file_name;               // 文件名
    std::string m_func_name;               // 函数名
    int m_file_line { -1 };                // 行号
    int32_t m_thread_id { -1 };            // 线程id
    int32_t m_pid { -1 };                  // 进程id
    LogStream m_stream;                    // 日志流
};

extern Logger g_logger;

#if __cplusplus >= 202002L
#define LOG_DEBUG                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::DEBUG) \
    mrpc::Logging(mrpc::LogLevel::DEBUG).stream()
#define LOG_INFO                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::INFO) \
    mrpc::Logging(mrpc::LogLevel::INFO).stream()
#define LOG_WARNING                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::WARNING) \
    mrpc::Logging(mrpc::LogLevel::WARNING).stream()
#define LOG_ERROR                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::ERROR) \
    mrpc::Logging(mrpc::LogLevel::ERROR).stream()
#define LOG_FATAL                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::FATAL) \
    mrpc::Logging(mrpc::LogLevel::FATAL).stream()

#else
#define LOG_DEBUG                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::DEBUG) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::DEBUG, __func__).stream()
#define LOG_INFO                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::INFO) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::INFO, __func__).stream()
#define LOG_WARNING                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::WARNING) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::WARNING, __func__).stream()
#define LOG_ERROR                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::ERROR) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::ERROR, __func__).stream()
#define LOG_FATAL                                              \
    if (mrpc::g_logger.getLogLevel() <= mrpc::LogLevel::FATAL) \
    mrpc::Logging(__FILE__, __LINE__, mrpc::LogLevel::FATAL, __func__).stream()
#endif

}// namespace mrpc

#endif// MRPC_COMMON_LOG_H