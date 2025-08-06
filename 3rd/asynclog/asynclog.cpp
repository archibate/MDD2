#include "asynclog.h"
#include "SPSC.h"
#include <fmt/format.h>
#include <cstring>


spsc_ring_queue<char, 1024> ring;



size_t logFormattedSize(const static_log_meta *meta, char *arguments)
{
    size_t n = 0;
    n += meta->preString.strLen;
    for (auto *a = meta->argMetas, *e = meta->endArgMetas; a != e; ++a) {
        n += a->formattedSize(arguments);
        n += a->postString.strLen;
    }
    return n;
}

char *logFormat(char *output, const static_log_meta *meta, char *arguments)
{
    output = (char *)memcpy(output, meta->preString.strBegin, meta->preString.strLen);
    for (auto *a = meta->argMetas, *ae = meta->endArgMetas; a != ae; ++a) {
        output = a->format(output, arguments + a->argOffset);
        output = (char *)memcpy(output, a->postString.strBegin, a->postString.strLen);
    }
    return output;
}


alignas(64) static_log_meta staticLogMetas[512];
alignas(64) char argumentsBuffer[1024];
alignas(64) char outputBuffer[65536];
alignas(64) char *outputPtr = outputBuffer;


void writeLog(const char *data, size_t size)
{
    fwrite(data, size, 1, stderr);
}


bool flushOutputBuffer()
{
    if (outputPtr != outputBuffer) {
        writeLog(outputBuffer, outputPtr - outputBuffer);
        outputPtr = outputBuffer;
        return true;
    }
    return false;
}

decltype(ring)::ring_reader reader;


void fetchRing()
{
    reader = ring.reader();

    while (true) {
        const static_log_meta *metaPtr;
        // while (reader.read_remain() < sizeof(void *)) {
        //     flushOutputBuffer();
        //     reader.read_wait();
        // }
        // reader.read_n((char *)&metaPtr, sizeof(void *));
        reader.blocking_read_n((char *)&metaPtr, sizeof(void *));

        if (metaPtr->argTotalSize > 0) {
            if (metaPtr->argTotalSize > sizeof argumentsBuffer) {
                throw;
            }
            reader.blocking_read_n(argumentsBuffer, metaPtr->argTotalSize);
        }

        size_t outputSize = logFormattedSize(metaPtr, argumentsBuffer);
        if (outputSize > sizeof outputBuffer) {
            throw;
        }
        if (outputSize > outputBuffer + sizeof outputBuffer - outputPtr) {
            flushOutputBuffer();
        }
        outputPtr = logFormat(outputPtr, metaPtr, argumentsBuffer);
    }
}
