#pragma once

#include <Disruptor/Disruptor.h>
#include <Disruptor/ThreadPerTaskScheduler.h>
#include <Disruptor/BusySpinWaitStrategy.h>
#include <Disruptor/BlockingWaitStrategy.h>
#include <Disruptor/SleepingWaitStrategy.h>
#include <Disruptor/SpinWaitWaitStrategy.h>
#include <Disruptor/TimeoutBlockingWaitStrategy.h>
#include <Disruptor/YieldingWaitStrategy.h>
#include <functional>
#include <utility>
#include <cassert>


template <class T>
class DisruptorPool
{
private:
    template <class F>
    class PoolWorker : public Disruptor::IWorkHandler<T>
    {
    private:
        F m_f;

    public:
        explicit PoolWorker(F f)
            : m_f(std::move(f))
        {}

        void onEvent(T &event) override final
        {
            m_f(event);
        }
    };

public:
    DisruptorPool() = default;

    ~DisruptorPool()
    {
        stop();
    }
public:
    void init(int32_t ringBufferSize, bool multiProducer, int32_t coreId)
    {
        m_scheduler = std::make_shared<Disruptor::ThreadPerTaskScheduler>();

        std::shared_ptr<Disruptor::IWaitStrategy> ptrWaitStrategy;
        if (coreId >= 0) {
            m_scheduler->bindCore(coreId);
            ptrWaitStrategy = std::make_shared<Disruptor::BusySpinWaitStrategy>();
        } else {
            ptrWaitStrategy = std::make_shared<Disruptor::BlockingWaitStrategy>();
        }
        auto producerType = multiProducer ?
            Disruptor::ProducerType::Single : Disruptor::ProducerType::Multi;

        m_disruptor = std::make_shared<Disruptor::disruptor<T>>([] {
            return T();
        }, ringBufferSize, m_scheduler, producerType, ptrWaitStrategy);
        m_ringBuffer = m_disruptor->ringBuffer();
    }

    template <class F>
    void start(F workerEntry, int32_t numThreads, int32_t workerCount)
    {
        std::vector<std::shared_ptr<Disruptor::IWorkHandler<T>>> workers;
        workers.clear();
        for (int32_t i = 0; i < workerCount; i++) {
            workers.push_back(std::make_shared<PoolWorker<F>>(workerEntry));
        }
        m_disruptor->handleEventsWithWorkerPool(workers);
        m_scheduler->start();
        m_disruptor->start();
    }

    template <class F>
    void start(F workerEntry)
    {
        m_disruptor->handleEventsWithWorkerPool(std::make_shared<PoolWorker<F>>(workerEntry));
        m_scheduler->start();
        m_disruptor->start();
    }

    void push(T const &event)
    {
        int64_t seq = m_ringBuffer->tryNext();
        while (seq < 0) [[unlikely]] {
            seq = m_ringBuffer->tryNext();
        }
        (*m_ringBuffer)[seq] = event;
        m_ringBuffer->publish(seq);
    }

    void push(T const *event, int32_t numEvents)
    {
        int64_t seq = m_ringBuffer->tryNext(numEvents);
        while (seq < 0) [[unlikely]] {
            seq = m_ringBuffer->tryNext(numEvents);
        }
        for (int32_t i = 0; i < numEvents; ++i) {
            (*m_ringBuffer)[seq] = event[i];
        }
        m_ringBuffer->publish(seq, seq + numEvents);
    }

    int64_t beginPush()
    {
        int64_t seq = m_ringBuffer->tryNext();
        while (seq < 0) [[unlikely]] {
            seq = m_ringBuffer->tryNext();
        }
        return seq;
    }

    int64_t beginPush(int32_t n)
    {
        int64_t seq = m_ringBuffer->tryNext(n);
        while (seq < 0) [[unlikely]] {
            seq = m_ringBuffer->tryNext(n);
        }
        return seq;
    }

    T &atPush(int64_t i)
    {
        return (*m_ringBuffer)[i];
    }

    void endPush(int64_t i)
    {
        m_ringBuffer->publish(i);
    }

    void endPush(int64_t i, int32_t n)
    {
        m_ringBuffer->publish(i, i + n);
    }

    void stop()
    {
        if (m_disruptor) {
            m_disruptor->shutdown();
        }

        if (m_scheduler) {
            m_scheduler->stop();
        }
    }

private:
    std::shared_ptr<Disruptor::ThreadPerTaskScheduler> m_scheduler;
    std::shared_ptr<Disruptor::disruptor<T>> m_disruptor;
    std::shared_ptr<Disruptor::RingBuffer<T>> m_ringBuffer;
};
