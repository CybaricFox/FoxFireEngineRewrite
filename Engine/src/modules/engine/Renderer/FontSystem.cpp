//
// Created by cmorg on 10/31/2026.
//

#include "FontSystem.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "src/modules/engine/Resources/ResourceSystem.h"
#include "stb/stb_truetype.h"

struct SystemFontData {
    DynamicArray<FontData> sizeVariants{};
    ULong binarySize = 0;
    String name{};
    void* binary = nullptr;
    int offset = 0;
    int index = 0;
    stbtt_fontinfo info{};
};

FontSystem* FontSystem::instance = nullptr;

static AssetMap<SystemFontData> systemFonts{};

bool FontSystem::loadSystemFont(const SystemFontConfig &newConfig) {
    Resource loadedResource{};

    if (!ResourceSystem::load(newConfig.resourceName, RESOURCE_TYPE_SYSTEM_FONT, loadedResource)) {
        Logger::logError("Failed to load system font " + newConfig.resourceName);
        return false;
    }

    SystemFontResourceData& resourceData = *static_cast<SystemFontResourceData *>(loadedResource.data);

    const unsigned int fontCount = resourceData.fonts.getLength();
    for (int i = 0; i < fontCount; i++) {
        SystemFont& font = resourceData.fonts[i];
        if (font.name.isEmpty()) {
            Logger::logError("Failed to load resource! Resource name is blank!");
            return false;
        }

        if (systemFonts.getAsset(font.name) != nullptr) {
            Logger::logError("A system font named " + font.name + " already exists.");
            return false;
        }

        SystemFontData& fontData = *systemFonts.createAsset(font.name, false);

        fontData.binarySize = resourceData.binarySize[i];
        fontData.binary = resourceData.binaries[i];
        fontData.name = font.name;
        fontData.index = static_cast<int>(systemFonts.getAssetIndex(font.name));
        fontData.sizeVariants.initialize();

        fontData.offset = 0;//stbtt_GetFontOffsetForIndex(static_cast<const unsigned char *>(fontData.binary), i);
        const int result = stbtt_InitFont(&fontData.info, static_cast<const unsigned char *>(fontData.binary), fontData.offset);
        if (result == 0) {
            Logger::logError("Failed to initialize system font " + loadedResource.path + " from index " + toString(i));
            return false;
        }

        //Create default size
        FontData variant{};
        if (!createSystemFontVariant(fontData, newConfig.defaultSize, fontData.name, variant)) {
            Logger::logError("Failed to create system font variant " + fontData.name + " from index " + toString(i));
            continue;
        }

        if (!setupFontData(variant)) {
            Logger::logError("Failed to setup system font data for " + fontData.name + " from index " + toString(i));
            continue;
        }

        fontData.sizeVariants.push(variant);
    }

    //Cleanup
    for (unsigned int i = 0; i < resourceData.fonts.getLength(); i++) {
        resourceData.fonts[i].name.destroy();
    }

    resourceData.fonts.shutdown();
    resourceData.binarySize.shutdown();
    resourceData.binaries.shutdown();

    FF_Memory::ff_free<SystemFontResourceData>(loadedResource.data, RESOURCE);

    loadedResource.data = nullptr;
    loadedResource.dataSize = 0;
    loadedResource.path.destroy();

    return true;
}

bool FontSystem::loadBitmapFont(const BitmapFontConfig &newConfig) {
    if (bitmapFonts.getAsset(newConfig.name) != nullptr) {
        Logger::logError("A Bitmap font named " + newConfig.name + " is already loaded.");
        return false;
    }

    if (bitmapFonts.getAssetCount() >= config.maxBitmapFonts) {
        Logger::logError("Font System has reached the maximum allowed bitmap fonts and cannot create any more!");
        return false;
    }

    BitmapFontData& data = *bitmapFonts.createAsset(newConfig.name, false);

    if (!ResourceSystem::load(newConfig.resourceName, RESOURCE_TYPE_BITMAP_FONT, data.loadedResource, nullptr)) {
        Logger::logError("Failed to load font " + newConfig.name + ".");
        return false;
    }

    data.resourceData = static_cast<BitmapFontResourceData *>(data.loadedResource.data);
    data.resourceData->data.atlas.texture = &TextureSystem::acquireTexture(true, false, "Fonts/" + data.resourceData->pages[0].file, TEXTURE_USE_MAP_DIFFUSE);

    return setupFontData(data.resourceData->data);
}

