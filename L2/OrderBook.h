#pragma once


#include "Stat.h"
#include "Snap.h"
#include "Tick.h"
#include "timestamp.h"
#include <map>
#include <stdexcept>
#include <vector>
#include <set>
#include <map>


struct OrderBook
{
    std::vector<int32_t> bidQty;
    std::vector<int32_t> askQty;
    std::map<int32_t, int32_t> ordPrice;

    std::vector<Snap> snapshots;

    int32_t stock;
    int32_t preClosePrice;
    int32_t upperLimitPrice;
    int32_t lowerLimitPrice;
    int32_t realOpenPrice;
    int32_t closePrice;
    int64_t floatVolume;

    int32_t virtOpenPrice;
    int32_t virtOpenMatchedQty;
    int32_t virtOpenUnmatchedQty;
    int32_t virtOpenBidRemainQty;
    int32_t virtOpenAskRemainQty;
    int32_t openPrice;

    int32_t lastTimestamp;

    int32_t numTrades;
    int32_t volume;
    int64_t amount;
    int32_t lastPrice;

    void initStat(Stat const &stat)
    {
        stock = stat.stock;
        preClosePrice = stat.preClosePrice;
        upperLimitPrice = stat.upperLimitPrice;
        lowerLimitPrice = stat.lowerLimitPrice;
        floatVolume = static_cast<int64_t>(std::round(stat.floatMV / (0.01 * stat.preClosePrice)));
        realOpenPrice = stat.openPrice;
        closePrice = stat.closePrice;

        bidQty.resize(upperLimitPrice - lowerLimitPrice + 1);
        askQty.resize(upperLimitPrice - lowerLimitPrice + 1);

        virtOpenPrice = preClosePrice;
        virtOpenMatchedQty = 0;
        virtOpenUnmatchedQty = 0;
        openPrice = 0;

        lastTimestamp = 9'15'00'000;
        numTrades = 0;
        volume = 0;
        amount = 0;
        lastPrice = 0;
    }

    int32_t &bidQtyAt(int32_t price)
    {
#ifndef NDEBUG
        if (!(price >= lowerLimitPrice && price <= upperLimitPrice)) throw std::out_of_range("bidQtyAt");
#endif
        return bidQty[price - lowerLimitPrice];
    }

    int32_t &askQtyAt(int32_t price)
    {
#ifndef NDEBUG
        if (!(price >= lowerLimitPrice && price <= upperLimitPrice)) throw std::out_of_range("askQtyAt");
#endif
        return askQty[price - lowerLimitPrice];
    }

    void tryUpdateSnap(int32_t timestamp, int32_t interval)
    {
        while (timestamp > lastTimestamp) {
            // printf("%d %d\n", timestamp, lastTimestamp);
            lastTimestamp = absoluteMillisecondsToTimestamp(
                timestampToAbsoluteMilliseconds(lastTimestamp, interval) + interval, interval);
            snapUpdate(lastTimestamp);
        }
    }

