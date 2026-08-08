//
// Created by cmorg on 8/1/2026.
//

#include "IMaterialSystem.h"

IMaterialSystem::~IMaterialSystem() {
    textureSystemRef = nullptr;
    backendRef = nullptr;
    resourceRef = nullptr;
    shaderRef = nullptr;
}

bool IMaterialSystem::initialize(const MaterialSystemConfig materialSystemConfig, ITextureSystem *system, RendererBackend *backend, ResourceSystem *resources, ShaderSystem* shaderSystem) {
    if (materialSystemConfig.maxMaterialCount == 0) {
        Logger::logFatal("Material system max material count must be greater than 0!");
        return false;
    }

    config = materialSystemConfig;

    resourceRef = resources;
    backendRef = backend;
    textureSystemRef = system;
    shaderRef = shaderSystem;

    return true;
}
