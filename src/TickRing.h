#pragma once


#include "MDS.h"
#include "RingBuffer.h"


struct alignas(64) TickRing
{
    RingBuffer<MDS::Tick, 0x40000> ring;

    void start() {}

    void stop() {}

    void pushTick(MDS::Tick const &tick)
    {
        ring.writeOne(tick);
    }

    size_t fetchTicks(MDS::Tick *buf, size_t size)
    {
        uint32_t n = ring.fetch();
        if (n > size) {
            n = size;
        }
        ring.read(buf, n);
        return n;
    }
};
