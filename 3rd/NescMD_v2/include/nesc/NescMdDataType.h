#ifndef __NESC_MD_DATA_TYPE_H__
#define __NESC_MD_DATA_TYPE_H__

#include "SseMdStruct.h"
#include "SzeMdStruct.h"

namespace NescForesight {
    struct SseStaticInfoField{
        StaticSseInfo * staticInfos;
        int count;
    };

    struct SzStaticInfoField{
        StaticSzInfo * staticInfos;
        int count;
    };
}

#endif  // __NESC_MD_DATA_TYPE_H__