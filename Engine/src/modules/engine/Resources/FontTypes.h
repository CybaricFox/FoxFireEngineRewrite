//
// Created by cmorg on 10/14/2026.
//

/**
*   @file FontTypes.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief Various structures used throughout the engine
 *  @version 1.0
 *  @date 10-14-2026
 *
 *  @copyright (c) 2026
 */

#pragma once
#include "EngineTextureTypes.h"
#include "src/modules/engine/Library/String.h"

enum FontType {
    FONT_TYPE_BITMAP,
    FONT_TYPE_SYSTEM
};

struct FontGlyph {
    int codepoint = 0;
    unsigned short x = 0;
    unsigned short y = 0;
    unsigned short width = 0;
    unsigned short height = 0;
    short xOffset = 0;
    short yOffset = 0;
    float advance = 0;
    unsigned char page = 0;
};

struct FontKerning {
    int codepointA = 0;
    int codepointB = 0;
    float amount = 0;
};

struct FontData {
    FontType type{};
    String name{};
    unsigned int size = 0;
    int height = 0;
    int base = 0;
    int atlasSizeX = 0;
    int atlasSizeY = 0;
    TextureMap atlas{};
    unsigned int glyphCount = 0;
    FontGlyph* glyphs = nullptr;
    unsigned int kerningCount = 0;
    FontKerning* kernings = nullptr;
    float tabAdvance = 0;
    unsigned int dataSize = 0;
    void* data = nullptr;
};

struct FontPage {
    char id = 0;
    String file{};
};

struct BitmapFontResourceData {
    FontData data{};
    unsigned int pageCount = 0;
    FontPage* pages = nullptr;
};
