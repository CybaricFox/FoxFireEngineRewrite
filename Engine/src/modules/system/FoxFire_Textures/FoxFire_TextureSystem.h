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
#include "src/modules/engine/Renderer/IRendererBackend.h"
#include "src/modules/engine/Resources/Contexts.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

/**
 * @brief Default texture system.
 */
class FOXFIRE_API FoxFire_TextureSystem final : public ITextureSystem {
private:
    Texture defaultDiffuseTexture{};
    Texture defaultSpecularTexture{};
    Texture defaultNormalTexture{};
    AssetMap<Texture, AssetContext> assets{};

    Texture& getDefaultByCase(TextureUseCase useCase);

    bool createDefaultTextures();
    void destroyDefaultTextures();
    bool loadTexture(Texture &texture, const String &fileName) const;
    void destroyTexture(Texture &texture) const;

public:
    FoxFire_TextureSystem();
    ~FoxFire_TextureSystem() override;

    bool initialize(unsigned int initialCapacity, IRendererBackend *backend, ResourceSystem* resources) override;
    void shutdown();

    Texture& getDefaultDiffuseTexture() override {return defaultDiffuseTexture;}
    Texture& getDefaultSpecularTexture() override {return defaultSpecularTexture;}
    Texture& getDefaultNormalTexture() override {return defaultNormalTexture;}

    Texture &acquireTexture(bool autoRelease, const String &fileName, TextureUseCase useCase) override;
    void releaseTexture(String name) override;
};
