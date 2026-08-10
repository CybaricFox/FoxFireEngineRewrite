//
// Created by cmorg on 8/1/2026.
//

#include "FoxFire_MaterialSystem.h"

bool FoxFire_MaterialSystem::initialize(const MaterialSystemConfig materialSystemConfig, ITextureSystem *system, RendererBackend *backend, ResourceSystem *resources, ShaderSystem* shaderSystem) {
    if (!IMaterialSystem::initialize(materialSystemConfig, system, backend, resources, shaderSystem)) return false;

    assets.initialize(config.maxMaterialCount);

    if (!createDefaultMaterial()) {
        Logger::logFatal("Failed to create default material!");
        return false;
    }

    return true;
}

void FoxFire_MaterialSystem::shutdown() {
    for (Material& material : assets.getData().getData()) {
        if (material.id != INVALID_ID_U32) {
            destroyMaterial(material);
        }
    }

    destroyMaterial(defaultMaterial);
}

Material & FoxFire_MaterialSystem::acquireMaterial(const String &name) {
    Resource materialResource{};
    if (!resourceRef->load(name, RESOURCE_TYPE_MATERIAL, materialResource)) {
        Logger::logError("Failed to load material resource!");
        return defaultMaterial;
    }

    Material* material = nullptr;
    if (materialResource.data) {
        material = &acquireMaterial(*static_cast<MaterialResourceData*>(materialResource.data));
    }

    resourceRef->unload(materialResource);

    if (!material) {
        Logger::logError("Failed to unload material resource!");
        return defaultMaterial;
    }

    return *material;
}

Material & FoxFire_MaterialSystem::acquireMaterial(const MaterialResourceData &config) {
    if (config.name == DEFAULT_MATERIAL_NAME) {
        return defaultMaterial;
    }

    if (Material* material = assets.acquireAsset(config.name)) {
        return *material;
    }

    AssetContext context{};
    context.bAutoRelease = config.bAutoRelease;

    Material* material = assets.createAsset(config.name, context);

    if (material == nullptr) return defaultMaterial;

    if (!loadMaterial(config, *material)) {
        Logger::logError("Failed to load material: " + config.name);
        return defaultMaterial;
    }

    Shader* shader = shaderRef->getShader(material->shaderId);
    if (materialShaderId == INVALID_ID_U32 && config.shaderName == DEFAULT_MATERIAL_SHADER_NAME) {
        materialShaderId = shader->getId();
        materialLocations.projection = shaderRef->getUniformIndex(*shader, "projection");
        materialLocations.view = shaderRef->getUniformIndex(*shader, "view");
        materialLocations.diffuseColor = shaderRef->getUniformIndex(*shader, "diffuse_color");
        materialLocations.diffuseTexture = shaderRef->getUniformIndex(*shader, "diffuse_texture");
        materialLocations.model = shaderRef->getUniformIndex(*shader, "model");
    } else if (uiShaderId == INVALID_ID_U32 && config.shaderName == DEFAULT_UI_SHADER_NAME){
        uiShaderId = shader->getId();
        uiShaderLocations.projection = shaderRef->getUniformIndex(*shader, "projection");
        uiShaderLocations.view = shaderRef->getUniformIndex(*shader, "view");
        uiShaderLocations.diffuseColor = shaderRef->getUniformIndex(*shader, "diffuse_color");
        uiShaderLocations.diffuseTexture = shaderRef->getUniformIndex(*shader, "diffuse_texture");
        uiShaderLocations.model = shaderRef->getUniformIndex(*shader, "model");
    }

    if (material->generation == INVALID_ID_U32) {
        material->generation = 0;
    } else {
        material->generation++;
    }

    material->id = context.index;
    Logger::logDebug("Successfully created new material: " + config.name);

    return *material;
}

void FoxFire_MaterialSystem::releaseMaterial(const String &name) {
    if (name == DEFAULT_MATERIAL_NAME) {
        Logger::logWarn("Cannot release the default material!");
        return;
    }

    Material* material = nullptr;
    if (assets.releaseAsset(name, material)) destroyMaterial(*material);
}

bool FoxFire_MaterialSystem::createDefaultMaterial() {
    defaultMaterial = Material{};
    defaultMaterial.name = DEFAULT_MATERIAL_NAME;
    defaultMaterial.diffuseColor = oneVector4f();
    defaultMaterial.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
    defaultMaterial.diffuseMap.texture = &textureSystemRef->getDefaultTexture();

    Shader* shader = shaderRef->getShader(DEFAULT_MATERIAL_SHADER_NAME);
    if (!backendRef->acquireInstanceResources(*shader, defaultMaterial.internalId, textureSystemRef->getDefaultTexture())) {
        Logger::logFatal("Failed to acquire resources for the default material.");
        return false;
    }

    return true;
}

