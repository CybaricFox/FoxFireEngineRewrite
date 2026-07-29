#pragma once

#include <foxfire_export.h>

#include "src/defines.h"

enum MemoryTag {
    UNKNOWN,
    GAME,
    RENDER,
    ARRAY,
    LINEAR_ALLOCATOR,
    DYNAMIC_ARRAY,
    CHAR_ARRAY,
    TEXTURE,
    MAX_TAGS
};

struct MemoryBlock {
    unsigned long totalAllocated;
    unsigned long taggedAllocations[MAX_TAGS];
    unsigned long allocationCount;
};

class FF_Memory {
private:;
    static MemoryBlock* memoryData;

    static String getStringFromTag(unsigned long tag);

public:
    static void* ff_allocate(unsigned long size, MemoryTag tag);
    static void ff_free(void* block, unsigned long size, MemoryTag tag);
    static FOXFIRE_API void* ff_clear(void* block, unsigned long size);
    static void* ff_copy(void* destination, const void* source, unsigned long size);
    static void* ff_move(void* destination, const void* source, unsigned long size);
    static void* ff_set(void* destination, int value, unsigned long size);
    static FOXFIRE_API String getMemoryUsage();
    static void initialize(MemoryBlock& memoryBlock);
    static void shutdown();
    static FOXFIRE_API unsigned long getAllocationCount();
    static bool isInitialized(){return memoryData != nullptr;}
};
