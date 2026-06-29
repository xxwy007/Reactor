#include "EventLoop.h"
#include "Channel.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>
constexpr int MAX_EVENTS = 1024;

EventLoop::EventLoop()
    : m_quit(false)
{
    m_epfd = epoll_create1(0);

    // 创建eventfd
    m_wakeupFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = m_wakeupFd;

    epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_wakeupFd, &ev);
}

EventLoop::~EventLoop()
{
    close(m_wakeupFd);
    close(m_epfd);
}

void EventLoop::loop()
{
    epoll_event events[MAX_EVENTS];

    while (!m_quit)
    {
        int nready = epoll_wait(m_epfd, events, MAX_EVENTS, -1);

        if (nready < 0)
        {
            continue;
        }

        for (int i = 0; i < nready; i++)
        {
            int fd = events[i].data.fd;
            if (fd == m_wakeupFd)
            {
                handleWakeup();
            }
            else
            {
                auto it = m_channels.find(fd);

                if (it != m_channels.end())
                {
                    Channel *channel = it->second;

                    channel->setRevents(
                        events[i].events);

                    channel->handleEvent();
                }
            }
        }
    }
}

void EventLoop::quit()
{
    m_quit = true;

    uint64_t one = 1;
    write(m_wakeupFd, &one, sizeof(one));
}

void EventLoop::updateChannel(Channel *channel)
{
    int fd = channel->getfd();

    epoll_event ev{};

    ev.events = channel->getEvents();
    ev.data.fd = fd;

    if (m_channels.count(fd) == 0)
    {
        epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
        m_channels[fd] = channel;
    }
    else
    {
        epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
    }
}

void EventLoop::removeChannel(Channel *channel)
{
    int fd = channel->getfd();

    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);

    m_channels.erase(fd);
}

void EventLoop::queueInLoop(Task task)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingTasks.push(
            std::move(task));
    }

    uint64_t one = 1;
    write(m_wakeupFd, &one, sizeof(one));
}

void EventLoop::handleWakeup()
{
    uint64_t one;
    read(m_wakeupFd, &one, sizeof(one));

    std::queue<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tasks.swap(m_pendingTasks);
    }

    while (!tasks.empty())
    {
        auto task = std::move(tasks.front());
        tasks.pop();
        task();
    }
}