#pragma once

#include "foxfire_export.h"
#include "src/defines.h"

enum MemoryTag {
    UNKNOWN,
    GAME,
    MAX_TAGS
};

class FOXFIRE_API FF_Memory {
private:
    struct MemoryBlock {
        unsigned long totalAllocated;
        unsigned long taggedAllocations[MAX_TAGS];
    };

    static MemoryBlock memoryData;

    String getStringFromTag(unsigned long tag);

public:
    FF_Memory();
    ~FF_Memory();

    void* ff_allocate(unsigned long size, MemoryTag tag);
    void ff_free(void* block, unsigned long size, MemoryTag tag);
    void* ff_clear(void* block, unsigned long size);
    void* ff_copy(void* destination, const void* source, unsigned long size);
    void* ff_set(void* destination, int value, unsigned long size);
    String getMemoryUsage();
};
