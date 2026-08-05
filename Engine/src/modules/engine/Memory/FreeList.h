/**
 *  @file FreeList.h
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
#include "src/modules/engine/Library/Logger.h"

/**
 * @brief Used within FreeLists to track free memory allocations within the list.
 */
struct FreeListNode {
    unsigned long offset = 0;
    unsigned long size = 0;
    FreeListNode* next = nullptr;
};

/**
 * @brief A FreeList allocates memory data for tracking purposes. A Free List tracks freed memory
 * to be re-allocated in the future.
 */
class FreeList {
private:
    /** @brief The total size of the FreeList's memory block*/
    unsigned long totalSize = 0;
    /** @brief The maximum number of nodes*/
    unsigned long maxEntries = 0;
    /** @brief Pointer to the first node in the list*/
    FreeListNode* first = nullptr;
    /** @brief The part of the memory block that contains the list of nodes.*/
    FreeListNode* nodes = nullptr;
    //FreeList does not own its memory.
    //FreeList itself is stored at the start of the block.
    /** @brief The Free List's memory block.*/
    void* memoryBlock = nullptr;

    FreeList() = default;

    /**
     * @brief Gets the next free node in the FreeList.
     * @return Pointer to the next free node.
     */
    [[nodiscard]] FreeListNode* getNode() const {
        for (unsigned long i = 1; i < maxEntries; i++) {
            if (nodes[i].size == 0) {
                nodes[i].next = nullptr;
                nodes[i].offset = 0;
                return &nodes[i];
            }
        }

        return nullptr;
    }

    /**
     * @brief clears all nodes and resets the list
     */
    void clear() const;

    /**
     * @brief Resets a node
     * @param node The node to reset
     */
    void freeNode(FreeListNode& node);

public:
    ~FreeList() = default;

    /**
     * @brief Destructs the Free List and clears the memory allocation. Does not de-allocate the memory block!
     */
    void shutdown() const;

    /**
     * Gets the memory requirement of the FreeList
     * @param size Size of the allocated memory.
     * @return Number of bytes required in memory.
     */
    static unsigned long calculateMemoryRequirement(const unsigned long size) {
        const unsigned long entries = size / (sizeof(void*) * sizeof(FreeListNode));
        return sizeof(FreeList) + (sizeof(FreeListNode) * entries);
    }

    /**
     * @brief Creates a free list using an allocated block of memory.
     * @param size Size of the memory block
     * @param memoryRequirement Required size of the memory block to create this FreeList
     * @param memory The memory allocation
     * @return A pointer to a FreeList
     */
    static FreeList* createFreeList(unsigned long size, unsigned long memoryRequirement, void* memory);

    /**
     * @brief Returns the amount of free space left in this FreeList.
     * @return remaining bytes
     */
    [[nodiscard]] unsigned long getFreeSpace() const {
        if (memoryBlock == nullptr) return 0;

        unsigned long runningTotal = 0;
        const FreeListNode* node = first;
        while (node) {
            runningTotal += node->size;
            node = node->next;
        }

        return runningTotal;
    }

    /**
     * @brief Allocates memory allocation data to the list.
     * @param size size of the memory allocation
     * @param offset OUT number of bytes from the first node
     * @return True on success, false on failure
     */
    bool allocate(unsigned long size, unsigned long &offset);

    /**
    * @brief Frees a node in the list
    * @param size size of the allocation
    * @param offset number of bytes from the first node
    * @return True on success, False on failure
    */
    bool free(unsigned long size, unsigned long offset);
};