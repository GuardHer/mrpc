
#include "src/common/log.h"

#include "src/common/config.h"

#if __cplusplus >= 201703L
#include <filesystem>
#endif
#include <stdio.h>
#include <sys/time.h>

#include <sstream>


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
        g_logger                  = new Logger(global_log_level);
    }
}


void Logger::pushLog(const std::string &msg)
{
    ScopeMutex<Mutex> lock(m_mutex);
    // m_buffer: vector
    m_buffer.push_back(msg);
    lock.unlock();
}

void Logger::log()
{
    ScopeMutex<Mutex> lock(m_mutex);
    std::vector<std::string> tmp;
    m_buffer.swap(tmp);
    lock.unlock();

    // while (!tmp.empty()) {
    //     std::string msg = tmp.front();
    //     tmp.
    // }
    // printf("%s\n", msg.c_str());
}

////////////////////// AsyncLogger /////////////////////////////
AsyncLogger::AsyncLogger(const std::string &file_name, const std::string &file_path, int32_t max_size)
    : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size)
{
    // 初始化信号量
    sem_init(&m_semaphore, 0, 0);
    // 创建线程
    pthread_create(&m_thread, nullptr, &AsyncLogger::Loop, this);
    // 等待信号量
    sem_wait(&m_semaphore);

    // 初始化条件变量
    pthread_cond_init(&m_condtion, nullptr);
}

void *AsyncLogger::Loop(void *arg)
{
    AsyncLogger *async_logger = static_cast<AsyncLogger *>(arg);
    sem_post(&async_logger->m_semaphore);
    while (1) {
        ScopeMutex<Mutex> guard_lock(async_logger->m_mutex);
        while (async_logger->m_buffer.empty()) {
            pthread_cond_wait(&(async_logger->m_condtion), async_logger->m_mutex.getMutex());
        }
        std::vector<std::string> tmp;
        tmp.swap(async_logger->m_buffer.front());
        async_logger->m_buffer.pop();
        guard_lock.unlock();

        // 获取当前时间 "%Y%m%d", eg: 20240317
        std::string time_str = getCurrentTimeDayString();
        if (time_str != async_logger->m_date) {
            async_logger->m_no          = 0;
            async_logger->m_reopen_flag = true;
            async_logger->m_date        = time_str;
        }
        if (async_logger->m_file_hanlder == nullptr) {
            async_logger->m_reopen_flag = true;
        }
    }
    return nullptr;
}


////////////////////// Logging /////////////////////////////
LogStream &Logging::stream()
{
    std::string time_str = getCurrentTimeMillisecondsString();

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