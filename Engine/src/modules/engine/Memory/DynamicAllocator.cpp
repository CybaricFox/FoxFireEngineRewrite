//
// Created by cmorg on 8/5/2026.
//

#include "DynamicAllocator.h"

#include "FF_Memory.h"

void DynamicAllocator::initialize(const unsigned long size, void *memory) {
    if (!memory) return;

    if (size == 0) {
        Logger::logError("Dynamic Allocator cannot have a size of 0!");
        return;
    }

    const unsigned long freeListRequirement = FreeList::calculateMemoryRequirement(size);

    totalSize = size;
    memoryBlock = static_cast<unsigned char *>(memory) + freeListRequirement;

    freeList.initialize(size, freeListRequirement, memory);
    FF_Memory::ff_clear(memoryBlock, size);
}

void DynamicAllocator::shutdown() {
    freeList.shutdown();
    FF_Memory::ff_clear(memoryBlock, totalSize);
    totalSize = 0;
}

void * DynamicAllocator::allocate(const unsigned long size) {
    unsigned long offset = 0;

    //Allocate from free list
    if (freeList.allocate(size, offset)) {
        void* memory = static_cast<unsigned char *>(memoryBlock) + offset;
        return memory;
    }

    Logger::logError("Dynamic Allocator cannot find a memory block large enough to allocate from.");
    const unsigned long available = freeList.getFreeSpace();
    Logger::logError("Requested size: " + std::to_string(size) + " Available: " + std::to_string(available));
    return nullptr;
}

bool DynamicAllocator::free(void *memory, const unsigned long size) {
    if (memory == nullptr) {
        Logger::logError("Dynamic Allocator requires a valid memory block to free!");
        return false;
    }

    if (memory < memoryBlock || memory > static_cast<unsigned char *>(memoryBlock) + totalSize) {
        void* endOfBlock = static_cast<unsigned char *>(memoryBlock) + totalSize;
        Logger::logError("Dynamic Allocator tryed to free memory block: " +
            std::to_string(*static_cast<unsigned char *>(memory)) + " outside of range: " +
            std::to_string(*static_cast<unsigned char *>(memoryBlock)) + " - " +
            std::to_string(*static_cast<unsigned char *>(endOfBlock)));
        return false;
    }

    const unsigned long offset = static_cast<unsigned char *>(memory) - static_cast<unsigned char *>(memoryBlock);
    if (!freeList.free(size, offset)) {
        Logger::logError("Dynamic Allocator failed to free memory block");
        return false;
    }

    return true;
}

unsigned long DynamicAllocator::getMemoryRequirement(const unsigned long size) {
    if (size == 0) {
        Logger::logError("Dynamic Allocator cannot have a size of 0!");
        return false;
    }

    const unsigned long freeListRequirement = FreeList::calculateMemoryRequirement(size);
    return freeListRequirement + size;
}
