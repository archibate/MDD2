#include "StockCompute.h"
#include "StockState.h"
#include "config.h"
#include "MDD.h"
#include "timestamp.h"
#include "IIRState.h"
#include "heatZone.h"
#include "radixSort.h"
#include "generatedModels.h"
#include <spdlog/spdlog.h>
#include <cstring>
#include <execution>
#include <emmintrin.h>

namespace
{

COLD_ZONE void logLimitUp(int32_t stockIndex, int32_t tickTimestamp, TickCache::Intent intent)
{
    auto &tickCache = MDD::g_tickCaches[stockIndex];
    int32_t linearTimestamp = (timestampLinear(tickTimestamp) + 90) / 100 * 100;
    int32_t minLinearTimestamp = tickCache.wantBuyTimestamp[0].load(std::memory_order_relaxed);
    int32_t minDt = std::abs(minLinearTimestamp - linearTimestamp);
    for (int32_t i = 1; i < tickCache.wantBuyTimestamp.size(); ++i) {
        int32_t wantTimestamp = tickCache.wantBuyTimestamp[i].load(std::memory_order_relaxed);
        int32_t dt = std::abs(wantTimestamp - linearTimestamp);
        if (dt < minDt) {
            minDt = dt;
            minLinearTimestamp = wantTimestamp;
        }
    }
    minLinearTimestamp += minLinearTimestamp & 1;

    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto &stockCompute = MDD::g_stockComputes[stockIndex];
    auto &stockState = MDD::g_stockStates[stockIndex];

    // todo: find the correct factor list time and dump:
    stockCompute.dumpFactors(timestampDelinear(minLinearTimestamp));

    SPDLOG_INFO("limit up model status: stock={} tickTimestamp={} roundTimestamp={} minTimestamp={} minDt={} toleranceDt={} intent={}", stockState.stockCode, tickTimestamp, timestampDelinear(linearTimestamp), timestampDelinear(minLinearTimestamp), minDt, kWantBuyTimeTolerance, magic_enum::enum_name(intent));
    SPDLOG_TRACE("limit up detected: stock={} timestamp={} intent={}", stockState.stockCode, tickTimestamp, magic_enum::enum_name(intent));
}

}


void StockCompute::start()
{
    stockCode = stockState().stockCode;
    upperLimitPrice = stockState().upperLimitPrice;
    preClosePrice = stockState().preClosePrice;

    upperLimitPriceApproach = static_cast<int32_t>(std::floor(upperLimitPrice / 1.02)) - 1;
    openPrice = stockState().preClosePrice;

    fState.currSnapshot.lastPrice = stockState().preClosePrice;
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.volume = 0;
    fState.currSnapshot.amount = 0;

    fState.iirState = std::make_unique<IIRState>();
    bState.iirState = std::make_unique<IIRState>();

    futureTimestamp = fState.nextTickTimestamp = 9'30'00'000;

#if RECORD
    factorListCache = std::make_unique<FactorList[]>(std::tuple_size_v<decltype(std::declval<TickCache>().wantBuyTimestamp)>);
#endif
}

COLD_ZONE void StockCompute::stop()
{
    alive = false;
    approachingLimitUp = false;
}

COLD_ZONE void StockCompute::dumpFactors(int32_t timestamp) const
{
    int32_t linearTimestamp = timestampLinear(timestamp);
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    for (int32_t i = 0; i < tickCache.wantBuyTimestamp.size(); ++i) {
        int32_t wantTimestamp = tickCache.wantBuyTimestamp[i].load(std::memory_order_relaxed);
        wantTimestamp += wantTimestamp & 1;
        if (linearTimestamp == wantTimestamp) {
#if RECORD
            factorListCache[i].dumpFactors(timestamp, stockCode);
#endif
            return;
        }
    }

    SPDLOG_ERROR("cannot find in factor cache: stock={} timestamp={}", stockCode, timestamp);
}

