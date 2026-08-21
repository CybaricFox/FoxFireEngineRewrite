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
    RESOURCE,
    TEXTURE,
    HASHMAP,
    REUSABLE_ARRAY,
    MATERIAL,
    ECS,
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
    DynamicAllocator allocator{};

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

    /**
     * @brief Adds the engines memory to the tracker without allocating it.
     * @param size Size of the engine.
     */
    static void trackEngineMemory(unsigned long size);
    static void untrackEngineMemory(unsigned long size);

    template<typename T>
    static T* ff_allocate_class(const unsigned long size, const MemoryTag tag) {
        if (size < sizeof(T)) {
            Logger::logError("ff_allocate_class requires that size be greater or equal to the class size.");
            return nullptr;
        }
        //Allocate the memory block
        void* destination = ff_allocate(size, tag);
        if (!destination) return nullptr;
        //construct the class
        T* result = static_cast<T *>(destination);

        return std::construct_at(result);
    }
    template<typename T>
    static void ff_free_class(void* block, const unsigned long size, const MemoryTag tag) {
        if (!block) return;

        if (size < sizeof(T)) {
            Logger::logError("ff_free_class requires that size be greater or equal to the class size.");
            return;
        }

        std::destroy_at(static_cast<T*>(block));
        ff_free(block, size, tag);
    }
};
