#include "StockState.h"
#include "stockCodes.h"


void StockState::start(int32_t stockIndex)
{
    stockCode = kStockCodes[stockIndex];
}

void StockState::stop()
{
}

void StockState::handleTick(MDS::Tick &tick)
{
    if (tick.isOrder()) {
        if (tick.isOrderCancel()) {
            onCancel(tick);
        } else {
            onOrder(tick);
        }
    } else if (tick.isTrade()) {
        onTrade(tick);
    }
}

#if SH
void StockState::onOrder(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice && tick.isSellOrder()) {
        if (tick.isOrderCancel()) {
            upSellOrders.erase(tick.orderNo());
        } else {
            upSellOrders.insert({tick.orderNo(), tick.orderQuantity()});
        }
    }

    bool limitUp = !tick.isOpenCall() && tick.isBuyOrder() && tick.price == upperLimitPrice;
    if (limitUp) [[likely]] {
    }
}

void StockState::onCancel(MDS::Tick &tick)
{
}

void StockState::onTrade(MDS::Tick &tick)
{
    if (tick.price == upperLimitPrice) {
        auto it = upSellOrders.find(tick.sellOrderNo);
        if (it != upSellOrders.end()) {
            it->second -= tick.quantity;
            if (it->second <= 0) {
                upSellOrders.erase(it);
            }
        }

        addTrade(tick.timestamp, tick.price, tick.quantity);

    } else {
        addTrade(tick.timestamp, tick.price, tick.quantity);
    }
}
#endif

#if SZ
void StockState::onOrder(MDS::Tick &tick)
{
}

void StockState::onCancel(MDS::Tick &tick)
{
}

void StockState::onTrade(MDS::Tick &tick)
{
}
#endif
