#pragma once

#include <array>
#include "MDS.h"


struct alignas(64) StockState
{
    int32_t stockCode;
    int32_t upperLimitPrice;

    void start(int32_t stockIndex);
    void stop();
    void handleTick(MDS::Tick &tick);

private:
    void onOrder(MDS::Tick &tick);
    void onCancel(MDS::Tick &tick);
    void onTrade(MDS::Tick &tick);
};
