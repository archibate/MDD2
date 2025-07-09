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
extern std::array<StockState, kStockCodes.size()> g_stockStates;

void start();
void stop();
bool isFinished();

}
