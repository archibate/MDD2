#pragma once


#include <atomic>
#include <emmintrin.h>


class FastMutex {
    std::atomic_flag m_atom{};

public:
    void lock()
    {
        if (!m_atom.test_and_set(std::memory_order_acquire)) {
            return;
        }
        _mm_pause();
        while (m_atom.test_and_set(std::memory_order_acquire)) {
            m_atom.wait(true, std::memory_order_relaxed);
        }
    }

    bool try_lock()
    {
        return !m_atom.test_and_set(std::memory_order_acquire);
    }

    void unlock()
    {
        m_atom.clear(std::memory_order_release);
        m_atom.notify_one();
    }
};

class SpinMutex {
    std::atomic_flag m_atom{};

public:
    void lock()
    {
        while (m_atom.test_and_set(std::memory_order_acquire)) {
            _mm_pause();
        }
    }

    bool try_lock()
    {
        return !m_atom.test_and_set(std::memory_order_acquire);
    }

    void unlock()
    {
        m_atom.clear(std::memory_order_release);
    }
};
