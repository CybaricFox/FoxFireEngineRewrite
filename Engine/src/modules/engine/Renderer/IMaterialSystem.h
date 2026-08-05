/**
*   @file IMaterialSystem.h
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

#include "ITextureSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_MATERIAL_NAME "default"

/**
 * @brief Abstract class that controls materials
 */
class FOXFIRE_API IMaterialSystem {
protected:
    RendererBackend* backendRef = nullptr;
    ResourceSystem* resourceRef = nullptr;
    ITextureSystem* textureSystemRef = nullptr;
public:
    virtual ~IMaterialSystem();

    virtual bool initialize(unsigned int initialCapacity, ITextureSystem *system, RendererBackend *backend, ResourceSystem* resources);

    virtual Material& getDefaultMaterial() = 0;

    /**
     * @brief Fetches a material from wherever the user stores it, or creates it if it doesn't exist.
     * @param name Name of the material to fetch.
     * @return The material or default if something goes wrong.
     */
    virtual Material& acquireMaterial(const String &name) = 0;
    /**
     * @brief Fetches a material from wherever the user stores it, or creates it if it doesn't exist.
     * @param config Material config data.
     * @return The material or default if something goes wrong.
     */
    virtual Material& acquireMaterial(const MaterialResourceData &config) = 0;

    /**
     * @brief Decrements a materials referenceCount and clears it if there are none.
     * @param name Name of the material
     */
    virtual void releaseMaterial(const String &name) = 0;
};
