#pragma once


#include <cstdint>
#include <immintrin.h>

// inline int32_t timestampLinear(int32_t timestamp)
// {
//     int32_t hours = timestamp / 10000000;
//     int32_t minutes = (timestamp / 100000) % 100;
//     int32_t seconds = (timestamp / 1000) % 100;
//     int32_t milliseconds = timestamp % 1000;
//
//     return (hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
// }

// inline int32_t timestampDelinear(int32_t time)
// {
//     int32_t milliseconds = time % 1000;
//     time /= 1000;
//     int32_t seconds = time % 60;
//     time /= 60;
//     int32_t minutes = time % 60;
//     time /= 60;
//     int32_t hours = time % 24;
//     return milliseconds + 1000 * (seconds + 100 * (minutes + 100 * hours));
// }

#if __AVX2__
inline int32_t timestampLinear(int32_t timestamp)
{
    __m128i time = _mm_set1_epi32(timestamp >> 5);
    // 100000, 10000000
    time = _mm_mul_epu32(time, _mm_set_epi32(0, 175921861, 0, 1801439851));
    time = _mm_srlv_epi64(time, _mm_set_epi64x(39, 49));
    time = _mm_mul_epu32(time, _mm_set_epi32(0, 40000, 0, 2400000));
    time = _mm_add_epi64(time, _mm_srli_si128(time, 8));
    return timestamp - _mm_cvtsi128_si32(time);
}

inline int32_t timestampDelinear(int32_t timestamp)
{
    __m128i time = _mm_set1_epi32(timestamp);
    // 60000, 3600000
    time = _mm_mul_epu32(time, _mm_set_epi32(0, 1172812403, 0, 2501999793U));
    time = _mm_srlv_epi64(time, _mm_set_epi64x(46, 53));
    time = _mm_mul_epu32(time, _mm_set_epi32(0, 40000, 0, 4000000));
    time = _mm_add_epi64(time, _mm_srli_si128(time, 8));
    return timestamp + _mm_cvtsi128_si32(time);
}
#else
inline int32_t timestampLinear(int32_t timestamp)
{
    uint32_t u = timestamp;
    uint16_t mh = uint16_t((uint64_t(u >> 5) * 175921861) >> 39);
    uint32_t hours = uint16_t((uint32_t(mh >> 2) * 5243) >> 17);
    return u - (hours * 60 + uint32_t(mh)) * 40000;
}

inline int32_t timestampDelinear(int32_t time)
{
    uint32_t u = time;
    uint16_t mh = uint16_t((uint64_t(u) * 1172812403) >> 46);
    uint16_t hours = uint16_t((uint32_t(mh) * 34953) >> 21);
    return u + 40000 * uint32_t(mh + 100 * hours);
}
#endif

inline int64_t timestampAbsLinear(int32_t timestamp)
{
    int64_t time = timestampLinear(timestamp);
    time -= 342'00'000;
    if (time > 2 * 60 * 60'000) {
        if (time < (3 * 60 + 30) * 60'000) {
            time = 2 * 60 * 60'000 + 100;
        } else {
            time -= (1 * 60 + 30) * 60'000 - 100;
        }
    }
    return time;
}

inline int32_t timestampAbsDelinear(int64_t time)
{
    if (time > 2 * 60 * 60'000) {
        time += (1 * 60 + 30) * 60'000 - 100;
    }
    time += 342'00'000;
    return timestampDelinear(time);
}

inline int32_t timestampAdvance(int32_t timestamp, int32_t ms)
{
    return timestampAbsDelinear(timestampAbsLinear(timestamp) + ms);
}

inline int32_t timestampAdvance100ms(int32_t timestamp)
{
    return timestampAbsDelinear(timestampAbsLinear(timestamp) + 100);
}

inline int32_t timestampDifference(int32_t t1, int32_t t2)
{
    return timestampAbsLinear(t1) - timestampAbsLinear(t2);
}
