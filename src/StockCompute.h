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

    [[gnu::always_inline]] bool isApproachingLimitUp() const
    {
        return approachingLimitUp;
    }

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
    bool approachingLimitUp{};

    FState fState;
    BState bState;

public:
    int32_t futureTimestamp{};
    FactorList factorList{};
private:

    struct UpSell {
        int32_t price;
        int32_t quantity;
    };

    absl::btree_map<int32_t, UpSell> upSellOrders;


    void onTick(MDS::Tick &tick);
    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);

    void computeFutureWantBuy();
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
