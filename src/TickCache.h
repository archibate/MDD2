#pragma once


#include <mutex>
#include <array>
#include <atomic>
#include <vector>
#include "MDS.h"
#include "FastMutex.h"


struct alignas(64) TickCache {
    /* StoC */ alignas(64) struct {
        std::vector<MDS::Tick> tickCachedW;
        SpinMutex tickCachedWMutex;
    };
    /* StoC */ alignas(64) std::atomic_flag tickCachedWEmpty{true};
    /* C */ alignas(64) struct {
        std::vector<MDS::Tick> tickCachedR;
        int32_t wantBuyCurrentIndex{};
    };
    /* CtoS */ alignas(64) std::atomic<int32_t> wantBuyTimestamp{};

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

    void pushWantBuy(int32_t timestamp)
    {
        wantBuyTimestamp.store(timestamp, std::memory_order_relaxed);
    }

    bool findWantBuy(int32_t timestamp)
    {
        int32_t wantTimestamp = wantBuyTimestamp.load(std::memory_order_relaxed);
        if (wantTimestamp >= timestamp) [[likely]] {
            // SPDLOG_INFO("wantTimestamp={} timestamp={}", wantTimestamp, timestamp);
            return true;
        }
        return false;
    }
};

