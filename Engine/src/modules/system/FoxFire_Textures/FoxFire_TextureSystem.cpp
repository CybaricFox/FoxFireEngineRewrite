//
// Created by cmorg on 7/31/2026.
//

#include "FoxFire_TextureSystem.h"

#include "src/modules/engine/Renderer/IRendererBackend.h"

bool FoxFire_TextureSystem::initialize(const unsigned int initialCapacity, IRendererBackend *backend, ResourceSystem *resources) {
    ITextureSystem::initialize(initialCapacity, backend, resources);

    assets.initialize(initialCapacity);

    createDefaultTextures();
    return true;
}

void FoxFire_TextureSystem::shutdown() {
    for (Texture* texture : assets.getAssetsAsArray()) {
        if (texture->generation != INVALID_ID_U32) {
            destroyTexture(*texture);
        }
    }

    destroyDefaultTextures();
}

Texture & FoxFire_TextureSystem::acquireTexture(const bool autoRelease, const bool skipLoad, const String& fileName, const TextureUseCase useCase) {
    if (fileName.empty()) {
        return getDefaultByCase(useCase);
    }

    if (fileName == DEFAULT_DIFFUSE_TEXTURE_NAME || fileName == DEFAULT_SPECULAR_TEXTURE_NAME || fileName == DEFAULT_NORMAL_TEXTURE_NAME) {
        Logger::logWarn("Texture system tried to acquire the default texture. Use getDefaultTexture() instead.");
        return getDefaultByCase(useCase);
    }

    if (Texture* texture = assets.acquireAsset(fileName); texture) {
        return *texture;
    }

    AssetContext context{};
    context.bAutoRelease = autoRelease;

    Texture* texture = assets.createAsset(fileName, context);
    texture->type = TEXTURE_2D;

    if (texture == nullptr) return getDefaultByCase(useCase);


    if (!skipLoad) {
        if (!loadTexture(*texture, fileName)) {
            Logger::logError("Failed to load texture: " + fileName);
            return getDefaultByCase(useCase);
        }
    }

    texture->id = context.index;
    Logger::logDebug("Successfully created new texture: " + fileName);

    return *texture;
}

void FoxFire_TextureSystem::releaseTexture(const String name) {
    if (name == DEFAULT_DIFFUSE_TEXTURE_NAME || name == DEFAULT_SPECULAR_TEXTURE_NAME || name == DEFAULT_NORMAL_TEXTURE_NAME) {
        return;
    }

    Texture* texture = nullptr;
    if (assets.releaseAsset(name, texture)) {
        destroyTexture(*texture);
    }
}

Texture & FoxFire_TextureSystem::acquireWritableTexture(const String name, const unsigned width, const unsigned height, const unsigned char channelCount, const bool isTransparent) {
    Texture& texture = acquireTexture(false, true, name, TEXTURE_USE_MAP_DIFFUSE);

    texture.name = name;
    texture.width = width;
    texture.height = height;
    texture.channelCount = channelCount;
    texture.generation = INVALID_ID_U32;
    texture.flags |= isTransparent ? TEXTURE_BIT_TRANSPARENT : 0;
    texture.flags |= TEXTURE_BIT_WRITABLE;
    texture.data = nullptr;
    texture.type = TEXTURE_2D;
    backendRef->createWritableTexture(texture);
    return texture;
}

Texture & FoxFire_TextureSystem::acquireCubeTexture(String name, bool autoRelease) {
    if (name == DEFAULT_DIFFUSE_TEXTURE_NAME) {
        Logger::logWarn("Call to acquire default texture from acquire cube texture. Please use getDefaultTexture.");
        return defaultDiffuseTexture;
    }

    if (Texture* texture = assets.acquireAsset(name); texture) {
        return *texture;
    }

    AssetContext context{};
    context.bAutoRelease = autoRelease;

    Texture* texture = assets.createAsset(name, context);
    texture->type = TEXTURE_CUBE;
    texture->id = context.index;

    String textureNames[6]{};
    textureNames[0] = name + "_r";
    textureNames[1] = name + "_l";
    textureNames[2] = name + "_u";
    textureNames[3] = name + "_d";
    textureNames[4] = name + "_f";
    textureNames[5] = name + "_b";

    if (!loadCubeTexture(name, textureNames, *texture)) {
        Logger::logError("Failed to load cube texture: " + name);
        return defaultDiffuseTexture;
    }

    return *texture;
}

