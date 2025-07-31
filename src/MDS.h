#pragma once

#include "config.h"
#include <cstdint>
#if REPLAY
#include "L2/Stat.h"
#include "L2/Tick.h"
#elif NE
#include <nesc/Const.h>
#include <nesc/SseMdStruct.h>
#include <nesc/SzeMdStruct.h>
#endif


namespace MDS
{

#if REPLAY
using Stat = L2::Stat;
using Tick = L2::Tick;
#elif NE

struct Stat {
    NescForesight::MarketType marketType;
    union {
        NescForesight::StaticSseInfo staticSseInfo;
        NescForesight::StaticSzInfo staticSzInfo;
    };
};

#if SH
struct Tick {
    union {
        NescForesight::TickMergeSse tickMergeSse;
        uint8_t messageType;
    };
};
#endif

#if SZ
struct Tick {
    union {
        NescForesight::TradeSz tradeSz;
        NescForesight::OrderSz orderSz;
        struct {
            uint8_t messageType;
            uint32_t sequence;
            uint8_t exchangeID;
            char securityID[9];
        };
    };
};
#endif

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
