#pragma once

#include <array>
#include <absl/container/btree_map.h>
#include <vector>
#include "MDS.h"
#include "FactorList.h"


struct StockCompute;

struct alignas(64) StockState
{
    int32_t stockCode{};
    int32_t upperLimitPrice{};
    int32_t preClosePrice{};
    bool alive{};

    void start();
    void stop(int32_t timestamp = 0);
    void onTick(MDS::Tick &tick);

private:
    int32_t stockIndex() const;
    StockCompute &stockCompute() const;
};
