#ifndef REACTOR_H
#define REACTOR_H
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <sys/eventfd.h>
#include "Channel.h"
#include "common/Buffer.h"
#include "RpcDispatcher.h"
#define BUFFER_LENGTH 1024

class EventLoop;
class ThreadPool;
class RpcMessage;
class Connection
{
public:
    Connection(EventLoop *loop, int fd)
        : m_fd(fd)
    {
        m_channel = std::make_shared<Channel>(loop, fd);
    }

    int fd() const
    {
        return m_fd;
    }

    std::shared_ptr<Channel> channel()
    {
        return m_channel;
    }

public:
    Buffer readBuffer;
    Buffer writeBuffer;

private:
    int m_fd;
    std::shared_ptr<Channel> m_channel;
};

class Reactor
{
public:
    Reactor(EventLoop *loop, RpcDispatcher& dispatcher);
    ~Reactor();

    bool addListenPort(uint16_t port = 9999);

    void registerService(const std::string &name, std::shared_ptr<Service> service);

private:
    int createServer(uint16_t port);

    void handleAccept(int listenfd);
    void handleRead(int clientfd);
    void handleWrite(int clientfd);

    std::string process(RpcMessage& msg);

private:
    ThreadPool* m_threadPool;
    EventLoop *m_loop;
    RpcDispatcher m_dispatcher;


    std::unordered_map<int, std::shared_ptr<Channel>> m_acceptChannels;
    std::unordered_map<int, std::shared_ptr<Connection>> m_conns;

    std::mutex m_connMtx;
};

#endif