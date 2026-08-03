//
// Created by cmorg on 7/28/2026.
//

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
};


