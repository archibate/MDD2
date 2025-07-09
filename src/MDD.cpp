#include "MDD.h"
#include "MDS.h"
#include "StockState.h"
#include "daily.h"
#include "stockCodes.h"

std::array<DisruptorPool<MDS::Tick>, kChannelCount> MDD::g_channelPool;
std::array<StockState, kStockCodes.size()> MDD::g_stockStates;


namespace
{

void handleTick(MDS::Tick &tick)
{
    int16_t id = kStockIdLut[static_cast<int16_t>(tick.stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }
    MDD::g_stockStates[id].handleTick(tick);
}

}

void MDD::start()
{
    for (int c = 0; c < g_channelPool.size(); ++c) {
        g_channelPool[c].init(kChannelPoolSize, true, kChannelCpuBegin + c);
        g_channelPool[c].start([] (MDS::Tick &tick) {
            handleTick(tick);
        });
    }

    for (int i = 0; i < g_stockStates.size(); ++i) {
        g_stockStates[i].start(i);
    }

    MDS::subscribe(kStockCodes.data(), kStockCodes.size());
    MDS::start();
}

void MDD::stop()
{
    MDS::stop();

    for (int i = 0; i < g_stockStates.size(); ++i) {
        g_stockStates[i].stop();
    }
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
