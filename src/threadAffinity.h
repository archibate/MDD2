#pragma once

#include <cstdint>
#include <sched.h>
#include <unistd.h>


inline void setThisThreadAffinity(int32_t cpu)
{
    if (cpu >= 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);
        sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    }
}
