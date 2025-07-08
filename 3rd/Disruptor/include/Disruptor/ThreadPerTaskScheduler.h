#pragma once

#include "Disruptor/ITaskScheduler.h"


namespace Disruptor
{

    class ThreadPerTaskScheduler : public ITaskScheduler
    {
    public:
        void start(std::int32_t numberOfThreads = 0) override;
        void stop() override;
        void bindCore(uint8_t coreIndex = 0) 
        { 
            m_bindCore = true; 
            beginCore = coreIndex;
        }

        std::future< void > scheduleAndStart(std::packaged_task< void() >&& task) override;

    private:
        std::atomic< bool > m_started{ false };
        std::atomic< bool > m_bindCore{ false };
        std::vector< std::thread > m_threads;
        uint8_t beginCore{0};
    };

} // namespace Disruptor
