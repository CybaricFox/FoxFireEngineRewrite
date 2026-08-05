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
#include "RendererBackend.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

#define DEFAULT_TEXTURE_NAME "default"

/**
 * @brief Abstract class that controls textures.
 */
class FOXFIRE_API ITextureSystem {
protected:
    RendererBackend* backendRef = nullptr;
    ResourceSystem* resourceRef = nullptr;

public:
    virtual ~ITextureSystem();

    virtual bool initialize(unsigned int initialCapacity, RendererBackend *backend, ResourceSystem* resources);

    virtual Texture& getDefaultTexture() = 0;

    /**
     * @brief Fetches a texture from wherever the user stores it. Creates it if it doesn't exist.
     * @param autoRelease Whether to automatically remove the texture when it has no references.
     * @param fileName Name of the texture.
     * @return The texture or default if something goes wrong.
     */
    virtual Texture &acquireTexture(bool autoRelease, const String &fileName) = 0;

    /**
     * @brief Clears the texture if it has no references.
     * @param name Name of the texture.
     */
    virtual void releaseTexture(String name) = 0;
};