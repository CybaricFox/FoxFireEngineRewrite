//
// Created by cmorg on 7/31/2026.
//

#pragma once

#define DEFAULT_TEXTURE_NAME "default"

#include "src/defines.h"
#include <foxfire_export.h>
#include "src/modules/engine/Memory/DynamicArray.h"
#include "src/modules/engine/Memory/HashMap.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Renderer/RendererBackend.h"
#include "src/modules/engine/Resources/Resource_Types.h"

struct TextureContext {
    //Number of pointers to this texture
    unsigned long referenceCount = 0;
    //Index of this texture in the array
    unsigned int index = 0;
    //Whether this texture will auto destroy when referenceCount hits 0.
    bool bAutoRelease = false;
};

class FOXFIRE_API FoxFire_TextureSystem final : public ITextureSystem {
private:
    Texture defaultTexture{};
    DynamicArray<Texture> textures{};
    HashMap<String, TextureContext> textureMap{};
    RendererBackend* backendRef = nullptr;
    DynamicArray<int> freeIndexes{}; //Acts like a stack. Last in, First out.

    bool createDefaultTextures();
    void destroyDefaultTextures();
    bool loadTexture(Texture &texture, const String &fileName, const String &subFolders);
    bool loadTexture(Texture &texture, const String &fileName);
    bool loadTextureHelper(Texture &texture, String &path, const String &fileName);

public:
    ~FoxFire_TextureSystem() override;

    bool initialize(unsigned int initialCapacity, RendererBackend* backend) override;
    void shutdown();

    Texture& getDefaultTexture() override {return defaultTexture;}

    Texture &acquireTexture(const String &fileName, bool autoRelease) override;
    Texture &acquireTexture(const String &fileName, const String &subPath, bool autoRelease) override;
    void releaseTexture(const String& name) override;
};
