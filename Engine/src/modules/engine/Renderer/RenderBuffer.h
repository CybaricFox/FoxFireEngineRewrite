//
// Created by cmorg on 10/4/2026.
//

#pragma once
#include "src/defines.h"
#include "src/modules/engine/Memory/FreeList.h"

/**
 *  @file RenderBuffer.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 10/4/2026
 *
 *  @copyright (c) 2026
 */

enum RenderBufferType {
    RENDERBUFFER_TYPE_UNKNOWN,
    RENDERBUFFER_TYPE_VERTEX,
    RENDERBUFFER_TYPE_INDEX,
    RENDERBUFFER_TYPE_UNIFORM,
    RENDERBUFFER_TYPE_STAGING,
    RENDERBUFFER_TYPE_READ,
    RENDERBUFFER_TYPE_STORAGE
};

struct IRenderBuffer {};

class RenderBuffer {
private:
    RenderBufferType type = RENDERBUFFER_TYPE_UNKNOWN;
    ULong totalSize = 0;
    ULong freeListMemoryRequirement = 0;
    FreeList bufferFreeList{};
    void* memory = nullptr;
    IRenderBuffer* data = nullptr;

public:
    void initialize(RenderBufferType newType, ULong newSize, bool useFreeList);
    void shutdown();

    [[nodiscard]] IRenderBuffer* getInternalBuffer() const {return data;}
    RenderBufferType getType() const {return type;}
    ULong getTotalSize() const {return totalSize;}

    void setInternalBuffer(IRenderBuffer* newData) {data = newData;}
    void setTotalSize(const ULong newSize) {totalSize = newSize;}

    bool allocateBuffer(ULong size, ULong &outOffset);
    bool freeBuffer(ULong size, ULong offset);
    bool resizeBuffer(ULong newSize);
};