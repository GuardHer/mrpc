#ifndef MRPC_NET_CODER_TINYPB_CODER_H
#define MRPC_NET_CODER_TINYPB_CODER_H

#include "src/net/coder/abstract_coder.h"
#include "src/net/coder/tinypb_protocol.h"
#include <memory>

namespace mrpc
{

class TinyPBCoder : public AbstractCoder
{
public:
    void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr &out_buffer) override;
    void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer) override;

private:
    std::string encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int &len);
    bool checkPack(const std::shared_ptr<TinyPBProtocol> &message);
};

}// namespace mrpc

#endif//MRPC_NET_CODER_TINYPB_CODER_H