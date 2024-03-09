#include "src/net/tcp/tcp_server.h"
#include "src/common/log.h"

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
    int client_fd = m_acceptor->accept();
    //FdEvent client_fd_event(client_fd);
    m_client_counts++;

    // TODO: 把clientfd添加到 IO 线程
    //m_io_threads->getIOThread()->getEventLoop()->addEpollEvent();

    LOG_INFO << "TcpServer succ get client, fd: " << client_fd << ", all client count: " << m_client_counts;
}

}// namespace mrpc