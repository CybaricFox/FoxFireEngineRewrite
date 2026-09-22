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

    /**
     * @brief Allocates memory.
     * @param size Size of the memory.
     * @param tag The type of memory. Used for tracking.
     * @return Pointer to the memory location.
     */
    static void* ff_allocate(unsigned long size, MemoryTag tag);

    /**
     * @brief Frees memory.
     * @param block Pointer to the memory location.
     * @param size Size of the memory.
     * @param tag The type of memory.
     */
    static void ff_free(void* block, unsigned long size, MemoryTag tag);

    /**
     * @brief Zeros out an area of memory.
     * @param block Pointer to the memory.
     * @param size Size of the memory.
     * @return
     */
    static void* ff_clear(void* block, unsigned long size);

    /**
     * @brief Copies the data from a memory location to another.
     * @param destination Location to copy to.
     * @param source Location to copy from.
     * @param size Size of the memory.
     * @return Pointer to the new location.
     */
    static void* ff_copy(void* destination, const void* source, unsigned long size);

    /**
     * @brief Moves the data from a memory location to another.
     * @param destination Location to move to.
     * @param source Location to move from.
     * @param size Size of the memory.
     * @return Pointer to the new location.
     */
    static void* ff_move(void* destination, const void* source, unsigned long size);

    /**
     * @brief Sets memory at the location.
     * @param destination Location to set to.
     * @param value Value to set.
     * @param size Size of the memory.
     * @return Pointer to the memory location.
     */
    static void* ff_set(void* destination, int value, unsigned long size);

    /**
     * @brief Creates a chart containing data on memory allocations.
     * @return String containing the chart.
     */
    static String getMemoryUsage();
    static bool initialize(MemoryConfig config);
    static void shutdown();

    /**
     * @brief Gets the number of allocations that are currently stored in memory.
     * @return
     */
    static unsigned long getAllocationCount();
    static bool isInitialized(){return memorySystem != nullptr;}

    /**
     * @brief Adds the engines memory to the tracker without allocating it.
     * @param size Size of the engine.
     */
    static void trackEngineMemory(unsigned long size);
    static void untrackEngineMemory(unsigned long size);

    /**
     * @brief Allocates memory and creates a class.
     * @tparam T Type of class.
     * @param size Size of the memory.
     * @param tag Memory Tag.
     * @return Pointer to the new object.
     */
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

    /**
     * @brief Frees memory and destroys a class.
     * @tparam T Type of class.
     * @param block Pointer to memory.
     * @param size Size of the memory.
     * @param tag Memory tag.
     */
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
