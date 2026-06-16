#ifndef EVENT_LOOP
#define EVENT_LOOP

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>

class EventLoop
{
public:
    using EventCallback = std::function<void(int, uint32_t)>;
    EventLoop();
    ~EventLoop();

    void loop();

    void updateEvent(int fd, uint32_t events);
    void addEvent(int fd, uint32_t events);
    void removeEvent(int fd);

    void queueInLoop(std::function<void()> func);
    void setEventCallback(EventCallback cb)
    {
        m_callback = std::move(cb);
    }

private:
    void handleWakeup();
private:
    int m_epfd;
    int m_wakeupfd;
    std::mutex m_mutex;
    std::queue<std::function<void()>> m_pendingTasks;

    EventCallback m_callback;
};
#endif