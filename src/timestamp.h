#pragma once


#include <cstdint>
#include "L2/timestamp.h"

[[gnu::always_inline]] inline int32_t timestampLinear(int32_t timestamp)
{
    return L2::timestampToMilliseconds(timestamp);
}

[[gnu::always_inline]] inline int32_t timestampDelinear(int32_t timestamp)
{
    return L2::millisecondsToTimestamp(timestamp);
}

[[gnu::always_inline]] inline int32_t timestampAdvance(int32_t timestamp, int32_t ms)
{
    return L2::positiveAbsoluteMillisecondsToTimestamp(
        L2::timestampToPositiveAbsoluteMilliseconds(
            timestamp) + ms);
}

[[gnu::always_inline]] inline int32_t timestampDifference(int32_t t1, int32_t t2)
{
    return L2::timestampToPositiveAbsoluteMilliseconds(t1) - L2::timestampToPositiveAbsoluteMilliseconds(t2);
}

[[gnu::always_inline]] inline int32_t timestampAdvance100ms(int32_t timestamp)
{
    return L2::positiveAbsoluteMillisecondsToTimestamp(
        L2::timestampToPositiveAbsoluteMilliseconds(
            timestamp) + 100);
}
