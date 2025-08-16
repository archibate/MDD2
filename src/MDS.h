#pragma once

#include "config.h"
#include "XeleCompat.h"
#include <cstdint>


namespace MDS
{

struct Stat
{
    uint32_t stock;
    uint32_t preClosePrice;
    uint32_t upperLimitPrice;
    uint32_t lowerLimitPrice;
};

struct Snap
{
    uint32_t stock;
    uint32_t timestamp;
    uint64_t volume;
    uint64_t amount;
    uint32_t numTrades;
    uint32_t lastPrice;
    uint32_t preClosePrice;
    uint32_t openPrice;
    uint32_t highPrice;
    uint32_t lowPrice;
    uint32_t upperLimitPrice;
    uint32_t upperLimitrice;
    uint32_t bidPrice[5];
    uint32_t askPrice[5];
    uint64_t bidQuantity[5];
    uint64_t askQuantity[5];
};

struct Tick
{
    union
    {
        XeleCompat::TickMergeSse tickMergeSse;
        XeleCompat::TradeSz tradeSz;
        XeleCompat::OrderSz orderSz;
        XeleCompat::CommonHead commonHead;
        uint8_t messageType;
    };
};

static_assert(sizeof(Tick) == 64);

void subscribe(int32_t const *stocks, int32_t n);
void start(const char *config);
void startReceive();
void stop();
void requestStop();
bool isFinished();
bool isStarted();

#if REPLAY
extern double g_timeScale;
#endif

}