bool FoxFire_MaterialSystem::loadMaterial(const MaterialResourceData &config, Material &material) const {
    material = Material{};
    material.name = config.name;
    material.shaderId = shaderRef->getId(config.shaderName);
    material.diffuseColor = config.diffuseColor;
    if (!config.mapName.empty()) {
        material.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
        material.diffuseMap.texture = &textureSystemRef->acquireTexture(true, config.mapName);
        if (material.diffuseMap.texture == nullptr) {
            Logger::logWarn("Unable to load texture: " + config.name + " for material: " + material.name);
            material.diffuseMap.texture = &textureSystemRef->getDefaultTexture();
        }
    } else {
        material.diffuseMap.use = TEXTURE_USE_UNKNOWN;
        material.diffuseMap.texture = nullptr;
    }

    Shader* shader = shaderRef->getShader(config.shaderName);
    if (!shader) {
        Logger::logError(material.name + " was unable to load its shader: " + config.shaderName);
        return false;
    }

    if (!backendRef->acquireInstanceResources(*shader, material.internalId, textureSystemRef->getDefaultTexture())) {
        Logger::logError("Failed to acquire resources for material " + material.name + "!");
        return false;
    }

    return true;
}

void FoxFire_MaterialSystem::destroyMaterial(Material &material) const {
    Logger::logDebug("Destroying material: " + material.name);

    if (material.diffuseMap.texture != nullptr) {
        textureSystemRef->releaseTexture(material.diffuseMap.texture->name);
    }

    if (material.shaderId != INVALID_ID_U32 && material.internalId != INVALID_ID_U32) {
        backendRef->releaseInstanceResources(*shaderRef->getShader(material.shaderId), material.internalId);
    }

    material = Material{};
}

FoxFire_MaterialSystem::FoxFire_MaterialSystem()
    :IMaterialSystem(sizeof(FoxFire_MaterialSystem))
{

}

bool FoxFire_MaterialSystem::applyGlobal(const unsigned int shaderId, Mat4* projection, Mat4* view) const {
    if (shaderId == materialShaderId) {
        if (!shaderRef->setUniform(materialLocations.projection, projection)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!shaderRef->setUniform(materialLocations.view, view)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
    } else if (shaderId == uiShaderId) {
        if (!shaderRef->setUniform(uiShaderLocations.projection, projection)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
        if (!shaderRef->setUniform(uiShaderLocations.view, view)) {
            Logger::logError("Failed to apply global material.");
            return false;
        }
    } else {
        Logger::logError("Invalid shader id: " + std::to_string(shaderId));
        return false;
    }

    shaderRef->applyGlobal();
    return true;
}

bool FoxFire_MaterialSystem::applyInstance(Material &material) const {
    if (!shaderRef->bindInstance(material.internalId)) {
        Logger::logError("Failed to bind material.");
        return false;
    }

    if (material.shaderId == materialShaderId) {
        if (!shaderRef->setUniform(materialLocations.diffuseColor, &material.diffuseColor)) {
            Logger::logError("Failed to apply instance material.");
            return false;
        }
        if (!shaderRef->setUniform(materialLocations.diffuseTexture, material.diffuseMap.texture)) {
            Logger::logError("Failed to apply instance material.");
            return false;
        }
    } else if (material.shaderId == uiShaderId) {
        if (!shaderRef->setUniform(uiShaderLocations.diffuseColor, &material.diffuseColor)) {
            Logger::logError("Failed to apply instance material.");
            return false;
        }
        if (!shaderRef->setUniform(uiShaderLocations.diffuseTexture, material.diffuseMap.texture)) {
            Logger::logError("Failed to apply instance material.");
            return false;
        }
    } else {
        Logger::logError("Invalid shader id: " + std::to_string(material.shaderId));
        return false;
    }

    shaderRef->applyInstance();
    return true;
}

bool FoxFire_MaterialSystem::applyLocal(const Material &material, Mat4* model) const {
    if (material.shaderId == materialShaderId) {
        return shaderRef->setUniform(materialLocations.model, model);
    }
    if (material.shaderId == uiShaderId) {
        return shaderRef->setUniform(uiShaderLocations.model, model);
    }

    Logger::logError("Invalid shader id: " + std::to_string(material.shaderId));
    return false;
}

FoxFire_MaterialSystem::~FoxFire_MaterialSystem() {
    shutdown();
}
