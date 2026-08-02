//
// Created by cmorg on 7/31/2026.
//

#include "FoxFire_TextureSystem.h"

#include "src/modules/engine/Renderer/RendererBackend.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../Engine/src/modules/engine/Renderer/stb/stb_image.h"

bool FoxFire_TextureSystem::initialize(const unsigned int initialCapacity, RendererBackend* backend) {
    backendRef = backend;
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
    backendRef = nullptr;
}

Texture & FoxFire_TextureSystem::acquireTexture(const bool autoRelease, const String& fileName, const String& subPath) {
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


    if (!loadTexture(*texture, fileName, subPath)) {
        Logger::logError("Failed to load texture: " + fileName + " with subpath: " + subPath);
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

bool FoxFire_TextureSystem::loadTexture(Texture& texture, const String &fileName, const String &subFolders = "") const {
    String path{};

    if (subFolders.empty()) {
        path = "Assets/" + fileName + ".";
    } else {
        path = "Assets/" + subFolders + "/" + fileName + ".";
    }

    constexpr int requiredChannelCount = 4;
    stbi_set_flip_vertically_on_load(true); //stb loads the image from down to top, this effectively makes it read top to down.
    //In the future, this should check for the extension automatically
    path.append("png");

    Texture tempTexture{};
    int width = static_cast<int>(tempTexture.width);
    int height = static_cast<int>(tempTexture.height);
    int channelCount = tempTexture.channelCount;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channelCount, requiredChannelCount);
    tempTexture.width = width;
    tempTexture.height = height;
    tempTexture.channelCount = channelCount;

    //overwrite the texture channel count with the one we are using
    tempTexture.channelCount = requiredChannelCount;

    if (!data) {
        if (stbi_failure_reason()) {
            Logger::logWarn("#1 Failed to load texture file: " + path + ": " + stbi_failure_reason());
            stbi__err(nullptr, 0);
        }
        return false;
    }

    const unsigned int currentGeneration = texture.generation;
    texture.generation = INVALID_ID;
    const unsigned long totalSize = tempTexture.width * tempTexture.height * requiredChannelCount;

    //transparency
    bool isTransparent = false;
    for (unsigned long i = 0; i < totalSize; i += requiredChannelCount) {
        const unsigned char a = data[i + 3];
        if (a < 255) {
            isTransparent = true;
            break;
        }
    }

    /*
    if (stbi_failure_reason()) {
        Logger::logWarn("#2 Failed to load texture file: " + path + ": " + stbi_failure_reason());
        stbi__err(nullptr, 0);
        return false;
    }
    */

    tempTexture.name = fileName;
    tempTexture.generation = INVALID_ID;
    tempTexture.bIsTransparent = isTransparent;

    backendRef->createTexture(data, tempTexture);
    destroyTexture(texture);
    texture = tempTexture;

    if (currentGeneration == INVALID_ID) {
        texture.generation = 0;
    } else {
        texture.generation = currentGeneration + 1;
    }

    stbi_image_free(data);
    return true;
}

void FoxFire_TextureSystem::destroyTexture(Texture &texture) const {
    backendRef->destroyTexture(texture);
    texture = Texture{};
}

FoxFire_TextureSystem::~FoxFire_TextureSystem() {
    shutdown();
}
