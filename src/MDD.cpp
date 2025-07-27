#include "MDD.h"
#include "MDS.h"
#include "constants.h"
#include "StockState.h"
#include "threadAffinity.h"
#include "DailyState.h"
#include "FactorList.h"
#include "dateTime.h"
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

std::array<std::jthread, kChannelCount> MDD::g_computeThreads;
std::unique_ptr<StockState[]> MDD::g_stockStates;
std::unique_ptr<StockCompute[]> MDD::g_stockComputes;
std::unique_ptr<TickCache[]> MDD::g_tickCaches;
std::vector<int32_t> MDD::g_stockCodes;
std::vector<int32_t> MDD::g_prevLimitUpStockCodes;

namespace
{

std::array<int16_t, 0x7FFF> g_stockIdLut;

void initStockArrays()
{
    for (int32_t s = 0; s < g_stockIdLut.size(); ++s) {
        g_stockIdLut[s] = -1;
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockIdLut[static_cast<int16_t>(MDD::g_stockCodes[i] & 0x7FFF)] = i;
    }

    MDD::g_tickCaches = std::make_unique<TickCache[]>(MDD::g_stockCodes.size());
    MDD::g_stockStates = std::make_unique<StockState[]>(MDD::g_stockCodes.size());
    MDD::g_stockComputes = std::make_unique<StockCompute[]>(MDD::g_stockCodes.size());
}

void parseDailyConfig(const char *config)
{
    SPDLOG_INFO("reading config file: {}", config);
    nlohmann::json json;
    try {
        std::ifstream(config) >> json;
    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }
    SPDLOG_INFO("config json: {}", json.dump());

    int32_t date = json["date"];
    if (date <= 0) {
        SPDLOG_ERROR("invalid config format for mdd: {}", json.dump());
        throw std::runtime_error("invalid config format for mdd");
    }
#if !REPLAY
    if (int32_t today = getToday(); date != today) {
        SPDLOG_ERROR("config file date not today: fileDate={} todayDate={}", date, today);
        throw std::runtime_error("config file date not today");
    }
#endif

#if REPLAY
    std::string factorFile = json.contains("factor_file") ? std::string(json["factor_file"])
        : REPLAY_DATA_PATH "/daily_csv/mdd2_factors_" MARKET_NAME_LOWER "_" + std::to_string(date) + ".bin";
#else
    std::string factorFile = json["factor_file"];
#endif
    std::ifstream bin(factorFile, std::ios::binary);
    if (!bin.is_open()) {
        SPDLOG_ERROR("cannot open daily factors for market={} date={} factorFile=[{}]", MARKET_NAME, date, factorFile);
        throw std::runtime_error("cannot open daily factors");
    }
    DailyHeader header{};
    bin.read((char *)&header, sizeof(header));

    constexpr int32_t kFileVersion = 250722;
    if (header.fileVersion != kFileVersion) {
        SPDLOG_ERROR("daily factor file version mismatch: fileVersion={} expectVersion={}",
                     header.fileVersion, kFileVersion);
        throw std::runtime_error("daily factor file version mismatch");
    }
    if (header.marketID != MARKET_ID) {
        SPDLOG_ERROR("daily factor file market id mismatch: fileMarketId={} expectMarketId={}",
                     header.marketID, MARKET_ID);
        throw std::runtime_error("daily factor file market id mismatch");
    }
    if (header.factorCount != FactorEnum::kMaxFactors) {
        SPDLOG_ERROR("daily factor file factor count mismatch: fileFactorCount={} expectFactorCount={}",
                     header.factorCount, FactorEnum::kMaxFactors);
        throw std::runtime_error("daily factor file factor count mismatch");
    }
    if (header.factorDtypeSize != sizeof(FactorList::ScalarType)) {
        SPDLOG_ERROR("daily factor file factor dtype mismatch: fileDtypeSize={} expectDtypeSize={}",
                     header.factorDtypeSize, sizeof(FactorList::ScalarType));
        throw std::runtime_error("daily factor file factor dtype mismatch");
    }

    if (header.today != date) {
        SPDLOG_ERROR("daily factor file today date mismatch: fileToday={} expectToday={}",
                     header.today, date);
        throw std::runtime_error("daily factor file today date mismatch");
    }
    if (!header.stockCount) {
        SPDLOG_ERROR("no stock subscription found in daily factor file for market={} date={}", MARKET_NAME, date);
        throw std::runtime_error("no stock subscription found in daily factor file");
    }
    SPDLOG_INFO("daily factor file contains numStocks={} numPrevLimitUp={} numFactors={}", header.stockCount, header.prevLimitUpCount, header.factorCount);

    MDD::g_stockCodes.resize(header.stockCount);
    bin.read((char *)MDD::g_stockCodes.data(), MDD::g_stockCodes.size() * sizeof(int32_t));
    if (!bin.good()) {
        SPDLOG_ERROR("failed to read all stock subscribes");
        throw std::runtime_error("failed to read all stock subscribes");
    }

    MDD::g_prevLimitUpStockCodes.resize(header.prevLimitUpCount);
    bin.read((char *)MDD::g_prevLimitUpStockCodes.data(), MDD::g_prevLimitUpStockCodes.size() * sizeof(int32_t));
    if (!bin.good()) {
        SPDLOG_ERROR("failed to read all prev-limit-up stocks");
        throw std::runtime_error("failed to read all prev-limit-up stocks");
    }

    static_assert(FactorEnum::kMaxFactors * sizeof(FactorList::ScalarType) == sizeof(FactorList));
    std::vector<FactorList> factors(header.stockCount);
    bin.read((char *)factors.data(), factors.size() * sizeof(FactorList));
    if (!bin.good()) {
        SPDLOG_ERROR("failed to read all stock factors");
        throw std::runtime_error("failed to read all stock factors");
    }

    bin.close();
    initStockArrays();

    MDD::g_stockComputes.reset(new StockCompute[MDD::g_stockCodes.size()]{});
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        MDD::g_stockComputes[i].factorList = factors[i];
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
    parseDailyConfig(config);

    MDS::subscribe(MDD::g_stockCodes.data(), MDD::g_stockCodes.size());
    MDS::start(config);
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_tickCaches[i].start();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockStates[i].start();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockComputes[i].start();
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

void MDD::requestStop()
{
    MDS::requestStop();
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
