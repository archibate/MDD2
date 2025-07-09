#include "MDS.h"
#include "MDD.h"
#include <stdexcept>
#include <unordered_set>
#include <thread>

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

void MDS::start()
{
    g_replayWorker = std::jthread([] (std::stop_token stop) {
        std::FILE *fp = std::fopen("/data/L2/SHL2/20250102/stock-l2-ticks.dat", "rb");
        if (!fp) {
            throw std::runtime_error("cannot open stock L2 ticks");
        }

        std::array<Tick, 1024> tickBuf;
        while (!stop.stop_requested()) [[likely]] {
            size_t n = std::fread(tickBuf.data(), sizeof(Tick), tickBuf.size(), fp);
            if (n <= 0) [[unlikely]] {
                break;
            }
            for (size_t i = 0; i < n; ++i) {
                Tick &tick = tickBuf[i];

                if (g_subscribedStocks.contains(tick.stock)) {
                    int32_t ch = tick.stock % MDD::g_channelPool.size();
                    MDD::g_channelPool[ch].push(tick);
                }
            }
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
