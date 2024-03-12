
#include "src/common/log.h"
#include "src/common/config.h"

#if __cplusplus >= 201703L
#include <filesystem>
#endif
#include <sstream>
#include <stdio.h>
#include <sys/time.h>


#define CLR_CLR "\033[0m"      /* 恢复颜色 */
#define CLR_BLACK "\033[30m"   /* 黑色字 */
#define CLR_RED "\033[31m"     /* 红色字 */
#define CLR_GREEN "\033[32m"   /* 绿色字 */
#define CLR_YELLOW "\033[33m"  /* 黄色字 */
#define CLR_BLUE "\033[34m"    /* 蓝色字 */
#define CLR_PURPLE "\033[35m"  /* 紫色字 */
#define CLR_SKYBLUE "\033[36m" /* 天蓝字 */
#define CLR_WHITE "\033[37m"   /* 白色字 */

#define CLR_BLK_WHT "\033[40;37m"     /* 黑底白字 */
#define CLR_RED_WHT "\033[41;37m"     /* 红底白字 */
#define CLR_GREEN_WHT "\033[42;37m"   /* 绿底白字 */
#define CLR_YELLOW_WHT "\033[43;37m"  /* 黄底白字 */
#define CLR_BLUE_WHT "\033[44;37m"    /* 蓝底白字 */
#define CLR_PURPLE_WHT "\033[45;37m"  /* 紫底白字 */
#define CLR_SKYBLUE_WHT "\033[46;37m" /* 天蓝底白字 */
#define CLR_WHT_BLK "\033[47;30m"     /* 白底黑字 */

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
            return "INFO ";
            break;
        case WARNING:
            return "WARNI";
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
const char *levelToColor(LogLevel level)
{
    switch (level) {
        case DEBUG:
            return CLR_BLUE;
            break;
        case INFO:
            return CLR_GREEN;
            break;
        case WARNING:
            return CLR_YELLOW;
            break;
        case ERROR:
            return CLR_RED;
            break;
        case FATAL:
            return CLR_RED_WHT;
            break;

        default:
            return CLR_CLR;
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
std::string extractFileName(const std::string &absolutePath)
{

#if __cplusplus >= 201703L

    std::filesystem::path p(absolutePath);
    return p.filename().string();
#else
    size_t lastSlash = absolutePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return absolutePath.substr(lastSlash + 1);
    }
    return absolutePath;
#endif
}

Logger::~Logger()
{
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

LogStream &Logging::stream()
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

    m_stream << levelToColor(m_level) << "[" << LogLevelToString(m_level) << "]" << CLR_CLR
             << "[" << time_str << "]"
             << "[" << m_pid << ":" << m_thread_id << "]"
             << "[" << m_file_name << ":" << m_file_line << "] - " << levelToColor(m_level);

    return m_stream;
}

Logging::~Logging()
{
    m_stream << CLR_CLR;
    m_stream << "\n";

    // 输出到 stdout
    {
        size_t n = fwrite(m_stream.toString().c_str(), 1, m_stream.size(), stdout);
        (void) n;
    }
    // TODO: 写入文件
    {
    }

    //
    if (m_level == LogLevel::FATAL) {
        fflush(stdout);
        abort();
    }
}

}// namespace mrpc