void StockCompute::loadFactors(FactorList const &factors)
{
    factorList = factors;
}

int64_t StockCompute::upSellOrderAmount() const
{
    int64_t amount = 0;
    for (auto const &[sellOrderId, sell]: upSellOrders) {
        int64_t q64 = sell.quantity;
        amount += sell.price * q64;
    }
    return amount;
}

HEAT_ZONE_ORDBOOK void StockCompute::onTick(MDS::Tick &tick)
{
#if REPLAY
    if (tick.stock == 0) [[unlikely]] {
        logLimitUp(stockIndex(), tick.timestamp / 10 * 10,
                   static_cast<TickCache::Intent>(tick.timestamp % 10));
        stop();
        return;
    }

    if (tick.isOrder()) {
        if (tick.isOrderCancel()) {
            onCancel(tick);
        } else {
            onOrder(tick);
        }

    } else if (tick.isTrade()) {
        onTrade(tick);
    }

#elif NE && SH
    if (tick.tickMergeSse.tickType == 0) [[unlikely]] {
        logLimitUp(stockIndex(), tick.tickMergeSse.tickTime * 10,
                   static_cast<TickCache::Intent>(tick.tickMergeSse.tickBSFlag));
        stop();
        return;
    }

    switch (tick.tickMergeSse.tickType) {
    case 'A':
        onOrder(tick);
        break;
    case 'D':
        onCancel(tick);
        break;
    case 'T':
        onTrade(tick);
        break;
    default:
        break;
    }

#elif NE && SZ
    if (tick.messageType == 0) [[unlikely]] {
        logLimitUp(stockIndex(), tick.tradeSz.transactTime / 10 * 10,
                   static_cast<TickCache::Intent>(tick.tradeSz.transactTime % 10));
        stop();
        return;
    }

    switch (tick.messageType) {
#error not implemented
    }
#endif
}

HEAT_ZONE_ORDBOOK void StockCompute::onTimer()
{
    if (!alive) [[unlikely]] {
        return;
    }
    approachingLimitUp = false;

    thread_local MDS::Tick tickBuf[256];
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    size_t n;

    while ((n = tickCache.fetchTicks(tickBuf)) != 0) {
        for (MDS::Tick *tick = tickBuf, *tickEnd = tickBuf + n; tick != tickEnd; ++tick) {
            onTick(*tick);
        }
    }

    if (approachingLimitUp) {
        futureTimestamp = fState.nextTickTimestamp;
        firstCompute = true;
    }
}

HEAT_ZONE_COMPUTE void StockCompute::onPostTimer()
{
    if (approachingLimitUp) {
        computeFutureWantBuy();
        futureTimestamp = timestampAdvance100ms(futureTimestamp);
        if (firstCompute) {
            firstCompute = false;
            computeFutureWantBuy();
            futureTimestamp = timestampAdvance100ms(futureTimestamp);
        }
    }
}

HEAT_ZONE_BUSY void StockCompute::onBusy()
{
    bool wantBuy = computeModel();
    asm volatile ("" :: "r" (wantBuy));
    _mm_pause();
}

HEAT_ZONE_ORDBOOK void StockCompute::onOrder(MDS::Tick &tick)
{
#if REPLAY
    if (tick.isSellOrder() && tick.price >= upperLimitPriceApproach) {
        UpSell sell;
        sell.price = tick.price;
        sell.quantity = tick.quantity;
        upSellOrders.insert({tick.orderNo(), sell});
    }

#elif NE && SH
    if (tick.tickMergeSse.price >= upperLimitPriceApproach * 10) {
        UpSell sell;
        sell.price = tick.tickMergeSse.price / 10;
        sell.quantity = tick.tickMergeSse.qty / 1000;
        upSellOrders.insert({tick.tickMergeSse.sellOrderNo, sell});
    }

#elif NE && SZ
#error not implemented
#endif
}

