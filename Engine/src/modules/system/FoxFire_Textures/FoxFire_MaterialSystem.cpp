//
// Created by cmorg on 8/1/2026.
//

#include "FoxFire_MaterialSystem.h"

bool FoxFire_MaterialSystem::initialize(const unsigned int initialCapacity, ITextureSystem *system, RendererBackend *backend, ResourceSystem *resources) {
    IMaterialSystem::initialize(initialCapacity, system, backend, resources);

    assets.initialize(initialCapacity);

    if (!createDefaultMaterial()) {
        Logger::logFatal("Failed to create default material!");
        return false;
    }

    return true;
}

void FoxFire_MaterialSystem::shutdown() {
    for (Material& material : assets.getData().getData()) {
        if (material.generation != INVALID_ID) {
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

    backendRef->createMaterial(defaultMaterial);

    return true;
}

bool FoxFire_MaterialSystem::loadMaterial(const MaterialResourceData &config, Material &material) const {
    material = Material{};
    material.name = config.name;
    material.materialType = config.materialType;
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

    if (!backendRef->createMaterial(material)) {
        Logger::logError("Failed to acquire resources for material: " + config.name);
        return false;
    }

    return true;
}

void FoxFire_MaterialSystem::destroyMaterial(Material &material) const {
    Logger::logDebug("Destroying material: " + material.name);

    if (material.diffuseMap.texture != nullptr) {
        textureSystemRef->releaseTexture(material.diffuseMap.texture->name);
    }

    backendRef->destroyMaterial(material);

    material = Material{};
}

FoxFire_MaterialSystem::~FoxFire_MaterialSystem() {
    shutdown();
}
