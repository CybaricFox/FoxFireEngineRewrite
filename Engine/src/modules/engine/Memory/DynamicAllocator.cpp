//
// Created by cmorg on 8/5/2026.
//

#include "DynamicAllocator.h"

#include "FF_Memory.h"

bool DynamicAllocator::getSizeAndAlignment(void *memory, ULong &outSize, unsigned short &outAlignment) {
    outSize = *reinterpret_cast<unsigned int *>(reinterpret_cast<ULong>(memory) - sizeof(unsigned int));

    const AllocationHeader& header = *reinterpret_cast<AllocationHeader *>(reinterpret_cast<ULong>(memory) + outSize);

    outAlignment = header.alignment;

    return true;
}

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

void * DynamicAllocator::allocate(const unsigned long size, const unsigned short alignment) {
    if (!memoryBlock) {
        Logger::logFatal("Dynamic Allocator memory is null!");
        return nullptr;
    }

    if (alignment == 0) {
        Logger::logError("Dynamic Allocator alignment cannot be 0!");
        return nullptr;
    }

    const ULong requiredSize = alignment + sizeof(AllocationHeader) + sizeof(unsigned int) + size;

    if (requiredSize > 4294967295U) {
        Logger::logFatal("Dynamic allocator allocation called with a size greater than 4GB. What in the actual fuck are you doing??? Not doing that, fuck you and your application.");
        return nullptr;
    }

    ULong baseOffset = 0;

    //Allocate from free list
    if (freeList.allocate(requiredSize, baseOffset)) {
        const auto memory = reinterpret_cast<void *>(static_cast<unsigned char*>(memoryBlock) + baseOffset);
        const ULong alignedOffset = alignMemory(reinterpret_cast<ULong>(memory) + sizeof(unsigned int), alignment);
        const auto memorySize = reinterpret_cast<unsigned int *>(alignedOffset - sizeof(unsigned int));

        *memorySize = size;

        AllocationHeader& header = *reinterpret_cast<AllocationHeader *>(alignedOffset + size);

        header.start = memory;
        header.alignment = alignment;

        return reinterpret_cast<void *>(alignedOffset);
    }

    Logger::logError("Dynamic Allocator cannot find a memory block large enough to allocate from.");
    const unsigned long available = freeList.getFreeSpace();
    Logger::logError("Requested size: " + std::to_string(size) + " Available: " + std::to_string(available));
    return nullptr;
}

bool DynamicAllocator::free(void *memory) {
    if (memory == nullptr) {
        Logger::logError("Dynamic Allocator requires a valid memory block to free!");
        return false;
    }

    if (memory < memoryBlock || memory > static_cast<unsigned char *>(memoryBlock) + totalSize) {
        void* endOfBlock = static_cast<unsigned char *>(memoryBlock) + totalSize;
        Logger::logError("Dynamic Allocator tryed to free memory block: " +
            std::to_string(reinterpret_cast<ULong>(memory)) + " outside of range: " +
            std::to_string(reinterpret_cast<ULong>(memoryBlock)) + " - " +
            std::to_string(reinterpret_cast<ULong>(endOfBlock)));
        return false;
    }

    const auto* memorySize = reinterpret_cast<unsigned int *>(static_cast<unsigned char *>(memory) - sizeof(unsigned int));
    AllocationHeader& header = *reinterpret_cast<AllocationHeader *>(static_cast<unsigned char *>(memory) + *memorySize);
    const ULong requiredSize = header.alignment + sizeof(AllocationHeader) + sizeof(unsigned int) + *memorySize;
    const ULong offset = reinterpret_cast<ULong>(header.start) - reinterpret_cast<ULong>(memoryBlock);

    if (!freeList.free(requiredSize, offset)) {
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
