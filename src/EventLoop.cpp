#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <mutex>
#include <iostream>

EventLoop::EventLoop()
{
    m_epfd = epoll_create1(0);

    m_wakeupfd = eventfd(0, EFD_NONBLOCK);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_wakeupfd;

    epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_wakeupfd, &ev);
}

EventLoop::~EventLoop()
{
    close(m_epfd);
    close(m_wakeupfd);
}

void EventLoop::loop()
{
    epoll_event evs[1024];

    std::cout << "Server already\n";  
    while (1)
    {
        int nready = epoll_wait(m_epfd, evs, 1024, -1);

        for (int i = 0; i < nready; i++)
        {
            int fd = evs[i].data.fd;

            if (fd == m_wakeupfd)
                handleWakeup();
            else{
                if(m_callback)
                {
                    m_callback(fd, evs[i].events);
                }
            }
        }
    }
}

void EventLoop::updateEvent(int fd, uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
}

void EventLoop::addEvent(int fd, uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
}

void EventLoop::removeEvent(int fd)
{
    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

void EventLoop::queueInLoop(std::function<void()> func)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_pendingTasks.push(func);
    }

    uint64_t one;

    write(m_wakeupfd, &one, sizeof(one));
}

void EventLoop::handleWakeup()
{
    uint64_t one;

    read(m_wakeupfd, &one, sizeof(one));

    std::queue<std::function<void()>> tasks;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::swap(tasks, m_pendingTasks);
    }

    while (!tasks.empty())
    {
        auto task = std::move(tasks.front());

        tasks.pop();

        task();
    }
}