bool FontSystem::verifyAtlas(FontData &font, const String &text) {
    switch (font.type) {
        case FONT_TYPE_BITMAP: return true;
        case FONT_TYPE_SYSTEM: {
            SystemFontData* fontData = systemFonts.getAsset(font.name);
            if (fontData == nullptr) {
                Logger::logError("Failed to verify system font " + font.name + ". System Font could not be found!");
                return false;
            }

            return instance->verifySystemFontVariantSize(*fontData, font, text);
        }
    }

    Logger::logError("Cannot verify font atlas. Unsupported font type.");
    return false;
}

bool FontSystem::setupFontData(FontData &font) const {
    font.atlas.filterMag = TEXTURE_FILTER_BILINEAR;
    font.atlas.filterMin = TEXTURE_FILTER_BILINEAR;
    font.atlas.repeatU = TEXTURE_CLAMP_TO_EDGE;
    font.atlas.repeatV = TEXTURE_CLAMP_TO_EDGE;
    font.atlas.repeatW = TEXTURE_CLAMP_TO_EDGE;
    font.atlas.use = TEXTURE_USE_MAP_DIFFUSE;

    if (!backendRef->acquireTextureMapResources(font.atlas)) {
        Logger::logError("Failed to acquire texture map resources for font " + font.name + ".");
        return false;
    }

    if (font.tabAdvance == 0) {
        float fontSpace = 0;
        for (unsigned int i = 0; i < font.glyphCount; i++) {
            if (font.glyphs[i].codepoint == '\t') {
                font.tabAdvance = font.glyphs[i].advance;
                break;
            }
            if (font.glyphs[i].codepoint == ' ') {
                fontSpace = font.glyphs[i].advance * 4;
            }
        }

        if (font.tabAdvance == 0) {
            if (fontSpace != 0) {
                font.tabAdvance = fontSpace;
            } else {
                font.tabAdvance = font.size * 4;
            }
        }
    }

    return true;
}

void FontSystem::cleanupFontData(FontData &font) const {
    backendRef->releaseTextureMapResources(font.atlas);

    if (font.type == FONT_TYPE_BITMAP && font.atlas.texture) {
        TextureSystem::releaseTexture(font.atlas.texture->name);
    }

    font.atlas.texture = nullptr;

    if (font.glyphs) {
        FF_Memory::ff_free_recursive<FontGlyph>(font.glyphs, ARRAY, font.glyphCount);
    }
    if (font.kernings) {
        FF_Memory::ff_free_recursive<FontKerning>(font.kernings, ARRAY, font.kerningCount);
    }

    font.name.destroy();
}

bool FontSystem::createSystemFontVariant(SystemFontData &font, const unsigned short size, const String &fontName, FontData &outVariant) {
    outVariant.atlasSizeX = 1024;
    outVariant.atlasSizeY = 1024;
    outVariant.size = size;
    outVariant.type = FONT_TYPE_SYSTEM;
    outVariant.name = fontName;
    outVariant.dataSize = sizeof(SystemFontVariantData);
    outVariant.data = FF_Memory::ff_allocate<SystemFontVariantData>(SYSTEM_FONT);

    SystemFontVariantData& data = *static_cast<SystemFontVariantData *>(outVariant.data);

    data.codepoints.initialize(96);
    data.codepoints.push(-1); //Always include a -1.
    for (int i = 0; i < 95; i++) {
        data.codepoints.push(i + 32);
    }

    //Create texture
    const String fontTextureName = "foxfire_system_font_texture_atlas_" + fontName + "_" + toString(font.index) + "_px" + toString(size);
    outVariant.atlas.texture = &TextureSystem::acquireWritableTexture(fontTextureName, outVariant.atlasSizeX, outVariant.atlasSizeY, 4, true);

    data.scale = stbtt_ScaleForPixelHeight(&font.info, size);
    int ascent, descent, lineGap = 0;
    stbtt_GetFontVMetrics(&font.info, &ascent, &descent, &lineGap);
    outVariant.height = static_cast<int>(static_cast<float>(ascent - descent + lineGap) * data.scale);

    return rebuildSystemFontVariantAtlas(font, outVariant);
}