Texture & FoxFire_TextureSystem::getDefaultByCase(const TextureUseCase useCase) {
    switch (useCase) {
        case TEXTURE_USE_MAP_DIFFUSE: return defaultDiffuseTexture;
        case TEXTURE_USE_MAP_SPECULAR: return defaultSpecularTexture;
        case TEXTURE_USE_MAP_NORMAL: return defaultNormalTexture;
        default: return defaultDiffuseTexture;
    }
}

//Generates a default texture during runtime so dependency on a file system isn't necessary for niche scenarios.
bool FoxFire_TextureSystem::createDefaultTextures() {
    unsigned char diffusePixels[16 * 16 * 4];
    FF_Memory::ff_set(diffusePixels, 255, 16 * 16 * 4 * sizeof(unsigned char));
    defaultDiffuseTexture.name = DEFAULT_DIFFUSE_TEXTURE_NAME;
    defaultDiffuseTexture.width = 16;
    defaultDiffuseTexture.height = 16;
    defaultDiffuseTexture.channelCount = 4;
    defaultDiffuseTexture.generation = INVALID_ID_U32;
    defaultDiffuseTexture.flags = 0;
    defaultDiffuseTexture.type = TEXTURE_2D;
    backendRef->createTexture(diffusePixels, defaultDiffuseTexture);

    unsigned char specularPixels[16 * 16 * 4];
    FF_Memory::ff_set(specularPixels, 0, 16 * 16 * 4 * sizeof(unsigned char));
    defaultSpecularTexture.name = DEFAULT_SPECULAR_TEXTURE_NAME;
    defaultSpecularTexture.width = 16;
    defaultSpecularTexture.height = 16;
    defaultSpecularTexture.channelCount = 4;
    defaultSpecularTexture.generation = INVALID_ID_U32;
    defaultSpecularTexture.flags = 0;
    defaultDiffuseTexture.type = TEXTURE_2D;
    backendRef->createTexture(specularPixels, defaultSpecularTexture);

    unsigned char normalPixels[16 * 16 * 4];
    FF_Memory::ff_set(normalPixels, 0, 16 * 16 * 4 * sizeof(unsigned char));

    for (unsigned long row = 0; row < 16; ++row) {
        for (unsigned long column = 0; column < 16; ++column) {
            const unsigned long index = (row * 16) + column;
            const unsigned long index_bpp = index * 4;
            normalPixels[index_bpp + 0] = 128;
            normalPixels[index_bpp + 1] = 128;
            normalPixels[index_bpp + 2] = 255;
            normalPixels[index_bpp + 3] = 255;
        }
    }
    defaultNormalTexture.name = DEFAULT_NORMAL_TEXTURE_NAME;
    defaultNormalTexture.width = 16;
    defaultNormalTexture.height = 16;
    defaultNormalTexture.channelCount = 4;
    defaultNormalTexture.generation = INVALID_ID_U32;
    defaultNormalTexture.flags = 0;
    defaultDiffuseTexture.type = TEXTURE_2D;
    backendRef->createTexture(normalPixels, defaultNormalTexture);

    return true;
}

void FoxFire_TextureSystem::destroyDefaultTextures() {
    destroyTexture(defaultDiffuseTexture);
    destroyTexture(defaultSpecularTexture);
    destroyTexture(defaultNormalTexture);
}

