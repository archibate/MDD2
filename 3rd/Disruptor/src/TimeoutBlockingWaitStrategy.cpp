#include "stdafx.h"
#include "TimeoutBlockingWaitStrategy.h"

#include <ostream>

#include <chrono>

#include "ISequenceBarrier.h"
#include "Sequence.h"
#include "TimeoutException.h"


namespace Disruptor
{

    TimeoutBlockingWaitStrategy::TimeoutBlockingWaitStrategy(ClockConfig::Duration timeout)
        : m_timeout(timeout)
    {
    }

     std::int64_t TimeoutBlockingWaitStrategy::waitFor(std::int64_t sequence,
                                                       Sequence& cursor,
                                                       ISequence& dependentSequence,
                                                       ISequenceBarrier& barrier)
    {
        auto timeSpan = std::chrono::microseconds(std::chrono::duration_cast< std::chrono::microseconds >(m_timeout).count());

        if (cursor.value() < sequence)
        {
            std::unique_lock< decltype(m_gate) > uniqueLock(m_gate);

            while (cursor.value() < sequence)
            {
                barrier.checkAlert();

                if (m_conditionVariable.wait_for(uniqueLock, timeSpan) == std::cv_status::timeout)
                    DISRUPTOR_THROW_TIMEOUT_EXCEPTION();
            }
        }

        std::int64_t availableSequence;
        while ((availableSequence = dependentSequence.value()) < sequence)
        {
            barrier.checkAlert();
        }

        return availableSequence;
    }

    void TimeoutBlockingWaitStrategy::signalAllWhenBlocking()
    {
        std::unique_lock< decltype(m_gate) > uniqueLock(m_gate);

        m_conditionVariable.notify_all();
    }

    void TimeoutBlockingWaitStrategy::writeDescriptionTo(std::ostream& stream) const
    {
        stream << "TimeoutBlockingWaitStrategy";
    }

} // namespace Disruptor
