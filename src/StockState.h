#pragma once

#include <array>
#include <absl/container/btree_map.h>
#include <vector>
#include "MDS.h"
#include "FactorList.h"


struct alignas(64) StockState
{
    int32_t stockCode{};
    int32_t upperLimitPrice{};
    int32_t upperLimitPriceApproach{};
    int32_t preClosePrice{};
    int32_t openPrice{};
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
        int32_t nextTickTimestamp{};
        Snapshot currSnapshot{};
        std::vector<Snapshot> snapshots;
    };

    struct BState
    {
        int32_t nextTickTimestamp{};
        Snapshot currSnapshot{};
        size_t oldSnapshotsCount{};
        bool savingMode{};
    };

    FState fState;
    BState bState;
    FactorList factorList{};

    struct UpSell {
        int32_t price;
        int32_t quantity;
    };

    // struct PendTrade {
    //     int32_t timestamp;
    //     int32_t sellOrderNo;
    //     int32_t price;
    //     int32_t quantity;
    // };

    absl::btree_map<int32_t, UpSell> upSellOrders;
    int32_t timestampLastTrade{};
    bool wantBuy{};
    bool approchingLimitUp{};

    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);

    void updateVirtTrade(int32_t timestamp);
    void addRealTrade(int32_t timestamp, int32_t price, int32_t quantity);

    void stepSnapshotUntil(int32_t timestamp);
    void saveSnapshot();
    void restoreSnapshot();
    void stepSnapshot();
    void decideWantBuy();
};
