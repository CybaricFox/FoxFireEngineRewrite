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

    /**
     * @brief Apply Global UBO to materials
     * @param shaderId Id of the shader
     * @param projection Camera projection
     * @param view Camera viewport
     * @param ambientColor World Ambient Color
     * @param viewPosition Camera position
     * @param renderMode (Editor Only) Mode to render
     * @return false on failure
     */
    virtual bool applyGlobal(unsigned int shaderId, Mat4 *projection, Mat4 *view, Vector4f *ambientColor, Vector3f *viewPosition, unsigned int renderMode) const = 0;

    /**
     * @brief Apply Instance UBO to material
     * @param material Material to apply to
     * @param update Whether the instance UBO should be updated (Once per frame)
     * @return false on failure
     */
    virtual bool applyInstance(Material& material, bool update) const = 0;

    /**
     * @brief Apply local UBO to material
     * @param material Material to apply to
     * @param model Model of the object
     * @return false on failure
     */
    virtual bool applyLocal(const Material &material, Mat4 *model) const = 0;
};
