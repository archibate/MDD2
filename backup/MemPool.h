#pragma once



#include <cstddef>
#include <cstdlib>
#include <new>


struct MemPool
{
    template <class T>
    T *allocate(size_t n) {
        return static_cast<T *>(allocate(n * sizeof(T)));
    }

    void *allocate(size_t n) {
        void *p = std::malloc(n);
        if (!p) {
            throw std::bad_alloc();
        }
        return p;
    }
};


template <MemPool *pool>
struct MemPoolAllocator
{
};
