#pragma once

#include <array>
#include <absl/container/flat_hash_map.h>
#include <vector>
#include "MDS.h"


struct alignas(64) StockState
{
    int32_t stockCode{};
    int32_t upperLimitPrice{};
    int32_t preClosePrice{};
    bool alive{};

    void start(int32_t stockIndex);
    void stop();
    void handleTick(MDS::Tick &tick);
    void handle10ms();

private:
#if SH
    absl::flat_hash_map<int32_t, int32_t> upSellOrders;
    std::vector<std::pair<int32_t, int32_t>> upTrades;
    int32_t timestampLast10ms{};
    int32_t timestampLastTick{};
#endif

    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);
#if SH
    void on10ms(int32_t timestamp);
#endif

    void addTrade(int32_t timestamp, int32_t price, int32_t quantity);
#if SH
    void updateVirtUpTrade(int32_t timestamp);
#endif
};
