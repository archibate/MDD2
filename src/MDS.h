#pragma once

#include <cstdint>
#include "L2/Tick.h"


namespace MDS
{

using Tick = L2::Tick;

void subscribe(int32_t const *stocks, int32_t n);
void start();
void stop();
bool isFinished();

}
