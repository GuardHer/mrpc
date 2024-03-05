#include "src/common/log.h"
#include "src/common/util.h"
#include <sys/time.h>
#include <sstream>
#include <stdio.h>

#define LOG_DEBUG(str, ...) \
	std::string msg = (new mrpc::LogEvent(mrpc::LogLevel::DEBUG)->toString()) + mrpc::formatString(str, ##__VA_ARGS__); \
	mrpc::g_logger->pushLog(msg); \
	mrpc::g_logger->log(); \

namespace mrpc
{
	static Logger *g_logger = nullptr;

	std::string LogLevelToString(LogLevel level)
	{
		switch (level)
		{
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

	Logger *Logger::GetGlobalLogger()
	{
		if (!g_logger)
			return g_logger;

		return new Logger();
	}

	void Logger::pushLog(const std::string &msg)
	{
		m_buffer.push(msg);
	}

	void Logger::log()
	{
		while (!m_buffer.empty())
		{
			std::string msg = m_buffer.front();
			m_buffer.pop();
			printf(msg.c_str());
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
		ss << "[" << LogLevelToString(m_level) << "]\t"
		   << "[" << time_str << "]\t"
		   << "[" << std::string(__FILE__) << __LINE__ << "]\t";

		return ss.str();
	}

}