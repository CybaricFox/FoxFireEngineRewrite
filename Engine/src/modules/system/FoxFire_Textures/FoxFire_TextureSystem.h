//
// Created by cmorg on 7/31/2026.
//

#pragma once

#include "src/defines.h"
#include <foxfire_export.h>

#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Renderer/RendererBackend.h"
#include "src/modules/engine/Resources/Contexts.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

class FOXFIRE_API FoxFire_TextureSystem final : public ITextureSystem {
private:
    Texture defaultTexture{};
    AssetMap<Texture, AssetContext> assets{};
    RendererBackend* backendRef = nullptr;

    bool createDefaultTextures();
    void destroyDefaultTextures();
    bool loadTexture(Texture &texture, const String &fileName, const String &subFolders) const;
    void destroyTexture(Texture &texture) const;

public:
    ~FoxFire_TextureSystem() override;

    bool initialize(unsigned int initialCapacity, RendererBackend *backend) override;
    void shutdown();

    Texture& getDefaultTexture() override {return defaultTexture;}

    Texture &acquireTexture(bool autoRelease, const String &fileName, const String &subPath) override;
    void releaseTexture(String name) override;
};
