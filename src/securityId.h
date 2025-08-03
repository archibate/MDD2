#pragma once


#include <cstdint>
// #include <immintrin.h>


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

// [[gnu::always_inline]] inline uint32_t atou6q(const char *s)
// {
//     __m128i m = _mm_loadl_epi64((const __m128i *)s);
//     m = _mm_sub_epi8(m, _mm_set1_epi8('0'));
//     m = _mm_unpacklo_epi8(m, _mm_setzero_si128());
//     m = _mm_mullo_epi16(m, _mm_set_epi16(0, 0, 1, 10, 100, 1000, 1, 10));
//     m = _mm_shuffle_epi32(m, 0b00111001);
//     m = _mm_hadd_epi16(m, m);
//     m = _mm_hadd_epi16(m, m);
//     m = _mm_unpacklo_epi16(m, _mm_setzero_si128());
//     m = _mm_mullo_epi32(m, _mm_set_epi32(0, 0, 10000, 1));
//     m = _mm_hadd_epi32(m, m);
//     return _mm_cvtsi128_si32(m);
// }
//
//
// [[gnu::always_inline]] inline uint32_t atou6q0(const char *s)
// {
//     __m128i m = _mm_loadl_epi64((const __m128i *)s);
//     m = _mm_sub_epi8(m, _mm_set1_epi8('0'));
//     m = _mm_unpacklo_epi8(m, _mm_setzero_si128());
//     m = _mm_mullo_epi16(m, _mm_set_epi16(0, 0, 1, 10, 100, 1000, 0, 0));
//     m = _mm_shuffle_epi32(m, 0b00001001);
//     m = _mm_hadd_epi16(m, m);
//     m = _mm_hadd_epi16(m, m);
//     return _mm_cvtsi128_si32(m);
// }


inline uint32_t securityId(const char *s)
{
    return atou6(s);
}
