#include "config.h"
#if REPLAY
#include "OES.h"
#include "MDD.h"
#include "heatZone.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace
{
std::atomic<int32_t> orderSysId{1};
}

void OES::start(const char *config)
{
    try {
        nlohmann::json json;
        std::ifstream(config) >> json;
    } catch (std::exception const &e) {
        SPDLOG_ERROR("config json parse failed: {}", e.what());
    }
}

void OES::stop()
{
}

HEAT_ZONE_REQORDER void OES::sendRequest(ReqOrder &reqOrder)
{
    SPDLOG_CRITICAL("oes replay send request: stock={} price={} quantity={}",
                    reqOrder.stockCode, reqOrder.price, reqOrder.quantity);

    OES::RspOrder rspOrder{};
    rspOrder.errorId = 0;
    rspOrder.messageType = 'R';
    rspOrder.stockCode = reqOrder.stockCode;
    rspOrder.orderStatus = 'O';
    rspOrder.orderSysId = orderSysId.fetch_add(1, std::memory_order_relaxed);
    rspOrder.orderPrice = reqOrder.price;
    rspOrder.orderQuantity = reqOrder.quantity;
    MDD::handleRspOrder(rspOrder);
}
#endif
