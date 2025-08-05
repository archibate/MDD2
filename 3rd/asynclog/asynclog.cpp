#include "asynclog.h"
#include "SPSC.h"
#include <fmt/format.h>
#include <cstring>


spsc_ring_queue<char, 1024> ring;



size_t logFormattedSize(static_log_meta &meta, char *arguments)
{
    size_t n = 0;
    n += meta.preString.strLen;
    for (auto *a = meta.argMetas, *e = meta.endArgMetas; a != e; ++a) {
        n += a->formattedSize(arguments);
        n += a->postString.strLen;
    }
    return n;
}

char *logFormat(char *buffer, static_log_meta &meta, char *arguments)
{
    buffer = (char *)memcpy(buffer, meta.preString.strBegin, meta.preString.strLen);
    for (auto *a = meta.argMetas, *ae = meta.endArgMetas; a != ae; ++a) {
        buffer = a->format(buffer, arguments + a->argOffset);
        buffer = (char *)memcpy(buffer, a->postString.strBegin, a->postString.strLen);
    }
    return buffer;
}
