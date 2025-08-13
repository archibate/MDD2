#pragma once

#include "OES.h"
#include <cstdint>

#if SPLIT_ORDER
using BuyRequest = OES::ReqOrderBatch;
#else
using BuyRequest = OES::ReqOrder;
#endif

void makeBuyRequest(BuyRequest &buyRequest, int32_t stockCode, int32_t upperLimitPrice, int32_t quantity);
void makeGCSellRequest(OES::ReqOrder &reqOrder, int32_t stockCode, double price, int64_t quantity);
