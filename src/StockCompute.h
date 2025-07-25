#pragma once

#include <absl/container/btree_map.h>
#include <vector>
#include "MDS.h"
#include "FactorList.h"


struct StockState;

struct alignas(64) StockCompute
{
    void start();
    void stop();
    void onTimer();
    void onPostTimer();
    void onBusy();

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

    int32_t upperLimitPriceApproach{};
    bool alive{true};
    bool approchingLimitUp{};

    FState fState;
    BState bState;

    int32_t futureTimestamp{};

    struct UpSell {
        int32_t price;
        int32_t quantity;
    };

    absl::btree_map<int32_t, UpSell> upSellOrders;

public:
    FactorList factorList{};

private:
    void onTick(MDS::Tick &tick);
    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);

    void computeFutureWantBuy(int32_t timestamp);
    void addRealTrade(int32_t timestamp, int32_t price, int32_t quantity);

    void stepSnapshotUntil(int32_t timestamp);
    void saveSnapshot();
    void restoreSnapshot();
    void stepSnapshot();
    bool decideWantBuy();
    bool computeModel();

    int32_t stockIndex() const;
    StockState &stockState() const;
};
