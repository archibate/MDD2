#include "MDD.h"
#include "MDS.h"
#include "OES.h"
#include "SPSC.h"
#include "constants.h"
#include "threadAffinity.h"
#include "DailyState.h"
#include "WantCache.h"
#include "FactorList.h"
#include "dateTime.h"
#include "tickProps.h"
#include "heatZone.h"
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace
{

#if (NE || OST) && SZ
const uint64_t g_szOffsetTransactTime = static_cast<uint64_t>(getToday()) * UINT64_C(1'00'00'00'000);
#endif

struct State
{
#if (NE || OST) && SH
    uint32_t upperLimitPrice1000;
#endif
#if (NE || OST) && SZ
    uint64_t upperLimitPrice10000;
    int64_t upRemainQty100;
#endif
#if REPLAY && SH
    int32_t upperLimitPrice;
#endif
#if REPLAY && SZ
    int32_t upperLimitPrice;
    uint64_t upRemainQty;
#endif
};

using TickRing = spsc_ring<MDS::Tick, 1024>;

std::array<std::jthread, kChannelCount> g_computeThreads;
std::vector<int32_t> g_stockCodes;
std::vector<int32_t> g_prevLimitUpStockCodes;
std::vector<FactorList> g_stockFactors;
std::array<int16_t, 0x2000> g_stockIdLut;
std::unique_ptr<State[]> g_stockStates;
std::unique_ptr<WantCache[]> g_wantCaches;
std::unique_ptr<TickRing[]> g_tickRings;
std::unique_ptr<OES::ReqOrder[]> g_buyRequests;

uint16_t linearizeStockIdRaw(int32_t stock)
{
    uint32_t u = static_cast<uint32_t>(stock);
#if SH
    u -= UINT32_C(600000);
#endif
    if (u & ~UINT32_C(0x1FFF)) {
        u = UINT32_C(0x1FFF);
    }
    // SH 600xxx 601xxx 603xxx 605xxx
    // SZ 000xxx 001xxx 002xxx 003xxx
    return static_cast<uint16_t>(u);
}

int32_t linearizeStockId(int32_t stock)
{
    return g_stockIdLut[linearizeStockIdRaw(stock)];
}

void initStockArrays()
{
    for (int32_t s = 0; s < g_stockIdLut.size(); ++s) {
        g_stockIdLut[s] = -1;
    }
    for (int32_t i = 0; i < g_stockCodes.size(); ++i) {
        g_stockIdLut[linearizeStockIdRaw(g_stockCodes[i])] = i;
    }

    // g_tickRings = std::make_unique<TickRing[]>(kChannelCount);
    // g_wantCaches = std::make_unique<WantCache[]>(g_stockCodes.size());
    // g_stockStates = std::make_unique<StockState[]>(g_stockCodes.size());
    // g_stockComputes = std::make_unique<StockCompute[]>(g_stockCodes.size());
    // g_tickRings = g_memPool.allocate<TickRing>(kChannelCount);
    // g_wantCaches = g_memPool.allocate<WantCache>(g_stockCodes.size());
    // g_stockStates = g_memPool.allocate<StockState>(g_stockCodes.size());
    // g_stockComputes = g_memPool.allocate<StockCompute>(g_stockCodes.size());
    g_stockStates = std::make_unique<State[]>(g_stockCodes.size());
    g_wantCaches = std::make_unique<WantCache[]>(g_stockCodes.size());
    g_tickRings = std::make_unique<TickRing[]>(g_stockCodes.size());
    g_buyRequests = std::make_unique<OES::ReqOrder[]>(g_stockCodes.size());
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

        if (json.contains("date")) {
            date = json["date"];
        }
        if (json.contains("factor_file")) {
            factorFile = json["factor_file"];
        }
    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    SPDLOG_INFO("daily start: market={} date={}", MARKET_NAME, date);
#if !REPLAY
    if (int32_t today = getToday(); date != today) {
        SPDLOG_WARN("config file date not today: fileDate={} todayDate={}", date, today);
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
    std::ifstream facFile(factorFile, std::ios::binary);
    if (!facFile.is_open()) {
        SPDLOG_ERROR("cannot open daily factors for market={} date={} factorFile=[{}]", MARKET_NAME, date, factorFile);
        throw std::runtime_error("cannot open daily factors");
    }
    DailyHeader header{};
    facFile.read((char *)&header, sizeof(header));

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
    SPDLOG_DEBUG("daily factor file contains numStocks={} numPrevLimitUp={} numFactors={}", header.stockCount, header.prevLimitUpCount, header.factorCount);

    g_stockCodes.resize(header.stockCount);
    facFile.read((char *)g_stockCodes.data(), g_stockCodes.size() * sizeof(int32_t));
    if (!facFile.good()) {
        SPDLOG_ERROR("failed to read all stock subscribes");
        throw std::runtime_error("failed to read all stock subscribes");
    }

    g_prevLimitUpStockCodes.resize(header.prevLimitUpCount);
    facFile.read((char *)g_prevLimitUpStockCodes.data(), g_prevLimitUpStockCodes.size() * sizeof(int32_t));
    if (!facFile.good()) {
        SPDLOG_ERROR("failed to read all prev-limit-up stocks");
        throw std::runtime_error("failed to read all prev-limit-up stocks");
    }

    static_assert(FactorEnum::kMaxFactors * sizeof(FactorList::ScalarType) == sizeof(FactorList));
    g_stockFactors.resize(header.stockCount);
    facFile.read((char *)g_stockFactors.data(), g_stockFactors.size() * sizeof(FactorList));
    if (!facFile.good()) {
        SPDLOG_ERROR("failed to read all stock factors");
        throw std::runtime_error("failed to read all stock factors");
    }

    facFile.close();
    initStockArrays();
}

WantCache::Intent checkWantBuyAtTimestamp(int32_t id, int32_t timestamp)
{
    return g_wantCaches[id].checkWantBuyAtTimestamp(timestamp);
}

void sendBuyRequest(int32_t id)
{
    return OES::sendReqOrder(g_buyRequests[id]);
}

void pushTickToRing(int32_t id, MDS::Tick &tick)
{
    if (!g_tickRings[id].write_one(tick)) [[unlikely]] {
        _Exit(233);
    }
}

}

HEAT_ZONE_TICK void MDD::handleTick(MDS::Tick &tick)
{
    int32_t stock = tickStockCode(tick);
    int32_t id = linearizeStockId(stock);
    if (id == -1) [[unlikely]] {
        return;
    }
    auto &state = g_stockStates[id];
#if REPLAY
#if SH
    bool limitUp = tick.buyOrderNo != 0
        && tick.sellOrderNo == 0
        && tick.quantity > 0
        && tick.price == state.upperLimitPrice
        && tick.timestamp >= 9'30'00'000
        && tick.timestamp < 14'57'00'000;
#endif
#if SZ
    bool limitUp = tick.buyOrderNo != 0
        && tick.sellOrderNo == 0
        && tick.quantity > state.upRemainQty
        && tick.price == state.upperLimitPrice
        && tick.timestamp >= 9'30'00'000
        && tick.timestamp < 14'57'00'000;
#endif
    if (limitUp) {
        auto intent = wantCache->checkWantBuyAtTimestamp(tick.timestamp);
        if (intent == WantCache::WantBuy || ALWAYS_BUY) [[likely]] {
            sendBuyRequest(id);
        }

        logStop(id, tick.timestamp + static_cast<int32_t>(intent));
        return;
    }

#if SZ
    if (tick.sellOrderNo != 0 && tick.price == upperLimitPrice) {
        if (tick.buyOrderNo == 0) {
            upRemainQty += tick.quantity;
        } else {
            upRemainQty -= tick.quantity;
        }
    }
#endif

#elif NE && SH
    bool limitUp = tick.tickMergeSse.tickType == 'A'
        && tick.tickMergeSse.tickBSFlag == '0'
        && tick.tickMergeSse.price == upperLimitPrice1000
        && tick.tickMergeSse.tickTime >= 9'30'00'00
        && tick.tickMergeSse.tickTime < 14'57'00'00;
    if (limitUp) {
        int32_t timestamp = tick.tickMergeSse.tickTime * 10;
        auto intent = wantCache->checkWantBuyAtTimestamp(timestamp);
        if (intent == WantCache::WantBuy || ALWAYS_BUY) [[likely]] {
            sendBuyRequest(id);
        }

        logStop(id, timestamp + static_cast<int32_t>(intent));
        return;
    }

#elif NE && SZ
    if (tick.messageType == NescForesight::MSG_TYPE_TRADE_SZ) {
        if (tick.tradeSz.tradePrice == state.upperLimitPrice10000) {
            state.upRemainQty100 -= tick.tradeSz.tradeQty;
            if (state.upRemainQty100 < 0 && tick.tradeSz.execType == 0x2) {
                int32_t timestamp = static_cast<uint32_t>(
                    tick.tradeSz.transactTime - g_szOffsetTransactTime);
                if (timestamp >= 9'30'00'000 && timestamp < 14'57'00'000) [[likely]] {
                    auto intent = checkWantBuyAtTimestamp(id, timestamp);
                    if (intent == WantCache::WantBuy || ALWAYS_BUY) [[likely]] {
                        sendBuyRequest(id);
                    }


                    logStop(id, timestamp + static_cast<int32_t>(intent));
                    return;
                }
            }
        }
    } else {
        // assert(tick.messageType == NescForesight::MSG_TYPE_ORDER_SZ);
        if (tick.orderSz.side == '2'
            && tick.orderSz.orderType == '2'
            && tick.orderSz.price == state.upperLimitPrice10000) {
            state.upRemainQty100 += tick.orderSz.qty;
        }
    }

#elif OST && SH
    bool limitUp = tick.tick.m_tick_type == 'A'
        && tick.tick.m_side_flag == '0'
        && tick.tick.m_order_price == upperLimitPrice1000
        && tick.tick.m_tick_time >= 9'30'00'00
        && tick.tick.m_tick_time < 14'57'00'00;
    if (limitUp) {
        int32_t timestamp = tick.tick.m_tick_time * 10;
        auto intent = wantCache->checkWantBuyAtTimestamp(timestamp);
        if (intent == WantCache::WantBuy || ALWAYS_BUY) [[likely]] {
            sendBuyRequest(id);
        }

        logStop(id, timestamp + static_cast<int32_t>(intent));
        return;
    }

#elif OST && SZ
    if (tick.head.m_message_type == sze_msg_type_trade) {
        if (tick.exe.m_exe_px == upperLimitPrice10000) {
            upRemainQty100 -= tick.exe.m_exe_qty;
            if (upRemainQty100 < 0 && tick.exe.m_exe_type == 0x2) {
                int32_t timestamp = static_cast<uint32_t>(
                    tick.head.m_quote_update_time - g_szOffsetTransactTime);
                if (timestamp >= 9'30'00'000 && timestamp < 14'57'00'000) [[likely]] {
                    auto intent = wantCache->checkWantBuyAtTimestamp(timestamp);
                    if (intent == WantCache::WantBuy || ALWAYS_BUY) [[likely]] {
                        sendBuyRequest(id);
                    }

                    logStop(id, timestamp + static_cast<int32_t>(intent));
                    return;
                }
            }
        }
    } else if (tick.head.m_message_type == sze_msg_type_order) [[likely]] {
        if (tick.order.m_side == '2'
            && tick.order.m_order_type == '2'
            && tick.order.m_px == upperLimitPrice10000) {
            upRemainQty100 += tick.order.m_qty;
        }
    }
#endif

    pushTickToRing(id, tick);
}

void MDD::handleSnap(MDS::Snap &snap)
{
}

void MDD::handleStatic(MDS::Stat &stat)
{
    int32_t stock = statStockCode(stat);
    int32_t id = linearizeStockId(stock);
    if (id == -1) [[unlikely]] {
        return;
    }
    auto &state = g_stockStates[id];

    int32_t upperLimitPrice = statUpperLimitPrice(stat);
    int32_t preClosePrice = statPreClosePrice(stat);

#if (NE || OST) && SH
    state.upperLimitPrice1000 = upperLimitPrice * 10;
#endif
#if (NE || OST) && SZ
    state.upperLimitPrice10000 = upperLimitPrice * 100;
#endif
#if REPLAY && SH
    state.upperLimitPrice = upperLimitPrice;
#endif
#if REPLAY && SZ
    state.upperLimitPrice = upperLimitPrice;
#endif
}

namespace
{

HEAT_ZONE_TIMER void computeThreadMain(int32_t channel, int32_t startId, int32_t stopId, std::stop_token stop);
// {
//     thread_local MDS::Tick tickBuf[1024];
//     std::vector<int32_t> approachStockIds;
//     std::vector<int64_t> remainSellAmounts;
//     approachStockIds.reserve(8);
//     remainSellAmounts.reserve(8);
//
//     int64_t sleepInterval = 95'000'000;
// #if REPLAY
//     sleepInterval = static_cast<int64_t>(sleepInterval * MDS::g_timeScale);
// #endif
//     int64_t nextSleepTime = steadyNow() + 200'000;
//     nextSleepTime += sleepInterval * channel / (kChannelCount + 1);
//
//     while (!stop.stop_requested()) [[likely]] {
//
//         approachStockIds.clear();
//         for (int32_t id = startId; id != stopId; ++id) {
//             g_stockComputes[id].clearApproach();
//         }
//
//         while (true) {
//             if (stop.stop_requested()) [[unlikely]] {
//                 return;
//             }
//
//             size_t n = g_tickRings[channel].fetchSomeTicks(tickBuf, sizeof tickBuf / sizeof tickBuf[0]);
//             for (size_t i = 0; i < n; ++i) {
//                 auto &tick = tickBuf[i];
//                 int32_t stock = tickStockCode(tick);
//                 int32_t id = linearizeStockId(stock);
//                 if (id == -1) [[unlikely]] {
//                     continue;
//                 }
//                 g_stockComputes[id].onTick(tick);
//             }
//             int64_t now = steadyNow();
//             if (now > nextSleepTime - 5'000) {
//                 break;
//             }
//             if (n == 0) {
//                 _mm_pause();
//                 if (now > nextSleepTime - 5'000) {
//                     break;
//                 }
//                 if (now > nextSleepTime - 8'000) {
//                     blockingSleepUntil(now + 2'000);
//                 }
//             }
//         }
//         blockingSleepUntil(nextSleepTime);
//         nextSleepTime += sleepInterval;
//
//         for (int32_t id = startId; id != stopId; ++id) {
//             if (g_stockComputes[id].checkApproach()) {
//                 approachStockIds.push_back(id);
//             }
//         }
//         if (!approachStockIds.empty()) {
//             if (approachStockIds.size() > 3) [[unlikely]] {
//                 remainSellAmounts.clear();
//                 for (int32_t id: approachStockIds) {
//                     remainSellAmounts.push_back(g_stockComputes[id].upSellOrderAmount());
//                 }
//                 std::nth_element(approachStockIds.begin(), approachStockIds.begin() + 2, approachStockIds.end(), [&] (int32_t &id1, int32_t &id2) {
//                     return remainSellAmounts[&id1 - approachStockIds.data()] < remainSellAmounts[&id2 - approachStockIds.data()];
//                 });
//                 approachStockIds.resize(2);
//             }
//             for (int32_t id: approachStockIds) {
//                 g_stockComputes[id].onApproach();
//             }
//         }
//     }
// }

}

void MDD::start(const char *config)
{
    parseDailyConfig(config);
    SPDLOG_INFO("found {} stocks", g_stockCodes.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    SPDLOG_INFO("initializing trade api");
    OES::start(config);

    std::this_thread::sleep_for(std::chrono::milliseconds(80));

    SPDLOG_INFO("initializing quote api");
    MDS::start(config);

    while (!OES::isStarted()) {
        SPDLOG_DEBUG("wait for trade initial");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SPDLOG_INFO("subscribing {} stocks", g_stockCodes.size());
    MDS::subscribe(g_stockCodes.data(), g_stockCodes.size());

    SPDLOG_INFO("start receiving stock quotes");
    MDS::startReceive();
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SPDLOG_INFO("mds receive started");

    SPDLOG_DEBUG("starting {} compute channels", kChannelCount);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        int32_t startId = (g_stockCodes.size() * c) / kChannelCount;
        int32_t stopId = (g_stockCodes.size() * (c + 1)) / kChannelCount;

        g_computeThreads[c] = std::jthread([c, startId, stopId] (std::stop_token stop) {
            setThisThreadAffinity(kChannelCpus[c]);
            computeThreadMain(c, startId, stopId, stop);
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    SPDLOG_INFO("system fully started");
}

void MDD::stop()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SPDLOG_DEBUG("stopping mds");
    MDS::stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    SPDLOG_DEBUG("stopping oes");
    OES::stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(60));

    SPDLOG_DEBUG("stopping {} compute channels", kChannelCount);
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_computeThreads[c].request_stop();
        g_computeThreads[c].join();
    }

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

HEAT_ZONE_RSPORDER void MDD::handleRspOrder(OES::RspOrder &rspOrder)
{
#if REPLAY
    int32_t stock = rspOrder.stockCode;
#elif XC || NE
    int32_t stock = rspOrder.userLocalID;
#elif OST
    int32_t stock = rspOrder.userLocalID;
#endif
    int32_t id = linearizeStockId(stock);
    if (id == -1) [[unlikely]] {
        return;
    }
}
