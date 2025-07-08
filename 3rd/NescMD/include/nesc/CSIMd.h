/*
 * CSIMd.h
 *
 *  Created on: 2024年1月3日
 * 中证指数结构体定义
 */
#ifndef _CSIMd_H_
#define _CSIMd_H_

#include <stdint.h>
namespace NescForesight {
    const int MSG_TYPE_MARKETDATA_CSI =      0x30;   //中证指数行情
    const int MSG_TYPE_WEIGHT_CSI     =      0x31;   //中证指数权重
    const int MSG_TYPE_ETF_IOPV_CSI   =      0x32;   //ETF 参考净值（IOPV）

#pragma pack(1)

    /**
     * 中证指数行情信息
    */

    struct CSIMarketData {
        uint8_t messageType;            // 消息类型 type=0x30，筛选记录类型=01的数据
        uint32_t sequence;              // udp输出包序号
        uint8_t exchangeID;             // 交易所id  上交所：1  深交所：2  中证指数：20
        char securityID[9];             // 证券ID
        char securityName[35];          // 指数简称
        uint32_t jyrq;                  // 交易日期 "行情文件所代表交易日期，内容为被用于计算的那天的交易日期。日期的格式为“YYYYMMDD”。其中，YYYY：年，MM：月，DD：日。"
        uint32_t jsrq;                  // 自然日期 "行情文件所代表的自然日期(北京时间),内容为被用于计算的那天的自然日期（北京时间 ） . 日 期 的 格 式 为 “YYYYMMDD”。如交易日为 2012年 1 月 19 日的全球指数收盘时北京时间已经是 2012 年 1月 20 日。"
        uint32_t timeStamp;             // 最新订单时间 HHMMSS
        char securityExchangeID;        // 市场代码 "1：上证所；2：深交所；3： 沪深；4：香港；5: 亚太； 0: 全球（附加说明见注）"
        double lastPrice;               // 最新指数
        double openPrice;               // 开盘指数 "当日开盘值，当前交易日开盘指数值。初始值为 0.0000。当值为 0.0000 时，说明指数未开盘"
        double highPrice;               // 最高指数
        double lowPrice;                // 最低指数
        double closePrice;              // 收盘指数 "当日收盘值，当前交易日收盘值。初始值为 0.0000。当值不为 0.0000 时，说明指数已收盘。"
        double preClosePrice;           // 前盘指数
        double change;                  // 涨跌
        double changeRate;              // 涨跌幅
        double totalVolume;             // 成交量（股） 单位：股
        double totalValue;              // 成交金额（万元） 单位：万元
        double exchangeRate;            // 汇率 "汇率，该汇率在盘中时为 0.00000000，收盘后，该汇率值为该指数收盘时计算指数所使用的汇率。例：若该指数为日经 225 指数以人民币计价的指数，则汇率为人民币对日元的汇率。若该指数为沪深 300 指数以美元计价的指数，则汇率为美元对人民币的汇率。其他若该指数不涉及汇率的情况下，则始终为 1.00000000。"
        char currency;                  // 币种标志 "使用货币。0：人民币；1：港币；2：美元；3：台币； 4：日元"
        uint32_t indexShowNo;           // 指数展示序号
        double closePrice2;             //当日收盘值 2 "当日收盘值 2，若该指数为全球指数，该收盘值为当日亚太区收盘值。初始值为0.0000。当值不为0.0000 时，说明指数亚太区已收盘。"
        double closePrice3;             // 当日收盘值 3 "当日收盘值 3，若该指数为全球指数，该收盘值为当日欧洲区收盘值。初始值为0.0000。当值不为0.0000 时，说明指数欧洲区已收盘。"
    };

    /**
     * 中证指数权重信息
    */
    struct CSIWeightInfo {
        uint8_t messageType;            // 消息类型 type=0x31，筛选记录类型=02的数据
        uint32_t sequence;              // udp输出包序号
        uint8_t exchangeID;             // "交易所id 上交所：1 深交所：2 中证指数：20"
        char securityID[9];             // 证券ID
        char securityName[35];          // 指数简称
        uint32_t jyrq;                  // 交易日期 "行情文件所代表交易日期，内容为被用于计算的那天的交易日期。日期的格式为“YYYYMMDD”。其中，YYYY：年，MM：月，DD：日。"
        uint32_t jsrq;                  // 自然日期 "行情文件所代表的自然日期(北京时间),内容为被用于计算的那天的自然日期（北京时间 ） . 日 期 的 格 式 为“YYYYMMDD”。如交易日为 2012年 1 月 19 日的全球指数收盘时北京时间已经是 2012 年 1月 20 日。"
        uint32_t timeStamp;             // 最新订单时间 HHMMSS
        char contractCode[9];           // 证券代码 "交易所代码+ID代码例：SH600028"
        char contractName[35];          // 证券名称
        double weightedRatio;           // "个股在指数中的权重比例（%）" 例：2.02075
        double lastPrice;               // 最新指数
        double influent;                // 影响点数值 "该股票在当前时点对指数的贡献点数 例 1： 2.5862 例 2： -3.2568"
    };

    /**
     * ETF 参考净值（IOPV）信息定义
    */
    struct CSIETFIOPV {
        uint8_t messageType;            // 消息类型 type=0x32，筛选记录类型=03的数据
        uint32_t sequence;              // udp输出包序号
        uint8_t exchangeID;             // "交易所id 上交所：1 深交所：2 中证指数：20"
        char securityID[9];             // 证券ID
        char securityName[35];          // 指数简称
        uint32_t jyrq;                  // 交易日期 "行情文件所代表交易日期，内容为被用于计算的那天的交易日期。日期的格式为“YYYYMMDD”。其中，YYYY：年，MM：月，DD：日。
        uint32_t jsrq;                  // 自然日期 "行情文件所代表的自然日期(北京时间),内容为被用于计算的那天的自然日期（北京时间 ） . 日 期 的 格 式 为“YYYYMMDD”。如交易日为 2012年 1 月 19 日的全球指数收盘时北京时间已经是 2012 年 1月 20 日。
        uint32_t timeStamp;             // 最新订单时间 HHMMSS
        char securityExchangeID;        // 市场代码 "1：上证所；2：深交所；3：沪深；4：香港；5: 亚太；0: 全球（附加说明见注）"
        double iopv;                    // 基金参考净值（IOPV）
    };

#pragma pack(8)
}
#endif
