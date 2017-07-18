#pragma once

#include "TaskQueue.h"
#include <algorithm>
#include <thread>

class MultiQueueThreadPool
{
public:

    explicit MultiQueueThreadPool(size_t threadCount = std::max(2u, std::thread::hardware_concurrency()));
    ~MultiQueueThreadPool();

    template<typename TaskT>
    auto ExecuteAsync(TaskT&& task)
    {
        const auto index = m_queueIndex++;
        return m_queues[index % m_queueCount].Push(std::forward<TaskT>(task));
    }

private:
    void Run(size_t queueIndex);

    std::vector<TaskQueue> m_queues;
    size_t       m_queueIndex{ 0 };
    const size_t m_queueCount;

    std::vector<std::thread> m_threads;
};

MultiQueueThreadPool::MultiQueueThreadPool(size_t threadCount)
    : m_queues{ threadCount }
    , m_queueCount{ threadCount }
{
    for (size_t index = 0; index != threadCount; ++index)
        m_threads.emplace_back([this, index] { Run(index); });
}

MultiQueueThreadPool::~MultiQueueThreadPool()
{
    for (auto& queue : m_queues)
        queue.SetEnabled(false);

    for (auto& thread : m_threads)
        thread.join();
}

void MultiQueueThreadPool::Run(size_t queueIndex)
{
    while (m_queues[queueIndex].IsEnabled())
    {
        TaskQueue::TaskType task;
        if (m_queues[queueIndex].WaitAndPop(task))
            task();
    }
}
