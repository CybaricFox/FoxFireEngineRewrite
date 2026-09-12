/**
*   @file EngineTextureTypes.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief Various structures for texture related resources
 *  @version 1.0
 *  @date 09-11-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "src/defines.h"

enum TextureUseCase {
    TEXTURE_USE_UNKNOWN,
    TEXTURE_USE_MAP_DIFFUSE,
    TEXTURE_USE_MAP_SPECULAR,
    TEXTURE_USE_MAP_NORMAL
};

enum TextureRepeat {
    TEXTURE_REPEAT = 0x1,
    TEXTURE_MIRRORED_REPEAT = 0x2,
    TEXTURE_CLAMP_TO_EDGE = 0x3,
    TEXTURE_CLAMP_TO_BORDER = 0x4
};

enum TextureFilter {
    TEXTURE_FILTER_NEAREST = 0x0,
    TEXTURE_FILTER_BILINEAR = 0x1
};

enum TextureFlag {
    TEXTURE_BIT_TRANSPARENT = 0x1,
    TEXTURE_BIT_WRITABLE = 0x2,
    TEXTURE_BIT_WRAPPED = 0x4
};
typedef unsigned char TextureFlagBits;

struct Texture {
    unsigned int id = INVALID_ID_U32;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char channelCount = 0;
    TextureFlagBits flags = 0;
    unsigned int generation = INVALID_ID_U32;
    String name{};
    void* data = nullptr;
};

struct TextureMap {
    Texture* texture = nullptr;
    TextureUseCase use{};
    TextureFilter filterMin{};
    TextureFilter filterMag{};
    TextureRepeat repeatU{};
    TextureRepeat repeatV{};
    TextureRepeat repeatW{};
    void* data = nullptr;
};

struct ImageResourceData {
    unsigned char channelCount = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char* pixels = nullptr;
};