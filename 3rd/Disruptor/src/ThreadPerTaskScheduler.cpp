#include "stdafx.h"

#include "ThreadPerTaskScheduler.h"
#include "ThreadHelper.h"

namespace Disruptor
{

    void ThreadPerTaskScheduler::start(std::int32_t)
    {
        if (m_started)
            return;

        m_started = true;
    }

    void ThreadPerTaskScheduler::stop()
    {
        if (!m_started)
            return;

        m_started = false;

        for (auto&& thread : m_threads)
        {
            if (thread.joinable())
                thread.join();
        }
    }

    std::future<void> ThreadPerTaskScheduler::scheduleAndStart(std::packaged_task<void()>&& task)
    {
        auto result = task.get_future();

        auto threadId = m_threads.size() + beginCore;

        m_threads.emplace_back([this, task{ move(task) }, threadId] () mutable
        {
            if (m_bindCore)
            {
                static const auto processorCount = std::thread::hardware_concurrency();

                const auto processorIndex = threadId % processorCount;

                const auto affinityMask = ThreadHelper::AffinityMask(1ull << processorIndex);

                ThreadHelper::setThreadAffinity(affinityMask);
            }
            while(m_started)
            {
                try
                {
                    task();
                }
                catch (...)
                {
                }
            }
        });

        return result;
    }

} // namespace Disruptor
