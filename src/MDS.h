#pragma once

#include "config.h"
#include <cstdint>
#if REPLAY
#include "L2/Tick.h"
#include "L2/Stat.h"
#elif NE
#include <nesc/Const.h>
#include <nesc/SseMdStruct.h>
#include <nesc/SzeMdStruct.h>
#endif


namespace MDS
{

#if REPLAY
using Tick = L2::Tick;
using Stat = L2::Stat;
#elif NE

#if SH
struct Tick {
    NescForesight::TickMergeSse tickMergeSse;
};
#endif

#if SZ
struct Tick {
    union {
        NescForesight::TradeSz tradeSz;
        NescForesight::OrderSz orderSz;
    };
};
#endif

struct Stat {
    NescForesight::MarketType marketType;
    union {
        NescForesight::StaticSseInfo staticSseInfo;
        NescForesight::StaticSzInfo staticSzInfo;
    };
};

#endif

void subscribe(int32_t const *stocks, int32_t n);
MDS::Stat getStatic(int32_t stock);
void start(const char *config);
void startReceive();
void stop();
void requestStop();
bool isFinished();
bool isStarted();

}
