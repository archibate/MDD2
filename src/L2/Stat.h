#pragma once


#include <cstdint>


namespace L2
{

struct Stat
{
    int32_t stock;
    int32_t openPrice;
    int32_t preClosePrice;
    int32_t highPrice;
    int32_t lowPrice;
    int32_t closePrice;
    int32_t upperLimitPrice;
    int32_t lowerLimitPrice;
    double floatMV;
};

}
