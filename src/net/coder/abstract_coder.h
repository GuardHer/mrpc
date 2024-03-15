#ifndef MRPC_NET_TCP_ABSTRACT_CODER_H
#define MRPC_NET_TCP_ABSTRACT_CODER_H

#include "src/net/coder/abstract_protocol.h"
#include "src/net/tcp/tcp_buffer.h"
#include <vector>

namespace mrpc
{

class AbstractCoder
{
public:
    typedef std::shared_ptr<AbstractCoder> s_ptr;

    virtual ~AbstractCoder(){};

public:
    /// @brief 编码, 将message对象转为字节流, 写入buffer
    virtual void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr &out_buffer) = 0;
    /// @brief 解码, 将buffer字节流转为message对象
    virtual void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer) = 0;
};

}// namespace mrpc

#endif//MRPC_NET_TCP_ABSTRACT_CODER_H