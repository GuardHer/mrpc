#ifndef MRPC_NET_TCP_STRING_CODER_H
#define MRPC_NET_TCP_STRING_CODER_H

#include "src/net/coder/abstract_coder.h"

namespace mrpc
{

class StringProtocol : public AbstractProtocol
{
public:
    typedef std::shared_ptr<StringProtocol> s_ptr;

public:
    std::string info;
};

class StringCoder : public AbstractCoder
{
public:
    void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr &out_buffer) override
    {
        for (auto message: messages) {
            out_buffer->wirteToBuffer(dynamic_cast<StringProtocol *>(message.get())->info);
        }
    }
    void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer) override
    {
        StringProtocol::s_ptr msg = std::make_shared<StringProtocol>();
        std::string tmp           = buffer->readAllAsString();
        msg->info                 = tmp;
        msg->m_msg_id             = "123465";
        out_messages.push_back(msg);
    }
};

}// namespace mrpc

#endif//MRPC_NET_TCP_STRING_CODER_H