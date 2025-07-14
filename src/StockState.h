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

    inline static constexpr std::array kMomentumDurations = {
        // 0.1, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0, 30.0,
        6'0, 30'0, 60'0, 120'0, 180'0, 300'0, 600'0, 1800'0,
    };

    struct FState
    {
        int32_t timestamp100ms{};
        Snapshot prevSnapshot{};
        Snapshot currSnapshot{};

        struct Momentum
        {
            struct Incre
            {
                double valueSum;
                double valueSquaredSum;
            };

            struct Result
            {
                double highMean; // momentum_h_0.1_m
                double highStd; // momentum_h_0.1_sd
                double highZScore; // momentum_h_0.1_z
                double openMean; // momentum_o_0.1_m
                double openStd; // momentum_o_0.1_sd
                double openZScore; // momentum_o_0.1_z
                double diffMean; // momentum_o_h_diff_0.1
                double diffZScore; // momentum_o_h_z_diff_0.1
            };

            std::array<Incre, kMomentumDurations.size()> incre;
            std::vector<double> changeRates;
        };
        Momentum momentum;
    };

    struct BState
    {
        int32_t timestamp100ms{};
        Snapshot prevSnapshot{};
        Snapshot currSnapshot{};
        bool savingMode{};

        struct Momentum
        {
            std::array<FState::Momentum::Incre, kMomentumDurations.size()> incre;
            size_t oldNumChangeRates{};
        };
        Momentum momentum;
    };

    FState fState;
    BState bState;

    struct FactorList
    {
        struct Momentum
        {
            double highMean; // momentum_h_0.1_m
            double highStd; // momentum_h_0.1_sd
            double highZScore; // momentum_h_0.1_z
            double openMean; // momentum_o_0.1_m
            double openStd; // momentum_o_0.1_sd
            double openZScore; // momentum_o_0.1_z
            double diffMean; // momentum_o_h_diff_0.1
            double diffZScore; // momentum_o_h_z_diff_0.1
        };

        std::array<Momentum, kMomentumDurations.size()> momentum;
    };

    FactorList factorList;

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
    void stepSnapshotUntil(int32_t timestamp100ms);
    void saveSnapshot();
    void restoreSnapshot();
    void stepSnapshot();
    void decideWantBuy();
};
