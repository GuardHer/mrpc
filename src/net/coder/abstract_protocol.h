#ifndef MRPC_NET_TCP_ABSTRACT_PROTOCOL_H
#define MRPC_NET_TCP_ABSTRACT_PROTOCOL_H

#include <memory>
#include <string>

#include "src/net/tcp/tcp_buffer.h"

namespace mrpc
{

struct AbstractProtocol
{
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;
    virtual ~AbstractProtocol(){};

public:
    std::string m_msg_id;

private:
};

}// namespace mrpc

#endif//MRPC_NET_TCP_ABSTRACT_PROTOCOL_H