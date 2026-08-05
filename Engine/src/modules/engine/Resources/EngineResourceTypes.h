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
#include "src/defines.h"
#include "src/modules/engine/Library/FF_Math.h"

enum TextureUseCase {
    TEXTURE_USE_UNKNOWN,
    TEXTURE_USE_MAP_DIFFUSE
};

enum ResourceType {
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_CUSTOM
};

enum RenderpassClearFlag {
    RENDERPASS_CLEAR_NONE = 0x0,
    RENDERPASS_CLEAR_COLOR = 0x1,
    RENDERPASS_CLEAR_DEPTH = 0x2,
    RENDERPASS_CLEAR_STENCIL = 0x4
};

enum EngineRenderpasses {
    ENGINE_RENDER_PASS_WORLD = 0x1,
    ENGINE_RENDER_PASS_UI = 0x2
};

enum MaterialType {
    MATERIAL_TYPE_WORLD,
    MATERIAL_TYPE_UI
};

struct Texture {
    unsigned int id = INVALID_ID;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char channelCount = 0;
    bool bIsTransparent = false;
    unsigned int generation = INVALID_ID;
    String name{};
    void* data = nullptr;
};

struct TextureMap {
    Texture* texture = nullptr;
    TextureUseCase use{};
};

struct Material {
    String name{};
    unsigned int id = INVALID_ID;
    unsigned int generation = INVALID_ID;
    unsigned int internalId = INVALID_ID;
    Vector4f diffuseColor{};
    TextureMap diffuseMap{};
    MaterialType materialType{};
};

struct Geometry {
    unsigned int id = INVALID_ID;
    unsigned int internalId = INVALID_ID;
    unsigned int generation = INVALID_ID;
    String name{};
    Material* material = nullptr;
};

struct Resource {
    unsigned int loaderId = INVALID_ID;
    String name{};
    String path{};
    unsigned long dataSize = 0;
    void* data = nullptr;
};

struct ImageResourceData {
    unsigned char channelCount = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char* pixels = nullptr;
};

struct MaterialResourceData {
    String name{};
    bool bAutoRelease = false;
    Vector4f diffuseColor{};
    String mapName{};
    MaterialType materialType{};
};

struct RenderpassProfile {
    EngineRenderpasses id{};
    unsigned char clearFlags = 0;
    Vector4f clearColor{};
};

struct RenderSystemProfile {
    EngineRenderpasses type{};
    bool bIs2D = false;
    bool bDepthTestEnabled = false;
};

struct GeometryRenderData {
    Mat4 model;
    Geometry* geometry;
};

struct GeometryData {
    unsigned int id = INVALID_ID;
    unsigned int generation = INVALID_ID;
    unsigned int vertexCount = 0;
    unsigned long vertexBufferOffset = 0;
    unsigned int vertexElementSize = 0;
    unsigned int indexCount = 0;
    unsigned long indexBufferOffset = 0;
    unsigned int indexElementSize = 0;
};


