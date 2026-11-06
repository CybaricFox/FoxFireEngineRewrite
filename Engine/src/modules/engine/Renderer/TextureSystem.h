/**
*   @file TextureSystem.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include <foxfire_export.h>
#include "IRendererBackend.h"
#include "src/modules/engine/Resources/ResourceSystem.h"
#include "src/modules/engine/Threads/JobSystem.h"

#define DEFAULT_DIFFUSE_TEXTURE_NAME "default"
#define DEFAULT_SPECULAR_TEXTURE_NAME "default_specular"
#define DEFAULT_NORMAL_TEXTURE_NAME "default_normal"

struct TextureLoadParams final : ThreadParam<TextureLoadParams> {
    String name{};
    Texture* outTexture = nullptr;
    Texture tempTexture{};
    unsigned int generation = INVALID_ID_U32;
    Resource imageResource{};
};

/**
 * @brief Abstract class that controls textures.
 */
class FOXFIRE_API TextureSystem {
private:
    static TextureSystem* instance;

    Texture defaultDiffuseTexture{};
    Texture defaultSpecularTexture{};
    Texture defaultNormalTexture{};
    AssetMap<Texture> assets{};

    Texture& getDefaultByCase(TextureUseCase useCase);

    bool createDefaultTextures();
    void destroyDefaultTextures();
    bool loadTexture(Texture &texture, const String &fileName) const;
    bool loadCubeTexture(const String &name, const String textureNames[6], Texture& texture) const;
    void destroyTexture(Texture &texture) const;

    static unsigned int textureLoadSuccess(void *params);
    static unsigned int textureLoadFail(void* params);
    static bool textureLoadStart(void *params, void *result);

    IRendererBackend* backendRef = nullptr;

public:
    bool initialize(unsigned int initialCapacity, IRendererBackend *backend);
    void shutdown();

    static Texture& getDefaultDiffuseTexture() {return instance->defaultDiffuseTexture;}
    static Texture& getDefaultSpecularTexture() {return instance->defaultSpecularTexture;}
    static Texture& getDefaultNormalTexture() {return instance->defaultNormalTexture;}

    /**
     * @brief Fetches a texture from wherever the user stores it. Creates it if it doesn't exist.
     * @param autoRelease Whether to automatically remove the texture when it has no references.
     * @param skipLoad
     * @param skipLoad
     * @param fileName Name of the texture.
     * @param useCase
     * @return The texture or default if something goes wrong.
     */
    static Texture &acquireTexture(bool autoRelease, bool skipLoad, const String &fileName, TextureUseCase useCase);
    static Texture& acquireWritableTexture(const String &name, unsigned int width, unsigned int height, unsigned char channelCount, bool isTransparent);
    static Texture& acquireCubeTexture(const String &name, bool autoRelease);

    /**
     * @brief Clears the texture if it has no references.
     * @param name Name of the texture.
     */
    static void releaseTexture(const String &name);

    static bool writeData(Texture &texture, unsigned offset, unsigned size, const unsigned char *data);
    static void wrapTexture(const String &name, unsigned int width, unsigned int height, unsigned int channelCount, bool isTransparent, bool
                            isWritable, bool registerTexture, void *data, Texture &outTexture);
};