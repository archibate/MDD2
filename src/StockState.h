#pragma once

#include <array>
#include <map>
#include "MDS.h"


struct alignas(64) StockState
{
    int32_t stockCode;
    int32_t upperLimitPrice;

    void start(int32_t stockIndex);
    void stop();
    void handleTick(MDS::Tick &tick);

private:
    std::map<int32_t, int32_t> upSellOrders;

    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);

    void addTrade(int32_t timestamp, int32_t price, int32_t quantity);
};
