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
#include "src/modules/engine/Threads/Mutex.h"

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
    RING_QUEUE,
    JOB,
    RENDER_BACKEND_MANUAL,
    RENDER_BACKEND_AUTO,
    GPU,
    STRING,
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
    Mutex allocationMutex{};

    FF_Memory() = default;

    static String getStringFromTag(unsigned long tag);
    static String getUnitForSize(ULong size, float& outAmount);

public:
    ~FF_Memory() = default;

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
    static bool getSizeAndAlignment(void* memory, ULong& size, unsigned short& alignment);

    static bool initialize(MemoryConfig config);
    static void shutdown();

    /**
     * @brief Gets the number of allocations that are currently stored in memory.
     * @return
     */
    static unsigned long getAllocationCount();
    static bool isInitialized(){return memorySystem != nullptr;}
    static void reportAllocation(ULong size, MemoryTag tag);
    static void removeReport(ULong size, MemoryTag tag);

    /**
     * @brief Allocates memory.
     * @param size Size of the memory.
     * @param tag The type of memory. Used for tracking.
     * @param alignment
     * @return Pointer to the memory location.
     */
    static void* ff_allocate_raw(unsigned long size, MemoryTag tag, unsigned short alignment = 1);

    /**
     * @brief Frees memory.
     * @param block Pointer to the memory location.
     * @param size Size of the memory.
     * @param tag The type of memory.
     * @param alignment
     */
    static void ff_free_raw(void* block, unsigned long size, MemoryTag tag, unsigned short alignment = 1);

    /**
     * @brief Allocates memory and creates a class.
     * @tparam T Type of class.
     * @param tag Memory Tag.
     * @param size Optional size. Use this for allocating space for a derived class but only constructing the base class atm.
     * @return Pointer to the new object.
     */
    template<typename T>
    static T* ff_allocate(const MemoryTag tag, ULong size = 0) {
        if (size < sizeof(T)) size = sizeof(T);

        //Allocate the memory block
        void* destination = ff_allocate_raw(size, tag, alignof(T));
        if (!destination) return nullptr;
        //construct the class
        T* result = static_cast<T *>(destination);

        return std::construct_at(result);
    }

    /**
     * @brief Frees memory and destroys a class.
     * @tparam T Type of class.
     * @param block Pointer to memory.
     * @param tag Memory tag.
     * @param size Should be set if freeing a parent class instead of the derived class.
     */
    template<typename T>
    static void ff_free(void* block, const MemoryTag tag, ULong size = 0) {
        if (!block) return;

        if (size < sizeof(T)) size = sizeof(T);

        std::destroy_at(static_cast<T*>(block));
        ff_free_raw(block, size, tag, alignof(T));
    }

    /**
     * @brief Allocates memory and creates a class multiple times.
     * @tparam T Type of class.
     * @param tag Memory Tag.
     * @param count Number of classes to create
     * @return Pointer to the new object.
     */
    template<typename T>
    static T* ff_allocate_recursive(const MemoryTag tag, const unsigned int count) {
        //Allocate the memory block
        void* destination = ff_allocate_raw(sizeof(T) * count, tag, alignof(T));
        if (!destination) return nullptr;

        T* origin = static_cast<T *>(destination);

        for (unsigned int i = 0; i < count; i++) {
            std::construct_at(&origin[i]);
        }

        return origin;
    }

    /**
     * @brief Frees memory and destroys multiple classes.
     * @tparam T Type of class.
     * @param block Pointer to memory.
     * @param tag Memory tag.
     * @param count Number of classes to destroy.
     */
    template<typename T>
    static void ff_free_recursive(void* block, const MemoryTag tag, const unsigned int count) {
        if (!block) return;

        T* origin = static_cast<T *>(block);

        for (unsigned int i = 0; i < count; i++) {
            std::destroy_at(&origin[i]);
        }

        ff_free_raw(block, sizeof(T) * count, tag, alignof(T));
    }

    /**
     * @brief Allocates a chunk of memory and does not create any classes. This should be used for memory containers like dynamic array.
     * @param size
     * @param tag
     * @param alignment
     * @return
     */
    static void* ff_allocate_storage(const ULong size, const MemoryTag tag, const unsigned short alignment = 1) {
        return ff_allocate_raw(size, tag, alignment);
    }

    /**
     * @brief Frees a chunk of memory. Does not destroy anything.
     * @tparam T
     * @param block
     * @param size
     * @param tag
     * @param alignment
     */
    static void ff_free_storage(void* block, const ULong size, const MemoryTag tag, const unsigned short alignment = 1) {
        ff_free_raw(block, size, tag, alignment);
    }
};
