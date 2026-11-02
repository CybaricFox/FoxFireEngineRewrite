//
// Created by cmorg on 11/1/2026.
//

#pragma once
#include "src/modules/engine/Library/String.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Resources/EngineTextureTypes.h"
#include "src/modules/engine/Resources/FontTypes.h"
#include "src/modules/engine/Resources/IResourceLoader.h"

/**
 *  @file SystemFontLoader.h
 *  @layer Engine
 *  @module Resources
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 11/1/2026
 *
 *  @copyright (c) 2026
 */

enum SystemFontFileType {
    SYSTEM_FONT_FILE_TYPE_NOT_FOUND,
    SYSTEM_FONT_FILE_TYPE_FSFB,
    SYSTEM_FONT_FILE_TYPE_FOX_SYSTEM_FONT
};

struct SupportedSystemFontFileType {
    String extensions{};
    SystemFontFileType type{};
    bool bIsBinary = false;
};

class SystemFontLoader final : public IResourceLoader {
private:
    bool importFoxSystemFont(FileHandler &file, const String &outName, String directoryPath, SystemFontResourceData &outResource);
    bool readFSFB(FileHandler& file, SystemFontResourceData& data);
    bool writeFSFB(String outFileName, SystemFontResourceData& resource);


public:
    SystemFontLoader();

    bool load(String name, Resource &outResource, String basePath, ILoaderParameters *params) override;
    void unload(Resource &resource) override;
};