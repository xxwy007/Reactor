#include "Reactor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "ThreadPool.h"
#include "common/RpcMessage.h"
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

Reactor::Reactor(EventLoop *loop,RpcDispatcher& dispatcher)
    : m_loop(loop)
    ,m_dispatcher(dispatcher)
{
    m_threadPool = new ThreadPool(5);
}

Reactor::~Reactor()
{
    delete m_threadPool;
}

bool Reactor::addListenPort(uint16_t port)
{
    int listenfd = createServer(port);

    auto channel = std::make_shared<Channel>(m_loop, listenfd);
    channel->setReadCallback(
        [this, listenfd]
        {
            handleAccept(listenfd);
        });

    channel->enableRead();

    m_acceptChannels[listenfd] = channel;
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

    std::shared_ptr<Connection> conn = std::make_shared<Connection>(m_loop, connfd);

    auto channel = conn->channel();
    m_acceptChannels[connfd] = channel;

    channel->setReadCallback(
        [this, connfd]
        {
            handleRead(connfd);
        });

    channel->setWriteCallback(
        [this, connfd]
        {
            handleWrite(connfd);
        });

    channel->enableRead();
    conn->channel() = channel;
    m_conns[connfd] = conn;

    std::cout << "accept: " << connfd << std::endl;
}

void Reactor::handleRead(int clientfd)
{
    auto &conn = m_conns[clientfd];
    char buf[BUFFER_LENGTH];
    int len = recv(clientfd, buf, BUFFER_LENGTH, 0);

    if (len <= 0)
    {
        auto conn = m_conns[clientfd];
        conn->channel()->disableAll();
        m_loop->removeChannel(conn->channel().get());

        close(clientfd);
        m_conns.erase(clientfd);
        return;
    }

    conn->readBuffer.append(buf, len);

    while (1)
    {
        RpcPacket req;

        if (!RpcCodec::decode(conn->readBuffer, req))
        {
            break;
        }

        m_threadPool->addTask([this, conn, req]
        { 
            RpcPacket rsp = m_dispatcher.dispatch(req);

            m_loop->queueInLoop([conn, rsp] {
                RpcCodec::encode(rsp, conn->writeBuffer);

                conn->channel()->enableWrite();
            });
        });
    }
}

void Reactor::handleWrite(int clientfd)
{
    auto &conn = m_conns[clientfd];
    int len = send(clientfd, conn->writeBuffer.peek(), conn->writeBuffer.readableBytes(), 0);
    if (len <= 0)
    {
        close(clientfd);
        m_conns.erase(clientfd);
        return;
    }

    conn->writeBuffer.retrieve(len);


    if (conn->writeBuffer.readableBytes() == 0)
    {
        conn->channel()->disableWrite();
    }
}

std::string Reactor::process(RpcMessage& msg)
{

    std::stringstream ss;
    ss << std::this_thread::get_id();
    ss << ": ";
    std::string tid = ss.str();

    std::string response = tid + "*_*";

    return response;
}
