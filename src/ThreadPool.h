#ifndef TREAD_POOL_H
#define TREAD_POOL_H

#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <future>
#include <type_traits>

class ThreadPool
{
public:
    ThreadPool(int threadSize = 3);
    ~ThreadPool();

    void addTask(std::function<void()> task);

    template <class F, class... Args>
    auto addTask_Args(F &&f, Args &&...args)
        -> std::future<std::invoke_result_t<F, Args...>>//result_of
    {
        using R = std::invoke_result_t<F, Args...>;

        //结构化绑定
        auto task = std::make_shared<std::packaged_task<R>>(
            [func = std::forward<F>(f), ... args_pack = std::forward<Args>(args)]() mutable
            {
                return func(std::forward<Args>(args_pack) ...);
            });
            

        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_tasks.emplace([task]()
                          { (*task)(); });
        }

        m_taskcv.notify_one();
        return task->get_future();
    }

private:
    void startThreadPool(size_t workSize);

private:
    std::queue<std::function<void()>> m_tasks;
    std::vector<std::thread> m_workers;

    std::mutex m_mtx;
    std::condition_variable m_taskcv;//条件变量
    bool m_stop = false;
};
#endif