#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>

Channel::Channel(EventLoop* loop, int fd)
    : m_loop(loop),
      m_fd(fd),
      m_events(0),
      m_revents(0)
{
}

void Channel::handleEvent()
{
    if(m_revents & EPOLLERR)
    {
        if(m_errorCallback)
            m_errorCallback();
    }


    if(m_revents & EPOLLHUP)
    {
        if(m_closeCallback)
            m_closeCallback();
    }


    if(m_revents & EPOLLIN)
    {
        if(m_readCallback)
            m_readCallback();
    }


    if(m_revents & EPOLLOUT)
    {
        if(m_writeCallback)
            m_writeCallback();
    }
}

int Channel::getfd() const
{
    return m_fd;
}

uint32_t Channel::getEvents() const
{
    return m_events;
}

void Channel::setReadCallback(EventCallback cb)
{
    m_readCallback = std::move(cb);
}

void Channel::setWriteCallback(EventCallback cb)
{
    m_writeCallback = std::move(cb);
}

void Channel::setCloseCallback(EventCallback cb)
{
    m_closeCallback = std::move(cb);
}

void Channel::setErrorCallback(EventCallback cb)
{
    m_errorCallback = std::move(cb);
}

void Channel::enableRead()
{
    m_events |= EPOLLIN;

    m_loop->updateChannel(this);
}

void Channel::enableWrite()
{
    m_events |= EPOLLOUT;
    m_loop->updateChannel(this);
}

void Channel::disableWrite()
{
    m_events &= ~EPOLLOUT;
    m_loop->updateChannel(this);
}

void Channel::disableAll()
{
    m_events = 0;
    m_loop->updateChannel(this);
}

void Channel::setRevents(uint32_t events)
{
    m_revents = events;
}
