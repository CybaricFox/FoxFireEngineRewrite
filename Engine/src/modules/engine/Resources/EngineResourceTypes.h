//
// Created by cmorg on 7/28/2026.
//

#pragma once
#include "src/defines.h"

enum TextureUseCase {
    TEXTURE_USE_UNKNOWN,
    TEXTURE_USE_MAP_DIFFUSE
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


