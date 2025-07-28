#pragma once


#include <cstdint>


namespace OES
{

struct ReqOrder
{
    int32_t stockCode;
    int32_t price;
    int32_t quantity;
    int32_t limitType;
};

struct RspOrder
{
    int32_t errorId;
    int32_t messageType;
    int32_t stockCode;
    int32_t orderStatus;
    int32_t orderSysId;
    int32_t orderPrice;
    int32_t tradePrice;
    int32_t orderQuantity;
    int32_t tradeQuantity;
    int32_t totalTradedQuantity;
    int32_t remainQuantity;
    int32_t cancelledQuantity;
};

void start(const char *config);
void stop();
void sendRequest(ReqOrder &reqOrder);

}
