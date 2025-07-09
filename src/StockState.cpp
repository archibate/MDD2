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

void StockState::onOrder(MDS::Tick &tick)
{
    bool limitUp = tick.isBuyOrder() && tick.price == upperLimitPrice;
    if (limitUp) {
    }
}
