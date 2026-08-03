//
// Created by cmorg on 7/31/2026.
//

#include "FoxFire_TextureSystem.h"

#include "src/modules/engine/Renderer/RendererBackend.h"

bool FoxFire_TextureSystem::initialize(const unsigned int initialCapacity, RendererBackend *backend, ResourceSystem *resources) {
    ITextureSystem::initialize(initialCapacity, backend, resources);

    assets.initialize(initialCapacity);

    createDefaultTextures();
    return true;
}

void FoxFire_TextureSystem::shutdown() {
    for (Texture& texture : assets.getData().getData()) {
        if (texture.generation != INVALID_ID) {
            destroyTexture(texture);
        }
    }

    destroyDefaultTextures();
}

Texture & FoxFire_TextureSystem::acquireTexture(const bool autoRelease, const String& fileName) {
    if (fileName == DEFAULT_TEXTURE_NAME) {
        Logger::logWarn("Texture system tried to acquire the default texture. Use getDefaultTexture() instead.");
        return defaultTexture;
    }

    if (Texture* texture = assets.acquireAsset(fileName); texture) {
        return *texture;
    }

    AssetContext context{};
    context.bAutoRelease = autoRelease;

    Texture* texture = assets.createAsset(fileName, context);

    if (texture == nullptr) return defaultTexture;


    if (!loadTexture(*texture, fileName)) {
        Logger::logError("Failed to load texture: " + fileName);
        return defaultTexture;
    }

    texture->id = context.index;
    Logger::logDebug("Successfully created new texture: " + fileName);

    return *texture;
}

void FoxFire_TextureSystem::releaseTexture(const String name) {
    if (name == DEFAULT_TEXTURE_NAME) {
        Logger::logWarn("Cannot release the default texture!");
        return;
    }

    Texture* texture = nullptr;
    if (assets.releaseAsset(name, texture)) destroyTexture(*texture);
}

//Generates a default texture during runtime so dependency on a file system isn't necessary for niche scenarios.
bool FoxFire_TextureSystem::createDefaultTextures() {
    //diffuse texture
    constexpr unsigned int dimensions = 256;
    constexpr unsigned int bpp = 4; //rgba
    constexpr unsigned pixelCount = dimensions * dimensions;
    unsigned char pixels[pixelCount * bpp];
    FF_Memory::ff_set(pixels, 255, sizeof(unsigned char) * pixelCount * bpp);

    for (unsigned long row = 0; row < dimensions; ++row) {
        for (unsigned long column = 0; column < dimensions; ++column) {
            const unsigned long index = (row * dimensions) + column;
            const unsigned long index_bpp = index * bpp;
            if (row % 2) {
                if (column % 2) {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            } else {
                if (!(column % 2)) {
                    pixels[index_bpp + 0] = 0;
                    pixels[index_bpp + 1] = 0;
                }
            }
        }
    }

    defaultTexture.name = DEFAULT_TEXTURE_NAME;
    defaultTexture.width = dimensions;
    defaultTexture.height = dimensions;
    defaultTexture.channelCount = 4;
    defaultTexture.generation = INVALID_ID;
    defaultTexture.bIsTransparent = false;
    backendRef->createTexture(pixels, defaultTexture);


    return true;
}

void FoxFire_TextureSystem::destroyDefaultTextures() {
    destroyTexture(defaultTexture);
}

bool FoxFire_TextureSystem::loadTexture(Texture& texture, const String &fileName) const {
    Resource imageResource{};
    if (!resourceRef->load(fileName, RESOURCE_TYPE_IMAGE, imageResource)) {
        Logger::logError("Failed to load image resource for texture: " + fileName);
        return false;
    }

    const auto* resourceData = static_cast<ImageResourceData *>(imageResource.data);

    Texture tempTexture{};
    tempTexture.width = resourceData->width;
    tempTexture.height = resourceData->height;
    tempTexture.channelCount = resourceData->channelCount;

    const unsigned int currentGeneration = texture.generation;
    texture.generation = INVALID_ID;
    const unsigned long totalSize = tempTexture.width * tempTexture.height * tempTexture.channelCount;

    //transparency
    bool isTransparent = false;
    for (unsigned long i = 0; i < totalSize; i += tempTexture.channelCount) {
        const unsigned char a = resourceData->pixels[i + 3];
        if (a < 255) {
            isTransparent = true;
            break;
        }
    }

    tempTexture.name = fileName;
    tempTexture.generation = INVALID_ID;
    tempTexture.bIsTransparent = isTransparent;

    backendRef->createTexture(resourceData->pixels, tempTexture);
    destroyTexture(texture);
    texture = tempTexture;

    if (currentGeneration == INVALID_ID) {
        texture.generation = 0;
    } else {
        texture.generation = currentGeneration + 1;
    }

    resourceRef->unload(imageResource);
    return true;
}

void FoxFire_TextureSystem::destroyTexture(Texture &texture) const {
    backendRef->destroyTexture(texture);
    texture = Texture{};
}

FoxFire_TextureSystem::~FoxFire_TextureSystem() {
    shutdown();
}
