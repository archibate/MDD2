#pragma once


#define HEAT_ZONE_REQORDER [[gnu::hot]]
#define HEAT_ZONE_RSPORDER [[gnu::hot]]
#define HEAT_ZONE_TICK [[gnu::hot]]
#define HEAT_ZONE_TIMER [[gnu::hot]]
#define HEAT_ZONE_ORDBOOK [[gnu::hot]]
#define HEAT_ZONE_SNAPSHOT [[gnu::hot]]
#define HEAT_ZONE_COMPUTE [[gnu::hot]]
#define HEAT_ZONE_MODEL [[gnu::hot]]
#define HEAT_ZONE_BUSY [[gnu::hot]]
#define COLD_ZONE [[gnu::noinline]] [[gnu::cold]]
