/**
*   @file ITextureSystem.h
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

#define DEFAULT_DIFFUSE_TEXTURE_NAME "default"
#define DEFAULT_SPECULAR_TEXTURE_NAME "default_specular"
#define DEFAULT_NORMAL_TEXTURE_NAME "default_normal"

/**
 * @brief Abstract class that controls textures.
 */
class FOXFIRE_API ITextureSystem {
private:
    unsigned long memorySize = 0;
protected:
    IRendererBackend* backendRef = nullptr;
    ResourceSystem* resourceRef = nullptr;

public:
    explicit ITextureSystem(const unsigned long derivedSize) {memorySize = derivedSize;}
    virtual ~ITextureSystem();

    virtual bool initialize(unsigned int initialCapacity, IRendererBackend *backend, ResourceSystem* resources);

    virtual Texture& getDefaultDiffuseTexture() = 0;
    virtual Texture& getDefaultSpecularTexture() = 0;
    virtual Texture& getDefaultNormalTexture() = 0;
    [[nodiscard]] unsigned long getMemorySize() const {return memorySize;}

    /**
     * @brief Fetches a texture from wherever the user stores it. Creates it if it doesn't exist.
     * @param autoRelease Whether to automatically remove the texture when it has no references.
     * @param fileName Name of the texture.
     * @param useCase
     * @param useCase
     * @return The texture or default if something goes wrong.
     */
    virtual Texture &acquireTexture(bool autoRelease, const String &fileName, TextureUseCase useCase) = 0;

    /**
     * @brief Clears the texture if it has no references.
     * @param name Name of the texture.
     */
    virtual void releaseTexture(String name) = 0;
};