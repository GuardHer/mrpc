#ifndef MRPC_NET_TCP_ABSTRACT_PROTOCOL_H
#define MRPC_NET_TCP_ABSTRACT_PROTOCOL_H

#include <memory>

namespace mrpc
{

class AbstractProtocol
{
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

private:
};

}// namespace mrpc

#endif//MRPC_NET_TCP_ABSTRACT_PROTOCOL_H