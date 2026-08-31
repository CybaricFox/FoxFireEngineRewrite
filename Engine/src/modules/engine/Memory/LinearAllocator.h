/**
*   @file LinearAllocator.h
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

/**
 * @brief Stores data in a block of memory. The allocator can only free the entire block and does not track allocation sizes.
 */
class LinearAllocator {
private:
    unsigned long totalSize = 0;
    unsigned long allocated = 0;
    void* block = nullptr;

public:
    LinearAllocator() = default;
    explicit LinearAllocator(const unsigned long size) {initialize(size);}
    ~LinearAllocator();
    void initialize(unsigned long size);
    void shutdown();

    void* allocate(unsigned long size);
    void freeAll();
};