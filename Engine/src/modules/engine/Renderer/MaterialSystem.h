/**
*   @file MaterialSystem.h
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

#include "TextureSystem.h"
#include "ShaderSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_MATERIAL_NAME "default"

struct MaterialSystemConfig {
    unsigned int maxMaterialCount = 0;
};

struct MaterialShaderUniformLocations {
    unsigned short projection = INVALID_ID_U16;
    unsigned short view = INVALID_ID_U16;
    unsigned short ambientColor = INVALID_ID_U16;
    unsigned short viewPosition = INVALID_ID_U16;
    unsigned short diffuseColor = INVALID_ID_U16;
    unsigned short diffuseTexture = INVALID_ID_U16;
    unsigned short specularTexture = INVALID_ID_U16;
    unsigned short normalTexture = INVALID_ID_U16;
    unsigned short shine = INVALID_ID_U16;
    unsigned short model = INVALID_ID_U16;
    unsigned short renderMode = INVALID_ID_U16;
};

struct UIShaderUniformLocations {
    unsigned short projection = INVALID_ID_U16;
    unsigned short view = INVALID_ID_U16;
    unsigned short diffuseColor = INVALID_ID_U16;
    unsigned short diffuseTexture = INVALID_ID_U16;
    unsigned short model = INVALID_ID_U16;
};

/**
 * @brief Abstract class that controls materials
 */
class FOXFIRE_API MaterialSystem {
private:
    static MaterialSystem* instance;

    IRendererBackend* backendRef = nullptr;

    MaterialSystemConfig config{};
    Material defaultMaterial{};
    AssetMap<Material> assets{};
    MaterialShaderUniformLocations materialLocations{};
    unsigned int materialShaderId = INVALID_ID_U32;
    UIShaderUniformLocations uiShaderLocations{};
    unsigned int uiShaderId = INVALID_ID_U32;

    bool createDefaultMaterial();
    bool loadMaterial(const MaterialResourceData &newConfig, Material& material) const;
    void destroyMaterial(Material& material) const;

public:
     bool initialize(MaterialSystemConfig materialSystemConfig, IRendererBackend *backend);
     void shutdown();

    /**
     * @brief Returns the default material.
     * @return
     */
     static Material& getDefaultMaterial() {return instance->defaultMaterial;}

    /**
     * @brief Fetches a material from wherever the user stores it, or creates it if it doesn't exist.
     * @param name Name of the material to fetch.
     * @return The material or default if something goes wrong.
     */
     static Material& acquireMaterial(const String &name);
    /**
     * @brief Fetches a material from wherever the user stores it, or creates it if it doesn't exist.
     * @param newConfig Material config data.
     * @return The material or default if something goes wrong.
     */
     Material& acquireMaterial(const MaterialResourceData &newConfig);

    /**
     * @brief Decrements a materials referenceCount and clears it if there are none.
     * @param name Name of the material
     */
     static void releaseMaterial(const String &name);

    /**
     * @brief Apply Global UBO to materials
     * @param shaderId Id of the shader
     * @param frameNumber
     * @param projection Camera projection
     * @param view Camera viewport
     * @param ambientColor World Ambient Color
     * @param viewPosition Camera position
     * @param renderMode (Editor Only) Mode to render
     * @return false on failure
     */
     bool applyGlobal(unsigned int shaderId, unsigned long frameNumber, Mat4 *projection, Mat4 *view, Vector4f *ambientColor, Vector3f *viewPosition, unsigned int renderMode) const;

    /**
     * @brief Apply Instance UBO to material
     * @param material Material to apply to
     * @param update Whether the instance UBO should be updated (Once per frame)
     * @return false on failure
     */
     bool applyInstance(Material& material, bool update) const;

    /**
     * @brief Apply local UBO to material
     * @param material Material to apply to
     * @param model Model of the object
     * @return false on failure
     */
     bool applyLocal(const Material &material, Mat4 *model) const;
};
