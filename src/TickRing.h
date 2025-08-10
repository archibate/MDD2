#pragma once


#include "MDS.h"


#if 0
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
#else
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

    void pushTick(MDS::Tick const &tick)
    {
        // writer.write(&tick, &tick + 1);
        ring.write(&tick, &tick + 1);
    }

    size_t fetchTicks(MDS::Tick *buf, size_t size)
    {
        // return reader.read_n(buf, size);
        return ring.read_some(buf, buf + size) - buf;
    }
};

#endif