HEAT_ZONE_ORDBOOK void StockCompute::onCancel(MDS::Tick &tick)
{
#if REPLAY
    if (tick.isSellOrder()) {
        auto it = upSellOrders.find(tick.orderNo());
        if (it != upSellOrders.end()) {
            upSellOrders.erase(it);
        }
    }

#elif NE && SH
    if (tick.tickMergeSse.tickBSFlag == '1') {
        auto it = upSellOrders.find(tick.tickMergeSse.sellOrderNo);
        if (it != upSellOrders.end()) {
            upSellOrders.erase(it);
        }
    }

#elif NE && SZ
#error not implemented
#endif
}

HEAT_ZONE_ORDBOOK void StockCompute::onTrade(MDS::Tick &tick)
{
#if REPLAY
    auto it = upSellOrders.find(tick.sellOrderNo);
    if (it != upSellOrders.end()) {
        it->second.quantity -= tick.quantity;
        if (it->second.quantity <= 0) {
            upSellOrders.erase(it);
        }
    }

    if (tick.timestamp < 9'30'00'000) {
        fState.currSnapshot.lastPrice = openPrice = tick.price;
        openVolume += tick.quantity;
        return;
    }

    if (tick.price >= upperLimitPriceApproach) {
        approachingLimitUp = true;
    }
    addRealTrade(tick.timestamp, tick.price, tick.quantity);

#elif NE && SH
    int32_t quantity = tick.tickMergeSse.qty / 1000;
    int32_t price = tick.tickMergeSse.price / 10;

    auto it = upSellOrders.find(tick.tickMergeSse.sellOrderNo);
    if (it != upSellOrders.end()) {
        it->second.quantity -= quantity;
        if (it->second.quantity <= 0) {
            upSellOrders.erase(it);
        }
    }

    if (tick.tickMergeSse.tickTime < 9'30'00'00) {
        fState.currSnapshot.lastPrice = openPrice = price;
        openVolume += quantity;
        return;
    }

    if (price >= upperLimitPriceApproach) {
        approachingLimitUp = true;
    }
    addRealTrade(tick.tickMergeSse.tickTime * 10, price, quantity);

#elif NE && SZ
#error not implemented
#endif
}

HEAT_ZONE_ORDBOOK void StockCompute::addRealTrade(int32_t timestamp, int32_t price, int32_t quantity)
{
    stepSnapshotUntil(timestamp);

    int64_t q64 = quantity;
    fState.currSnapshot.lastPrice = price;
    ++fState.currSnapshot.numTrades;
    fState.currSnapshot.volume += q64;
    fState.currSnapshot.amount += price * q64;
}

HEAT_ZONE_COMPUTE void StockCompute::computeFutureWantBuy()
{
    saveSnapshot();
    stepSnapshotUntil(futureTimestamp);

    for (auto const &[sellOrderId, sell]: upSellOrders) {
        int64_t q64 = sell.quantity;
        ++fState.currSnapshot.numTrades;
        fState.currSnapshot.volume += q64;
        fState.currSnapshot.amount += sell.price * q64;
    }
    fState.currSnapshot.lastPrice = upperLimitPrice;
    stepSnapshot();

    bool wantBuy = decideWantBuy();
    auto &tickCache = MDD::g_tickCaches[stockIndex()];
    tickCache.pushWantBuyTimestamp(futureTimestamp, wantBuy);
#if RECORD
    factorListCache[(tickCache.wantBuyCurrentIndex - 1) & (tickCache.wantBuyTimestamp.size() - 1)] = factorList;
#endif

    restoreSnapshot();
}

HEAT_ZONE_SNAPSHOT void StockCompute::stepSnapshotUntil(int32_t timestamp)
{
    if (timestamp > fState.nextTickTimestamp) {
        stepSnapshot();
        while (timestamp > (fState.nextTickTimestamp = timestampAdvance100ms(fState.nextTickTimestamp))) {
            stepSnapshot();
        }
    }
}

