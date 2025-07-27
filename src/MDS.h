#pragma once

#include <cstdint>
#include "L2/Tick.h"
#include "L2/Stat.h"


namespace MDS
{

using Tick = L2::Tick;
using Stat = L2::Stat;

void subscribe(int32_t const *stocks, int32_t n);
MDS::Stat getStatic(int32_t stock);
void start(const char *config);
void stop();
void requestStop();
bool isFinished();
bool isStarted();

}
