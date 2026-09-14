/**
*   @file EngineTypes.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief Various structures used throughout the engine
 *  @version 1.0
 *  @date 09-11-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#define MAX_MATERIAL_COUNT 1024
#define MAX_GEOMETRY_COUNT 4096
#include "EngineTextureTypes.h"
#include "src/defines.h"
#include "src/modules/engine/Library/FF_Math.h"

enum RendererBackendType {
    VULKAN,
    DIRECTX
};

struct Material {
    String name{};
    unsigned int id = INVALID_ID_U32;
    unsigned int generation = INVALID_ID_U32;
    unsigned int internalId = INVALID_ID_U32;
    Vector4f diffuseColor{};
    TextureMap diffuseMap{};
    TextureMap specularMap{};
    TextureMap normalMap{};
    float shine = 0;
    unsigned int shaderId = INVALID_ID_U32;
    unsigned int frameNumber = INVALID_ID_U32;
};

struct Geometry {
    unsigned int id = INVALID_ID_U32;
    unsigned int internalId = INVALID_ID_U32;
    unsigned short generation = INVALID_ID_U16;
    Vector3f center{};
    Extent3D extent{};
    String name{};
    Material* material = nullptr;
};