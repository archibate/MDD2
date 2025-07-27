#pragma once


#include <mutex>
#include <spdlog/spdlog.h>
#include <array>
#include <atomic>
#include <vector>
#include "MDS.h"
#include "FastMutex.h"
#include "timestamp.h"
#include "constants.h"


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
    /* CtoS */ alignas(64) std::array<std::atomic<int32_t>, 16> wantBuyTimestamp{};

    void start()
    {
        tickCachedW.reserve(512);
        tickCachedR.reserve(512);
    }

    void stop() {}

    [[gnu::always_inline]] void pushTick(MDS::Tick const &tick)
    {
        {
            std::lock_guard lck(tickCachedWMutex);
            tickCachedW.push_back(tick);
        }
        tickCachedWEmpty.clear(std::memory_order_release);
    }

    [[gnu::always_inline]] bool tryObtainReadCopy()
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

    [[gnu::always_inline]] std::vector<MDS::Tick> &getReadCopy()
    {
        return tickCachedR;
    }

    [[gnu::always_inline]] void pushWantBuyTimestamp(int32_t timestamp, bool wantBuy)
    {
        timestamp = timestampLinear(timestamp);
        if (wantBuy) {
            --timestamp;
        }
        wantBuyTimestamp[wantBuyCurrentIndex].store(timestamp, std::memory_order_relaxed);
        ++wantBuyCurrentIndex;
        wantBuyCurrentIndex &= wantBuyTimestamp.size() - 1;
    }

    enum Intent
    {
        WantBuy,
        DontBuy,
        NotSure,
    };

    [[gnu::always_inline]] Intent checkWantBuyAtTimestamp(int32_t timestamp)
    {
        timestamp = (timestampLinear(timestamp) + 90) / 100 * 100;
        int32_t minTimestamp = wantBuyTimestamp[0].load(std::memory_order_relaxed);
        int32_t minDt = std::abs(minTimestamp - timestamp);
        for (int32_t i = 1; i < wantBuyTimestamp.size(); ++i) {
            int32_t wantTimestamp = wantBuyTimestamp[i].load(std::memory_order_relaxed);
            int32_t dt = std::abs(wantTimestamp - timestamp);
            if (dt < minDt) {
                minDt = dt;
                minTimestamp = wantTimestamp;
            }
        }
        if (minDt > kWantBuyTimeTolerance) [[unlikely]] {
            return NotSure;
        }
        if (!(minTimestamp & 1)) [[unlikely]] {
            return DontBuy;
        }
        return WantBuy;
    }
};

