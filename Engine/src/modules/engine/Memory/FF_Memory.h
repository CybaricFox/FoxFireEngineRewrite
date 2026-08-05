/**
*   @file FF_Memory.h
 *  @layer Engine
 *  @module Memory
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include <foxfire_export.h>

#include "DynamicAllocator.h"
#include "src/defines.h"

/**
 * @brief Tag an allocation belongs to. Used to track memory allocation types.
 */
enum MemoryTag {
    UNKNOWN,
    GAME,
    RENDER,
    ARRAY,
    LINEAR_ALLOCATOR,
    DYNAMIC_ARRAY,
    AVAILABLE_TAG_SLOT,
    TEXTURE,
    HASHMAP,
    REUSABLE_ARRAY,
    MATERIAL,
    MAX_TAGS
};

/**
 * @brief Structure of data that contains information about allocations.
 */
struct MemoryStats {
    unsigned long totalAllocated = 0;
    unsigned long taggedAllocations[MAX_TAGS]{};
};

struct MemoryConfig {
    unsigned long totalAllocationSize = 0;
};

class FOXFIRE_API FF_Memory {
private:;
    static FF_Memory* memorySystem;

    MemoryStats memoryData{};
    MemoryConfig config{};
    unsigned long allocationCount = 0;
    unsigned long allocationMemoryRequirement = 0;
    DynamicAllocator* allocator = nullptr;
    void* allocatorMemory = nullptr;

    FF_Memory() = default;

    static String getStringFromTag(unsigned long tag);

public:
    ~FF_Memory() = default;

    static void* ff_allocate(unsigned long size, MemoryTag tag);
    static void ff_free(void* block, unsigned long size, MemoryTag tag);
    static void* ff_clear(void* block, unsigned long size);
    static void* ff_copy(void* destination, const void* source, unsigned long size);
    static void* ff_move(void* destination, const void* source, unsigned long size);
    static void* ff_set(void* destination, int value, unsigned long size);
    static String getMemoryUsage();
    static bool initialize(MemoryConfig config);
    static void shutdown();
    static unsigned long getAllocationCount();
    static bool isInitialized(){return memorySystem != nullptr;}
};
