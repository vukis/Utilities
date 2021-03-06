#pragma once

#include "TaskQueue.h"
#include <algorithm>
#include <thread>

class WorkStealingThreadPool
{
public:

    explicit WorkStealingThreadPool(size_t threadCount = std::max(1u, std::thread::hardware_concurrency()));
    ~WorkStealingThreadPool();

    template<typename TaskT>
    auto ExecuteAsync(TaskT&& task)
    {
        const auto index = m_queueIndex++;
        for (size_t n = 0; n != m_queues.size()*m_tryoutCount; ++n)
        {
            // Here we need not to std::forward just copy task.
            // Because if the universal reference of task has bound to an r-value reference 
            // then std::forward will have the same effect as std::move and thus task is not required to contain a valid task. 
            // Universal reference must only be std::forward'ed a exactly zero or one times.
            auto result = m_queues[(index + n) % m_queues.size()].TryPush(task); 

            if (result)
                return std::move(*result);
        }
        return m_queues[index % m_queues.size()].Push(std::forward<TaskT>(task));
    }

private:

    void Run(size_t queueIndex);

    std::vector<TaskQueue> m_queues;
    std::atomic<size_t>    m_queueIndex{ 0 };
    const size_t m_tryoutCount{ 1 };

    std::vector<std::thread> m_threads;
};

WorkStealingThreadPool::WorkStealingThreadPool(size_t threadCount)
    : m_queues{ threadCount }
{
    for (size_t index = 0; index != threadCount; ++index)
        m_threads.emplace_back([this, index] { Run(index); });
}

WorkStealingThreadPool::~WorkStealingThreadPool()
{
   const bool finishTasks = false;
    
    for (auto& queue : m_queues)
        queue.SetEnabled(finishTasks);

    for (auto& thread : m_threads)
        thread.join();
}

void WorkStealingThreadPool::Run(size_t queueIndex)
{
    while (m_queues[queueIndex].IsEnabled())
    {
        TaskQueue::TaskPtrType task;
        for (size_t n = 0; n != m_queues.size()*m_tryoutCount; ++n)
        {
            if (m_queues[(queueIndex + n) % m_queues.size()].TryPop(task))
                break;
        }

        if (!task && !m_queues[queueIndex].WaitAndPop(task))
            return;

        (*task)();
    }
}
