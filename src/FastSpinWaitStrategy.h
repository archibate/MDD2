#pragma once

#include <Disruptor/Disruptor.h>
#include <Disruptor/IWaitStrategy.h>
#include <functional>
#include <chrono>


class FastSpinWaitStrategy final : public Disruptor::IWaitStrategy
{
public:
    using TickCount = std::chrono::steady_clock::duration::rep;

    FastSpinWaitStrategy(
        std::function<void()> idleCallback,
        std::chrono::milliseconds idleTicksInit,
        std::chrono::milliseconds idleTickInterval)
        : m_idleCallback(std::move(idleCallback))
        , m_idleTicksInit(duration_cast<std::chrono::steady_clock::duration>(idleTicksInit).count())
        , m_idleTicksInterval(duration_cast<std::chrono::steady_clock::duration>(idleTickInterval).count())
    {
    }

    std::int64_t waitFor(std::int64_t sequence,
                         Disruptor::Sequence &/*cursor*/,
                         Disruptor::ISequence &dependentSequence,
                         Disruptor::ISequenceBarrier &barrier) override
    {
        std::int64_t availableSequence;
        if ((availableSequence = dependentSequence.value()) < sequence) {

            TickCount lastTick = std::chrono::steady_clock::now().time_since_epoch().count();
            TickCount idleTicks = m_idleTicksInit;

            while ((availableSequence = dependentSequence.value()) < sequence) {
                barrier.checkAlert();

                TickCount nowTick = std::chrono::steady_clock::now().time_since_epoch().count();
                if (nowTick > lastTick + idleTicks) {
                    lastTick += idleTicks;
                    idleTicks = m_idleTicksInterval;
                    m_idleCallback();
                }
            }
        }
        return availableSequence;
    }

    void signalAllWhenBlocking() override
    {
    }

    void writeDescriptionTo(std::ostream &stream) const override
    {
        stream << "BusySpinWaitStrategy";
    }

private:
    std::function<void()> m_idleCallback;
    TickCount m_idleTicksInit;
    TickCount m_idleTicksInterval;
};
