#include "MDS.h"
#include "MDD.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fstream>
#include <unordered_set>
#include <thread>

// #define DATA_BASE "/data"
#define DATA_BASE "/home/ubuntu/data-cache"
#define DATA_PATH DATA_BASE "/L2/SHL2/20250102"

namespace
{

std::jthread g_replayWorker;
std::unordered_set<int32_t> g_subscribedStocks;
std::atomic_bool g_isFinished{false};

}


void MDS::subscribe(int32_t const *stocks, int32_t n)
{
    g_subscribedStocks.insert(stocks, stocks + n);
}

MDS::Stat MDS::getStatic(int32_t stock)
{
    std::string line;
    std::ifstream csv(DATA_PATH "/stock-metadata.csv");
    if (!csv.is_open()) {
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

void MDS::start()
{
    g_replayWorker = std::jthread([] (std::stop_token stop) {
        std::FILE *fp = std::fopen(DATA_PATH "/stock-l2-ticks.dat", "rb");
        if (!fp) {
            throw std::runtime_error("cannot open stock L2 ticks");
        }

        std::vector<Tick> tickBuf;
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
        SPDLOG_INFO("done reading ticks");

        size_t i = 0;
        while (!stop.stop_requested()) [[likely]] {
            Tick &tick = tickBuf[i];

            if (g_subscribedStocks.contains(tick.stock)) {
                int32_t ch = tick.stock % MDD::g_channelPool.size();
                MDD::g_channelPool[ch].push(tick);
            }
            ++i;
        }

        std::fclose(fp);
        g_isFinished.store(true);
    });
}

void MDS::stop()
{
    g_replayWorker.join();
}

bool MDS::isFinished()
{
    return g_isFinished.load() == true;
}
