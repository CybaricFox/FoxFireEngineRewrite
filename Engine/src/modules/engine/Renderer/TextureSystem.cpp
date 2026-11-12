//
// Created by cmorg on 7/31/2026.
//

#include "TextureSystem.h"

#include "src/modules/engine/Resources/ResourceSystem.h"

TextureSystem* TextureSystem::instance = nullptr;

Texture & TextureSystem::getDefaultByCase(const TextureUseCase useCase) {
    switch (useCase) {
        case TEXTURE_USE_MAP_DIFFUSE: return defaultDiffuseTexture;
        case TEXTURE_USE_MAP_SPECULAR: return defaultSpecularTexture;
        case TEXTURE_USE_MAP_NORMAL: return defaultNormalTexture;
        default: return defaultDiffuseTexture;
    }
}

bool TextureSystem::createDefaultTextures() {
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

void TextureSystem::destroyDefaultTextures() {
    destroyTexture(defaultDiffuseTexture);
    destroyTexture(defaultSpecularTexture);
    destroyTexture(defaultNormalTexture);
}

bool TextureSystem::loadTexture(Texture &texture, const String &fileName) const {
    TextureLoadParams loadParams{};
    loadParams.name = fileName;
    loadParams.outTexture = &texture;
    loadParams.generation = texture.generation;

    const JobContext job = JobSystem::getInstance().createJob<TextureLoadParams, TextureLoadParams>(textureLoadStart, textureLoadSuccess, textureLoadFail, &loadParams, RESOURCE_LOAD_JOB, JOB_PRIORITY_HIGH);
    JobSystem::getInstance().submit(job);
    return true;
}

bool TextureSystem::loadCubeTexture(const String &name, const String textureNames[6], Texture &texture) const {
    unsigned char* pixels = nullptr;
    ULong size = 0;
    for (unsigned int i = 0; i < 6; ++i) {
        ImageParameters imageParameters{};
        imageParameters.bFlipY = false;

        Resource imageResource{};
        if (!ResourceSystem::load(textureNames[i], RESOURCE_TYPE_IMAGE, imageResource, &imageParameters)) {
            Logger::logWarn("Failed to load cube image resource for texture: " + textureNames[i]);
            //Instead of failing, try to use the base texture for the cubemap.
            if (!ResourceSystem::load(name, RESOURCE_TYPE_IMAGE, imageResource, &imageParameters)) {
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
            pixels = FF_Memory::ff_allocate_recursive<unsigned char>(ARRAY, size * 6);
        } else {
            if (texture.width != resourceData->width || texture.height != resourceData->height || texture.channelCount != resourceData->channelCount) {
                Logger::logError("All textures for a cube map must have the same width, height, and channel count.");
                FF_Memory::ff_free_recursive<unsigned char>(pixels, ARRAY, size * 6);
                return false;
            }
        }

        FF_Memory::ff_copy(pixels + size * i, resourceData->pixels, size);
        ResourceSystem::unload(imageResource);
    }

    backendRef->createTexture(pixels, texture);
    FF_Memory::ff_free_recursive<unsigned char>(pixels, ARRAY, size * 6);

    return true;
}

void TextureSystem::destroyTexture(Texture &texture) const {
    backendRef->destroyTexture(texture);
    texture = Texture{};
}

unsigned int TextureSystem::textureLoadSuccess(void *params) {
    auto& textureParams = *static_cast<TextureLoadParams *>(params);
    const auto& resourceData = *static_cast<ImageResourceData *>(textureParams.imageResource.data);

    instance->backendRef->createTexture(resourceData.pixels, textureParams.tempTexture);
    Texture oldTexture = *textureParams.outTexture;
    *textureParams.outTexture = textureParams.tempTexture;
    instance->backendRef->destroyTexture(oldTexture);

    if (textureParams.generation == INVALID_ID_U32) {
        textureParams.outTexture->generation = 0;
    } else {
        textureParams.outTexture->generation = textureParams.generation + 1;
    }

    Logger::logDebug("Successfully loaded texture " + textureParams.name + " on job thread.");

    ResourceSystem::unload(textureParams.imageResource);

    return 0;
}

unsigned int TextureSystem::textureLoadFail(void *params) {
    auto& textureParams = *static_cast<TextureLoadParams *>(params);

    Logger::logError("Failed to load texture: " + textureParams.name);
    ResourceSystem::unload(textureParams.imageResource);

    return 0;
}

bool TextureSystem::textureLoadStart(void *params, void *result) {
    auto& loadParams = *static_cast<TextureLoadParams *>(params);

    ImageParameters resourceParams{};
    resourceParams.bFlipY = true;

    const bool loadResult = ResourceSystem::load(loadParams.name, RESOURCE_TYPE_IMAGE, loadParams.imageResource, &resourceParams);

    const auto imageData = static_cast<ImageResourceData *>(loadParams.imageResource.data);

    loadParams.tempTexture.width = imageData->width;
    loadParams.tempTexture.height = imageData->height;
    loadParams.tempTexture.channelCount = imageData->channelCount;
    loadParams.generation = loadParams.outTexture->generation;
    loadParams.outTexture->generation = INVALID_ID_U32; //Prevents the texture from being used while its loading

    const ULong totalSize = loadParams.tempTexture.width * loadParams.tempTexture.height * loadParams.tempTexture.channelCount;
    bool isTransparent = false;
    for (ULong i = 0; i < totalSize; i+= loadParams.tempTexture.channelCount) {
        if (imageData->pixels[i + 3] < 255) {
            isTransparent = true;
            break;
        }
    }

    loadParams.tempTexture.name = loadParams.name;
    loadParams.tempTexture.generation = INVALID_ID_U32;
    loadParams.tempTexture.flags |= isTransparent ? TEXTURE_BIT_TRANSPARENT : 0;
    *static_cast<TextureLoadParams*>(result) = loadParams;

    return loadResult;
}

bool TextureSystem::initialize(const unsigned int initialCapacity, IRendererBackend *backend) {
    backendRef = backend;

    instance = this;

    assets.initialize(initialCapacity);

    createDefaultTextures();

    return true;
}

void TextureSystem::shutdown() {
    for (Texture* texture : assets.getAssetsAsArray()) {
        if (texture->generation != INVALID_ID_U32) {
            destroyTexture(*texture);
        }
    }

    destroyDefaultTextures();

    assets.shutdown();

    instance = nullptr;

    backendRef = nullptr;
}

Texture & TextureSystem::acquireTexture(const bool autoRelease, const bool skipLoad, const String &fileName, const TextureUseCase useCase) {
    if (fileName.isEmpty()) {
        return instance->getDefaultByCase(useCase);
    }

    if (fileName == DEFAULT_DIFFUSE_TEXTURE_NAME || fileName == DEFAULT_SPECULAR_TEXTURE_NAME || fileName == DEFAULT_NORMAL_TEXTURE_NAME) {
        Logger::logWarn("Texture system tried to acquire the default texture. Use getDefaultTexture() instead.");
        return instance->getDefaultByCase(useCase);
    }

    if (Texture* texture = instance->assets.acquireAsset(fileName); texture) {
        return *texture;
    }

    Texture* texture = instance->assets.createAsset(fileName, autoRelease);
    texture->type = TEXTURE_2D;

    if (texture == nullptr) return instance->getDefaultByCase(useCase);

    if (!skipLoad) {
        if (!instance->loadTexture(*texture, fileName)) {
            Logger::logError("Failed to load texture: " + fileName);
            return instance->getDefaultByCase(useCase);
        }
    }

    texture->id = instance->assets.getAssetIndex(fileName);
    Logger::logDebug("Successfully created new texture: " + fileName);

    return *texture;
}

Texture & TextureSystem::acquireWritableTexture(const String &name, const unsigned int width, const unsigned int height, const unsigned char channelCount, const bool isTransparent) {
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
    instance->backendRef->createWritableTexture(texture);
    return texture;
}

Texture & TextureSystem::acquireCubeTexture(const String &name, const bool autoRelease) {
    if (name == DEFAULT_DIFFUSE_TEXTURE_NAME) {
        Logger::logWarn("Call to acquire default texture from acquire cube texture. Please use getDefaultTexture.");
        return instance->defaultDiffuseTexture;
    }

    if (Texture* texture = instance->assets.acquireAsset(name); texture) {
        return *texture;
    }

    Texture* texture = instance->assets.createAsset(name, autoRelease);
    texture->type = TEXTURE_CUBE;
    texture->id = instance->assets.getAssetIndex(name);

    String textureNames[6]{};
    textureNames[0] = name + "_r";
    textureNames[1] = name + "_l";
    textureNames[2] = name + "_u";
    textureNames[3] = name + "_d";
    textureNames[4] = name + "_f";
    textureNames[5] = name + "_b";

    if (!instance->loadCubeTexture(name, textureNames, *texture)) {
        Logger::logError("Failed to load cube texture: " + name);
        return instance->defaultDiffuseTexture;
    }

    return *texture;
}

void TextureSystem::releaseTexture(const String &name) {
    if (name == DEFAULT_DIFFUSE_TEXTURE_NAME || name == DEFAULT_SPECULAR_TEXTURE_NAME || name == DEFAULT_NORMAL_TEXTURE_NAME) {
        return;
    }

    Texture* texture = nullptr;
    if (instance->assets.releaseAsset(name, texture)) {
        instance->destroyTexture(*texture);
    }
}

bool TextureSystem::writeData(Texture &texture, const unsigned offset, const unsigned size, const unsigned char *data) {
    instance->backendRef->writeTextureData(texture, offset, size, data);
    return true;
}

void TextureSystem::wrapTexture(const String &name, const unsigned int width, const unsigned int height, const unsigned int channelCount, const bool isTransparent, const bool isWritable, const bool registerTexture, void *data, Texture& outTexture) {
    Texture* texture = nullptr;

    if (registerTexture) {
        texture = &acquireTexture(false, true, name, TEXTURE_USE_MAP_DIFFUSE);
    } else {
        texture = &outTexture;
        texture->id = INVALID_ID_U32;
    }

    texture->name = name;
    texture->width = width;
    texture->height = height;
    texture->channelCount = channelCount;
    texture->generation = INVALID_ID_U32;
    texture->flags |= isTransparent ? TEXTURE_BIT_TRANSPARENT : 0;
    texture->flags |= isWritable ? TEXTURE_BIT_WRITABLE : 0;
    texture->flags |= TEXTURE_BIT_WRAPPED;
    texture->data = data;
    texture->type = TEXTURE_2D;
}