HEAT_ZONE_COMPUTE void StockCompute::saveSnapshot()
{
    bState.savingMode = true;
    bState.nextTickTimestamp = fState.nextTickTimestamp;
    bState.currSnapshot = fState.currSnapshot;
    bState.oldSnapshotsCount = fState.snapshots.size();
    *bState.iirState = *fState.iirState;
    bState.iirState.swap(fState.iirState);
}

HEAT_ZONE_COMPUTE void StockCompute::restoreSnapshot()
{
    bState.savingMode = false;
    fState.nextTickTimestamp = bState.nextTickTimestamp;
    fState.currSnapshot = bState.currSnapshot;
    fState.snapshots.resize(bState.oldSnapshotsCount);
    fState.iirState.swap(bState.iirState);
}

HEAT_ZONE_SNAPSHOT void StockCompute::stepSnapshot()
{
    fState.iirState->addVolumeTick(fState.currSnapshot.volume);
    fState.snapshots.push_back(fState.currSnapshot);
    fState.currSnapshot.numTrades = 0;
    fState.currSnapshot.volume = 0;
    fState.currSnapshot.amount = 0;
}

namespace
{

HEAT_ZONE_COMPUTE void computeSkewKurt(double &skew, double &kurt, double *first, double *last)
{
    size_t n = last - first;
    if (n <= 3) {
        skew = kurt = NAN;
        return;
    }

    double sum = 0;
    for (double *p = first; p != last; ++p) {
        sum += *p;
    }
    double mean = sum / n;

    double variance = 0;
    double skewness = 0;
    double kurtosis = 0;
    for (double *p = first; p != last; ++p) {
        double value = *p - mean;
        double valueSqr = value * value;
        variance += valueSqr;
        skewness += valueSqr * value;
        kurtosis += valueSqr * valueSqr;
    }

    if (variance <= 0) {
        skew = kurt = 0;
        return;
    }
    double var = variance / n;
    double std = std::sqrt(var);

    skew = skewness / (n * var * std);
    kurt = kurtosis / (n * var * var) - 3;

    skew = skew * std::sqrt(n * (n - 1)) / (n - 2);
    kurt = (kurt * (n + 1) + 6) * (n - 1) / ((n - 2) * (n - 3));
}

}

HEAT_ZONE_COMPUTE bool StockCompute::decideWantBuy()
{
    if (fState.snapshots.empty()) [[unlikely]] {
        return false;
    }

    computeMomentum();
    computeVolatility();
    computeKaiyuan();
    computeCrowdind();

    return computeModel();
}

HEAT_ZONE_COMPUTE void StockCompute::computeMomentum()
{
    for (int32_t m = 0; m < kMomentumDurations.size(); ++m) {
        auto &factor = factorList.momentum[m];

        if (fState.snapshots.size() >= kMomentumDurations[m] + 1) {
            {
                double sum = 0;
                double sumSqr = 0;
                double prevPrice = fState.snapshots[0].lastPrice;
                for (int32_t t = 1; t < kMomentumDurations[m] + 1; ++t) {
                    double currPrice = fState.snapshots[t].lastPrice;
                    double changeRate = currPrice / prevPrice - 1.0;
                    sum += changeRate;
                    sumSqr += changeRate * changeRate;
                    prevPrice = currPrice;
                }
                double mean = sum * (1.0 / kMomentumDurations[m]);
                double variance = sumSqr * (1.0 / kMomentumDurations[m]) - mean * mean;
                double std = std::sqrt(std::max(0.0, variance));

                factor.openMean = mean;
                factor.openStd = std;
                factor.openZScore = std > 1e-10 ? mean / std : 0.0;
            }

            {
                double sum = 0;
                double sumSqr = 0;
                double prevPrice = prevPrice = fState.snapshots[fState.snapshots.size() - kMomentumDurations[m] - 1].lastPrice;
                for (int32_t t = fState.snapshots.size() - kMomentumDurations[m]; t < fState.snapshots.size(); ++t) {
                    double currPrice = fState.snapshots[t].lastPrice;
                    double changeRate = currPrice / prevPrice - 1.0;
                    sum += changeRate;
                    sumSqr += changeRate * changeRate;
                    prevPrice = currPrice;
                }
                double mean = sum * (1.0 / kMomentumDurations[m]);
                double variance = sumSqr * (1.0 / kMomentumDurations[m]) - mean * mean;
                double std = std::sqrt(std::max(0.0, variance));

                factor.highMean = mean;
                factor.highStd = std;
                factor.highZScore = std > 1e-10 ? mean / std : 0.0;
            }

            factor.diffMean = factor.openMean - factorList.momentum[m].highMean;
            factor.diffZScore = factor.openZScore - factor.highZScore;

        } else {
            std::memset(&factor, 0, sizeof(factor));
        }
    }
}

