
#include "src/common/log.h"

#include "src/common/config.h"
#include "src/net/eventloop.h"

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
Logger g_logger        = singleton<Logger>::get_mutable_instance();
static int g_log_color = 0;

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


Logger::Logger(LogLevel level) : m_set_level(level)
{
    printf("---------------------------Logger---------------------------------\n");
}

void Logger::syncLoop()
{
    std::vector<std::string> tmp;
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.swap(tmp);
    lock.unlock();

    m_async_logger->push(tmp);
}

Logger::~Logger()
{
}

void Logger::InitGlobalLogger()
{
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    g_log_color               = Config::GetGlobalConfig()->m_log_color;
    g_logger.setLevel(global_log_level);

    if (g_logger.m_async_logger.use_count() == 0 &&
        g_logger.m_timer_event.use_count() == 0) {
        g_logger.initLog();
    }
}


void Logger::pushLog(const std::string &msg)
{
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push_back(msg);
    lock.unlock();
}

void Logger::initLog()
{
    auto config    = Config::GetGlobalConfig();
    m_async_logger = std::make_shared<AsyncLogger>(config->m_log_file_name, config->m_log_file_path, config->m_log_max_file_size);
    m_timer_event  = std::make_shared<TimerEvent>(config->m_log_sync_inteval, true, std::bind(&Logger::syncLoop, this));
    EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
}

////////////////////// AsyncLogger /////////////////////////////
AsyncLogger::AsyncLogger(const std::string &file_name, const std::string &file_path, int32_t max_size)
    : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size)
{
    // 初始化信号量
    sem_init(&m_semaphore, 0, 0);
    // 初始化条件变量
    pthread_cond_init(&m_condtion, nullptr);
    // 创建线程
    pthread_create(&m_thread, nullptr, &AsyncLogger::Loop, this);

    // 等待信号量
    sem_wait(&m_semaphore);
}

AsyncLogger::~AsyncLogger()
{
    // 唤醒线程
    pthread_cond_signal(&m_condtion);
    stop();
    flush();
    if (m_file_hanlder) {
        fclose(m_file_hanlder);
        if (m_file_hanlder) {
            delete m_file_hanlder;
            m_file_hanlder = nullptr;
        }
    }
}

void AsyncLogger::push(const std::vector<std::string> &buffer)
{
    ScopeMutex<Mutex> guard_lock(m_mutex);
    m_buffer.push(buffer);
    guard_lock.unlock();

    // 唤醒线程
    pthread_cond_signal(&m_condtion);
}

void AsyncLogger::stop()
{
    m_stop_flag = true;
}

void AsyncLogger::flush()
{
    if (m_file_hanlder) {
        fflush(m_file_hanlder);
    }
}


void *AsyncLogger::Loop(void *arg)
{
    AsyncLogger *async_logger = static_cast<AsyncLogger *>(arg);
    sem_post(&async_logger->m_semaphore);
    while (1) {
        /// 获取日志数据
        ScopeMutex<Mutex> guard_lock(async_logger->m_mutex);
        while (async_logger->m_buffer.empty()) {
            // 等待被唤醒
            pthread_cond_wait(&(async_logger->m_condtion), async_logger->m_mutex.getMutex());
        }
        std::vector<std::string> tmp;
        tmp.swap(async_logger->m_buffer.front());
        async_logger->m_buffer.pop();
        guard_lock.unlock();

        /// 获取当前时间 "%Y%m%d", eg: 20240317
        std::string time_str = getCurrentTimeDayString();
        if (time_str != async_logger->m_date) {
            async_logger->m_no          = 0;
            async_logger->m_reopen_flag = true;
            async_logger->m_date        = time_str;
        }
        if (async_logger->m_file_hanlder == nullptr) {
            // 如果文件描述符为空, 设置需要重新打开文件描述符
            async_logger->m_reopen_flag = true;
        }

        /// 构建path
        std::stringstream ss;
        ss << async_logger->m_file_path << "/"
           << async_logger->m_file_name << "_"
           << time_str << ".";
        std::string log_file_name = ss.str() + std::to_string(async_logger->m_no) + ".log";

        // printf("%s: %d\n", log_file_name.c_str(), async_logger->m_buffer.size());

        /// 判断是否需要重新打开文件描述符
        if (async_logger->m_reopen_flag) {
            if (async_logger->m_file_hanlder) {
                fclose(async_logger->m_file_hanlder);
            }
            async_logger->m_file_hanlder = fopen(log_file_name.c_str(), "a");
            async_logger->m_reopen_flag  = false;
        }

        /// 检查 当前文件大小是否超过设定的最大大小, 超过则重新打开一个文件
        if (ftell(async_logger->m_file_hanlder) > async_logger->m_max_file_size) {
            if (async_logger->m_file_hanlder) {
                fclose(async_logger->m_file_hanlder);
            }
            log_file_name                = ss.str() + std::to_string(++async_logger->m_no) + ".log";
            async_logger->m_file_hanlder = fopen(log_file_name.c_str(), "a");
            async_logger->m_reopen_flag  = false;
        }

        // 写入文件
        for (auto &i: tmp) {
            if (!i.empty()) {
                fwrite(i.c_str(), 1, i.length(), async_logger->m_file_hanlder);
            }
        }

        // 刷新
        fflush(async_logger->m_file_hanlder);

        if (async_logger->m_stop_flag) {
            break;
        }
    }
    return nullptr;
}


////////////////////// Logging /////////////////////////////
LogStream &Logging::stream()
{
    std::string time_str = getCurrentTimeMillisecondsString();

    if (g_log_color) {
        m_stream << levelToColor(m_level) << "[" << LogLevelToString(m_level) << "]" << CLR_CLR
                 << "[" << time_str << "]"
                 << "[" << m_pid << ":" << m_thread_id << "]"
                 << "[" << m_file_name << ":" << m_file_line << "] - " << levelToColor(m_level);
    } else {
        m_stream << "[" << LogLevelToString(m_level) << "]"
                 << "[" << time_str << "]"
                 << "[" << m_pid << ":" << m_thread_id << "]"
                 << "[" << m_file_name << ":" << m_file_line << "] - ";
    }

    return m_stream;
}

Logging::~Logging()
{
    if (g_log_color) {
        m_stream << CLR_CLR;
    }
    m_stream << "\n";

    std::string tmp = m_stream.toString();

    // 输出到 stdout
    {
        size_t n = fwrite(tmp.c_str(), 1, m_stream.size(), stdout);
        (void) n;
    }
    // TODO: 写入文件
    // Logger::GetGlobalLogger()->pushLog(tmp);
    g_logger.pushLog(tmp);

    //
    if (m_level == LogLevel::FATAL) {
        fflush(stdout);
        abort();
    }
}

}// namespace mrpc