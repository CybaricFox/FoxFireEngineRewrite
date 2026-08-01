//
// Created by cmorg on 8/1/2026.
//

#include "FoxFire_MaterialSystem.h"

bool FoxFire_MaterialSystem::initialize(const unsigned int initialCapacity, ITextureSystem* system, RendererBackend* backend) {
    backendReference = backend;
    textureSystemReference = system;

    assets.initialize(initialCapacity);

    if (!createDefaultMaterial()) {
        Logger::logFatal("Failed to create default material!");
        return false;
    }

    return true;
}

void FoxFire_MaterialSystem::shutdown() {
    for (Material& material : assets.getData()) {
        if (material.generation != INVALID_ID) {
            destroyMaterial(material);
        }
    }

    destroyMaterial(defaultMaterial);
    backendReference = nullptr;
    textureSystemReference = nullptr;
}

Material & FoxFire_MaterialSystem::acquireMaterial(const String &name, const String &subPath) {
    MaterialConfig config{};

    String path{};
    if (subPath.empty()) {
        path = "Assets/" + name + ".FoxMaterial";
    } else {
        path = "Assets/" + subPath + "/" + name + ".FoxMaterial";
    }

    if (!loadMaterialFile(path, config)) {
        Logger::logError("Failed to load material file: " + path);
        return defaultMaterial;
    }

    return acquireMaterial(config);
}

Material & FoxFire_MaterialSystem::acquireMaterial(const MaterialConfig &config) {
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
    defaultMaterial.diffuseMap.texture = &textureSystemReference->getDefaultTexture();

    backendReference->createMaterial(defaultMaterial);

    return true;
}

bool FoxFire_MaterialSystem::loadMaterial(const MaterialConfig &config, Material &material) const {
    material = Material{};
    material.name = config.name;
    material.diffuseColor = config.diffuseColor;
    if (!config.mapName.empty()) {
        material.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
        material.diffuseMap.texture = &textureSystemReference->acquireTexture(true, config.mapName, "");
        if (material.diffuseMap.texture == nullptr) {
            Logger::logWarn("Unable to load texture: " + config.name + " for material: " + material.name);
            material.diffuseMap.texture = &textureSystemReference->getDefaultTexture();
        }
    } else {
        material.diffuseMap.use = TEXTURE_USE_UNKNOWN;
        material.diffuseMap.texture = nullptr;
    }

    if (!backendReference->createMaterial(material)) {
        Logger::logError("Failed to acquire resources for material: " + config.name);
        return false;
    }

    return true;
}

void FoxFire_MaterialSystem::destroyMaterial(Material &material) const {
    Logger::logDebug("Destroying material: " + material.name);

    if (material.diffuseMap.texture != nullptr) {
        textureSystemReference->releaseTexture(material.diffuseMap.texture->name);
    }

    backendReference->destroyMaterial(material);

    material = Material{};
}

bool FoxFire_MaterialSystem::loadMaterialFile(const String &path, MaterialConfig &config) {
    //Open material file
    FileHandler file;
    if (!file.openFile(path, READ, false)) {
        Logger::logError("Failed to open material file: " + path);
        return false;
    }

    String line;
    unsigned int lineNumber = 0;
    unsigned long bytesRead = 0;

    //While there are lines to read
    while (file.readLine(line, 511, bytesRead)) {
        lineNumber++;

        StringUtils::trim(line);

        //Ignore if line is empty
        if (line.empty()) continue;

        //Ignore comments
        if (line[0] == '#') continue;

        //Find the equal sign on the line if it exists
        const unsigned long equalIndex = line.find('=');
        if (equalIndex == String::npos || equalIndex >= line.length()) {
            Logger::logWarn("Potential format issue found in: " + path + " Failed to find '=' on line" + std::to_string(lineNumber));
            continue;
        }

        //Get the name of the variable on the left and right of the =
        String variable = line.substr(0, equalIndex);
        String value = line.substr(equalIndex + 1);
        StringUtils::trim(variable);
        StringUtils::trim(value);

        //Parse the line
        if (variable == "version") Logger::logDebug("Version: " + value);
        else if (variable == "name") {
            Logger::logDebug("Name: " + value);
            config.name = value;
        }
        else if (variable == "diffuse_color") {
            Logger::logDebug("Diffuse Color: " + value);
            if (!stringToVector4f(value, config.diffuseColor)) {
                Logger::logWarn("Error reading diffuse_color in file: " + path);
                config.diffuseColor = oneVector4f();
            }
        }
        else if (variable == "diffuse_map_name") {
            Logger::logDebug("Diffuse Map Name: " + value);
            config.mapName = value;
        }
    }

    file.closeFile();
    return true;
}

FoxFire_MaterialSystem::~FoxFire_MaterialSystem() {
    shutdown();
}