bool FontSystem::rebuildSystemFontVariantAtlas(const SystemFontData &font, FontData &variant) {
    SystemFontVariantData& data = *static_cast<SystemFontVariantData *>(variant.data);

    const unsigned int packImageSize = variant.atlasSizeX * variant.atlasSizeY * sizeof(unsigned char);
    const auto pixels = static_cast<unsigned char *>(FF_Memory::ff_allocate_raw(packImageSize, ARRAY));
    const unsigned int codepointCount = data.codepoints.getLength();
    const auto packedChars = FF_Memory::ff_allocate_recursive<stbtt_packedchar>(ARRAY, codepointCount);

    stbtt_pack_context packContext{};
    if (!stbtt_PackBegin(&packContext, pixels, variant.atlasSizeX, variant.atlasSizeY, 0, 1, nullptr)) {
        Logger::logError("Failed to pack font data.");
        return false;
    }

    stbtt_pack_range range{};
    range.first_unicode_codepoint_in_range = 0;
    range.font_size = static_cast<float>(variant.size);
    range.num_chars = static_cast<int>(codepointCount);
    range.chardata_for_range = packedChars;
    range.array_of_unicode_codepoints = data.codepoints.getData();
    if (!stbtt_PackFontRanges(&packContext, static_cast<const unsigned char *>(font.binary), 0, &range, 1)) {
        Logger::logError("Failed to pack font range data.");
        return false;
    }

    stbtt_PackEnd(&packContext);

    //Convert from single channel to rgba
    const auto rgb = static_cast<unsigned char *>(FF_Memory::ff_allocate_raw(packImageSize * 4, ARRAY));
    for (int i = 0; i < packImageSize; i++) {
        rgb[(i*4)+0] = pixels[i];
        rgb[(i*4)+1] = pixels[i];
        rgb[(i*4)+2] = pixels[i];
        rgb[(i*4)+3] = pixels[i];
    }

    TextureSystem::writeData(*variant.atlas.texture, 0, packImageSize * 4, rgb);

    FF_Memory::ff_free_raw(pixels, packImageSize, ARRAY);
    FF_Memory::ff_free_raw(rgb, packImageSize * 4, ARRAY);

    //Regenerate glyphs
    if (variant.glyphs && variant.glyphCount > 0) {
        FF_Memory::ff_free_recursive<FontGlyph>(variant.glyphs, ARRAY, variant.glyphCount);
    }

    variant.glyphCount = codepointCount;
    variant.glyphs = FF_Memory::ff_allocate_recursive<FontGlyph>(ARRAY, codepointCount);

    for (unsigned int i = 0; i < variant.glyphCount; i++) {
        const stbtt_packedchar& pc = packedChars[i];
        FontGlyph& glyph = variant.glyphs[i];
        glyph.codepoint = data.codepoints[i];
        glyph.page = 0;
        glyph.xOffset = static_cast<short>(pc.xoff);
        glyph.yOffset = static_cast<short>(pc.yoff);
        glyph.x = pc.x0;
        glyph.y = pc.y0;
        glyph.width = pc.x1 - pc.x0;
        glyph.height = pc.y1 - pc.y0;
        glyph.advance = pc.xadvance;
    }

    //Regenerate Kernings
    if (variant.kernings && variant.kerningCount > 0) {
        FF_Memory::ff_free_recursive<FontKerning>(variant.kernings, ARRAY, variant.kerningCount);
    }
    variant.kerningCount = stbtt_GetKerningTableLength(&font.info);
    if (variant.kerningCount > 0) {
        variant.kernings = FF_Memory::ff_allocate_recursive<FontKerning>(ARRAY, variant.kerningCount);
        const auto kerningTable = FF_Memory::ff_allocate_recursive<stbtt_kerningentry>(ARRAY, variant.kerningCount);
        const int entryCount = stbtt_GetKerningTable(&font.info, kerningTable, static_cast<int>(variant.kerningCount));
        if (entryCount != variant.kerningCount) {
            Logger::logError("Kerning count and entry count mismatch! Kerning Count: " + toString(variant.kerningCount) + " Entry Count: " + toString(entryCount));
            return false;
        }

        for (unsigned int i = 0; i < variant.kerningCount; i++) {
            FontKerning& kerning = variant.kernings[i];
            kerning.codepointA = kerningTable[i].glyph1;
            kerning.codepointB = kerningTable[i].glyph2;
            kerning.amount = static_cast<float>(kerningTable[i].advance);
        }

        FF_Memory::ff_free_recursive<stbtt_kerningentry>(kerningTable, ARRAY, variant.kerningCount);
    } else {
        variant.kernings = nullptr;
    }

    FF_Memory::ff_free_recursive<stbtt_packedchar>(packedChars, ARRAY, codepointCount);

    return true;
}

