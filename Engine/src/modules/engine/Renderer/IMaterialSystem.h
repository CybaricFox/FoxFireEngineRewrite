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
#include "ShaderSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_MATERIAL_NAME "default"

struct MaterialSystemConfig {
    unsigned int maxMaterialCount = 0;
};

/**
 * @brief Abstract class that controls materials
 */
class FOXFIRE_API IMaterialSystem {
private:
    unsigned long memorySize = 0;

protected:
    IRendererBackend* backendRef = nullptr;
    ResourceSystem* resourceRef = nullptr;
    ITextureSystem* textureSystemRef = nullptr;
    ShaderSystem* shaderRef = nullptr;

    MaterialSystemConfig config{};
public:
    explicit IMaterialSystem(const unsigned long derivedSize) {memorySize = derivedSize;}
    virtual ~IMaterialSystem();

    virtual bool initialize(MaterialSystemConfig materialSystemConfig, ITextureSystem *system, IRendererBackend *backend, ResourceSystem* resources, ShaderSystem* shaderSystem);

    virtual Material& getDefaultMaterial() = 0;
    [[nodiscard]] unsigned long getMemorySize() const {return memorySize;}

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

    virtual bool applyGlobal(unsigned int shaderId, Mat4 *projection, Mat4 *view, Vector4f *ambientColor, Vector3f *viewPosition, unsigned int
                             renderMode) const = 0;
    virtual bool applyInstance(Material& material) const = 0;
    virtual bool applyLocal(const Material &material, Mat4 *model) const = 0;
};
