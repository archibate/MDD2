#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>


template <class Kernel, size_t ...I>
void repeatCall(Kernel &&kernel, std::index_sequence<I...>) noexcept(noexcept((kernel(std::integral_constant<size_t, I>{}), ...)))
{
    (kernel(std::integral_constant<size_t, I>{}), ...);
}

template <size_t repeat, class Kernel>
void repeatCall(Kernel &&kernel) noexcept(noexcept(repeatCall(std::forward<Kernel>(kernel), std::make_index_sequence<repeat>{})))
{
    repeatCall(std::forward<Kernel>(kernel), std::make_index_sequence<repeat>{});
}

template <size_t shift, size_t halfpass, size_t offset, size_t size>
size_t radixSort32Size(size_t num) noexcept
{
    const uint32_t N = uint32_t(1) << shift;
    return num * size * sizeof(uint32_t) + N * sizeof(uint32_t);
}

template <size_t shift, size_t halfpass, size_t offset, size_t size>
void radixSort32(void *__restrict buf, uint32_t *__restrict in, size_t num) noexcept
{
    const uint32_t N = uint32_t(1) << shift;

    uint32_t *__restrict out = reinterpret_cast<uint32_t *>(buf);
    uint32_t *__restrict count = reinterpret_cast<uint32_t *>(out + num * size);

    auto kernel = [] <size_t pass> (uint32_t *__restrict count, uint32_t *__restrict out, uint32_t *__restrict in, size_t num) noexcept {
        uint32_t *end = in + num * size;

        memset(count, 0, N * sizeof(uint32_t));
        for (uint32_t *p = in; p != end; p += size) {
            uint32_t x = p[offset];
            uint32_t bin = (x >> (pass * shift)) & (N - 1);
            ++count[bin];
        }

        for (uint32_t n = 0, current = 0; n != N; ++n) {
            uint32_t c = count[n];
            count[n] = current;
            current += c;
        }

        for (uint32_t *p = in; p != end; p += size) {
            uint32_t x = p[offset];
            uint32_t bin = (x >> (pass * shift)) & (N - 1);
            memcpy(&out[count[bin]++ * size], p, size * sizeof(uint32_t));
        }
    };

    repeatCall<halfpass>([=] (auto i) noexcept {
        kernel.template operator()<i << 1>(count, out, in, num);
        kernel.template operator()<(i << 1) | 1>(count, in, out, num);
    });
}

template <size_t shift, size_t halfpass, size_t offset, size_t size>
size_t radixSort64Size(size_t num) noexcept
{
    const uint64_t N = uint64_t(1) << shift;
    return num * size * sizeof(uint64_t) + N * sizeof(uint64_t);
}

template <size_t shift, size_t halfpass, size_t offset, size_t size>
void radixSort64(void *__restrict buf, uint64_t *__restrict in, size_t num) noexcept
{
    const uint64_t N = uint64_t(1) << shift;

    uint64_t *__restrict out = reinterpret_cast<uint64_t *>(buf);
    uint64_t *__restrict count = reinterpret_cast<uint64_t *>(out + num * size);

    auto kernel = [] <size_t pass> (uint64_t *__restrict count, uint64_t *__restrict out, uint64_t *__restrict in, size_t num) noexcept {
        uint64_t *end = in + num * size;

        memset(count, 0, N * sizeof(uint64_t));
        for (uint64_t *p = in; p != end; p += size) {
            uint64_t x = p[offset];
            uint64_t bin = (x >> (pass * shift)) & (N - 1);
            ++count[bin];
        }

        for (uint64_t n = 0, current = 0; n != N; ++n) {
            uint64_t c = count[n];
            count[n] = current;
            current += c;
        }

        for (uint64_t *p = in; p != end; p += size) {
            uint64_t x = p[offset];
            uint64_t bin = (x >> (pass * shift)) & (N - 1);
            memcpy(&out[count[bin]++ * size], p, size * sizeof(uint64_t));
        }
    };

    repeatCall<halfpass>([=] (auto i) noexcept {
        kernel.template operator()<i << 1>(count, out, in, num);
        kernel.template operator()<(i << 1) | 1>(count, in, out, num);
    });
}

inline void *__restrict getRadixSortBuf(size_t size) noexcept
{
    thread_local std::pair<std::unique_ptr<uint8_t[]>, size_t> buffer{};
    if (buffer.second < size)
    {
        buffer.first = std::make_unique_for_overwrite<uint8_t[]>(size);
        buffer.second = size;
    }
    return buffer.first.get();
}

template <size_t shift, size_t pass, size_t elemsize, size_t offset, size_t size>
void radixSort(void *__restrict in, size_t num) noexcept
{
    static_assert(offset % elemsize == 0);
    static_assert(size % elemsize == 0);
    static_assert(pass % 2 == 0);
    if constexpr (elemsize == sizeof(uint32_t)) {
        void *__restrict buf = getRadixSortBuf(radixSort32Size<shift, pass / 2, offset / elemsize, size / elemsize>(num));
        radixSort32<shift, pass / 2, offset / elemsize, size / elemsize>(
            buf, reinterpret_cast<uint32_t *>(in), num);
    } else if constexpr (elemsize == sizeof(uint64_t)) {
        void *__restrict buf = getRadixSortBuf(radixSort64Size<shift, pass / 2, offset / elemsize, size / elemsize>(num));
        radixSort64<shift, pass / 2, offset / elemsize, size / elemsize>(
            buf, reinterpret_cast<uint64_t *>(in), num);
    } else {
        static_assert(elemsize != elemsize);
    }
}
