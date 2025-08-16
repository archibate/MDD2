#include "config.h"
#if REPLAY
#include "MDS.h"
#include "MDD.h"
#include "L2/Tick.h"
#include "L2/Stat.h"
#include "XeleCompat.h"
#include "dateTime.h"
#include "timestamp.h"
#include "securityId.h"
#include "threadAffinity.h"
#include "clockMonotonic.h"
#include "radixSort.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>

namespace
{

#if REPLAY && SZ
const uint64_t g_szOffsetTransactTime = static_cast<uint64_t>(getToday()) * UINT64_C(1'00'00'00'000);
#endif

std::jthread g_replayThread;
std::vector<L2::Stat> g_marketStatics;
std::atomic_bool g_isFinished{false};
std::atomic_bool g_isStarted{false};
int32_t g_date;

void loadMarketStatic()
{
    std::string line;
    std::ifstream csv((REPLAY_DATA_PATH "/L2/" MARKET_NAME "L2/" + std::to_string(g_date) + "/stock-metadata.csv").c_str());
    if (!csv.is_open()) {
        SPDLOG_ERROR("cannot open stock metadata for market={} date={}", MARKET_NAME, g_date);
        throw std::runtime_error("cannot open stock metadata");
    }

    std::getline(csv, line);

    L2::Stat stat{};
    while (std::getline(csv, line)) {
        std::istringstream iss(line);
        std::string token;
        std::getline(iss, token, ',');

        std::getline(iss, token, ',');
        stat.stock = std::stoi(token);
        std::getline(iss, token, ',');
        std::getline(iss, token, ',');
        stat.openPrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.preClosePrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.highPrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.lowPrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.closePrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.upperLimitPrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.lowerLimitPrice = std::round(100 * std::stod(token));
        std::getline(iss, token, ',');
        stat.floatMV = std::stod(token) * 10000.0;
        g_marketStatics.push_back(stat);
    }
}

}

namespace MDS
{
double g_timeScale = 1.0 / 10.0;
}

void MDS::start(const char *config)
{
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;

        g_date = json["date"];
        if (json.contains("time_speed")) {
            g_timeScale = 1.0 / double(json["time_speed"]);
        }

    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
        throw;
    }

    SPDLOG_INFO("mds replay date: {}", g_date);
    SPDLOG_INFO("mds replay time speed: {}", 1.0 / g_timeScale);
    if (g_date <= 20000000) {
        SPDLOG_ERROR("invalid date to replay: {}", g_date);
        throw std::runtime_error("invalid config date");
    }
}

