#pragma once


#include "MDS.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include "SPSC.h"


struct alignas(64) TickRing
{
    // using RingType = spsc_ring_queue<MDS::Tick, 0x40000>;
    // // 256K * 64B = 16MB
    //
    // alignas(64) RingType::ring_reader reader;
    // alignas(64) RingType::ring_writer writer;
    // alignas(64) RingType ring;

    spsc_ring<MDS::Tick, 0x40000> ring;

    void start()
    {
        // reader = ring.reader();
        // writer = ring.writer();
    }

    void stop() {}

    COLD_ZONE void onOverflow()
    {
        SPDLOG_WARN("ring queue overflow");
    }

    void pushTick(MDS::Tick const &tick)
    {
        if (!ring.write_one(tick)) [[unlikely]] {
            onOverflow();
        }
    }

    size_t fetchSomeTicks(MDS::Tick *buf, size_t size)
    {
        return ring.read_some(buf, buf + size) - buf;
    }
};
