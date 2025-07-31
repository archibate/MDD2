#pragma once

#include <absl/container/btree_map.h>
#include <vector>
#include "MDS.h"
#include "config.h"
#include "FactorList.h"
#include "IIRState.h"


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

    int64_t upSellOrderAmount() const;
    void dumpFactors(int32_t timestamp) const;

private:
    struct Snapshot
    {
        int32_t lastPrice;
        int32_t numTrades;
        int64_t volume;
        int64_t amount;
    };

    struct FState
    {
        int32_t nextTickTimestamp{};
        Snapshot currSnapshot{};
        std::vector<Snapshot> snapshots;
        std::unique_ptr<IIRState> iirState{};
    };

    struct BState
    {
        int32_t nextTickTimestamp{};
        Snapshot currSnapshot{};
        size_t oldSnapshotsCount{};
        std::unique_ptr<IIRState> iirState{};
        bool savingMode{};
    };

    int32_t upperLimitPrice{};
    int32_t preClosePrice{};
    int32_t upperLimitPriceApproach{};
    bool alive{true};
    bool approachingLimitUp{};
    bool firstCompute{};

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
    int32_t openPrice{};
    int64_t openVolume{};
    double floatMV{};

#if REPLAY
    std::unique_ptr<FactorList[]> factorListCache;
#endif

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

    void computeMomentum();
    void computeVolatility();
    void computeKaiyuan();
    void computeCrowdind();

    int32_t stockIndex() const;
    StockState &stockState() const;
};