bool FontSystem::verifySystemFontVariantSize(const SystemFontData &font, FontData &variant, const String &text) {
    auto& variantData = *static_cast<SystemFontVariantData *>(variant.data);

    unsigned int addedCodepointCount = 0;
    for (unsigned int i = 0; i < text.getSizeInBytes();) {
        int codepoint = 0;
        unsigned char advance = 0;

        if (!text.getCodepoint(i, codepoint, advance)) {
            Logger::logError("Failed to get codepoint from text.");
            i++;
            continue;
        }

        i += advance;

        //Skip ascii codepoints
        if (codepoint < 128) {
            continue;
        }

        const unsigned int codepointCount = variantData.codepoints.getLength();
        bool found = false;
        for (unsigned int j = 95; j < codepointCount; j++) { //Start at 95 because -1-95 are guaranteed
            if (variantData.codepoints[j] == codepoint) {
                found = true;
                break;
            }
        }
        if (!found) {
            variantData.codepoints.push(codepoint);
            addedCodepointCount++;
        }
    }

    //Rebuild the atlas if codepoints are added
    if (addedCodepointCount > 0) {
        return rebuildSystemFontVariantAtlas(font, variant);
    }

    return true;
}

bool FontSystem::initialize(const FontSystemConfig &newConfig, IRendererBackend *backendSystemRef) {
    instance = this;

    backendRef = backendSystemRef;

    if (newConfig.maxBitmapFonts == 0 || newConfig.maxSystemFonts == 0) {
        Logger::logFatal("Maximum bitmap and system fonts cannot be 0!");
        return false;
    }

    config = newConfig;
    bitmapFonts.initialize(config.maxBitmapFonts);
    systemFonts.initialize(config.maxSystemFonts);

    //Load default fonts
    for (unsigned int i = 0; i < config.defaultBitmapFontCount; i++) {
        if (!loadBitmapFont(config.bitmapFontConfigs[i])) {
            Logger::logError("Failed to load bitmap font: " + config.bitmapFontConfigs[i].name);
        }
    }
    for (unsigned int i = 0; i < config.defaultSystemFontCount; i++) {
        if (!loadSystemFont(config.systemFontConfigs[i])) {
            Logger::logError("Failed to load system font: " + config.systemFontConfigs[i].name);
        }
    }

    return true;
}

