//
// Created by cmorg on 7/6/2026.
//

#include "LinearAllocator.h"

#include "FF_Memory.h"
#include "src/modules/engine/Library/Logger.h"

LinearAllocator::~LinearAllocator() {
    shutdown();
}

void LinearAllocator::initialize(const unsigned long size) {
    totalSize = size;
    allocated = 0;

    if (!block) {
        block = FF_Memory::ff_allocate(totalSize, LINEAR_ALLOCATOR);
    }
}

void LinearAllocator::shutdown() {
    allocated = 0;

    if (block) {
        FF_Memory::ff_free(block, totalSize, LINEAR_ALLOCATOR);
    }

    block = nullptr;
    totalSize = 0;
}

void* LinearAllocator::allocate(const unsigned long size) {
    if (block) {
        //Overflow
        if (allocated + size > totalSize) {
            const unsigned long remaining = totalSize - allocated;
            Logger::logError("Failed to allocate: " + std::to_string(size) + "B! Not enough memory space allocated! Remaining size: " + std::to_string(remaining) + "B!");
            return nullptr;
        }

        void* result = static_cast<unsigned char*>(block) + allocated;
        allocated += size;
        return result;
    }

    Logger::logError("Linear allocator failed to allocate! block was not initialized!");
    return nullptr;
}

void LinearAllocator::freeAll() {
    if (block) {
        allocated = 0;
        FF_Memory::ff_clear(block, totalSize);
    }
}
