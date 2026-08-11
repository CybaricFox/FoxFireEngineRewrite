/**
*   @file FoxFire_MaterialSystem.h
 *  @layer System
 *  @module FoxFire_Textures
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 08-05-2026
 *
 *  @copyright (c) 2026
 */

#pragma once

#include "src/defines.h"
#include <foxfire_export.h>

#include "src/modules/engine/Library/AssetMap.h"
#include "src/modules/engine/Memory/HashMap.h"
#include "src/modules/engine/Renderer/IMaterialSystem.h"
#include "src/modules/engine/Renderer/ITextureSystem.h"
#include "src/modules/engine/Resources/Contexts.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

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
};

struct UIShaderUniformLocations {
    unsigned short projection = INVALID_ID_U16;
    unsigned short view = INVALID_ID_U16;
    unsigned short diffuseColor = INVALID_ID_U16;
    unsigned short diffuseTexture = INVALID_ID_U16;
    unsigned short model = INVALID_ID_U16;
};

/**
 * @brief Default material system.
 */
class FOXFIRE_API FoxFire_MaterialSystem final : public IMaterialSystem{
private:
    Material defaultMaterial{};
    AssetMap<Material, AssetContext> assets{};
    MaterialShaderUniformLocations materialLocations{};
    unsigned int materialShaderId = INVALID_ID_U32;
    UIShaderUniformLocations uiShaderLocations{};
    unsigned int uiShaderId = INVALID_ID_U32;

    void shutdown();

    bool createDefaultMaterial();
    bool loadMaterial(const MaterialResourceData &config, Material& material) const;
    void destroyMaterial(Material& material) const;

public:
    FoxFire_MaterialSystem();
    ~FoxFire_MaterialSystem() override;

    bool initialize(MaterialSystemConfig materialSystemConfig, ITextureSystem *system, IRendererBackend *backend, ResourceSystem *resources, ShaderSystem* shaderSystem) override;

    Material& acquireMaterial(const String &name) override;
    Material& acquireMaterial(const MaterialResourceData &config) override;
    void releaseMaterial(const String &name) override;
    bool applyGlobal(unsigned int shaderId, Mat4 *projection, Mat4 *view, Vector4f *ambientColor, Vector3f* viewPosition) const override;
    bool applyInstance(Material& material) const override;
    bool applyLocal(const Material &material, Mat4 *model) const override;

    Material& getDefaultMaterial() override {return defaultMaterial;}
};