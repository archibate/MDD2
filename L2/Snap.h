#pragma once


#include <cstdint>


const int32_t kBookLevels = 5;


struct Snap
{
    int32_t stock;
    int32_t timestamp;
    int32_t lastPrice;
    int32_t preClosePrice;
    int32_t numTrades;
    int32_t volume;
    int64_t amount;
    int32_t bidPrice[kBookLevels];
    int32_t bidQuantity[kBookLevels];
    int32_t askPrice[kBookLevels];
    int32_t askQuantity[kBookLevels];
};