namespace
{

void readReplayTicks(std::vector<L2::Tick> &tickBuf)
{
    std::FILE *fp = std::fopen((REPLAY_DATA_PATH "/L2/" MARKET_NAME "L2/" + std::to_string(g_date) + "/stock-l2-ticks.dat").c_str(), "rb");
    if (!fp) {
        SPDLOG_ERROR("cannot open L2 ticks for market={} date={}", MARKET_NAME, g_date);
        throw std::runtime_error("cannot open stock L2 ticks");
    }

    if (std::fseek(fp, 0, SEEK_END) < 0) {
        throw std::runtime_error("cannot seek ticks file");
    }
    long pos = std::ftell(fp);
    if (pos < 0) {
        throw std::runtime_error("cannot tell ticks file size");
    }
    std::rewind(fp);
    tickBuf.resize(pos / sizeof(L2::Tick));
    SPDLOG_DEBUG("reading {} ticks from file", tickBuf.size());

    size_t n = std::fread(tickBuf.data(), sizeof(L2::Tick), tickBuf.size(), fp);
    if (n != tickBuf.size()) [[unlikely]] {
        throw std::runtime_error("cannot read all ticks from file");
    }
    std::fclose(fp);
    fp = nullptr;

    SPDLOG_DEBUG("sorting {} ticks", tickBuf.size());
    radixSort<8, 4, sizeof(uint32_t), offsetof(L2::Tick, timestamp), sizeof(L2::Tick)>(tickBuf.data(), tickBuf.size());
}

void l2ToMdsTick(L2::Tick const &l2, MDS::Tick &mds)
{
#if SH
    mds.tickMergeSse.messageType = XeleCompat::MSG_TYPE_TICK_MERGE_SSE;
    mds.tickMergeSse.sequence = 0; /* unused */
    mds.tickMergeSse.exchangeID = 1;
    fmtSecurityId(mds.tickMergeSse.securityID, l2.stock);
    mds.tickMergeSse.tickType = l2.isTrade() ? 'T' : (l2.isOrderCancel() ? 'D' : 'A');
    mds.tickMergeSse.tickBSFlag = l2.isBuy() ? '0' : '1';
    mds.tickMergeSse.bizIndex = 0; /* unused */
    mds.tickMergeSse.channelNo = 0; /* unused */
    mds.tickMergeSse.tickTime = static_cast<uint32_t>(l2.timestamp) / 10;
    mds.tickMergeSse.buyOrderNo = static_cast<uint64_t>(l2.buyOrderNo);
    mds.tickMergeSse.sellOrderNo = static_cast<uint64_t>(l2.sellOrderNo);
    mds.tickMergeSse.price = static_cast<uint32_t>(l2.price) * 10;
    mds.tickMergeSse.qty = static_cast<uint64_t>(l2.quantity) * 1000;
#endif

#if SZ
    if (l2.isTrade()) {
        mds.tradeSz.messageType = XeleCompat::MSG_TYPE_TRADE_SZ;
        mds.tradeSz.sequence = 0; /* unused */
        mds.tradeSz.exchangeID = 2;
        fmtSecurityId(mds.tradeSz.securityID, l2.stock);
        mds.tradeSz.execType = l2.isOrderCancel() ? 0x1 : 0x2;
        mds.tradeSz.applSeqNum = 0; /* unused */
        mds.tradeSz.transactTime = g_szOffsetTransactTime + l2.timestamp;
        mds.tradeSz.tradePrice = static_cast<uint64_t>(l2.price) * 100;
        mds.tradeSz.tradeQty = static_cast<uint64_t>(l2.quantity) * 100;
        mds.tradeSz.bidapplSeqnum = static_cast<uint64_t>(l2.buyOrderNo);
        mds.tradeSz.offerapplSeqnum = static_cast<uint64_t>(l2.sellOrderNo);
    } else {
        mds.orderSz.messageType = XeleCompat::MSG_TYPE_ORDER_SZ;
        mds.orderSz.sequence = 0; /* unused */
        mds.orderSz.exchangeID = 2;
        fmtSecurityId(mds.orderSz.securityID, l2.stock);
        mds.orderSz.rsvd = 0;
        mds.orderSz.side = l2.isBuy() ? '1' : '2';
        mds.orderSz.orderType = '2';
        mds.orderSz.applSeqNum = static_cast<uint64_t>(l2.orderNo()); /* unused */
        mds.orderSz.transactTime = g_szOffsetTransactTime + l2.timestamp;
        mds.orderSz.price = static_cast<uint64_t>(l2.price) * 100;
        mds.orderSz.qty = static_cast<uint64_t>(l2.quantity) * 100;
        mds.orderSz.channelNo = 0; /* unused */
        mds.orderSz.mdstreamid = 0; /* unused */
    }
#endif
}

void replayMain(std::vector<L2::Tick> &tickBuf, std::stop_token stop)
{
    if (MDS::g_timeScale > 0) {
        // int64_t lastTimestamp = timestampAbsLinear(9'30'00'000);
        int64_t lastTimestamp = timestampAbsLinear(9'24'00'000);
        int64_t nextSleepTime = monotonicTime();
        int64_t timeScale = static_cast<int64_t>(1'000'000 * MDS::g_timeScale);
        monotonicSleepFor(100'000'000);

        for (size_t i = 0; i < tickBuf.size() && !stop.stop_requested(); ++i) [[likely]] {
            L2::Tick &l2tick = tickBuf[i];

            if (l2tick.timestamp > lastTimestamp) {
                int64_t thisTimestamp = timestampAbsLinear(l2tick.timestamp);
                int64_t dt = thisTimestamp - lastTimestamp;
                if (dt >= 1'000'000) {
                    dt -= (dt - 1'000'000) / 10;
                }
                lastTimestamp = thisTimestamp;
                nextSleepTime += dt * timeScale;
                monotonicSleepUntil(nextSleepTime);
            }

            MDS::Tick mdsTick;
            l2ToMdsTick(l2tick, mdsTick);
            MDD::handleTick(mdsTick);
        }

    } else {
        for (size_t i = 0; i < tickBuf.size() && !stop.stop_requested(); ++i) [[likely]] {
            L2::Tick &l2tick = tickBuf[i];
            MDS::Tick mdsTick;
            l2ToMdsTick(l2tick, mdsTick);
            MDD::handleTick(mdsTick);
        }
    }
}

}

void MDS::startReceive()
{
    SPDLOG_DEBUG("loading market statics");
    loadMarketStatic();

    SPDLOG_DEBUG("publishing {} statics", g_marketStatics.size());
    for (auto &l2stat: g_marketStatics) {
        MDS::Stat mdsStat{};
        mdsStat.stock = l2stat.stock;
        mdsStat.preClosePrice = l2stat.preClosePrice;
        mdsStat.upperLimitPrice = l2stat.upperLimitPrice;
        mdsStat.lowerLimitPrice = l2stat.lowerLimitPrice;
        MDD::handleStatic(mdsStat);
    }

    g_replayThread = std::jthread([] (std::stop_token stop) {
        SPDLOG_INFO("start loading L2 ticks");
        std::vector<L2::Tick> tickBuf;
        readReplayTicks(tickBuf);
        setThisThreadAffinity(kMDSBindCpu);
        SPDLOG_DEBUG("loaded {} ticks", tickBuf.size());
        g_isStarted.store(true);
        monotonicSleepFor(10'000'000);

        SPDLOG_DEBUG("publishing {} open snapshots", g_marketStatics.size());
        for (auto &stat: g_marketStatics) {
            MDS::Snap snap{};
            snap.stock = stat.stock;
            snap.timestamp = 9'25'00'000;
            snap.preClosePrice = stat.preClosePrice;
            snap.openPrice = stat.openPrice;
            snap.lastPrice = stat.openPrice;
            MDD::handleSnap(snap);
        }

        SPDLOG_INFO("start replaying");
        replayMain(tickBuf, stop);
        SPDLOG_DEBUG("replay finished");
        g_isFinished.store(true);
    });
}

void MDS::stop()
{
    g_replayThread.request_stop();
    g_replayThread.join();
}

void MDS::requestStop()
{
    g_replayThread.request_stop();
}

bool MDS::isFinished()
{
    return g_isFinished.load() == true;
}

bool MDS::isStarted()
{
    return g_isStarted.load() == true;
}
#endif
