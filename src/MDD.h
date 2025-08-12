#pragma once

#include <array>
#include <thread>
#include "MDS.h"
#include "OES.h"
#include "constants.h"
#include "StockState.h"
#include "StockCompute.h"
#include "WantCache.h"
#include "TickRing.h"
#include "MemPool.h"


namespace MDD
{

extern std::array<std::jthread, kChannelCount> g_computeThreads;
extern StockState *g_stockStates;
extern StockCompute *g_stockComputes;
extern WantCache *g_wantCaches;
extern TickRing *g_tickRings;
extern std::vector<int32_t> g_stockCodes;
extern std::vector<int32_t> g_prevLimitUpStockCodes;

void start(const char *config);
void stop();
void requestStop();
bool isFinished();
void handleTick(MDS::Tick &tick);
void handleSnap(MDS::Snap &snap);
void handleStatic(MDS::Stat &stat);
void handleRspOrder(OES::RspOrder &rspOrder);

}
