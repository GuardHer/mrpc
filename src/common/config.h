#ifndef MPRC_COMMON_CONFIG_H
#define MRPC_COMMON_CONFIG_H

#include <map>
#include <string>
namespace mrpc
{

class Config
{
public:
    Config(const char *xmlfile);

public:
    /// @brief  获取全局config对象
    /// @return g_config
    static Config *GetGlobalConfig();

    /// @brief 初始化全局config对象
    /// @param xmlfile : 配置xml文件路径
    static void SetGlobalConfig(const char *xmlfile);

    void setLogFileName(const std::string &fileName) { m_log_file_name = fileName; }

public:
    std::string m_log_level;      // 日志级别
    std::string m_log_file_name;  // 日志文件名
    std::string m_log_file_path;  // 日志路径
    int m_log_max_file_size { 0 };// 最大日志文件大小
    int m_log_sync_inteval { 0 }; // 间隔 ms
    int m_log_color { 0 };
};

}// namespace mrpc

#endif// MRPC_COMMON_CONFIG_H