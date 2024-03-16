#include <functional>
#include <iostream>
#include <memory>

// 定义模拟的 TcpConnection 类
class TcpConnection
{
public:
    // 省略 TcpConnectionPtr 的定义
};

// 定义模拟的 AbstractProtocol 类
class AbstractProtocol
{
public:
    virtual ~AbstractProtocol(){};
    virtual void print() const = 0;


    virtual void setCnt(int c) = 0;

public:
    int cnt = 100;
};

class TinyPbProtocol : public AbstractProtocol
{
public:
    void print() const override
    {
        std::cout << "cnt: " << cnt << std::endl;
        std::cout << "cnt2: " << cnt2 << std::endl;
    }

    void setCnt(int c) override
    {
        cnt = c;
        cnt2 = c + cnt;
    }

    void testprint()
    {
        std::cout << "Hello World!" << std::endl;
    }

public:
    int cnt2 = 1000;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::shared_ptr<AbstractProtocol> AbstractProtocolPtr;
typedef std::shared_ptr<TinyPbProtocol> TinyPbProtocolPtr;

// 定义 RpcChannel 类
class RpcChannel
{
public:
    // 定义连接回调函数类型
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &, const AbstractProtocolPtr &)>;

    // 设置连接回调函数
    void setConnectionCallBack(const ConnectionCallback &cb)
    {
        m_conn_callback = cb;
    }

    // 模拟连接建立
    void establishConnection()
    {
        // 假设连接成功
        TcpConnectionPtr conn;       // 模拟连接对象
        AbstractProtocolPtr protocol;// 模拟协议对象
        if (m_conn_callback) {
            m_conn_callback(nullptr, nullptr);// 调用连接回调函数
        }
    }

private:
    ConnectionCallback m_conn_callback;
};

// 定义 Client 类
class Client
{
public:
    // 设置连接回调函数
    void setConnectionCallbackForRpcChannel()
    {
        TinyPbProtocolPtr protocol = std::make_shared<TinyPbProtocol>();
        protocol->setCnt(100);
        // 将当前类的 onConnection 方法设置为回调函数
        rpc_channel.setConnectionCallBack(std::bind(&Client::onConnection, this, std::placeholders::_1, protocol));
    }

    // 连接建立时的处理方法
    void onConnection(const TcpConnectionPtr &conn, const AbstractProtocolPtr &protocol)
    {
        auto message = std::dynamic_pointer_cast<TinyPbProtocol>(protocol);
        message->print();
        message->setCnt(1555);
        std::cout << std::endl;
        message->print();
        message->testprint();
    }

public:
    RpcChannel rpc_channel;
};

int main()
{
    Client client;
    // 创建或获取 AbstractProtocol 对象

    // 设置连接回调函数并传递 protocol 对象
    client.setConnectionCallbackForRpcChannel();
    // 模拟连接建立
    client.rpc_channel.establishConnection();
    return 0;
}
