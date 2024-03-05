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
    static Config *GetGlobalConfig();
    static void SetGlobalConfig(const char *xmlfile);

public:
    std::string m_log_level;
};

}// namespace mrpc

#endif// MRPC_COMMON_CONFIG_H