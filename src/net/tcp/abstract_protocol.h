#ifndef MRPC_NET_TCP_ABSTRACT_PROTOCOL_H
#define MRPC_NET_TCP_ABSTRACT_PROTOCOL_H

#include <memory>
#include <string>

#include "src/net/tcp/tcp_buffer.h"

namespace mrpc
{

class AbstractProtocol
{
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;
    virtual ~AbstractProtocol(){};

public:
    std::string getReqId() const { return m_req_id; }
    void setReqId(const std::string &value) { m_req_id = value; }

protected:
    std::string m_req_id;

private:
};

}// namespace mrpc

#endif//MRPC_NET_TCP_ABSTRACT_PROTOCOL_H