#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadSize)
{
    startThreadPool(threadSize);
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_stop = true;
    }

    m_taskcv.notify_all();

    for(auto& work : m_workers)
    {
        work.join();
    }
}

void ThreadPool::addTask(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(m_mtx);

        m_tasks.push(task);
    }

    m_taskcv.notify_one();
}

void ThreadPool::startThreadPool(size_t workSize)
{
    for(int i = 0; i < workSize; i++)
    {
        m_workers.emplace_back([this]{
            while(1)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(m_mtx);
                    m_taskcv.wait(lock,[this]{
                        return m_stop || !m_tasks.empty(); 
                    });

                    if(m_stop && m_tasks.empty())
                    {
                        return;
                    }

                    task = std::move(m_tasks.front());

                    m_tasks.pop();
                }

                task();
            }
        });
    }
}