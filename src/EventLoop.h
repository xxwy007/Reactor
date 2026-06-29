#ifndef EVENT_LOOP
#define EVENT_LOOP

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <unordered_map>

class Channel;

class EventLoop
{
public:
    using Task = std::function<void()>;

public:
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    void queueInLoop(Task task);
private:
    void handleWakeup();

private:
    int m_epfd;
    int m_wakeupFd;
    bool m_quit;
    std::unordered_map<int, Channel*> m_channels;
    std::queue<Task> m_pendingTasks;

    std::mutex m_mutex;
};
#endif