#pragma once

#include <array>
#include "MDS.h"
#include "daily.h"
#include "DisruptorPool.h"
#include "stockCodes.h"
#include "StockState.h"


namespace MDD
{

extern std::array<DisruptorPool<MDS::Tick>, kChannelCount> g_channelPool;
extern std::unique_ptr<StockState[]> g_stockStates;

void start();
void stop();
bool isFinished();

}
