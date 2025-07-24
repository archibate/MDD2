#include "MDD.h"
#include "MDS.h"
#include "daily.h"
#include "StockState.h"
#include "stockCodes.h"
#include <spdlog/spdlog.h>
#include <sched.h>
#include <unistd.h>

std::array<std::jthread, kChannelCount> MDD::g_computeThreads;
std::unique_ptr<StockState[]> MDD::g_stockStates;
std::unique_ptr<StockCompute[]> MDD::g_stockComputes;
std::unique_ptr<TickCache[]> MDD::g_tickCaches;

void handleTick(MDS::Tick &tick)
{
    int32_t id = kStockIdLut[static_cast<int16_t>(tick.stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }
    MDD::g_stockStates[id].onTick(tick);
}

void computeThreadMain(int32_t c, std::stop_token stop)
{
    const int32_t startId = (kStockCodes.size() * c) / kChannelCount;
    const int32_t stopId = (kStockCodes.size() * (c + 1)) / kChannelCount;

    while (!stop.stop_requested()) [[likely]] {
        for (int32_t id = startId; id != stopId; ++id) {
            MDD::g_stockComputes[id].onTimer();
        }
        for (int32_t id = startId; id != stopId; ++id) {
            MDD::g_stockComputes[id].onPostTimer();
        }
    }
}

void MDD::start()
{
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c] = std::jthread([c] (std::stop_token stop) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(c, &cpuset);
            sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
            computeThreadMain(c, stop);
        });
    }

    MDD::g_stockStates = std::make_unique<StockState[]>(kStockCodes.size());
    MDD::g_stockComputes = std::make_unique<StockCompute[]>(kStockCodes.size());
    MDD::g_tickCaches = std::make_unique<TickCache[]>(kStockCodes.size());
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockStates[i].start();
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockComputes[i].start();
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_tickCaches[i].start();
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
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_tickCaches[i].stop();
    }
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
