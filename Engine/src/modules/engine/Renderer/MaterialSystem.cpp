//
// Created by cmorg on 8/1/2026.
//

#include "MaterialSystem.h"

MaterialSystem* MaterialSystem::instance = nullptr;

bool MaterialSystem::createDefaultMaterial() {
    defaultMaterial = Material{};
    defaultMaterial.name = DEFAULT_MATERIAL_NAME;
    defaultMaterial.diffuseColor = oneVector4f();
    defaultMaterial.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
    defaultMaterial.diffuseMap.texture = &TextureSystem::getDefaultDiffuseTexture();
    defaultMaterial.specularMap.use = TEXTURE_USE_MAP_SPECULAR;
    defaultMaterial.specularMap.texture = &TextureSystem::getDefaultSpecularTexture();
    defaultMaterial.normalMap.use = TEXTURE_USE_MAP_NORMAL;
    defaultMaterial.normalMap.texture = &TextureSystem::getDefaultNormalTexture();

    TextureMap* maps[3]{&defaultMaterial.diffuseMap, &defaultMaterial.specularMap, &defaultMaterial.normalMap};
    const Shader* shader = ShaderSystem::getShader("Fox_Fire_Material_Shader");
    if (!backendRef->acquireInstanceResources(*shader, defaultMaterial.internalId, maps)) {
        Logger::logFatal("Failed to acquire resources for the default material.");
        return false;
    }

    defaultMaterial.shaderId = shader->getId();

    return true;
}

bool MaterialSystem::loadMaterial(const MaterialResourceData &newConfig, Material &material) const {
    material = Material{};
    material.name = newConfig.name;
    material.shaderId = ShaderSystem::getId(newConfig.shaderName);
    material.diffuseColor = newConfig.diffuseColor;
    material.shine = newConfig.shine;

    material.diffuseMap.filterMin = TEXTURE_FILTER_BILINEAR;
    material.diffuseMap.filterMag = TEXTURE_FILTER_BILINEAR;
    material.diffuseMap.repeatU = TEXTURE_REPEAT;
    material.diffuseMap.repeatV = TEXTURE_REPEAT;
    material.diffuseMap.repeatW = TEXTURE_REPEAT;
    if (!backendRef->acquireTextureMapResources(material.diffuseMap)) {
        Logger::logError("Failed to acquire resources for a materials diffuse map.");
        return false;
    }

    if (!newConfig.diffuseName.isEmpty()) {
        material.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
        material.diffuseMap.texture = &TextureSystem::acquireTexture(true, false, newConfig.diffuseName, TEXTURE_USE_MAP_DIFFUSE);
        if (material.diffuseMap.texture == nullptr) {
            Logger::logWarn("Unable to load texture: " + newConfig.name + " for material: " + material.name);
            material.diffuseMap.texture = &TextureSystem::getDefaultDiffuseTexture();
        }
    } else {
        material.diffuseMap.use = TEXTURE_USE_UNKNOWN;
        material.diffuseMap.texture = &TextureSystem::getDefaultDiffuseTexture();
    }

    material.specularMap.filterMin = TEXTURE_FILTER_BILINEAR;
    material.specularMap.filterMag = TEXTURE_FILTER_BILINEAR;
    material.specularMap.repeatU = TEXTURE_REPEAT;
    material.specularMap.repeatV = TEXTURE_REPEAT;
    material.specularMap.repeatW = TEXTURE_REPEAT;
    if (!backendRef->acquireTextureMapResources(material.specularMap)) {
        Logger::logError("Failed to acquire resources for a materials specular map.");
        return false;
    }

    if (!newConfig.specularName.isEmpty()) {
        material.specularMap.use = TEXTURE_USE_MAP_SPECULAR;
        material.specularMap.texture = &TextureSystem::acquireTexture(true, false, newConfig.specularName, TEXTURE_USE_MAP_SPECULAR);
        if (material.specularMap.texture == nullptr) {
            Logger::logWarn("Unable to load texture: " + newConfig.name + " for material: " + material.name);
            material.specularMap.texture = &TextureSystem::getDefaultSpecularTexture();
        }
    } else {
        material.specularMap.use = TEXTURE_USE_UNKNOWN;
        material.specularMap.texture = &TextureSystem::getDefaultSpecularTexture();;
    }

    material.normalMap.filterMin = TEXTURE_FILTER_BILINEAR;
    material.normalMap.filterMag = TEXTURE_FILTER_BILINEAR;
    material.normalMap.repeatU = TEXTURE_REPEAT;
    material.normalMap.repeatV = TEXTURE_REPEAT;
    material.normalMap.repeatW = TEXTURE_REPEAT;
    if (!backendRef->acquireTextureMapResources(material.normalMap)) {
        Logger::logError("Failed to acquire resources for a materials normal map.");
        return false;
    }

    if (!newConfig.normalName.isEmpty()) {
        material.normalMap.use = TEXTURE_USE_MAP_NORMAL;
        material.normalMap.texture = &TextureSystem::acquireTexture(true, false, newConfig.normalName, TEXTURE_USE_MAP_NORMAL);
        if (material.normalMap.texture == nullptr) {
            Logger::logWarn("Unable to load texture: " + newConfig.name + " for material: " + material.name);
            material.normalMap.texture = &TextureSystem::getDefaultNormalTexture();
        }
    } else {
        material.normalMap.use = TEXTURE_USE_UNKNOWN;
        material.normalMap.texture = &TextureSystem::getDefaultNormalTexture();;
    }

    const Shader* shader = ShaderSystem::getShader(newConfig.shaderName);
    if (!shader) {
        Logger::logError(material.name + " was unable to load its shader: " + newConfig.shaderName);
        return false;
    }

    TextureMap* maps[3]{&material.diffuseMap, &material.specularMap, &material.normalMap};
    if (!backendRef->acquireInstanceResources(*shader, material.internalId, maps)) {
        Logger::logError("Failed to acquire resources for material " + material.name + "!");
        return false;
    }

    return true;
}

