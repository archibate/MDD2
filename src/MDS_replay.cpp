#include "config.h"
#if REPLAY
#include "MDS.h"
#include "MDD.h"
#include "timestamp.h"
#include "threadAffinity.h"
#include "clockMonotonic.h"
#include "radixSort.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fstream>
#include <thread>
#include <absl/container/flat_hash_set.h>
#include <nlohmann/json.hpp>

namespace
{

std::jthread g_replayThread;
absl::flat_hash_set<int32_t> g_subscribedStocks;
std::vector<MDS::Stat> g_marketStatics;
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

    MDS::Stat stat{};
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

void MDS::subscribe(int32_t const *stocks, int32_t n)
{
    g_subscribedStocks.insert(stocks, stocks + n);
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

void readReplayTicks(std::vector<MDS::Tick> &tickBuf)
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
    tickBuf.resize(pos / sizeof(MDS::Tick));
    SPDLOG_INFO("reading {} ticks from file", tickBuf.size());

    size_t n = std::fread(tickBuf.data(), sizeof(MDS::Tick), tickBuf.size(), fp);
    if (n != tickBuf.size()) [[unlikely]] {
        throw std::runtime_error("cannot read all ticks from file");
    }
    std::fclose(fp);
    fp = nullptr;

    SPDLOG_DEBUG("sorting {} ticks", tickBuf.size());
    radixSort<8, 4, sizeof(uint32_t), offsetof(MDS::Tick, timestamp), sizeof(MDS::Tick)>(tickBuf.data(), tickBuf.size());
}

void replayMain(std::vector<MDS::Tick> &tickBuf, std::stop_token stop)
{
    if (MDS::g_timeScale > 0) {
        // int64_t lastTimestamp = timestampAbsLinear(9'30'00'000);
        int64_t lastTimestamp = timestampAbsLinear(9'24'00'000);
        int64_t nextSleepTime = monotonicTime();
        int64_t timeScale = static_cast<int64_t>(1'000'000 * MDS::g_timeScale);
        monotonicSleepFor(100'000'000);

        for (size_t i = 0; i < tickBuf.size() && !stop.stop_requested(); ++i) [[likely]] {
            MDS::Tick &tick = tickBuf[i];

            if (tick.timestamp > lastTimestamp) {
                int64_t thisTimestamp = timestampAbsLinear(tick.timestamp);
                int64_t dt = thisTimestamp - lastTimestamp;
                if (dt >= 1'000'000) {
                    dt -= (dt - 1'000'000) / 10;
                }
                lastTimestamp = thisTimestamp;
                nextSleepTime += dt * timeScale;
                monotonicSleepUntil(nextSleepTime);
            }

            MDD::handleTick(tick);
        }

    } else {
        for (size_t i = 0; i < tickBuf.size() && !stop.stop_requested(); ++i) [[likely]] {
            MDS::Tick &tick = tickBuf[i];
            MDD::handleTick(tick);
        }
    }
}

}

void MDS::startReceive()
{
    SPDLOG_INFO("loading market statics");
    loadMarketStatic();

    SPDLOG_DEBUG("publishing {} statics", g_marketStatics.size());
    for (auto &stat: g_marketStatics) {
        MDD::handleStatic(stat);
    }

    g_replayThread = std::jthread([] (std::stop_token stop) {
        SPDLOG_INFO("start loading L2 ticks");
        std::vector<Tick> tickBuf;
        readReplayTicks(tickBuf);
        setThisThreadAffinity(kMDSBindCpu);
        SPDLOG_DEBUG("loaded {} ticks", tickBuf.size());
        g_isStarted.store(true);
        monotonicSleepFor(100'000'000);

        SPDLOG_DEBUG("publishing {} open snapshots", g_marketStatics.size());
        for (auto &stat: g_marketStatics) {
            MDS::Snap snap{};
            snap.stock = stat.stock;
            snap.timestamp = 9'25'00'000;
            snap.preClosePrice = stat.preClosePrice;
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
