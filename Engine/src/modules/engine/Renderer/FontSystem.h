//
// Created by cmorg on 10/31/2026.
//

#pragma once
#include "../UserInterface/UIText.h"
#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Library/String.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Resources/FontTypes.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

/**
 *  @file FontSystem.h
 *  @layer Engine
 *  @module UserInterface
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 10/31/2026
 *
 *  @copyright (c) 2026
 */

struct SystemFontConfig {
    String name{};
    unsigned short defaultSize = 12;
    String resourceName{};
};

struct BitmapFontConfig {
    String name{};
    unsigned short size = 0;
    String resourceName{};
};

struct FontSystemConfig {
    unsigned char defaultSystemFontCount = 0;
    SystemFontConfig* systemFontConfigs = nullptr;
    unsigned char defaultBitmapFontCount = 0;
    BitmapFontConfig* bitmapFontConfigs = nullptr;
    unsigned char maxSystemFonts = 0;
    unsigned char maxBitmapFonts = 0;
    bool bAutoRelease = false;
};

struct BitmapFontData {
    Resource loadedResource{};
    BitmapFontResourceData* resourceData = nullptr;
};

class FontSystem {
private:
    static FontSystem* instance;

    ResourceSystem* resourceRef = nullptr;
    ITextureSystem* textureRef = nullptr;
    IRendererBackend* backendRef = nullptr;

    FontSystemConfig config{};
    AssetMap<BitmapFontData, AssetContext> bitmapFonts{};

    bool loadSystemFont(SystemFontConfig& newConfig);
    bool loadBitmapFont(const BitmapFontConfig& newConfig);

    bool setupFontData(FontData& font) const;
    void cleanupFontData(FontData& font) const;

public:
    bool initialize(const FontSystemConfig& newConfig, ResourceSystem* resourceSystemRef, ITextureSystem* textureSystemRef, IRendererBackend* backendSystemRef);
    void shutdown();

    static bool acquireFont(const String &fontName, unsigned short fontSize, UIText& text);
    bool releaseFont(UIText &text);

    /**
     * @brief Verifies that a system font has all characters in the given string.
     * @param font The font to check.
     * @param text The string to check.
     * @return
     */
    static bool verifyAtlas(const FontData& font, String text);
};