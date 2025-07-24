#include "MDD.h"
#include "MDS.h"
#include "daily.h"
#include "StockState.h"
#include "stockCodes.h"
#include "threadAffinity.h"
#include <chrono>
#include <spdlog/spdlog.h>

std::array<std::jthread, kChannelCount> MDD::g_computeThreads;
std::unique_ptr<StockState[]> MDD::g_stockStates;
std::unique_ptr<StockCompute[]> MDD::g_stockComputes;
std::unique_ptr<TickCache[]> MDD::g_tickCaches;

void MDD::handleTick(MDS::Tick &tick)
{
    int32_t id = kStockIdLut[static_cast<int16_t>(tick.stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }
    MDD::g_stockStates[id].onTick(tick);
}

namespace
{

void computeThreadMain(int32_t c, std::chrono::steady_clock::time_point startTime, std::stop_token stop)
{
    setThisThreadAffinity(kChannelCpuBegin + c);

    const int32_t startId = (kStockCodes.size() * c) / kChannelCount;
    const int32_t stopId = (kStockCodes.size() * (c + 1)) / kChannelCount;

#if REPLAY_REAL_TIME
    const auto kSleepInterval = duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::milliseconds(99) TIME_SCALE);
    auto nextSleepTime = startTime + kSleepInterval * c / kChannelCount + kSleepInterval;
#endif
    while (!stop.stop_requested()) [[likely]] {
#if REPLAY_REAL_TIME
#if 0
        for (int32_t id = startId; std::chrono::steady_clock::now() < nextSleepTime; id = id == stopId ? startId : id + 1) {
            MDD::g_stockComputes[id].onBusy();
        }
#else
        spinSleepUntil(nextSleepTime);
#endif
        nextSleepTime += kSleepInterval;
#endif

        for (int32_t id = startId; id != stopId; ++id) {
            MDD::g_stockComputes[id].onTimer();
        }
        for (int32_t id = startId; id != stopId; ++id) {
            MDD::g_stockComputes[id].onPostTimer();
        }
    }
}

}

void MDD::start()
{
    MDD::g_tickCaches = std::make_unique<TickCache[]>(kStockCodes.size());
    MDD::g_stockStates = std::make_unique<StockState[]>(kStockCodes.size());
    MDD::g_stockComputes = std::make_unique<StockCompute[]>(kStockCodes.size());

    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_tickCaches[i].start();
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockStates[i].start();
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockComputes[i].start();
    }

    MDS::subscribe(kStockCodes.data(), kStockCodes.size());
    MDS::start();
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    SPDLOG_INFO("starting {} compute channels", kChannelCount);
    auto t0 = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c] = std::jthread([t0, c] (std::stop_token stop) {
            computeThreadMain(c, t0, stop);
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
}

void MDD::stop()
{
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c].request_stop();
        g_computeThreads[c].join();
    }

    MDS::stop();

    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockComputes[i].stop();
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_stockStates[i].stop();
    }
    for (int32_t i = 0; i < kStockCodes.size(); ++i) {
        g_tickCaches[i].stop();
    }

    g_stockComputes.reset();
    g_stockStates.reset();
    g_tickCaches.reset();
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