HEAT_ZONE_COMPUTE void StockCompute::computeVolatility()
{
    std::memset(&factorList.volatility, -1, sizeof(factorList.volatility));

    size_t n = fState.snapshots.size() / 5 + 1;
    thread_local std::vector<int64_t> volumeGather;
    thread_local std::vector<int64_t> amountGather;
    thread_local std::vector<double> vwap;

    volumeGather.clear();
    amountGather.clear();
    volumeGather.resize(n);
    amountGather.resize(n);
    amountGather[0] = openVolume * openPrice;
    volumeGather[0] = openVolume;
    for (size_t i = 0; i < fState.snapshots.size(); ++i) {
        volumeGather[i / 5] += fState.snapshots[i].volume;
        amountGather[i / 5] += fState.snapshots[i].amount;
    }

    vwap.resize(n);
    double lastVWAP = openPrice;
    for (size_t i = 0; i < n; ++i) {
        if (volumeGather[i] != 0) {
            lastVWAP = static_cast<double>(amountGather[i]) / static_cast<double>(volumeGather[i]);
        }
        vwap[i] = lastVWAP;
    }

    double p0 = preClosePrice;
    double p4 = p0 * 1.04;
    double p5 = p0 * 1.05;
    double p6 = p0 * 1.06;
    double p7 = p0 * 1.07;
    double p8 = p0 * 1.08;
    double p9 = p0 * 1.09;

    struct LCSRange
    {
        int64_t amount{};
        int64_t volume{};
        int32_t count{};

        int32_t start{0};
        int32_t stop{-1};
        int32_t curr{-1};
        int32_t last{-1};

        void lcsAdd(int32_t i)
        {
            if (curr == -1 || i != last + 1) {
                curr = i;
            } else if (i - curr >= stop - start) {
                start = curr;
                stop = i;
            }
            last = i;
        }
    };

    LCSRange ranges[3];
    for (size_t i = 0; i < n; ++i) {
        double v = vwap[i];
        if (v >= p4 && v <= p7) {
            ranges[0].lcsAdd(i);
            ranges[0].amount += amountGather[i];
            ranges[0].volume += volumeGather[i];
            ++ranges[0].count;
        }
        if (v >= p5 && v <= p8) {
            ranges[1].lcsAdd(i);
            ranges[1].amount += amountGather[i];
            ranges[1].volume += volumeGather[i];
            ++ranges[1].count;
        }
        if (v >= p6 && v <= p9) {
            ranges[2].lcsAdd(i);
            ranges[2].amount += amountGather[i];
            ranges[2].volume += volumeGather[i];
            ++ranges[2].count;
        }
    }

    double floatMV = factorList.floatMV;
    int64_t totalAmount = 0;
    int64_t totalVolume = 0;
    for (size_t i = 0; i < n; ++i) {
        totalAmount += amountGather[i];
        totalVolume += volumeGather[i];
    }

    computeSkewKurt(factorList.volatility[0].vwapSkew, factorList.volatility[0].vwapKurt, vwap.data(), vwap.data() + vwap.size());
    factorList.volatility[2].vwapSkew = factorList.volatility[1].vwapSkew = factorList.volatility[0].vwapSkew;
    factorList.volatility[2].vwapKurt = factorList.volatility[1].vwapKurt = factorList.volatility[0].vwapKurt;

    for (int32_t l = 0; l < 3; ++l) {
        auto &range = ranges[l];
        auto &factor = factorList.volatility[l];

        if (range.start <= range.stop) {
            double sumSqr = 0;
            double sumPosSqr = 0;
            double sumPosSqrSqr = 0;
            int32_t nPositive = 0;
            for (int32_t i = std::max(1, range.start); i <= range.stop; ++i) {
                double r = vwap[i] / vwap[i - 1] - 1.0;
                double r2 = r * r;
                sumSqr += r2;
                if (r > 0) {
                    sumPosSqr += r2;
                    sumPosSqrSqr += r2 * r2;
                    ++nPositive;
                }
            }

            if (sumSqr != 0) {
                factor.upVolume = std::sqrt(sumPosSqr);
                factor.upRatio = sumPosSqr / sumSqr;
                if (nPositive > 1) {
                    factor.upStd = std::sqrt(std::max(0.0, (sumPosSqrSqr - (sumPosSqr * sumPosSqr) / (nPositive - 1)) / (nPositive - 1)));
                }
            }

            factor.time = range.count * 0.25;
            factor.consecTime = (range.stop - range.start + 1) * 0.25;
            factor.turnover = static_cast<double>(range.amount) / floatMV * 0.01;
            factor.amountRatio = static_cast<double>(range.amount) / totalAmount;
            factor.volumeRatio = static_cast<double>(range.volume) / totalVolume;

            int64_t lcsAmount = 0;
            int64_t lcsVolume = 0;
            for (int32_t i = range.start; i <= range.stop; ++i) {
                lcsAmount += amountGather[i];
                lcsVolume += volumeGather[i];
            }
            factor.consecTurnover = static_cast<double>(lcsAmount) / floatMV * 0.01;
            factor.consecAmountRatio = static_cast<double>(lcsAmount) / totalAmount;
            factor.consecVolumeRatio = static_cast<double>(lcsVolume) / totalVolume;

            computeSkewKurt(factor.consecVwapSkew, factor.consecVwapKurt, vwap.data() + range.start, vwap.data() + range.stop + 1);
        }
    }
}

