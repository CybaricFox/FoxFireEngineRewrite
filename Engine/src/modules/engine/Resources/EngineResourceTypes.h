/**
*   @file EngineResourceTypes.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief Various structures of resourcse
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "EngineTypes.h"
#include "src/defines.h"
#include "src/modules/engine/Library/FF_Math.h"

enum ResourceType {
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_MESH,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_CUSTOM
};

struct Resource {
    unsigned int loaderId = INVALID_ID_U32;
    String name{};
    String path{};
    unsigned long dataSize = 0;
    void* data = nullptr;
};

struct MaterialResourceData {
    String name{};
    String shaderName{};
    bool bAutoRelease = false;
    Vector4f diffuseColor{};
    String diffuseName{};
    String specularName{};
    String normalName{};
    float shine = 0;
};

struct GeometryRenderData {
    Mat4 model;
    Geometry* geometry;
};

struct GeometryData {
    unsigned int id = INVALID_ID_U32;
    unsigned int generation = INVALID_ID_U32;
    unsigned int vertexCount = 0;
    unsigned long vertexBufferOffset = 0;
    unsigned int vertexElementSize = 0;
    unsigned int indexCount = 0;
    unsigned long indexBufferOffset = 0;
    unsigned int indexElementSize = 0;
};