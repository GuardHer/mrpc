
#include "log.h"
#include "config.h"
#include "util.h"

#include <sstream>
#include <stdio.h>
#include <sys/time.h>

namespace mrpc
{
static Logger *g_logger = nullptr;

std::string LogLevelToString(LogLevel level)
{
    switch (level) {
        case DEBUG:
            return "DEBUG";
            break;
        case INFO:
            return "INFO";
            break;
        case WARNING:
            return "WARNING";
            break;
        case ERROR:
            return "ERROR";
            break;
        case FATAL:
            return "FATAL";
            break;

        default:
            return "UNKNOWN";
            break;
    }
}
LogLevel StringToLogLevel(const std::string &string_level)
{
    if (string_level == "DEBUG")
        return LogLevel::DEBUG;
    else if (string_level == "INFO")
        return LogLevel::INFO;
    else if (string_level == "WARNING")
        return LogLevel::WARNING;
    else if (string_level == "ERROR")
        return LogLevel::ERROR;
    else if (string_level == "FATAL")
        return LogLevel::FATAL;
    else
        return LogLevel::UNKNOWN;
}


Logger *Logger::GetGlobalLogger()
{
    return g_logger;
}
void Logger::InitGlobalLogger()
{
    if (g_logger == nullptr) {
        LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
        g_logger = new Logger(global_log_level);
    }
}

void Logger::pushLog(const std::string &msg)
{
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(msg);
    lock.unlock();
}

void Logger::log()
{
    ScopeMutex<Mutex> lock(m_mutex);
    std::queue<std::string> tmp;
    m_buffer.swap(tmp);
    lock.unlock();

    while (!tmp.empty()) {
        std::string msg = tmp.front();
        tmp.pop();
        printf("%s\n", msg.c_str());
    }
}

std::string LogEvent::toString()
{
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);

    struct tm now_time_t;
    localtime_r(&now_time.tv_sec, &now_time_t);

    char buf[128];
    strftime(&buf[0], 123, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);

    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_thread_id = getThreadId();
    m_pid = getPid();

    std::stringstream ss;
    ss << "[" << LogLevelToString(m_level) << "]"
       << "[" << time_str << "]"
       << "[" << m_pid << ":" << m_thread_id << "]";
       //<< "[" << std::string(__FILE__) << ":" << __LINE__ << "]-";

    return ss.str();
}

}// namespace mrpc