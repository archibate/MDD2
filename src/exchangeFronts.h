#pragma once

#include "config.h"
#include <array>

#if SPLIT_ORDER

#if XC || NE
constexpr std::array kExchangeFronts = {3+1, 4+1, 6+1};
#endif
#if OST
constexpr std::array kExchangeFronts = {1, 5, 6, 7};
#endif

#endif
