#ifndef __Const_H__
#define __Const_H__

namespace NescForesight {
    enum EMdMsgType
    {
        eShSnapShot = 0,       // 上交L2快照
        eShBestOrders,         // 上交L2订单明细，最多揭示50笔
        eShIndex,              // 上交L2指数行情
        eShTrade,              // 上交L2逐笔成交
        eShOrder,              // 上交L2逐笔委托
        eShAfterSnap,          // 上交L2盘后快照
        eShTickMerge,          // 上交L2逐笔合并
        eShAfterTrade,         // 上交L2盘后逐笔成交
        eShBondSnapShot,       // 上交L2债券快照
        eShBondTick,           // 上交L2债券逐笔
        eShStatic,             // 上交静态行情
        eShOrderBookSnap,      // 上交订单簿行情
        eShSnapShotLevel1,     // 上交L1快照
        eShIndexLevel1,        // 上交L1指数行情
        eShAfterSnapLevel1,    // 上交L1盘后快照
        eShBondSnapShotLevel1, // 上交L1债券快照
        eSzSnapShot = 101,     // 深交L2快照
        eSzBestOrders,         // 深交L2订单明细，最多揭示50笔
        eSzIndex,              // 深交L2指数行情
        eSzTrade,              // 深交L2逐笔成交
        eSzOrder,              // 深交L2逐笔委托
        eSzAfterSnap,          // 深交L2盘后快照
        eSzBlockTrade,         // 深交L2盘后定价大宗交易
        eSzBondTrade,          // 深交L2债券逐笔成交
        eSzBondBlockTrade,     // 深交L2债券大额逐笔成交
        eSzBondOrder,          // 深交L2债券逐笔委托
        eSzBondBlockOrder,     // 深交L2债券大额逐笔委托
        eSzBondSnapShot,       // 深交L2债券快照
        eSzBondBestOrders,     // 深交L2债券订单明细，最多揭示50笔
        eSzBiddingTrade,       // 深交L2竞价成交
        eSzBiddingOrder,       // 深交L2竞价委托
        eSzOrderBookSnap,      // 深交订单簿行情
        eSzSnapShotLevel1,     // L1快照
        eSzIndexLevel1,        // L1指数行情
        eSzAfterSnapLevel1,    // L1深交盘后快照
        eSzBlockTradeLevel1,   // L1深交盘后定价大宗交易
        eSzBondSnapShotLevel1, // L1债券快照
        eCsiMarketData = 201,  // 中证指数行情信息
        eCsiWeightInfo,        // 中证指数权重信息
        eCsiEtfIopv,           // ETF 参考净值（IOPV）信息定义

    };

    enum MarketType
    {
        SSE = 1,               //上交所
        SZE,                   //深交所
        CSI = 20               //中证指数
    };
}
#endif