    void endUpdateSnap(int32_t interval)
    {
        int32_t endTime = absoluteMillisecondsToTimestamp(timestampToAbsoluteMilliseconds(
            15'00'00'000, interval) + interval, interval);
        tryUpdateSnap(endTime, interval);
    }

    void addTick(Tick &tick)
    {
        // Fix SZ U-limit orders:
        // if (!(tick.price >= lowerLimitPrice && tick.price <= upperLimitPrice)) {
        //     tick.price = 0;
        // }

        if (tick.isOrder()) {
            if (tick.isOrderCancel()) {
                auto it = ordPrice.find(tick.orderNo());
                if (it != ordPrice.end()) {
                    tick.price = it->second;
                    ordPrice.erase(it);
                }
            } else {
                if (tick.price != 0) {
                    ordPrice.insert({tick.orderNo(), tick.price});
                }
            }

            if (tick.isBuyOrder()) {
                if (tick.price != 0) {
                    bidQtyAt(tick.price) += tick.quantity;
                }
            } else {
                if (tick.price != 0) {
                    askQtyAt(tick.price) += tick.quantity;
                }
            }

        } else if (tick.isTrade()) {
            auto it = ordPrice.find(tick.buyOrderNo);
            if (it != ordPrice.end()) {
                bidQtyAt(it->second) -= tick.quantity;
            }
            it = ordPrice.find(tick.sellOrderNo);
            if (it != ordPrice.end()) {
                askQtyAt(it->second) -= tick.quantity;
            }

            if (openPrice == 0) {
                openPrice = tick.price;
            }
            ++numTrades;
            volume += tick.quantity;
            amount += tick.price * static_cast<int64_t>(tick.quantity);
            lastPrice = tick.price;
        }
    }

    void snapUpdate(int32_t timestamp)
    {
        Snap &snap = snapshots.emplace_back();
        snap.stock = stock;
        snap.timestamp = timestamp;
        snap.preClosePrice = preClosePrice;
        snap.numTrades = numTrades;
        snap.volume = volume;
        snap.amount = amount;
        snap.lastPrice = lastPrice;
        numTrades = 0;
        volume = 0;
        amount = 0;

        if (openPrice == 0) {
            std::vector<int32_t> askQtyAccum(upperLimitPrice - lowerLimitPrice + 1);
            std::vector<int32_t> bidQtyAccum(upperLimitPrice - lowerLimitPrice + 1);
            int32_t askQtySum = 0;
            int32_t bidQtySum = 0;

            for (int32_t p = 0; p <= upperLimitPrice - lowerLimitPrice; ++p) {
                askQtySum += askQty[p];
                askQtyAccum[p] = askQtySum;
            }
            for (int32_t p = upperLimitPrice - lowerLimitPrice; p >= 0; --p) {
                bidQtySum += bidQty[p];
                bidQtyAccum[p] = bidQtySum;
            }

            int32_t maxQ = 0;
            int32_t maxI = 0;
            int32_t maxISigned = 0;
            int32_t maxP = 0;
            for (int32_t p = 0; p <= upperLimitPrice - lowerLimitPrice; ++p) {
                int32_t transaction = std::min(askQtyAccum[p], bidQtyAccum[p]);
                int32_t imbalanceSigned = bidQtyAccum[p] - askQtyAccum[p];
                int32_t imbalance = std::abs(imbalanceSigned);
                if (transaction > maxQ || (transaction == maxQ && (p <= preClosePrice - lowerLimitPrice ? imbalance <= maxI : imbalance < maxI))) {
                    maxQ = transaction;
                    maxI = imbalance;
                    maxISigned = imbalanceSigned;
                    maxP = p;
                }
            }
            if (maxQ != 0) {
                virtOpenPrice = maxP + lowerLimitPrice;
                virtOpenMatchedQty = maxQ;
                virtOpenUnmatchedQty = maxISigned;
            } else {
                virtOpenPrice = preClosePrice;
                virtOpenMatchedQty = 0;
                virtOpenUnmatchedQty = 0;
            }

            virtOpenAskRemainQty = askQtySum - virtOpenMatchedQty;
            virtOpenBidRemainQty = bidQtySum - virtOpenMatchedQty;

            snap.bidPrice[0] = virtOpenPrice;
            snap.bidQuantity[0] = virtOpenMatchedQty;
            if (maxISigned > 0) {
                snap.bidPrice[1] = virtOpenPrice;
                snap.bidQuantity[1] = maxISigned;
            }

            snap.askPrice[0] = virtOpenPrice;
            snap.askQuantity[0] = virtOpenMatchedQty;
            if (maxISigned < 0) {
                snap.askPrice[1] = virtOpenPrice;
                snap.askQuantity[1] = -maxISigned;
            }

        } else {
            size_t level = 0;
            for (int32_t p = 0; p <= upperLimitPrice - lowerLimitPrice; ++p) {
                if (askQty[p] != 0) {
                    snap.askPrice[level] = p + lowerLimitPrice;
                    snap.askQuantity[level] = askQty[p];
                    ++level;
                    if (level >= kBookLevels) {
                        break;
                    }
                }
            }
            for (int32_t p = upperLimitPrice - lowerLimitPrice; p >= 0; --p) {
                if (bidQty[p] != 0) {
                    snap.bidPrice[level] = p + lowerLimitPrice;
                    snap.bidQuantity[level] = bidQty[p];
                    ++level;
                    if (level >= kBookLevels) {
                        break;
                    }
                }
            }
        }
    }
};
