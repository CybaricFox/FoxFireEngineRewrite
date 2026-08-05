/**
*   @file FoxFire_TextureSystem.h
 *  @layer System
 *  @module FoxFire_Textures
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "src/defines.h"
#include <foxfire_export.h>

#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Renderer/RendererBackend.h"
#include "src/modules/engine/Resources/Contexts.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

/**
 * @brief Default texture system.
 */
class FOXFIRE_API FoxFire_TextureSystem final : public ITextureSystem {
private:
    Texture defaultTexture{};
    AssetMap<Texture, AssetContext> assets{};

    bool createDefaultTextures();
    void destroyDefaultTextures();
    bool loadTexture(Texture &texture, const String &fileName) const;
    void destroyTexture(Texture &texture) const;

public:
    ~FoxFire_TextureSystem() override;

    bool initialize(unsigned int initialCapacity, RendererBackend *backend, ResourceSystem* resources) override;
    void shutdown();

    Texture& getDefaultTexture() override {return defaultTexture;}

    Texture &acquireTexture(bool autoRelease, const String &fileName) override;
    void releaseTexture(String name) override;
};
