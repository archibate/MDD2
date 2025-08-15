#pragma once


#include "MDS.h"
#include "securityId.h"
#include <cmath>


inline int32_t tickStockCode(MDS::Tick &tick)
{
#if REPLAY
    int32_t stock = tick.stock;
#elif NE && SH
    int32_t stock = securityId(tick.tickMergeSse.securityID);
#elif NE && SZ
    int32_t stock = securityId(tick.securityID);
#elif OST && SH
    int32_t stock = securityId(tick.tick.m_symbol_id);
#elif OST && SZ
    int32_t stock = securityId(tick.head.m_symbol);
#endif
    return stock;
}


inline int32_t snapStockCode(MDS::Snap &snap)
{
#if REPLAY
    int32_t stock = snap.stock;
#elif NE
    int32_t stock = securityId(snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->securityID
                               : snap.snapshotSz->securityID);
#elif OST
    int32_t stock = securityId(snap.isSz
                               ? snap.szeLev2->m_header.m_symbol
                               : snap.sseLev2->m_symbol_id);
#endif
    return stock;
}

inline int32_t snapTimestamp(MDS::Snap &snap)
{
#if REPLAY
    int32_t timestamp = snap.timestamp;
#elif NE
    int32_t timestamp = (snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->timeStamp * UINT32_C(1000)
                               : snap.snapshotSz->timeStamp % UINT64_C(1'00'00'00'000));
#elif OST
#error not implemented
#endif
    return timestamp;
}

inline double snapOpenPrice(MDS::Snap &snap)
{
#if REPLAY
    double price = snap.lastPrice / 100.0;
#elif NE
    double price = snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->openPrice / 1000.0
                               : snap.snapshotSz->openPrice / 1000000.0;
#elif OST
#error not implemented
#endif
    return price;
}

inline double snapPreClosePrice(MDS::Snap &snap)
{
#if REPLAY
    double price = snap.preClosePrice / 100.0;
#elif NE
    double price = snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->preClosePrice / 1000.0
                               : snap.snapshotSz->preClosePrice / 10000.0;
#elif OST
#error not implemented
#endif
    return price;
}

inline double snapPrice(MDS::Snap &snap, int32_t n)
{
    if (n == 0) {
#if REPLAY
        return snap.lastPrice / 100.0;
#elif NE
        return snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->lastPrice / 1000.0
                               : snap.snapshotSz->lastPrice / 1000000.0;
#elif OST
#error not implemented
#endif
    } else if (n > 0) {
#if REPLAY
        return snap.bidPrice[n - 1] / 100.0;
#elif NE
        return snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->bidInfo[n - 1].price / 1000.0
                               : snap.snapshotSz->bidInfo[n - 1].price / 1000000.0;
#elif OST
#error not implemented
#endif
    } else {
#if REPLAY
        return snap.askPrice[-n - 1] / 100.0;
#elif NE
        return snap.marketType == NescForesight::SSE
                               ? snap.snapshotSse->askInfo[-n - 1].price / 1000.0
                               : snap.snapshotSz->askInfo[-n - 1].price / 1000000.0;
#elif OST
#error not implemented
#endif
    }
    return 0;
}

inline int32_t statStockCode(MDS::Stat &stat)
{
#if REPLAY
    int32_t stock = stat.stock;
#elif NE && SH
    int32_t stock = securityId(stat.staticSseInfo.securityID);
#elif NE && SZ
    int32_t stock = securityId(stat.staticSzInfo.securityID);
#elif OST
    int32_t stock = securityId(stat.depthMarketData.InstrumentID);
#endif
    return stock;
}

inline int32_t statUpperLimitPrice(MDS::Stat &stat)
{
#if REPLAY
    int32_t price = stat.upperLimitPrice;
#elif NE
    int32_t price = 0;
    switch (stat.marketType) {
        case NescForesight::SSE: {
            price = static_cast<int32_t>(std::round(stat.staticSseInfo.upperLimitPrice * 100));
        } break;
        case NescForesight::SZE: {
            price = static_cast<int32_t>(std::round(stat.staticSzInfo.upperLimitPrice * 100));
        } break;
    }
#elif OST
    int32_t price = static_cast<int32_t>(std::round(stat.depthMarketData.UpperLimitPrice * 100));
#endif
    return price;
}

inline int32_t statPreClosePrice(MDS::Stat &stat)
{
#if REPLAY
    int32_t price = stat.preClosePrice;
#elif NE
    int32_t price = 0;
    switch (stat.marketType) {
        case NescForesight::SSE: {
            price = static_cast<int32_t>(std::round(stat.staticSseInfo.prevClosePx * 100));
        } break;
        case NescForesight::SZE: {
            price = static_cast<int32_t>(std::round(stat.staticSzInfo.prevClosePx * 100));
        } break;
    }
#elif OST
    int32_t price = static_cast<int32_t>(std::round(stat.depthMarketData.PreClosePrice * 100));
#endif
    return price;
}
