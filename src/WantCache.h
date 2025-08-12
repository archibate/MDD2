#pragma once


#include <mutex>
#include <array>
#include <atomic>
#include <vector>
#include "MDS.h"
#include "timestamp.h"
#include "constants.h"


struct alignas(64) WantCache
{
    /* CtoS */ alignas(64) std::array<std::atomic<int32_t>, 16> wantBuyTimestamp{};
    /* C */ alignas(64) int32_t wantBuyCurrentIndex{};

    void start() {}

    void stop() {}

    // [[gnu::always_inline]] void pushTick(MDS::Tick const &tick)
    // {
    //     tickRing.writeOne(tick);
    // }
    //
    // template <size_t BufSize>
    // [[gnu::always_inline]] size_t fetchTicks(MDS::Tick (&buf)[BufSize])
    // {
    //     uint32_t n = tickRing.fetch();
    //     if (n == 0) {
    //         return 0;
    //     }
    //     if (n > BufSize) {
    //         n = BufSize;
    //     }
    //     tickRing.read(buf, n);
    //     return n;
    // }

    void pushWantBuyTimestamp(int32_t timestamp, bool wantBuy)
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

    Intent checkWantBuyAtTimestamp(int32_t timestamp)
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

