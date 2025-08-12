#pragma once

#include <array>
#include <thread>
#include "MDS.h"
#include "OES.h"
#include "constants.h"


namespace MDD
{

void start(const char *config);
void stop();
void requestStop();
bool isFinished();
void handleTick(MDS::Tick &tick);
void handleSnap(MDS::Snap &snap);
void handleStatic(MDS::Stat &stat);
void handleRspOrder(OES::RspOrder &rspOrder);

}
