/**
*   @file DynamicAllocator.h
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
#include "FreeList.h"

/**
 * @brief Memory Allocator for allocating different size objects to one area of memory. It saves a pointer to every
 * allocation and reuses freed space.
 */
class DynamicAllocator {
private:
    DynamicAllocator() = default;

    void* allocatorMemory = nullptr;
    void* memoryBlock = nullptr;
    unsigned long totalSize = 0;
    FreeList* freeList = nullptr;
    void* freeListMemory = nullptr;

public:
    ~DynamicAllocator() = default;
    void shutdown();

    static unsigned long getMemoryRequirement(unsigned long size);
    static DynamicAllocator *createDynamicAllocator(unsigned long size, void *memory);

    [[nodiscard]] unsigned long getFreeSpace() const {
        return freeList->getFreeSpace();
    }

    [[nodiscard]] void* allocate(unsigned long size) const;
    bool free(void* memory, unsigned long size) const;

};
