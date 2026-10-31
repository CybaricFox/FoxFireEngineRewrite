//
// Created by cmorg on 10/31/2026.
//

#include "FontSystem.h"

FontSystem* FontSystem::instance = nullptr;

bool FontSystem::loadSystemFont(SystemFontConfig &newConfig) {
    Logger::logError("System fonts are not yet supported.");
    return false;
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

    AssetContext context{};
    BitmapFontData& data = *bitmapFonts.createAsset(newConfig.name, context);

    if (!resourceRef->load(newConfig.resourceName, RESOURCE_TYPE_BITMAP_FONT, data.loadedResource, nullptr)) {
        Logger::logError("Failed to load font " + newConfig.name + ".");
        return false;
    }

    data.resourceData = static_cast<BitmapFontResourceData *>(data.loadedResource.data);
    data.resourceData->data.atlas.texture = &textureRef->acquireTexture(true, false, "Fonts/" + data.resourceData->pages[0].file, TEXTURE_USE_MAP_DIFFUSE);

    return setupFontData(data.resourceData->data);
}

bool FontSystem::verifyAtlas(const FontData &font, String text) {
    if (font.type == FONT_TYPE_BITMAP) return true; //Bitmaps do not need to be verified

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
        textureRef->releaseTexture(font.atlas.texture->name);
    }

    font.atlas.texture = nullptr;

    if (font.glyphs) {
        FF_Memory::ff_free_recursive<FontGlyph>(font.glyphs, ARRAY, font.glyphCount);
    }
    if (font.kernings) {
        FF_Memory::ff_free_recursive<FontKerning>(font.kernings, ARRAY, font.kerningCount);
    }
}

bool FontSystem::initialize(const FontSystemConfig &newConfig, ResourceSystem *resourceSystemRef, ITextureSystem *textureSystemRef, IRendererBackend* backendSystemRef) {
    instance = this;

    resourceRef = resourceSystemRef;
    textureRef = textureSystemRef;
    backendRef = backendSystemRef;

    if (newConfig.maxBitmapFonts == 0 || newConfig.maxSystemFonts == 0) {
        Logger::logFatal("Maximum bitmap and system fonts cannot be 0!");
        return false;
    }

    config = newConfig;
    bitmapFonts.initialize(config.maxBitmapFonts);

    //Load default fonts
    for (unsigned int i = 0; i < config.defaultBitmapFontCount; i++) {
        if (!loadBitmapFont(config.bitmapFontConfigs[i])) {
            Logger::logError("Failed to load bitmap font: " + config.bitmapFontConfigs[i].name);
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

    resourceRef = nullptr;
    textureRef = nullptr;
    backendRef = nullptr;
    instance = nullptr;
}

bool FontSystem::acquireFont(const String &fontName, unsigned short fontSize, UIText &text) {
    switch (text.getType()) {
        case UI_TEXT_TYPE_BITMAP: {
            const BitmapFontData* data = instance->bitmapFonts.acquireAsset(fontName);
            if (data == nullptr) {
                Logger::logError("Failed to acquire font " + fontName + ".");
                return false;
            }

            text.setFont(&data->resourceData->data);
            return true;
        }
        case UI_TEXT_TYPE_SYSTEM: {
            Logger::logError("System fonts are not yet supported");
            return false;
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
