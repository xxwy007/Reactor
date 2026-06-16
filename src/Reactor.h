#ifndef REACTOR_H
#define REACTOR_H

#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <sys/eventfd.h>
#include "ThreadPool.h"
#include "EventLoop.h"

#define BUFFER_LENGTH 1024
struct Connection
{
    int fd{-1};

    std::string readBuffer;
    std::string writeBuffer;

    uint32_t events{0};

    std::mutex connMtx;
};

class Reactor
{
public:
    Reactor(EventLoop* loop);
    ~Reactor();

    bool addListenPort(uint16_t port = 9999);
    void run();

private:
    int createServer(uint16_t port);

    void handleAccept(int listenfd);
    void handleWakeup();
    void handleRead(int clientfd);
    void handleWrite(int clientfd);

    bool isListenFd(int fd);

    void process(std::shared_ptr<Connection> conn);
    void onEvent(int fd, uint32_t events);

private:
    ThreadPool* m_threadPool;
    EventLoop* m_loop;

    std::vector<int> m_listenfds;
    std::unordered_map<int, std::shared_ptr<Connection>> m_conns;
};

#endif