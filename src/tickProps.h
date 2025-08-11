#pragma once


#include "MDS.h"
#include "securityId.h"


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
                               ? snap.snapshotSse.securityID
                               : snap.snapshotSz.securityID);
#elif OST
    int32_t stock = securityId(snap.isSz
                               ? snap.szeLev2.m_header.m_symbol
                               : snap.sseLev2.m_symbol_id);
#endif
    return stock;
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
