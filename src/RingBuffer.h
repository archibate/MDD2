#pragma once


#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>
#include <exception>
#include <cstdio>
#include <cstdlib>


template <class T, uint32_t N>
struct alignas(64) RingBuffer
{
private:
    static_assert((N & (N - 1)) == 0, "N must be power-of-two");

    alignas(64) std::atomic<uint32_t> write_ok_pos{0};
    alignas(64) std::unique_ptr<T[]> ring = std::make_unique<T[]>(N);
    alignas(64) uint32_t write_pos{0};
    alignas(64) uint32_t read_pos{0};

public:
    void write(T const *buf, uint32_t n) noexcept
    {
        for (T const *p = buf, *pe = buf + n; p != pe; ++p, ++write_pos) {
            ring[write_pos & (N - 1)] = *p;
        }
        write_ok_pos.store(write_pos, std::memory_order_release);
    }

    void writeOne(T const &value) noexcept
    {
        ring[write_pos & (N - 1)] = value;
        ++write_pos;
        write_ok_pos.store(write_pos, std::memory_order_release);
    }

    uint32_t fetch() noexcept
    {
        uint32_t w_pos = write_ok_pos.load(std::memory_order_acquire);
        uint32_t n = w_pos - read_pos;
        if (n >= N) [[unlikely]] {
            std::fprintf(stderr, "RingBuffer overflow: %d\n", n);
            std::fflush(stderr);
            std::_Exit(1);
        }
        return n;
    }

    T readOne() noexcept
    {
        T value = ring[read_pos & (N - 1)];
        ++read_pos;
        return value;
    }

    void read(T *buf, uint32_t n) noexcept
    {
        for (T *p = buf, *pe = buf + n; p != pe; ++p, ++read_pos) {
            *p = ring[read_pos & (N - 1)];
        }
    }
};
