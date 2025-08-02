#pragma once

#include <array>
#include <absl/container/btree_map.h>
#include <vector>
#include "config.h"
#include "MDS.h"
#include "OES.h"
#include "FactorList.h"


struct StockCompute;
struct WantCache;
struct TickRing;

struct alignas(64) StockState
{
    int32_t stockCode{};
    int32_t upperLimitPrice{};
    int32_t preClosePrice{};
    bool alive{};

#if NE && SH
    uint32_t upperLimitPrice1000{};
#endif
#if NE && SZ
    uint64_t upperLimitPrice10000{};
#endif

    WantCache *wantCache{};
    TickRing *tickRing{(TickRing *)-1};

    std::unique_ptr<OES::ReqOrder> reqOrder{};

    void start();
    void setChannelId(int32_t channelId);
    void setStatic(MDS::Stat const &stat);
    void stop(int32_t timestamp = 0);
    void onTick(MDS::Tick &tick);
    void onRspOrder(OES::RspOrder &rspOrder);

private:
    int32_t stockIndex() const;
    StockCompute &stockCompute() const;
};
