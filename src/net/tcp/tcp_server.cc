#include "src/net/tcp/tcp_server.h"
#include "src/common/log.h"
#include "src/net/tcp/tcp_connection.h"


namespace mrpc
{

TcpServer::TcpServer(NetAddr::s_ptr addr)
    : m_addr(addr)
{

    init();

    LOG_INFO << "tcpserver listen succ on: " << m_addr->toString();
}

TcpServer::~TcpServer()
{
    if (m_main_event_loop) {
        delete m_main_event_loop;
        m_main_event_loop = nullptr;
    }
}


void TcpServer::init()
{

    // 创建 acceptor
    m_acceptor = std::make_shared<TcpAcceptor>(m_addr);

    // main reactor
    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    // sub reactors
    m_io_threads = new IOThreadPool(2);

    // main reactor监听事件fd
    listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    // 设置 main reactor 只负责监听可读事件, 只负责获取连接的client
    listen_fd_event->listen(FdEvent::EVENT_IN, std::bind(&TcpServer::onAccept, this));
    // 添加到 main reactor 的 eventloop
    m_main_event_loop->addEpollEvent(listen_fd_event);
}

void TcpServer::start()
{
    m_io_threads->start();
    m_main_event_loop->loop();
}

void TcpServer::onAccept()
{
    auto re = m_acceptor->accept();
    auto client_fd = re.first;
    auto peer_addr = re.second;
    //FdEvent client_fd_event(client_fd);
    m_client_counts++;

    // TODO: 把clientfd添加到 IO 线程
    IOThread *io_thread = m_io_threads->getIOThread();
    TcpConnection::s_ptr conn = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 1024, peer_addr, m_addr);
    conn->setState(ConnState::Connected);
    conn->setCloseCallback(std::bind(&TcpServer::onClientClose, this, std::placeholders::_1));
    m_clients.insert(conn);

    LOG_INFO << "TcpServer succ get client, fd: " << client_fd << ", all client count: " << m_client_counts;
}

void TcpServer::onClientClose(const TcpConnectionPtr &conn)
{
    LOG_INFO << "delete conn: " << conn->getPeerAddr()->toString();
    LOG_INFO << "conn.count: " << conn.use_count();

    auto size = m_clients.erase(conn);

    LOG_INFO << "conn.count: " << conn.use_count();
    LOG_INFO << "success delete count: " << size << ", current client count: " << m_clients.size();
}

}// namespace mrpc