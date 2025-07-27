#include "config.h"
#if REPLAY
#include "MDS.h"
#include "MDD.h"
#include "L2/timestamp.h"
#include "threadAffinity.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <execution>
#include <fstream>
#include <unordered_set>
#include <thread>
#include <nlohmann/json.hpp>

namespace
{

std::jthread g_replayThread;
std::unordered_set<int32_t> g_subscribedStocks;
std::atomic_bool g_isFinished{false};
std::atomic_bool g_isStarted{false};
int32_t g_date;

}

void MDS::subscribe(int32_t const *stocks, int32_t n)
{
    g_subscribedStocks.insert(stocks, stocks + n);
}

MDS::Stat MDS::getStatic(int32_t stock)
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
        if (stat.stock != stock) {
            continue;
        }
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
        break;
    }

    return stat;
}

void MDS::start(const char *config)
{
    {
        nlohmann::json json;
        try {
            std::ifstream(config) >> json;
        } catch (std::exception const &e) {
            SPDLOG_ERROR("config json parse failed: {}", e.what());
            throw;
        }

        g_date = json["date"];
        if (g_date <= 0) {
            SPDLOG_ERROR("invalid config format for mds replay: {}", json.dump());
            throw std::runtime_error("invalid config format for mds replay");
        }
    }

    g_replayThread = std::jthread([] (std::stop_token stop) {
        std::vector<Tick> tickBuf;

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
            tickBuf.resize(pos / sizeof(Tick));
            SPDLOG_INFO("reading {} ticks from file", tickBuf.size());

            size_t n = std::fread(tickBuf.data(), sizeof(Tick), tickBuf.size(), fp);
            if (n != tickBuf.size()) [[unlikely]] {
                throw std::runtime_error("cannot read all ticks from file");
            }
            std::fclose(fp);
            fp = nullptr;

#if REPLAY_REAL_TIME
            SPDLOG_INFO("sorting {} ticks", tickBuf.size());
            std::stable_sort(std::execution::par_unseq, tickBuf.begin(), tickBuf.end(), [] (Tick const &lhs, Tick const &rhs) {
                return lhs.timestamp < rhs.timestamp;
            });
#endif

            setThisThreadAffinity(kMDSBindCpu);
            SPDLOG_INFO("start publishing {} ticks", tickBuf.size());
            g_isStarted.store(true);
        }

        size_t i = 0;
#if REPLAY_REAL_TIME
        int32_t lastTimestamp = tickBuf.empty() ? 0 : tickBuf.front().timestamp;
        auto nextSleepTime = std::chrono::steady_clock::now();
#endif
        while (i < tickBuf.size() && !stop.stop_requested()) [[likely]] {
            Tick &tick = tickBuf[i];

#if REPLAY_REAL_TIME
            if (tick.timestamp > lastTimestamp) {
                int64_t dt = L2::timestampToAbsoluteMilliseconds(tick.timestamp, 10) - L2::timestampToAbsoluteMilliseconds(lastTimestamp, 10);
                lastTimestamp = tick.timestamp;
                nextSleepTime += duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(dt) TIME_SCALE);
                spinSleepUntil(nextSleepTime);
            } else if (tick.timestamp < lastTimestamp) [[unlikely]] {
                throw std::runtime_error("timestamp out of order");
            }
#endif

            if (g_subscribedStocks.contains(tick.stock)) {
                MDD::handleTick(tick);
            }
            ++i;
        }

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
