#pragma once

#include <array>
#include <absl/container/btree_map.h>
#include <vector>
#include "MDS.h"


struct alignas(64) StockState
{
    int32_t stockCode{};
    int32_t upperLimitPrice{};
    int32_t upperLimitPriceApproach{};
    int32_t preClosePrice{};
    bool alive{};

    void start(int32_t stockIndex);
    void stop();
    void handleTick(MDS::Tick &tick);
    void handleTimer();

private:
#if SH
    struct UpTrade {
        int32_t timestamp;
        int32_t sellOrderNo;
        int32_t price;
        int32_t quantity;
    };

    absl::btree_map<int32_t, int32_t> upSellOrders;
    std::vector<UpTrade> upTrades;
    int32_t lastActiveBuyPrice{};
    int32_t timestampVirtTradePred{};
    int32_t timestampLastTick{};
    bool approchingLimitUp{};
    bool wantBuy{};
#endif

    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);
#if SH
    void on10ms(int32_t timestamp);
#endif

    void addTrade(int32_t timestamp, int32_t price, int32_t quantity);
#if SH
    void updateVirtTradePred(int32_t timestamp);
#endif
};
