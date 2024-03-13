#ifndef MRPC_NET_CALLBACK_H
#define MRPC_NET_CALLBACK_H

#include <functional>
#include <memory>

namespace mrpc
{

class TcpBuffer;
class AbstractProtocol;
class TcpConnection;

typedef std::shared_ptr<AbstractProtocol> AbstractProtocolPtr;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const AbstractProtocolPtr &)> WriteCompleteCallback;
typedef std::function<void(const AbstractProtocolPtr &)> ReadCallback;
typedef std::function<void(const AbstractProtocolPtr &, TcpBuffer *)> MessageCallback;


}// namespace mrpc

#endif//MRPC_NET_CALLBACK_H