#ifndef MRPC_NET_TCP_STRING_CODER_H
#define MRPC_NET_TCP_STRING_CODER_H

#include "src/net/tcp/abstract_coder.h"

namespace mrpc
{

class StringProtocol : public AbstractProtocol
{
public:
    std::string info;
};

class StringCoder : public AbstractCoder
{
public:
    void encode(std::vector<AbstractProtocol *> &messages, TcpBuffer::s_ptr out_buffer) override
    {
        for (auto message: messages) {
            out_buffer->wirteToBuffer(dynamic_cast<StringProtocol *>(message)->info);
        }
    }
    void decode(std::vector<AbstractProtocol *> &out_messages, TcpBuffer::s_ptr buffer) override
    {
        StringProtocol *msg = new StringProtocol();
        std::string tmp = buffer->readAllAsString();
        msg->info = tmp;
        out_messages.push_back(msg);
    }
};

}// namespace mrpc

#endif//MRPC_NET_TCP_STRING_CODER_H