void FontSystem::shutdown() {
    for (unsigned short i = 0; i < config.maxBitmapFonts; i++) {
        BitmapFontData* data = bitmapFonts.getAssetAtIndex(i);
        if (data != nullptr) {
            FontData& fontData = data->resourceData->data;
            cleanupFontData(fontData);
            FF_Memory::ff_free_recursive<FontPage>(data->resourceData->pages, ARRAY, data->resourceData->pageCount);

            FF_Memory::ff_free<BitmapFontResourceData>(data->resourceData, RESOURCE);
        }
    }

    bitmapFonts.shutdown();

    for (unsigned short i = 0; i < config.maxSystemFonts; i++) {
        SystemFontData* data = systemFonts.getAssetAtIndex(i);
        if (data != nullptr) {
            for (FontData& fontData : data->sizeVariants) {
                cleanupFontData(fontData);
                if (fontData.data) {
                    const auto variantData = static_cast<SystemFontVariantData *>(fontData.data);
                    variantData->codepoints.shutdown();
                    FF_Memory::ff_free<SystemFontVariantData>(fontData.data, SYSTEM_FONT);
                    fontData.data = nullptr;
                }
            }
            data->sizeVariants.shutdown();

            FF_Memory::ff_free_raw(data->binary, data->binarySize, RESOURCE);
            data->name.destroy();
        }
    }

    systemFonts.shutdown();

    backendRef = nullptr;
    instance = nullptr;
}

bool FontSystem::acquireFont(const String &fontName, const unsigned short fontSize, UIText &text) {
    switch (text.getType()) {
        case UI_TEXT_TYPE_BITMAP: {
            const BitmapFontData* data = instance->bitmapFonts.acquireAsset(fontName);
            if (data == nullptr) {
                Logger::logError("Failed to acquire font " + fontName + ".");
                return false;
            }

            //Decrement reference count for the old font if any
            if (!text.getFontName().isEmpty()) {
                switch (text.getType()) {
                    case UI_TEXT_TYPE_BITMAP: {
                        instance->bitmapFonts.releaseAsset(text.getFontName());
                        break;
                    }
                    case UI_TEXT_TYPE_SYSTEM: {
                        systemFonts.releaseAsset(text.getFontName());
                        break;
                    }
                }
            }

            text.setFont(&data->resourceData->data);
            return true;
        }
        case UI_TEXT_TYPE_SYSTEM: {
            SystemFontData* data = systemFonts.acquireAsset(fontName);
            if (data == nullptr) {
                Logger::logError("Failed to acquire font " + fontName + ".");
                return false;
            }

            for (FontData& fontData : data->sizeVariants) {
                if (fontData.size == fontSize) {
                    //Decrement reference count for the old font if any
                    if (!text.getFontName().isEmpty()) {
                        switch (text.getType()) {
                            case UI_TEXT_TYPE_BITMAP: {
                                instance->bitmapFonts.releaseAsset(text.getFontName());
                                break;
                            }
                            case UI_TEXT_TYPE_SYSTEM: {
                                systemFonts.releaseAsset(text.getFontName());
                                break;
                            }
                        }
                    }

                    text.setFont(&fontData);
                    return true;
                }
            }

            //No variant exists, so create one
            FontData variant{};
            if (!instance->createSystemFontVariant(*data, fontSize, data->name, variant)) {
                Logger::logError("Failed to create font " + fontName + " at index " + toString(data->index) + " with size " + toString(fontSize) + ".");
                return false;
            }

            if (!instance->setupFontData(variant)) {
                Logger::logError("Failed to setup font data!");
            }

            //Decrement reference count for the old font if any
            if (!text.getFontName().isEmpty()) {
                switch (text.getType()) {
                    case UI_TEXT_TYPE_BITMAP: {
                        instance->bitmapFonts.releaseAsset(text.getFontName());
                        break;
                    }
                    case UI_TEXT_TYPE_SYSTEM: {
                        systemFonts.releaseAsset(text.getFontName());
                        break;
                    }
                }
            }

            data->sizeVariants.push(variant);
            text.setFont(&data->sizeVariants[data->sizeVariants.getLength() - 1]);
            return true;
        }
        default: {
            Logger::logError("Unrecognized UIText type.");
            return false;
        }
    }
}

bool FontSystem::releaseFont(UIText &text) {
    text.setFont(nullptr);
    bitmapFonts.releaseAsset(text.getFontName());
    return true;
}
