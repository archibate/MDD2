#include "MDD.h"
#include "MDS.h"
#include "OES.h"
#include "constants.h"
#include "StockState.h"
#include "threadAffinity.h"
#include "DailyState.h"
#include "FactorList.h"
#include "dateTime.h"
#include "securityId.h"
#include "heatZone.h"
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

std::array<std::jthread, kChannelCount> MDD::g_computeThreads;
std::unique_ptr<StockState[]> MDD::g_stockStates;
std::unique_ptr<StockCompute[]> MDD::g_stockComputes;
std::unique_ptr<WantCache[]> MDD::g_wantCaches;
std::unique_ptr<TickRing[]> MDD::g_tickRings;
std::vector<int32_t> MDD::g_stockCodes;
std::vector<int32_t> MDD::g_prevLimitUpStockCodes;

#if REPLAY
namespace MDS
{
extern double g_timeScale;
}
#endif

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

    MDD::g_tickRings = std::make_unique<TickRing[]>(kChannelCount);
    MDD::g_wantCaches = std::make_unique<WantCache[]>(MDD::g_stockCodes.size());
    MDD::g_stockStates = std::make_unique<StockState[]>(MDD::g_stockCodes.size());
    MDD::g_stockComputes = std::make_unique<StockCompute[]>(MDD::g_stockCodes.size());
}

void parseDailyConfig(const char *config)
{
    SPDLOG_INFO("reading config file: {}", config);

    int32_t date = 0;
    std::string factorFile;
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
        SPDLOG_DEBUG("config json: {}", json.dump());

        if (json.contains("factor_file")) {
            date = json["date"];
        }
        if (json.contains("factor_file")) {
            factorFile = json["factor_file"];
        }
    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    // if (date <= 0) {
    //     SPDLOG_ERROR("invalid config date: {}", date);
    //     throw std::runtime_error("invalid config date");
    // }

    SPDLOG_INFO("daily start: market={} date={}", MARKET_NAME, date);
#if !REPLAY
    if (int32_t today = getToday(); date != today) {
        SPDLOG_ERROR("config file date not today: fileDate={} todayDate={}", date, today);
        throw std::runtime_error("config file date not today");
    }
#endif

    if (factorFile.empty()) {
#if REPLAY
        factorFile = REPLAY_DATA_PATH "/daily_csv/mdd2_factors_" MARKET_NAME_LOWER "_" + std::to_string(date) + ".bin";
#else
        SPDLOG_ERROR("no factor file provided in config");
        throw std::runtime_error("no factor file provided in config");
#endif
    }
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

    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        MDD::g_stockComputes[i].loadFactors(factors[i]);
    }
}

HEAT_ZONE_TIMER void computeThreadMain(int32_t channel, int32_t startId, int32_t stopId, int64_t nextSleepTime, int64_t sleepInterval, std::stop_token stop)
{
    std::vector<int32_t> approachStockIds;
    std::vector<int64_t> remainSellAmounts;
    approachStockIds.reserve(8);
    remainSellAmounts.reserve(8);

    while (!stop.stop_requested()) [[likely]] {
#if BUSY_COMPUTE
        for (int32_t id = startId; steadyNow() < nextSleepTime && !stop.stop_requested(); id = id == stopId ? startId : id + 1) {
            MDD::g_stockComputes[id].onBusy();
        }
#else
        blockingSleepUntil(nextSleepTime);
#endif
        nextSleepTime += sleepInterval;

        approachStockIds.clear();
        for (int32_t id = startId; id != stopId; ++id) {
            MDD::g_stockComputes[id].clearApproach();
        }

        thread_local MDS::Tick tickBuf[1024];
        size_t n = MDD::g_tickRings[channel].fetchTicks(tickBuf, sizeof tickBuf / sizeof tickBuf[0]);
        for (size_t i = 0; i < n; ++i) {
            auto &tick = tickBuf[i];
#if REPLAY
            int32_t stock = tick.stock;
#elif NE && SH
            int32_t stock = securityId(tick.tickMergeSse.securityID);
#elif NE && SZ
            int32_t stock = securityId(tick.securityID);
#endif
            int32_t id = g_stockIdLut[static_cast<int16_t>(stock & 0x7FFF)];
            if (id == -1) [[unlikely]] {
                continue;
            }
            MDD::g_stockComputes[id].onTick(tick);
        }

        for (int32_t id = startId; id != stopId; ++id) {
            if (MDD::g_stockComputes[id].checkApproach()) {
                approachStockIds.push_back(id);
            }
        }
        if (!approachStockIds.empty()) {
            if (approachStockIds.size() > 3) [[unlikely]] {
                remainSellAmounts.clear();
                for (int32_t id: approachStockIds) {
                    remainSellAmounts.push_back(MDD::g_stockComputes[id].upSellOrderAmount());
                }
                std::nth_element(approachStockIds.begin(), approachStockIds.begin() + 2, approachStockIds.end(), [&] (int32_t &id1, int32_t &id2) {
                    return remainSellAmounts[&id1 - approachStockIds.data()] < remainSellAmounts[&id2 - approachStockIds.data()];
                });
                approachStockIds.resize(2);
            }
            for (int32_t id: approachStockIds) {
                MDD::g_stockComputes[id].onApproach();
            }
        }
    }
}

}

