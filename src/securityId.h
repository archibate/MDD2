#pragma once


#include <cstdint>
#include <immintrin.h>


#if __AVX2__
inline uint32_t securityId(const char *s)
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

inline void fmtSecurityId(char securityID[9], int32_t stock)
{
    securityID[8] = 0;
    securityID[7] = 0;
    securityID[6] = 0;
    securityID[5] = '0' + stock % 10;
    stock /= 10;
    securityID[4] = '0' + stock % 10;
    stock /= 10;
    securityID[3] = '0' + stock % 10;
    stock /= 10;
    securityID[2] = '0' + stock % 10;
    stock /= 10;
    securityID[1] = '0' + stock % 10;
    stock /= 10;
    securityID[0] = '0' + stock % 10;
}
