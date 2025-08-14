#pragma once

#include <cstdint>
#include <span>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <immintrin.h>

// #if __AVX2__
// inline size_t arrayContains(std::span<int32_t> const &a, int32_t x)
// {
//     const size_t n = a.size();
//     int32_t *const p0 = a.data();
//     int32_t *p = p0;
//     int32_t *pe = p0 + (n & -7);
//     int32_t *tpe = p0 + n;
//     const __m256i value = _mm256_set1_epi32(x);
//     while (p != pe) {
//         __m256i data = _mm256_loadu_si256((const __m256i *)p);
//         uint32_t m = _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(data, value)));
//         if (m) {
//             return (p - p0) + __builtin_ffs(m);
//         }
//         p += 8;
//     }
//     while (p != pe) {
//         if (*p == x) {
//             return p - p0;
//         }
//         ++p;
//     }
//     return -1;
// }
// #else
inline size_t arrayContains(std::span<int32_t> const &a, int32_t x)
{
    auto it = std::find(a.begin(), a.end(), x);
    return it != a.end() ? it - a.begin() : -1;
}
// #endif

inline double computeMean(std::span<double> const &a)
{
    double sum = 0;
    size_t count = 0;
    for (auto it = a.begin(); it != a.end(); ++it) {
        double x = *it;
        if (!std::isnan(x)) {
            sum += *it;
            ++count;
        }
    }
    return count ? sum / count : NAN;
}
