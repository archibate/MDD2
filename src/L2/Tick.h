#pragma once

#include <cstdint>
#include <cmath>


namespace L2
{

struct Tick
{
    int32_t stock;
    int32_t timestamp;
    int32_t price;
    int32_t quantity;
    int32_t buyOrderNo;
    int32_t sellOrderNo;

    bool isBuyOrder() const {
        return buyOrderNo != 0 && sellOrderNo == 0;
    }

    bool isSellOrder() const {
        return buyOrderNo == 0 && sellOrderNo != 0;
    }

    bool isOrder() const {
        return (buyOrderNo != 0) != (sellOrderNo != 0);
    }

    int32_t orderNo() const {
        return buyOrderNo != 0 ? buyOrderNo : sellOrderNo;
    }

    bool isTrade() const {
        return buyOrderNo != 0 && sellOrderNo != 0;
    }

    bool isMeta() const {
        return buyOrderNo == 0 && sellOrderNo == 0;
    }

    bool isOrderCancel() const {
        return quantity < 0;
    }

    bool isOrderOrder() const {
        return quantity >= 0;
    }

    bool isOrderMarket() const {
        return price == 0;
    }

    bool isOpenCall() const {
        return timestamp < 9'30'00'000;
    }

    bool isTradeActiveSell() const {
        return buyOrderNo < sellOrderNo && timestamp >= 9'30'00'000;
    }

    bool isTradeActiveBuy() const {
        return buyOrderNo > sellOrderNo && timestamp >= 9'30'00'000;
    }

    bool isSell() const {
        return buyOrderNo < sellOrderNo;
    }

    bool isBuy() const {
        return buyOrderNo > sellOrderNo;
    }

    int32_t orderQuantity() const {
        return std::abs(quantity);
    }
};

}
