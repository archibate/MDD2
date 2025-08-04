#pragma once


#include <cstdint>
// #include <smmintrin.h>


[[gnu::always_inline]] inline uint32_t atou6(const char *s)
{
    uint32_t i;
    i = 100000 * (*s++ - '0');
    i += 10000 * (*s++ - '0');
    i += 1000 * (*s++ - '0');
    i += 100 * (*s++ - '0');
    i += 10 * (*s++ - '0');
    i += *s++ - '0';
    return i;
}

// [[gnu::always_inline]] inline uint32_t atou4(const char input[4]) {
//     // Load 4 bytes into a 32-bit temporary
//     uint32_t tmp;
//     __builtin_memcpy(&tmp, input, 4);
//
//     // Zero-extend bytes to 32-bit integers
//     __m128i ascii = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(tmp));
//
//     __m128i digits = _mm_sub_epi32(ascii, _mm_set1_epi32('0'));
//
//     // Set multipliers: [1000, 100, 10, 1]
//     __m128i multipliers = _mm_set_epi32(1, 10, 100, 1000);
//
//     // Multiply digits by multipliers
//     __m128i products = _mm_mullo_epi32(digits, multipliers);
//
//     // Horizontal sum: shift and add elements
//     __m128i sum_a = _mm_add_epi32(products, _mm_srli_si128(products, 8));
//     __m128i sum_b = _mm_add_epi32(sum_a, _mm_srli_si128(sum_a, 4));
//
//     // Extract the result
//     return _mm_cvtsi128_si32(sum_b);
// }

inline uint32_t securityId(const char *s)
{
    return atou6(s);
}
