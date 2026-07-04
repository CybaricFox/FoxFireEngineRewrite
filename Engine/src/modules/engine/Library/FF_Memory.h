#pragma once

#include "foxfire_export.h"
#include "src/defines.h"

enum MemoryTag {
    UNKNOWN,
    GAME,
    RENDER,
    ARRAY,
    MAX_TAGS
};

class FF_Memory {
private:
    struct MemoryBlock {
        unsigned long totalAllocated;
        unsigned long taggedAllocations[MAX_TAGS];
    };

    static MemoryBlock memoryData;

    static String getStringFromTag(unsigned long tag);

public:
    static void* ff_allocate(unsigned long size, MemoryTag tag);
    static void ff_free(void* block, unsigned long size, MemoryTag tag);
    static void* ff_clear(void* block, unsigned long size);
    static void* ff_copy(void* destination, const void* source, unsigned long size);
    static void* ff_set(void* destination, int value, unsigned long size);
    static String getMemoryUsage();
    static void initialize();
};
