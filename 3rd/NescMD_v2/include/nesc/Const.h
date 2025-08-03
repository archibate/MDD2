#ifndef __Const_H__
#define __Const_H__

#include "SseMdStruct.h"
#include "SzeMdStruct.h"
#include "CSIMd.h"

namespace NescForesight {

    const int EXCHANGE_SSE_BEGIN  = 1000; 
    const int EXCHANGE_SZE_BEGIN  = 2000; 
    const int EXCHANGE_CSI_BEGIN  = 20000;

    enum EMdMsgType
    {
        eShSnapShot            = MSG_TYPE_SNAPSHOT_SSE         + EXCHANGE_SSE_BEGIN,    // 上交L2快照
        eShBestOrders          = MSG_TYPE_BEST_ORDERS_SSE      + EXCHANGE_SSE_BEGIN,    // 上交L2订单明细，最多揭示50笔
        eShIndex               = MSG_TYPE_INDEX_SSE            + EXCHANGE_SSE_BEGIN,    // 上交L2指数行情
        eShAfterSnap           = MSG_TYPE_AFTER_SNAP_SSE       + EXCHANGE_SSE_BEGIN,    // 上交L2盘后快照
        eShTickMerge           = MSG_TYPE_TICK_MERGE_SSE       + EXCHANGE_SSE_BEGIN,    // 上交L2逐笔合并
        eShAfterTrade          = MSG_TYPE_AFTER_TRADE_SSE      + EXCHANGE_SSE_BEGIN,    // 上交L2盘后逐笔成交
        eShBondSnapShot        = MSG_TYPE_BOND_SNAPSHOT_SSE    + EXCHANGE_SSE_BEGIN,    // 上交L2债券快照
        eShBondTick            = MSG_TYPE_BOND_TICK_SSE        + EXCHANGE_SSE_BEGIN,    // 上交L2债券逐笔
        eShStatic              = MSG_TYPE_STATIC_SSE           + EXCHANGE_SSE_BEGIN,    // 上交静态行情
        eShOrderBookSnap       = MSG_TYPE_ORDER_BOOK_SNAP_SSE  + EXCHANGE_SSE_BEGIN,    // 上交订单簿行情
        eShSnapShotLevel1      = MSG_TYPE_SNAPSHOT_L1_SSE      + EXCHANGE_SSE_BEGIN,    // 上交L1快照
        eShIndexLevel1         = MSG_TYPE_INDEX_L1_SSE         + EXCHANGE_SSE_BEGIN,    // 上交L1指数行情
        eShAfterSnapLevel1     = MSG_TYPE_AFTER_SNAP_L1_SSE    + EXCHANGE_SSE_BEGIN,    // 上交L1盘后快照
        eShBondSnapShotLevel1  = MSG_TYPE_BOND_SNAPSHOT_L1_SSE + EXCHANGE_SSE_BEGIN,    // 上交L1债券快照
        eShOptionLevel1        = MSG_TYPE_OPTION_L1_SSE        + EXCHANGE_SSE_BEGIN,    // 上交L1期权行情

        eSzSnapShot            = MSG_TYPE_SNAPSHOT_SZ          + EXCHANGE_SZE_BEGIN,    // 深交L2快照
        eSzBestOrders          = MSG_TYPE_BEST_ORDERS_SZ       + EXCHANGE_SZE_BEGIN,    // 深交L2订单明细，最多揭示50笔
        eSzIndex               = MSG_TYPE_INDEX_SZ             + EXCHANGE_SZE_BEGIN,    // 深交L2指数行情
        eSzTrade               = MSG_TYPE_TRADE_SZ             + EXCHANGE_SZE_BEGIN,    // 深交L2逐笔成交
        eSzOrder               = MSG_TYPE_ORDER_SZ             + EXCHANGE_SZE_BEGIN,    // 深交L2逐笔委托
        eSzAfterSnap           = MSG_TYPE_AFTER_SNAP_SZ        + EXCHANGE_SZE_BEGIN,    // 深交L2盘后快照
        eSzBlockTrade          = MSG_TYPE_BLOCKTRADE_SZ        + EXCHANGE_SZE_BEGIN,    // 深交L2盘后定价大宗交易
        eSzBondTrade           = MSG_TYPE_BOND_TRADE_SZ        + EXCHANGE_SZE_BEGIN,    // 深交L2债券逐笔成交
        eSzBondBlockTrade      = MSG_TYPE_BOND_BLOCKTRADE_SZ   + EXCHANGE_SZE_BEGIN,    // 深交L2债券大额逐笔成交
        eSzBondOrder           = MSG_TYPE_BOND_ORDER_SZ        + EXCHANGE_SZE_BEGIN,    // 深交L2债券逐笔委托
        eSzBondBlockOrder      = MSG_TYPE_BOND_BLOCKORDER_SZ   + EXCHANGE_SZE_BEGIN,    // 深交L2债券大额逐笔委托
        eSzBondSnapShot        = MSG_TYPE_BOND_SNAPSHOT_SZ     + EXCHANGE_SZE_BEGIN,    // 深交L2债券快照
        eSzBondBestOrders      = MSG_TYPE_BOND_BEST_ORDERS_SZ  + EXCHANGE_SZE_BEGIN,    // 深交L2债券订单明细，最多揭示50笔
        eSzBiddingTrade        = MSG_TYPE_BOND_BID_TRADE_SZ    + EXCHANGE_SZE_BEGIN,    // 深交L2竞价成交
        eSzBiddingOrder        = MSG_TYPE_BOND_BID_ORDER_SZ    + EXCHANGE_SZE_BEGIN,    // 深交L2竞价委托
        eSzOrderBookSnap       = MSG_TYPE_ORDER_BOOK_SNAP_SZ   + EXCHANGE_SZE_BEGIN,    // 深交订单簿行情
        eSzSnapShotLevel1      = MSG_TYPE_SNAPSHOT_L1_SZ       + EXCHANGE_SZE_BEGIN,    // L1快照
        eSzIndexLevel1         = MSG_TYPE_INDEX_L1_SZ          + EXCHANGE_SZE_BEGIN,    // L1指数行情
        eSzAfterSnapLevel1     = MSG_TYPE_AFTER_SNAP_L1_SZ     + EXCHANGE_SZE_BEGIN,    // L1深交盘后快照
        eSzBlockTradeLevel1    = MSG_TYPE_BLOCKTRADE_L1_SZ     + EXCHANGE_SZE_BEGIN,    // L1深交盘后定价大宗交易
        eSzBondSnapShotLevel1  = MSG_TYPE_BOND_SNAPSHOT_L1_SZ  + EXCHANGE_SZE_BEGIN,    // L1债券快照
        eSzOptionLevel1        = MSG_TYPE_OPTION_L1_SZ         + EXCHANGE_SZE_BEGIN,    // L1深交期权行情

        eCsiMarketData         = MSG_TYPE_MARKETDATA_CSI       + EXCHANGE_CSI_BEGIN,    // 中证指数行情信息
        eCsiWeightInfo         = MSG_TYPE_WEIGHT_CSI           + EXCHANGE_CSI_BEGIN,    // 中证指数权重信息
        eCsiEtfIopv            = MSG_TYPE_ETF_IOPV_CSI         + EXCHANGE_CSI_BEGIN     // ETF 参考净值（IOPV）信息定义
    };

    enum MarketType
    {
        SSE = 1,               //上交所
        SZE,                   //深交所
        CSI = 20               //中证指数
    };
}
#endif