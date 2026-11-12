//
// Created by cmorg on 10/4/2026.
//

#include "RenderBuffer.h"

#include "src/modules/engine/Memory/FF_Memory.h"

void RenderBuffer::initialize(const RenderBufferType newType, const ULong newSize, const bool useFreeList) {
    type = newType;
    totalSize = newSize;

    if (useFreeList) {
        freeListMemoryRequirement = FreeList::calculateMemoryRequirement(newSize);
        memory = FF_Memory::ff_allocate_storage(freeListMemoryRequirement, RENDER);
        bufferFreeList.initialize(newSize, freeListMemoryRequirement, memory);
    }
}

void RenderBuffer::shutdown() {
    if (freeListMemoryRequirement > 0) {
        bufferFreeList.shutdown();
        FF_Memory::ff_free_storage(memory, freeListMemoryRequirement, RENDER);
        freeListMemoryRequirement = 0;
    }

    memory = nullptr;
}

bool RenderBuffer::allocateBuffer(const ULong size, ULong &outOffset) {
    if (freeListMemoryRequirement == 0) {
        Logger::logWarn("Allocate called on render buffer with no free list. Offset will not be valid. Call loadRenderBufferRange instead.");
        outOffset = 0;
        return true;
    }

    return bufferFreeList.allocate(size, outOffset);
}

bool RenderBuffer::freeBuffer(const ULong size, const ULong offset) {
    if (freeListMemoryRequirement == 0) {
        Logger::logWarn("Free called on render buffer with no free list.");
        return true;
    }

    return bufferFreeList.free(size, offset);
}

bool RenderBuffer::resizeBuffer(const ULong newSize) {
    if (newSize <= totalSize) {
        Logger::logError("Render buffer resize cannot resize because the new size " + toString(newSize) + " is less than or equal to the buffer size " + toString(totalSize));
        return false;
    }

    if (freeListMemoryRequirement > 0) {
        const ULong newMemoryRequirement = FreeList::calculateMemoryRequirement(newSize);
        void* newBlock = FF_Memory::ff_allocate_storage(newMemoryRequirement, RENDER);
        void* oldBlock = nullptr;
        if (!bufferFreeList.resize(newBlock, newSize, oldBlock)) {
            Logger::logError("Failed to resize render buffer");
            FF_Memory::ff_free_storage(newBlock, newMemoryRequirement, RENDER);
            return false;
        }

        FF_Memory::ff_free_storage(oldBlock, freeListMemoryRequirement, RENDER);
        freeListMemoryRequirement = newMemoryRequirement;
        memory = newBlock;
    }

    return true;
}
