#include "MDD.h"
#include "MDS.h"
#include "StockState.h"
#include "daily.h"
#include "stockCodes.h"
#include <spdlog/spdlog.h>
#include <sched.h>
#include <unistd.h>
#include <emmintrin.h>

std::array<DisruptorPool<MDS::Tick>, kChannelCount> MDD::g_channelPool;
std::unique_ptr<StockState[]> MDD::g_stockStates;

namespace
{

void initStockCodes()
{
    MDD::g_stockStates = std::make_unique<StockState[]>(kStockCodes.size());
}

void handleTick(int32_t ch, MDS::Tick &tick)
{
    int32_t id = kStockIdLut[static_cast<int16_t>(tick.stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }

    MDD::g_stockStates[id].handleTick(tick);
}

}

void MDD::start()
{
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_channelPool[c].init(kChannelPoolSize, true, kChannelCpuBegin + c);
        g_channelPool[c].start([c] (MDS::Tick &tick) {
            handleTick(c, tick);
        });
    }

    initStockCodes();

    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockStates[i].start(i);
    }

    MDS::subscribe(kStockCodes.data(), kStockCodes.size());
    MDS::start();
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void MDD::stop()
{
    MDS::stop();

    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockStates[i].stop();
    }
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
