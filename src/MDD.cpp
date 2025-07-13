#include "MDD.h"
#include "MDS.h"
#include "StockState.h"
#include "daily.h"
#include "stockCodes.h"
#include <spdlog/spdlog.h>
#include <thread>
#include <sched.h>
#include <unistd.h>
#include <emmintrin.h>

std::array<DisruptorPool<MDS::Tick>, kChannelCount> MDD::g_channelPool;
std::array<StockState, kStockCodes.size()> MDD::g_stockStates;

namespace
{

#if SH
std::array<int32_t, kStockCodes.size()> g_lastTimestampPerStock;
std::array<std::atomic<std::chrono::steady_clock::duration::rep>, kStockCodes.size()> g_brustEndTime;
#endif
std::jthread g_timerThread;

void timerThread(std::stop_token stop)
{
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(kTimerCpu, &cpu);
    sched_setaffinity(gettid(), sizeof(cpu), &cpu);

    while (!stop.stop_requested()) [[likely]] {
        auto nowTime = std::chrono::steady_clock::now().time_since_epoch().count();
#if SH
        for (int32_t id = 0; id < kStockCodes.size(); ++id) {
            if (nowTime > g_brustEndTime[id].load(std::memory_order_relaxed)) {
                g_brustEndTime[id].fetch_add(duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::milliseconds(10) TIME_SCALE).count(), std::memory_order_relaxed);
                MDS::Tick tick{};
                tick.stock = kStockCodes[id];
                tick.timestamp = -1;
                MDD::g_channelPool[kStockChannels[id]].push(tick);
            }
        }
#endif
        _mm_pause();
    }
}

void handleTick(int32_t ch, MDS::Tick &tick)
{
    int32_t id = kStockIdLut[static_cast<int16_t>(tick.stock & 0x7FFF)];
    if (id == -1) [[unlikely]] {
        return;
    }

#if SH
    if (tick.timestamp != g_lastTimestampPerStock[id]) {
        auto t = std::chrono::steady_clock::now().time_since_epoch().count();
        t += duration_cast<std::chrono::steady_clock::duration>(std::chrono::microseconds(kBrustProcessMicroseconds) TIME_SCALE).count();
        g_brustEndTime[id].store(t, std::memory_order_relaxed);
        g_lastTimestampPerStock[id] = tick.timestamp;
    }
#endif

    MDD::g_stockStates[id].handleTick(tick);
}

}

void MDD::start()
{
    for (int32_t c = 0; c < kChannelCount; ++c) {
        g_channelPool[c].init(kChannelPoolSize, true, kChannelCpuBegin + c);
        g_channelPool[c].start([c] (MDS::Tick &tick) {
            handleTick(c, tick);
        });
    }

    for (int32_t i = 0; i < g_stockStates.size(); ++i) {
        g_stockStates[i].start(i);
    }

    MDS::subscribe(kStockCodes.data(), kStockCodes.size());
    MDS::start();
    while (!MDS::isStarted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    g_timerThread = std::jthread(timerThread);
}

void MDD::stop()
{
    g_timerThread.request_stop();
    g_timerThread.join();

    MDS::stop();

    for (int32_t i = 0; i < g_stockStates.size(); ++i) {
        g_stockStates[i].stop();
    }
}

bool MDD::isFinished()
{
    return MDS::isFinished();
}