void MaterialSystem::destroyMaterial(Material &material) const {
    Logger::logDebug("Destroying material: " + material.name);

    if (material.shaderId != INVALID_ID_U32 && material.internalId != INVALID_ID_U32) {
        backendRef->releaseInstanceResources(*ShaderSystem::getShader(material.shaderId), material.internalId);
    }

    backendRef->releaseTextureMapResources(material.diffuseMap);
    backendRef->releaseTextureMapResources(material.specularMap);
    backendRef->releaseTextureMapResources(material.normalMap);

    if (material.diffuseMap.texture != nullptr) {
        TextureSystem::releaseTexture(material.diffuseMap.texture->name);
    }

    if (material.specularMap.texture != nullptr) {
        TextureSystem::releaseTexture(material.specularMap.texture->name);
    }

    if (material.normalMap.texture != nullptr) {
        TextureSystem::releaseTexture(material.normalMap.texture->name);
    }

    material = Material{};
}

bool MaterialSystem::initialize(const MaterialSystemConfig materialSystemConfig, IRendererBackend *backend) {
    if (materialSystemConfig.maxMaterialCount == 0) {
        Logger::logFatal("Material system max material count must be greater than 0!");
        return false;
    }

    instance = this;

    config = materialSystemConfig;

    backendRef = backend;

    assets.initialize(config.maxMaterialCount);

    if (!createDefaultMaterial()) {
        Logger::logFatal("Failed to create default material!");
        return false;
    }

    return true;
}

void MaterialSystem::shutdown() {
    for (Material* material : assets.getAssetsAsArray()) {
        if (material->id != INVALID_ID_U32) {
            destroyMaterial(*material);
        }
    }

    assets.shutdown();

    destroyMaterial(defaultMaterial);

    backendRef = nullptr;
}

Material & MaterialSystem::acquireMaterial(const String &name) {
    Resource materialResource{};
    if (!ResourceSystem::load(name, RESOURCE_TYPE_MATERIAL, materialResource)) {
        Logger::logError("Failed to load material resource!");
        return instance->defaultMaterial;
    }

    Material* material = nullptr;
    if (materialResource.data) {
        material = &instance->acquireMaterial(*static_cast<MaterialResourceData*>(materialResource.data));
    }

    ResourceSystem::unload(materialResource);

    if (!material) {
        Logger::logError("Failed to unload material resource!");
        return instance->defaultMaterial;
    }

    return *material;
}

Material & MaterialSystem::acquireMaterial(const MaterialResourceData &newConfig) {
    if (newConfig.name == DEFAULT_MATERIAL_NAME) {
        return defaultMaterial;
    }

    if (Material* material = assets.acquireAsset(newConfig.name)) {
        return *material;
    }

    Material* material = assets.createAsset(newConfig.name, newConfig.bAutoRelease);

    if (material == nullptr) return defaultMaterial;

    if (!loadMaterial(newConfig, *material)) {
        Logger::logError("Failed to load material: " + newConfig.name);
        return defaultMaterial;
    }

    Shader* shader = ShaderSystem::getShader(material->shaderId);
    if (materialShaderId == INVALID_ID_U32 && newConfig.shaderName == "Fox_Fire_Material_Shader") {
        materialShaderId = shader->getId();
        materialLocations.projection = ShaderSystem::getUniformIndex(*shader, "projection");
        materialLocations.view = ShaderSystem::getUniformIndex(*shader, "view");
        materialLocations.viewPosition = ShaderSystem::getUniformIndex(*shader, "view_position");
        materialLocations.ambientColor = ShaderSystem::getUniformIndex(*shader, "ambient_color");
        materialLocations.diffuseColor = ShaderSystem::getUniformIndex(*shader, "diffuse_color");
        materialLocations.diffuseTexture = ShaderSystem::getUniformIndex(*shader, "diffuse_texture");
        materialLocations.specularTexture = ShaderSystem::getUniformIndex(*shader, "specular_texture");
        materialLocations.normalTexture = ShaderSystem::getUniformIndex(*shader, "normal_texture");
        materialLocations.shine = ShaderSystem::getUniformIndex(*shader, "shine");
        materialLocations.model = ShaderSystem::getUniformIndex(*shader, "model");
        materialLocations.renderMode = ShaderSystem::getUniformIndex(*shader, "mode");
    } else if (uiShaderId == INVALID_ID_U32 && newConfig.shaderName == "Fox_Fire_UI_Shader"){
        uiShaderId = shader->getId();
        uiShaderLocations.projection = ShaderSystem::getUniformIndex(*shader, "projection");
        uiShaderLocations.view = ShaderSystem::getUniformIndex(*shader, "view");
        uiShaderLocations.diffuseColor = ShaderSystem::getUniformIndex(*shader, "diffuse_color");
        uiShaderLocations.diffuseTexture = ShaderSystem::getUniformIndex(*shader, "diffuse_texture");
        uiShaderLocations.model = ShaderSystem::getUniformIndex(*shader, "model");
    }

    if (material->generation == INVALID_ID_U32) {
        material->generation = 0;
    } else {
        material->generation++;
    }

    material->id = assets.getAssetIndex(newConfig.name);
    Logger::logDebug("Successfully created new material: " + newConfig.name);

    return *material;
}

