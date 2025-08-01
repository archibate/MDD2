#pragma once


#include <cstdint>
#include <cstddef>
#include <atomic>


template <class T, size_t N>
class alignas(64) spsc_ring_queue
{
    static_assert(std::atomic<T *>::is_always_lock_free);

    alignas(64) std::atomic<T *> write_ok_pos;
    alignas(64) std::atomic<T *> read_ok_pos;
    alignas(64) struct {
        T data[N];
        T data_end[0];
    };

public:
    spsc_ring_queue() noexcept : write_ok_pos(data), read_ok_pos(data_end - 1) {
    }

    spsc_ring_queue(spsc_ring_queue &&) = delete;
    spsc_ring_queue &operator=(spsc_ring_queue &&) = delete;
    ~spsc_ring_queue() = default;

    class ring_reader
    {
        spsc_ring_queue *queue;
        T *read_pos;
        T *read_end_pos;

    public:
        ring_reader() = default;

        explicit ring_reader(spsc_ring_queue *queue_) noexcept : queue(queue_) {
            read_end_pos = queue->write_ok_pos.load(std::memory_order_acquire);
            read_pos = read_end_pos;
        }

        bool read_wait() noexcept {
            read_end_pos = queue->write_ok_pos.load(std::memory_order_relaxed);
            if (read_pos != read_end_pos) {
                return true;
            }
#if __cpp_lib_atomic_wait
            queue->write_ok_pos.wait(read_pos, std::memory_order_relaxed);
#endif
            return read_pos != read_end_pos;
        }

        size_t read_remain() noexcept {
            if (read_pos == read_end_pos) {
                read_end_pos = queue->write_ok_pos.load(std::memory_order_acquire);
            }
            ptrdiff_t diff = read_end_pos - read_pos;
            if (diff < 0) {
                diff += N;
            }
            return static_cast<size_t>(diff);
        }

        T *read(T *buf, T *buf_end) noexcept {
            T *p = buf;
            while (p != buf_end) {
                if (read_pos == read_end_pos) {
                    read_end_pos = queue->write_ok_pos.load(std::memory_order_acquire);
                    if (read_pos == read_end_pos) {
                        break;
                    } else {
                        continue;
                    }
                }
                *p = *read_pos;
                ++p;
                ++read_pos;
                if (read_pos == queue->data_end) {
                    read_pos = queue->data;
                }
            }
            T *r_ok_pos = read_pos;
            if (r_ok_pos == queue->data) {
                r_ok_pos = queue->data_end;
            }
            --r_ok_pos;
            queue->read_ok_pos.store(r_ok_pos, std::memory_order_relaxed);
            return p;
        }

        size_t read_n(T *buf, size_t n) noexcept {
            return read(buf, buf + n) - buf;
        }
    };

    class ring_writer
    {
        spsc_ring_queue *queue;
        T *write_pos;
        T *write_end_pos;

    public:
        ring_writer() = default;

        explicit ring_writer(spsc_ring_queue *queue_) noexcept : queue(queue_) {
            write_end_pos = queue->read_ok_pos.load(std::memory_order_acquire);
            write_pos = write_end_pos;
        }

        bool write_wait() noexcept {
            write_end_pos = queue->read_ok_pos.load(std::memory_order_relaxed);
            if (write_pos != write_end_pos) {
                return true;
            }
#if __cpp_lib_atomic_wait
            queue->read_ok_pos.wait(write_pos, std::memory_order_relaxed);
#endif
            return write_pos != write_end_pos;
        }

        size_t write_remain() noexcept {
            if (write_pos == write_end_pos) {
                write_end_pos = queue->read_ok_pos.load(std::memory_order_acquire);
            }
            ptrdiff_t diff = write_end_pos - write_pos;
            if (diff < 0) {
                diff += N;
            }
            return static_cast<size_t>(diff);
        }

        const T *write(const T *buf, const T *buf_end) noexcept {
            const T *p = buf;
            while (p != buf_end) {
                if (write_pos == write_end_pos) {
                    write_end_pos = queue->read_ok_pos.load(std::memory_order_relaxed);
                    if (write_pos == write_end_pos) {
                        break;
                    } else {
                        continue;
                    }
                }
                *write_pos = *p;
                ++p;
                ++write_pos;
                if (write_pos == queue->data_end) {
                    write_pos = queue->data;
                }
            }
            queue->write_ok_pos.store(write_pos, std::memory_order_release);
            return p;
        }

        size_t write_n(const T *buf, size_t n) noexcept {
            return write(buf, buf + n) - buf;
        }
    };

    ring_reader reader() noexcept {
        return ring_reader(this);
    }

    ring_writer writer() noexcept {
        return ring_writer(this);
    }
};
