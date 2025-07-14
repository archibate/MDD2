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

private:
    struct Snapshot
    {
        int32_t lastPrice;
        int32_t numTrades;
        int64_t quantity;
        int64_t amount;
    };

    struct FState
    {
        int32_t snapshotTimestamp100ms{};
        Snapshot prevSnapshot{};
        Snapshot currSnapshot{};
        std::vector<double> priceSeq;
    };

    struct BState
    {
        int32_t snapshotTimestamp100ms{};
        Snapshot prevSnapshot{};
        Snapshot currSnapshot{};
        size_t priceSeqOldSize{};
        bool savingMode{};
    };

    FState fState;
    BState bState;

#if SH
    struct UpSell {
        int32_t price;
        int32_t quantity;
    };

    struct PendTrade {
        int32_t timestamp;
        int32_t sellOrderNo;
        int32_t price;
        int32_t quantity;
    };

    absl::btree_map<int32_t, UpSell> upSellOrders;
    std::vector<PendTrade> pendTrades;
    int32_t timestampVirtPred100ms{};
    int32_t timestampLastTick{};
    int64_t upSellChangeSinceVirtPred{};
    bool wantBuy{};
    bool approchingLimitUp{};
#endif

#if SZ
    struct UpSell {
        int32_t price;
        int32_t quantity;
    };

    struct PendTrade {
        int32_t timestamp;
        int32_t sellOrderNo;
        int32_t price;
        int32_t quantity;
    };

    absl::btree_map<int32_t, UpSell> upSellOrders;
    int32_t upSellVolume{};
    int32_t timestampVirtTradePred100ms{};
    int32_t timestampLastTick{};
    bool approchingLimitUp{};
    bool wantBuy{};
#endif

    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);
#if SH
    void onTimer();
#endif

#if SH
    void virtPred100ms(int32_t timestamp);
    void updateVirtTradePred(int32_t timestamp100ms);
#endif

    void addTrade(int32_t timestamp100ms, int32_t price, int32_t quantity);
    void saveSnapshot();
    void restoreSnapshot();
    void stepSnapshot();
    void decideWantBuy();
};
