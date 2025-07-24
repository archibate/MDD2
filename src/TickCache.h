#pragma once


#include <mutex>
#include <atomic>
#include <vector>
#include "MDS.h"
#include "FastMutex.h"


struct alignas(64) TickCache {
    alignas(64) struct {
        std::vector<MDS::Tick> tickCachedW;
        SpinMutex tickCachedWMutex;
    };
    alignas(64) std::atomic_flag tickCachedWEmpty{true};
    alignas(64) std::vector<MDS::Tick> tickCachedR;

    void start()
    {
        tickCachedW.reserve(512);
        tickCachedR.reserve(512);
    }

    void stop() {}

    void pushTick(MDS::Tick const &tick)
    {
        {
            std::lock_guard lck(tickCachedWMutex);
            tickCachedW.push_back(tick);
        }
        tickCachedWEmpty.clear(std::memory_order_release);
    }

    bool tryObtainReadCopy()
    {
        if (tickCachedWEmpty.test_and_set(std::memory_order_acquire)) {
            return false;
        }
        tickCachedR.clear();
        {
            std::lock_guard lck(tickCachedWMutex);
            tickCachedW.swap(tickCachedR);
        }
        return true;
    }

    std::vector<MDS::Tick> &getReadCopy()
    {
        return tickCachedR;
    }
};

