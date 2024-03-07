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

public:
    std::string m_log_level;
};

}// namespace mrpc

#endif// MRPC_COMMON_CONFIG_H