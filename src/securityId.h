#pragma once


#include <cstdint>
// #include <cstdlib>
#include <immintrin.h>


// inline uint32_t atou6(const char *s)
// {
//     uint32_t i;
//     i = 100000 * (*s++ - '0');
//     i += 10000 * (*s++ - '0');
//     i += 1000 * (*s++ - '0');
//     i += 100 * (*s++ - '0');
//     i += 10 * (*s++ - '0');
//     i += *s++ - '0';
//     return i;
// }

// inline uint32_t atou4(const char input[4]) {
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

// inline int32_t securityId(const char *s)
// {
//     // return std::strtoul(s, nullptr, 10);
//     return atou6(s);
// }

// inline int32_t securityId(const char *s)
// {
//
// }

#if __AVX2__
inline int32_t securityId(const char *s)
{
    __m128i m8 = _mm_loadl_epi64((const __m128i *)s);
    m8 = _mm_sub_epi8(m8, _mm_set1_epi8('0'));
    __m256i m32 = _mm256_cvtepu8_epi32(m8);
    m32 = _mm256_mullo_epi32(m32, _mm256_set_epi32(0, 0, 1, 10, 100, 1000, 10000, 100000));
    __m128i sum = _mm_add_epi32(_mm256_castsi256_si128(m32), _mm256_extractf128_si256(m32, 1));
    sum = _mm_add_epi32(sum, _mm_srli_si128(sum, 8));
    sum = _mm_add_epi32(sum, _mm_srli_si128(sum, 4));
    return _mm_cvtsi128_si32(sum);
}
#else
inline int32_t securityId(const char *s)
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
#endif
