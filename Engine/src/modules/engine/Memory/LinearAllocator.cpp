//
// Created by cmorg on 7/6/2026.
//

#include "LinearAllocator.h"

#include "FF_Memory.h"
#include "src/modules/engine/Library/Logger.h"

LinearAllocator::LinearAllocator(const unsigned long size, void *memory) {
    totalSize = size;
    allocated = 0;
    bOwnsMemory = memory == nullptr;

    if (memory) {
        block = memory;
    } else {
        block = FF_Memory::ff_allocate(totalSize, LINEAR_ALLOCATOR);
    }
}

LinearAllocator::~LinearAllocator() {
    allocated = 0;

    if (bOwnsMemory && block) {
        FF_Memory::ff_free(block, totalSize, LINEAR_ALLOCATOR);
    }

    block = nullptr;
    totalSize = 0;
    bOwnsMemory = false;
}

bool LinearAllocator::allocate(const unsigned long size) {
    if (block) {
        //Overflow
        if (allocated + size > totalSize) {
            const unsigned long remaining = totalSize - allocated;
            Logger::logError("Failed to allocate: " + std::to_string(size) + "B! Not enough memory space allocated! Remaining size: " + std::to_string(remaining) + "B!");
            return false;
        }

        block = &block + allocated;
        allocated += size;
        return true;
    }

    Logger::logError("Linear allocator failed to allocate! block was not initialized!");
    return false;
}

void LinearAllocator::freeAll() {
    if (block) {
        allocated = 0;
        FF_Memory::ff_clear(block, totalSize);
    }
}
