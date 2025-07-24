#pragma once

#include <array>
#include <thread>
#include "MDS.h"
#include "daily.h"
#include "stockCodes.h"
#include "StockState.h"
#include "StockCompute.h"
#include "TickCache.h"


namespace MDD
{

extern std::array<std::jthread, kChannelCount> g_computeThreads;
extern std::unique_ptr<StockState[]> g_stockStates;
extern std::unique_ptr<StockCompute[]> g_stockComputes;
extern std::unique_ptr<TickCache[]> g_tickCaches;

void start();
void stop();
bool isFinished();
void handleTick(MDS::Tick &tick);

}
