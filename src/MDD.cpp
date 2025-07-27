#include "MDD.h"
#include "MDS.h"
#include "constants.h"
#include "StockState.h"
#include "threadAffinity.h"
#include "DailyState.h"
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

std::array<std::jthread, kChannelCount> MDD::g_computeThreads;
std::unique_ptr<StockState[]> MDD::g_stockStates;
std::unique_ptr<StockCompute[]> MDD::g_stockComputes;
std::unique_ptr<TickCache[]> MDD::g_tickCaches;
std::vector<int32_t> MDD::g_stockCodes;

namespace
{

std::array<int16_t, 0x7FFF> g_stockIdLut;

void initStockCodes()
{
    for (int32_t s = 0; s < g_stockIdLut.size(); ++s) {
        g_stockIdLut[s] = -1;
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockIdLut[static_cast<int16_t>(MDD::g_stockCodes[i] & 0x7FFF)] = i;
    }
}

void computeThreadMain(int32_t c, std::chrono::steady_clock::time_point startTime, std::stop_token stop)
{
    setThisThreadAffinity(kChannelCpuBegin + c);

    const int32_t startId = (MDD::g_stockCodes.size() * c) / kChannelCount;
    const int32_t stopId = (MDD::g_stockCodes.size() * (c + 1)) / kChannelCount;

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

        int32_t approachCount = 0;
        for (int32_t id = startId; id != stopId; ++id) {
            MDD::g_stockComputes[id].onTimer();
            if (MDD::g_stockComputes[id].isApproachingLimitUp()) {
                ++approachCount;
            }
        }
        if (approachCount > 0) {
            for (int32_t id = startId; id != stopId; ++id) {
                MDD::g_stockComputes[id].onPostTimer();
            }
        }
    }
}

}

void MDD::handleTick(MDS::Tick &tick)
{
    int32_t id = g_stockIdLut[static_cast<int16_t>(tick.stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }
    MDD::g_stockStates[id].onTick(tick);
}

void MDD::start(const char *config)
{
    {
        nlohmann::json json;
        std::ifstream(config) >> json;

        int32_t date = json["date"];
        if (date <= 0) {
            throw std::runtime_error("invalid config for mdd");
        }

        std::ifstream fac(DATA_PATH "/mdd2_factors_" MARKET_NAME "_" + std::to_string(date) + ".bin", std::ios::binary);
        DailyHeader header{};
        fac.read((char *)&header, sizeof(header));
    }

    if (MDD::g_stockCodes.empty()) {
        throw std::runtime_error("no stocks to subscribe!");
    }

    MDD::g_tickCaches = std::make_unique<TickCache[]>(MDD::g_stockCodes.size());
    MDD::g_stockStates = std::make_unique<StockState[]>(MDD::g_stockCodes.size());
    MDD::g_stockComputes = std::make_unique<StockCompute[]>(MDD::g_stockCodes.size());

    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_tickCaches[i].start();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockStates[i].start();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockComputes[i].start();
    }

    MDS::subscribe(MDD::g_stockCodes.data(), MDD::g_stockCodes.size());
    MDS::start(config);
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    SPDLOG_INFO("starting {} compute channels", kChannelCount);
    auto t0 = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c] = std::jthread([c, t0] (std::stop_token stop) {
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

    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockComputes[i].stop();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockStates[i].stop();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
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
