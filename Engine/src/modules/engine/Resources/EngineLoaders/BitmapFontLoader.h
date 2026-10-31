//
// Created by cmorg on 10/14/2026.
//

#pragma once
#include "src/modules/engine/Resources/FontTypes.h"
#include "src/modules/engine/Resources/IResourceLoader.h"

/**
 *  @file BitmapFontLoader.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 10/14/2026
 *
 *  @copyright (c) 2026
 */

enum BitmapFontFileType {
    BITMAP_FONT_FILE_NOT_FOUND,
    BITMAP_FONT_FILE_FOXFIRE_BINARY,
    BITMAP_FONT_FILE_FOXFONT
};

struct SupportedBitmapFontFileType {
    String extension{};
    BitmapFontFileType type{};
    bool bIsBinary = false;
};

class BitmapFontLoader final : public IResourceLoader{
private:
    bool importFoxFont(const FileHandler& file, const String& name, BitmapFontResourceData& outData);
    bool readFoxfireBinaryFont(FileHandler& file, BitmapFontResourceData& data);
    bool writeFoxfireBinaryFont(const String& filePath, const BitmapFontResourceData& data);

public:
    BitmapFontLoader();

    bool load(String name, Resource &outResource, String basePath, ILoaderParameters *params) override;

    void unload(Resource &resource) override;
};