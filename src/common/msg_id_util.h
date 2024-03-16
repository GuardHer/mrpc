#ifndef MRPC_COMMON_MSG_ID_UTIL_H
#define MRPC_COMMON_MSG_ID_UTIL_H

#include <string>

namespace mrpc
{

class MsgIdUtil
{
public:
    static std::string GenMsgId();
};

}// namespace mrpc

#endif//MRPC_COMMON_MSG_ID_UTIL_H