HEAT_ZONE_TICK void MDD::handleTick(MDS::Tick &tick)
{
#if REPLAY
    int32_t stock = tick.stock;
#elif NE && SH
    int32_t stock = securityId(tick.tickMergeSse.securityID);
#elif NE && SZ
    int32_t stock = securityId(tick.securityID);
#endif
    int32_t id = g_stockIdLut[static_cast<int16_t>(stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }
    MDD::g_stockStates[id].onTick(tick);
}

HEAT_ZONE_RSPORDER void MDD::handleRspOrder(OES::RspOrder &rspOrder)
{
#if REPLAY
    int32_t stock = rspOrder.stockCode;
#elif XC || NE
    int32_t stock = securityId(rspOrder.xeleRsp.SecuritiesID);
#endif
    int32_t id = g_stockIdLut[static_cast<int16_t>(stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }
    MDD::g_stockStates[id].onRspOrder(rspOrder);
    // todo: forward to StockState
}

void MDD::start(const char *config)
{
    parseDailyConfig(config);
    SPDLOG_INFO("found {} stocks", MDD::g_stockCodes.size());

    SPDLOG_INFO("initializing trade api");
    OES::start(config);

    SPDLOG_INFO("initializing quote api");
    MDS::start(config);

    while (!OES::isStarted()) {
        SPDLOG_DEBUG("wait for trade initial");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SPDLOG_INFO("starting {} stocks", MDD::g_stockCodes.size());
    for (int32_t i = 0; i < kChannelCount; ++i) {
        g_tickRings[i].start();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_wantCaches[i].start();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockStates[i].start();
    }

    SPDLOG_INFO("preparing {} compute channels", kChannelCount);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        int32_t startId = (MDD::g_stockCodes.size() * c) / kChannelCount;
        int32_t stopId = (MDD::g_stockCodes.size() * (c + 1)) / kChannelCount;

        for (int32_t id = startId; id != stopId; ++id) {
            g_stockStates[id].setChannelId(c);
        }
    }

    SPDLOG_INFO("subscribing {} stocks", MDD::g_stockCodes.size());
    MDS::subscribe(MDD::g_stockCodes.data(), MDD::g_stockCodes.size());

    SPDLOG_INFO("start receiving stock quotes");
    MDS::startReceive();
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SPDLOG_INFO("mds receive started");

    SPDLOG_INFO("setting up {} stock statics", MDD::g_stockCodes.size());
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockStates[i].setStatic(MDS::getStatic(g_stockStates[i].stockCode));
        g_stockComputes[i].start();
    }

    SPDLOG_INFO("starting {} compute channels", kChannelCount);
    int64_t startTime = steadyNow() + 20'000'000;
    for (int32_t c = 0; c < kChannelCount; ++c) {
        int32_t startId = (MDD::g_stockCodes.size() * c) / kChannelCount;
        int32_t stopId = (MDD::g_stockCodes.size() * (c + 1)) / kChannelCount;

        g_computeThreads[c] = std::jthread([c, startId, stopId, startTime] (std::stop_token stop) {
            setThisThreadAffinity(kChannelCpus[c]);

            int64_t sleepInterval = 98'521'100;
#if REPLAY
            sleepInterval *= MDS::g_timeScale;
#endif
            int64_t nextSleepTime = startTime + sleepInterval * c / kChannelCount + sleepInterval;
            computeThreadMain(c, startId, stopId, nextSleepTime, sleepInterval, stop);
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    SPDLOG_INFO("system fully started");
}

void MDD::stop()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SPDLOG_INFO("stopping mds");
    MDS::stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    SPDLOG_INFO("stopping oes");
    OES::stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    SPDLOG_INFO("stopping {} compute channels", kChannelCount);

    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c].request_stop();
        g_computeThreads[c].join();
    }
    SPDLOG_INFO("stopping {} stocks", MDD::g_stockCodes.size());

    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockComputes[i].stop();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_stockStates[i].stop();
    }
    for (int32_t i = 0; i < MDD::g_stockCodes.size(); ++i) {
        g_wantCaches[i].stop();
    }
    for (int32_t i = 0; i < kChannelCount; ++i) {
        g_tickRings[i].stop();
    }

    g_stockComputes.reset();
    g_stockStates.reset();
    g_wantCaches.reset();
    g_tickRings.reset();
    SPDLOG_INFO("system fully stopped");
}

void MDD::requestStop()
{
    MDS::requestStop();
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