HEAT_ZONE_COMPUTE void StockCompute::computeKaiyuan()
{
    struct Transaction
    {
        double meanAmount;
        double sumAmount;
        double changeRate;

        bool operator<(Transaction const &right) const
        {
            return meanAmount < right.meanAmount;
        }
    };

    thread_local std::vector<Transaction> transactions;
    transactions.clear();
    transactions.reserve(fState.snapshots.size() / 10);
    double prevPrice = fState.snapshots[0].lastPrice;
    for (size_t i = 1; i < fState.snapshots.size(); ++i) {
        if (fState.snapshots[i].numTrades != 0) {
            double currPrice = fState.snapshots[i].lastPrice;
            double r = currPrice / prevPrice - 1.0;
            prevPrice = currPrice;

            double a = static_cast<double>(fState.snapshots[i].amount);
            transactions.push_back({a / fState.snapshots[i].numTrades, a, r});
        }
    }

    std::memset(&factorList.kaiyuan, -1, sizeof(factorList.kaiyuan));
    if (!transactions.empty()) {
        radixSort<16, 4, sizeof(double), offsetof(Transaction, meanAmount), sizeof(Transaction)>(transactions.data(), transactions.size());

        auto it096 = transactions.begin() + static_cast<size_t>(std::ceil(0.096 * transactions.size()));
        auto it096f = transactions.begin() + static_cast<size_t>(std::floor(0.096 * transactions.size()));
        auto it10 = transactions.begin() + static_cast<size_t>(std::ceil(0.10 * transactions.size()));
        auto it10f = transactions.begin() + static_cast<size_t>(std::floor(0.10 * transactions.size()));
        auto it50 = transactions.begin() + static_cast<size_t>(std::ceil(0.50 * transactions.size()));
        auto it70 = transactions.begin() + static_cast<size_t>(std::ceil(0.70 * transactions.size()));
        auto it90 = transactions.begin() + static_cast<size_t>(std::ceil(0.90 * transactions.size()));
        auto it96 = transactions.begin() + static_cast<size_t>(std::ceil(0.96 * transactions.size()));

        double A0 = transactions.front().meanAmount;
        double A100 = transactions.back().meanAmount;
        double A096 = it096->meanAmount;
        double A10 = it10->meanAmount;
        double A096f = it096f->meanAmount;
        double A10f = it10f->meanAmount;
        double A96 = it96->meanAmount;
        if (A100 > A0) {
            factorList.kaiyuan.quantile = (0.5 * (A10 + A10f) - A0) / (A100 - A0);
        }
        if (A96 > A0) {
            factorList.kaiyuan.trimmedQuantile = (0.5 * (A096 + A096f) - A0) / (A96 - A0);
        }

        double sumT = 0;
        double sumA = 0;
        double sumTA = 0;
        double sumT2 = 0;
        double sumA2 = 0;
        for (auto it = transactions.begin(); it != it96; ++it) {
            sumT += it->meanAmount;
            sumA += it->sumAmount;
            sumTA += it->meanAmount * it->sumAmount;
            sumT2 += it->meanAmount * it->meanAmount;
            sumA2 += it->sumAmount * it->sumAmount;
        }
        double size = it96 - transactions.begin();
        double numerator = sumTA * size - sumT * sumA;
        double denominatorX = sumT2 * size - sumT * sumT;
        double denominatorY = sumA2 * size - sumA * sumA;
        factorList.kaiyuan.trimmedCorrelation = numerator / std::sqrt(std::max(0.0, denominatorX * denominatorY));

        for (auto it = it96; it != transactions.end(); ++it) {
            sumT += it->meanAmount;
            sumA += it->sumAmount;
            sumTA += it->meanAmount * it->sumAmount;
            sumT2 += it->meanAmount * it->meanAmount;
            sumA2 += it->sumAmount * it->sumAmount;
        }
        size = transactions.size();
        numerator = sumTA * size - sumT * sumA;
        denominatorX = sumT2 * size - sumT * sumT;
        denominatorY = sumA2 * size - sumA * sumA;
        factorList.kaiyuan.correlation = numerator / std::sqrt(std::max(0.0, denominatorX * denominatorY));

        double sr = 0;
        for (auto it = it90; it != transactions.end(); ++it) {
            sr += it->changeRate;
        }
        factorList.kaiyuan.signalReturn10 = sr;
        for (auto it = it70; it != it90; ++it) {
            sr += it->changeRate;
        }
        factorList.kaiyuan.signalReturn30 = sr;
        for (auto it = it50; it != it70; ++it) {
            sr += it->changeRate;
        }
        factorList.kaiyuan.signalReturn50 = sr;
    }
}

HEAT_ZONE_COMPUTE void StockCompute::computeCrowdind()
{
    fState.iirState->finalCompute(factorList.crowdind);
}

HEAT_ZONE_MODEL bool StockCompute::computeModel()
{
    return predictModel(factorList.rawFactors);
}

int32_t StockCompute::stockIndex() const
{
    return this - MDD::g_stockComputes.get();
}

StockState &StockCompute::stockState() const
{
    return MDD::g_stockStates[stockIndex()];
}
