#include "src/common/config.h"
#include <tinyxml2.h>

using namespace tinyxml2;

#define READ_XML_NODE(name, root)                                            \
    XMLElement *name##_node = root->FirstChildElement(#name);                \
    if (!name##_node) {                                                      \
        printf("Start mrpc server error, faild to read node [%s]\n", #name); \
        exit(0);                                                             \
    }

#define READ_STR_XML_NODE(name, root)                                        \
    XMLElement *name##_node = root->FirstChildElement(#name);                \
    if (!name##_node || name##_node->GetText() == nullptr) {                 \
        printf("Start mrpc server error, faild to read node [%s]\n", #name); \
        exit(0);                                                             \
    }                                                                        \
    std::string name##_str = std::string(name##_node->GetText());

namespace mrpc
{
static Config *g_config = nullptr;

Config::Config(const char *xmlfile)
{
    XMLDocument *xml_doc = new XMLDocument();
    XMLError ret         = xml_doc->LoadFile(xmlfile);
    if (ret != XML_SUCCESS) {
        printf("Start mrpc server error, faild to read config xml file: %s\n", xmlfile);
        exit(0);
    }

    READ_XML_NODE(root, xml_doc);
    READ_XML_NODE(log, root_node);

    READ_STR_XML_NODE(log_level, log_node);

    m_log_level = log_level_str;
}


void Config::SetGlobalConfig(const char *xmlfile)
{
    if (g_config == nullptr) {
        g_config = new Config(xmlfile);
    }
}
Config *Config::GetGlobalConfig()
{
    return g_config;
}

}// namespace mrpc