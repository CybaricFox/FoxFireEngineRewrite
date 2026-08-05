//
// Created by cmorg on 8/5/2026.
//

#include "DynamicAllocator.h"

#include "FF_Memory.h"

DynamicAllocator* DynamicAllocator::createDynamicAllocator(const unsigned long size, void *memory) {
    if (!memory) return nullptr;

    if (size == 0) {
        Logger::logError("Dynamic Allocator cannot have a size of 0!");
        return nullptr;
    }

    const unsigned long freeListRequirement = FreeList::calculateMemoryRequirement(size);


    const auto allocator = new (memory) DynamicAllocator();
    allocator->allocatorMemory = memory;
    allocator->totalSize = size;
    allocator->freeListMemory = static_cast<unsigned char *>(allocator->allocatorMemory) + sizeof(DynamicAllocator);
    allocator->memoryBlock = static_cast<unsigned char *>(allocator->freeListMemory) + freeListRequirement;

    allocator->freeList = FreeList::createFreeList(size, freeListRequirement, allocator->freeListMemory);
    FF_Memory::ff_clear(allocator->memoryBlock, size);
    return allocator;
}

void DynamicAllocator::shutdown() {
    if (allocatorMemory) {
        freeList->shutdown();
        FF_Memory::ff_clear(memoryBlock, totalSize);
        totalSize = 0;

        void* mem = allocatorMemory;
        std::destroy_at(static_cast<DynamicAllocator *>(mem));
    }
}

void * DynamicAllocator::allocate(const unsigned long size) const {
    unsigned long offset = 0;

    //Allocate from free list
    if (freeList->allocate(size, offset)) {
        void* memory = static_cast<unsigned char *>(memoryBlock) + offset;
        return memory;
    }

    Logger::logError("Dynamic Allocator cannot find a memory block large enough to allocate from.");
    const unsigned long available = freeList->getFreeSpace();
    Logger::logError("Requested size: " + std::to_string(size) + " Available: " + std::to_string(available));
    return nullptr;
}

bool DynamicAllocator::free(void *memory, const unsigned long size) const {
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
    if (!freeList->free(size, offset)) {
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
    return freeListRequirement + sizeof(DynamicAllocator) + size;
}
