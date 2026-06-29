#ifndef CHANNEL_H
#define CHANNEL_H

#include <cstdint>
#include <functional>
class EventLoop;

class Channel
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);

    void handleEvent();

    void enableRead();
    void enableWrite();
    void disableWrite();
    void disableAll();

    int getfd() const;
    uint32_t getEvents() const;

    void setRevents(uint32_t events);
    void setReadCallback(EventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);

private:
    EventLoop* m_loop;
    int m_fd;
    // 用户关注的事件
    uint32_t m_events;
    // epoll返回的实际事件
    uint32_t m_revents;

    EventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    EventCallback m_errorCallback;
};

#endif