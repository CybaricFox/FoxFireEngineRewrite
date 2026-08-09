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
    /** @brief The memory available for allocation. Starts after the free list nodes */
    void* memoryBlock = nullptr;
    unsigned long totalSize = 0;
    FreeList freeList{};

public:
    DynamicAllocator() = default;
    DynamicAllocator(const unsigned long size, void *memory) {initialize(size, memory);}
    ~DynamicAllocator() = default;

    void initialize(unsigned long size, void *memory);
    void shutdown();

    static unsigned long getMemoryRequirement(unsigned long size);

    [[nodiscard]] unsigned long getFreeSpace() const {
        return freeList.getFreeSpace();
    }

    [[nodiscard]] void* allocate(unsigned long size);
    bool free(void* memory, unsigned long size);

};
