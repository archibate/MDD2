#pragma once


#include <cstddef>


struct string_slice
{
    const char *strBegin;
    size_t strLen;
};


struct argument_meta
{
    size_t argOffset;
    char *(*format)(char *buffer, void *instance);
    size_t (*formattedSize)(void *instance);
    void (*destroy)(void *instance);
    string_slice postString;
};


struct static_log_meta
{
    string_slice preString;
    argument_meta *argMetas;
    argument_meta *endArgMetas;
    size_t argTotalSize;
};
