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

Reactor::Reactor()
    : m_threadPool(5)
{
    m_epfd = epoll_create1(0);
    m_wakeupFd = eventfd(0, EFD_NONBLOCK);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_wakeupFd;

    epoll_ctl(
        m_epfd,
        EPOLL_CTL_ADD,
        m_wakeupFd,
        &ev);
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

    close(m_epfd);
}

bool Reactor::addListenPort(uint16_t port)
{
    int fd = createServer(port);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);

    m_listenfds.push_back(fd);

    return true;
}
void Reactor::run()
{
    epoll_event evs[MAX_EVENTS];

    while (1)
    {
        int nready = epoll_wait(m_epfd, evs, MAX_EVENTS, -1);

        for (int i = 0; i < nready; i++)
        {
            int fd = evs[i].data.fd;

            if (fd == m_wakeupFd)
            {
                handleWakeup();
            }
            else if (isListenFd(fd))
                handleAccept(fd);
            else if (evs[i].events & EPOLLIN)
                handleRead(fd);
            else if (evs[i].events & EPOLLOUT)
                handleWrite(fd);
        }
    }
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

    int flags =
        fcntl(connfd,
              F_GETFL,
              0);

    fcntl(connfd,
          F_SETFL,
          flags | O_NONBLOCK);

    std::shared_ptr<Connection> conn = std::make_shared<Connection>();
    conn->fd = connfd;

    m_conns[connfd] = conn;

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = connfd;

    epoll_ctl(m_epfd, EPOLL_CTL_ADD, connfd, &ev);

    std::cout << "accept: " << connfd << std::endl;
}

void Reactor::handleWakeup()
{
    uint64_t val;

    read(m_wakeupFd, &val, sizeof(val));

    std::queue<int> localQueue;

    {
        std::lock_guard lock(m_pendingMtx);
        std::swap(localQueue, m_pendingWrite);
    }

    while (!localQueue.empty())
    {
        int fd = localQueue.front();

        localQueue.pop();

        epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.fd = fd;
        epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
    }
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

    m_threadPool.addTask([this, conn]
                         { process(conn); });
}

void Reactor::handleWrite(int clientfd)
{
    auto &conn = m_conns[clientfd];
    epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = clientfd;

    int len = send(clientfd, conn->writeBuffer.data(), conn->writeBuffer.size(), 0);

    epoll_ctl(m_epfd, EPOLL_CTL_MOD, clientfd, &ev);
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

    {
        std::lock_guard lock(m_pendingMtx);
        m_pendingWrite.push(conn->fd);
    }
    uint64_t one = 1;

    write(m_wakeupFd, &one, sizeof(one));
}
