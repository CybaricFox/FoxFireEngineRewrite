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

#define MAX_MATERIAL_COUNT 1024
#define MAX_GEOMETRY_COUNT 4096

enum TextureUseCase {
    TEXTURE_USE_UNKNOWN,
    TEXTURE_USE_MAP_DIFFUSE,
    TEXTURE_USE_MAP_SPECULAR,
    TEXTURE_USE_MAP_NORMAL
};

enum ResourceType {
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_STATIC_MESH,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_CUSTOM
};

enum RenderpassClearFlag {
    RENDERPASS_CLEAR_NONE = 0x0,
    RENDERPASS_CLEAR_COLOR = 0x1,
    RENDERPASS_CLEAR_DEPTH = 0x2,
    RENDERPASS_CLEAR_STENCIL = 0x4
};

enum ShaderAttributeType {
    SHADER_ATTRIBUTE_TYPE_FLOAT32 = 0U,
    SHADER_ATTRIBUTE_TYPE_FLOAT32_2 = 1U,
    SHADER_ATTRIBUTE_TYPE_FLOAT32_3 = 2U,
    SHADER_ATTRIBUTE_TYPE_FLOAT32_4 = 3U,
    SHADER_ATTRIBUTE_TYPE_MATRIX_4 = 4U,
    SHADER_ATTRIBUTE_TYPE_INT8 = 5U,
    SHADER_ATTRIBUTE_TYPE_UINT8 = 6U,
    SHADER_ATTRIBUTE_TYPE_INT16 = 7U,
    SHADER_ATTRIBUTE_TYPE_UINT16 = 8U,
    SHADER_ATTRIBUTE_TYPE_INT32 = 9U,
    SHADER_ATTRIBUTE_TYPE_UINT32 = 10U,
};

enum ShaderUniformType {
    SHADER_UNIFORM_TYPE_FLOAT32 = 0U,
    SHADER_UNIFORM_TYPE_FLOAT32_2 = 1U,
    SHADER_UNIFORM_TYPE_FLOAT32_3 = 2U,
    SHADER_UNIFORM_TYPE_FLOAT32_4 = 3U,
    SHADER_UNIFORM_TYPE_INT8 = 4U,
    SHADER_UNIFORM_TYPE_UINT8 = 5U,
    SHADER_UNIFORM_TYPE_INT16 = 6U,
    SHADER_UNIFORM_TYPE_UINT16 = 7U,
    SHADER_UNIFORM_TYPE_INT32 = 8U,
    SHADER_UNIFORM_TYPE_UINT32 = 9U,
    SHADER_UNIFORM_TYPE_MATRIX_4 = 10U,
    SHADER_UNIFORM_TYPE_SAMPLER = 11U,
    SHADER_UNIFORM_TYPE_CUSTOM = 255U
};

enum ShaderStage {
    SHADER_STAGE_VERTEX = 0x00000001,
    SHADER_STAGE_GEOMETRY = 0x00000002,
    SHADER_STAGE_FRAGMENT = 0x00000004,
    SHADER_STAGE_COMPUTE = 0x00000008
};

enum ShaderScope {
    SHADER_SCOPE_GLOBAL,
    SHADER_SCOPE_INSTANCE,
    SHADER_SCOPE_LOCAL
};

enum RendererBackendType {
    VULKAN,
    DIRECTX
};

struct Texture {
    unsigned int id = INVALID_ID_U32;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char channelCount = 0;
    bool bIsTransparent = false;
    unsigned int generation = INVALID_ID_U32;
    String name{};
    void* data = nullptr;
};

struct TextureMap {
    Texture* texture = nullptr;
    TextureUseCase use{};
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
};

struct Geometry {
    unsigned int id = INVALID_ID_U32;
    unsigned int internalId = INVALID_ID_U32;
    unsigned int generation = INVALID_ID_U32;
    String name{};
    Material* material = nullptr;
};

struct Resource {
    unsigned int loaderId = INVALID_ID_U32;
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
    String shaderName{};
    bool bAutoRelease = false;
    Vector4f diffuseColor{};
    String diffuseName{};
    String specularName{};
    String normalName{};
    float shine = 0;
};

struct RenderpassProfile {
    String name{};
    unsigned int id = INVALID_ID_U32;
    unsigned char clearFlags = 0;
    Vector4f clearColor{};
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

/**
 * @brief Per frame packer containing geometry data.
 */
struct RenderPacket {
    float deltaTime;
    unsigned int geometryCount;
    GeometryRenderData* geometries;
    unsigned int uiGeometryCount;
    GeometryRenderData* uiGeometries;
};

