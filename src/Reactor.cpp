#include "Reactor.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

Reactor::Reactor(EventLoop* loop)
    : m_loop(loop)
{
    m_loop->setEventCallback([this](int fd, uint32_t events){
        onEvent(fd, events);
    });

    m_threadPool = new ThreadPool(5);
}

Reactor::~Reactor()
{
    for (auto &[fd, conn] : m_conns)
    {
        close(fd);
    }

    for (auto fd : m_listenfds)
    {
        close(fd);
    }

    delete m_threadPool;
}

bool Reactor::addListenPort(uint16_t port)
{
    int fd = createServer(port);

    m_listenfds.push_back(fd);

    m_loop->addEvent(fd, EPOLLIN);

    return true;
}

int Reactor::createServer(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
        return -1;

    int opt = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(port);
    serAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (sockaddr *)&serAddr, sizeof(serAddr)) == -1)
    {
        perror("bind");
        return -2;
    }
    listen(fd, 10);
    int flags = fcntl(fd, F_GETFL, 0);

    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    std::cout << "Create Server at port " << port << std::endl;
    return fd;
}

void Reactor::handleAccept(int listenfd)
{
    sockaddr_in client{};
    socklen_t len = sizeof(client);

    int connfd = accept(listenfd, (sockaddr *)&client, &len);

    if (connfd < 0)
        return;

    int flags = fcntl(connfd, F_GETFL, 0);

    fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

    std::shared_ptr<Connection> conn = std::make_shared<Connection>();
    conn->fd = connfd;
    m_conns[connfd] = conn;

    m_loop->addEvent(connfd, EPOLLIN);

    std::cout << "accept: " << connfd << std::endl;
}

void Reactor::handleRead(int clientfd)
{
    auto &conn = m_conns[clientfd];
    char buf[BUFFER_LENGTH];
    int len = recv(clientfd, buf, BUFFER_LENGTH, 0);

    if (len == 0)
    {
        close(clientfd);
        m_conns.erase(clientfd);
        return;
    }

    conn->readBuffer.assign(buf, len);

    m_threadPool->addTask([this, conn]
    { 
        process(conn); 
        m_loop->queueInLoop([this, conn]{
            m_loop->updateEvent(conn->fd, EPOLLOUT);
        });         
    });
}

void Reactor::handleWrite(int clientfd)
{
    auto &conn = m_conns[clientfd];
    epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = clientfd;

    int len = send(clientfd, conn->writeBuffer.data(), conn->writeBuffer.size(), 0);

    m_loop->updateEvent(clientfd, EPOLLIN);
}

bool Reactor::isListenFd(int fd)
{
    return std::find(m_listenfds.begin(), m_listenfds.end(), fd) != m_listenfds.end();
}

void Reactor::process(std::shared_ptr<Connection> conn)
{

    std::stringstream ss;
    ss << std::this_thread::get_id();
    ss << ": ";
    std::string tid = ss.str();

    std::string response = tid + conn->readBuffer + "*_*";

    {
        std::lock_guard lock(conn->connMtx);
        conn->writeBuffer = response;
    }
}

void Reactor::onEvent(int fd, uint32_t events)
{
    if (isListenFd(fd))
    {
        handleAccept(fd);
    }
    else if (events & EPOLLIN)
    {
        handleRead(fd);
    }
    else if (events & EPOLLOUT)
    {
        handleWrite(fd);
    }
}
