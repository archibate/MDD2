#pragma once

#include <array>
#include <absl/container/btree_map.h>
#include <vector>
#include "config.h"
#include "exchangeFronts.h"
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

#if (NE || OST) && SH
    uint32_t upperLimitPrice1000{};
#endif
#if (NE || OST) && SZ
    uint64_t upperLimitPrice10000{};
    uint64_t offsetTransactTime{};
    int64_t upRemainQty100{};
#endif
#if REPLAY && SZ
    uint64_t upRemainQty{};
#endif

    WantCache *wantCache{};
    TickRing *tickRing{};

#if SPLIT_ORDER
    std::unique_ptr<OES::ReqOrderBatch> reqOrderBatch{};
    std::unique_ptr<std::array<OES::ReqCancel, kExchangeFronts.size()>> reqCancels{};
#else
    std::unique_ptr<OES::ReqOrder> reqOrder{};
    std::unique_ptr<OES::ReqCancel> reqCancel{};
#endif

    void start();
    void setChannelId(int32_t channelId);
    void onStatic(MDS::Stat const &stat);
    void stop(int32_t timestamp = 0);
    void onTick(MDS::Tick &tick);
    void onSnap(MDS::Snap &snap);
    void onRspOrder(OES::RspOrder &rspOrder);

private:
    void sendBuyRequest();
    int32_t stockIndex() const;
    StockCompute &stockCompute() const;
};