bool FoxFire_TextureSystem::loadTexture(Texture& texture, const String &fileName) const {
    ImageParameters params{};
    params.bFlipY = true;

    Resource imageResource{};
    if (!resourceRef->load(fileName, RESOURCE_TYPE_IMAGE, imageResource, &params)) {
        Logger::logError("Failed to load image resource for texture: " + fileName);
        return false;
    }

    const auto* resourceData = static_cast<ImageResourceData *>(imageResource.data);

    Texture tempTexture{};
    tempTexture.width = resourceData->width;
    tempTexture.height = resourceData->height;
    tempTexture.channelCount = resourceData->channelCount;

    const unsigned int currentGeneration = texture.generation;
    texture.generation = INVALID_ID_U32;
    const unsigned long totalSize = tempTexture.width * tempTexture.height * tempTexture.channelCount;

    //transparency
    TextureFlagBits flags = 0;
    for (unsigned long i = 0; i < totalSize; i += tempTexture.channelCount) {
        const unsigned char a = resourceData->pixels[i + 3];
        if (a < 255) {
            flags = TEXTURE_BIT_TRANSPARENT;
            break;
        }
    }

    tempTexture.name = fileName;
    tempTexture.generation = INVALID_ID_U32;
    tempTexture.flags = flags;

    backendRef->createTexture(resourceData->pixels, tempTexture);
    destroyTexture(texture);
    texture = tempTexture;

    if (currentGeneration == INVALID_ID_U32) {
        texture.generation = 0;
    } else {
        texture.generation = currentGeneration + 1;
    }

    resourceRef->unload(imageResource);
    return true;
}

bool FoxFire_TextureSystem::loadCubeTexture(const String &name, const String textureNames[6], Texture &texture) const {
    unsigned char* pixels = nullptr;
    ULong size = 0;
    for (unsigned int i = 0; i < 6; ++i) {
        ImageParameters imageParameters{};
        imageParameters.bFlipY = false;

        Resource imageResource{};
        if (!resourceRef->load(textureNames[i], RESOURCE_TYPE_IMAGE, imageResource, &imageParameters)) {
            Logger::logWarn("Failed to load cube image resource for texture: " + textureNames[i]);
            //Instead of failing, try to use the base texture for the cubemap.
            if (!resourceRef->load(name, RESOURCE_TYPE_IMAGE, imageResource, &imageParameters)) {
                Logger::logError("Failed to convert image to cube map: " + name);
                return false;
            }
        }

        const auto resourceData = static_cast<ImageResourceData *>(imageResource.data);
        if (!pixels) {
            texture.width = resourceData->width;
            texture.height = resourceData->height;
            texture.channelCount = resourceData->channelCount;
            texture.flags = 0;
            texture.generation = 0;
            texture.name = name;
            size = texture.width * texture.height * texture.channelCount;
            pixels = static_cast<unsigned char *>(FF_Memory::ff_allocate(sizeof(unsigned char) * size * 6, ARRAY));
        } else {
            if (texture.width != resourceData->width || texture.height != resourceData->height || texture.channelCount != resourceData->channelCount) {
                Logger::logError("All textures for a cube map must have the same width, height, and channel count.");
                FF_Memory::ff_free(pixels, sizeof(unsigned char) * size * 6, ARRAY);
                pixels = nullptr;
                return false;
            }
        }

        FF_Memory::ff_copy(pixels + size * i, resourceData->pixels, size);
        resourceRef->unload(imageResource);
    }

    backendRef->createTexture(pixels, texture);
    FF_Memory::ff_free(pixels, sizeof(unsigned char) * size * 6, ARRAY);
    pixels = nullptr;

    return true;
}

void FoxFire_TextureSystem::destroyTexture(Texture &texture) const {
    backendRef->destroyTexture(texture);
    texture = Texture{};
}

FoxFire_TextureSystem::FoxFire_TextureSystem()
    : ITextureSystem(sizeof(FoxFire_TextureSystem))
{
}

FoxFire_TextureSystem::~FoxFire_TextureSystem() {
    shutdown();
}