void MaterialSystem::releaseMaterial(const String &name) {
    if (name == DEFAULT_MATERIAL_NAME) {
        Logger::logWarn("Cannot release the default material!");
        return;
    }

    Material* material = nullptr;
    if (instance->assets.releaseAsset(name, material)) {
        instance->destroyMaterial(*material);
    }
}

bool MaterialSystem::applyGlobal(const unsigned int shaderId, const unsigned long frameNumber, Mat4 *projection, Mat4 *view, Vector4f *ambientColor, Vector3f *viewPosition, unsigned int renderMode) {
    Shader* shader = ShaderSystem::getShader(shaderId);
    if (!shader) return false;
    if (shader->getFrameNumber() == frameNumber) return true;

    if (shaderId == instance->materialShaderId) {
        if (!ShaderSystem::setUniform(instance->materialLocations.projection, projection)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!ShaderSystem::setUniform(instance->materialLocations.view, view)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!ShaderSystem::setUniform(instance->materialLocations.ambientColor, ambientColor)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!ShaderSystem::setUniform(instance->materialLocations.viewPosition, &viewPosition)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!ShaderSystem::setUniform(instance->materialLocations.renderMode, &renderMode)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
    } else if (shaderId == instance->uiShaderId) {
        if (!ShaderSystem::setUniform(instance->uiShaderLocations.projection, projection)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!ShaderSystem::setUniform(instance->uiShaderLocations.view, view)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
    } else {
        Logger::logError("Invalid shader id: " + toString(shaderId));
        return false;
    }

    if (!ShaderSystem::applyGlobal()) {
        Logger::logWarn("Failed to apply global material.");
        return false;
    }

    shader->setFrameNumber(frameNumber);

    return true;
}

bool MaterialSystem::applyInstance(Material &material, const bool update) {
    if (!ShaderSystem::bindInstance(material.internalId)) {
        Logger::logError("Failed to bind material.");
        return false;
    }

    if (update) {
        if (material.shaderId == instance->materialShaderId) {
            if (!ShaderSystem::setUniform(instance->materialLocations.diffuseColor, &material.diffuseColor)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }
            if (!ShaderSystem::setUniform(instance->materialLocations.diffuseTexture, &material.diffuseMap)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }
            if (!ShaderSystem::setUniform(instance->materialLocations.specularTexture, &material.specularMap)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }
            if (!ShaderSystem::setUniform(instance->materialLocations.normalTexture, &material.normalMap)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }
            if (!ShaderSystem::setUniform(instance->materialLocations.shine, &material.shine)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }

        } else if (material.shaderId == instance->uiShaderId) {
            if (!ShaderSystem::setUniform(instance->uiShaderLocations.diffuseColor, &material.diffuseColor)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }
            if (!ShaderSystem::setUniform(instance->uiShaderLocations.diffuseTexture, &material.diffuseMap)) {
                Logger::logError("Failed to apply instance material.");
                return false;
            }
        } else {
            Logger::logError("Invalid shader id: " + toString(material.shaderId));
            return false;
        }
    }

    if (!ShaderSystem::applyInstance(update)) {
        Logger::logWarn("Failed to apply shader instance!");
        return false;
    }
    return true;
}

bool MaterialSystem::applyLocal(const Material &material, Mat4 *model) {
    if (material.shaderId == instance->materialShaderId) {
        return ShaderSystem::setUniform(instance->materialLocations.model, model);
    }
    if (material.shaderId == instance->uiShaderId) {
        return ShaderSystem::setUniform(instance->uiShaderLocations.model, model);
    }

    Logger::logError("Invalid shader id: " + toString(material.shaderId));
    return false;
}
