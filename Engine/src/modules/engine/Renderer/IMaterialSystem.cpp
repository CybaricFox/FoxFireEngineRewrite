//
// Created by cmorg on 8/1/2026.
//

#include "IMaterialSystem.h"

IMaterialSystem::~IMaterialSystem() {
    textureSystemRef = nullptr;
    backendRef = nullptr;
    resourceRef = nullptr;
}

bool IMaterialSystem::initialize(unsigned int initialCapacity, ITextureSystem *system, RendererBackend *backend, ResourceSystem *resources) {
    resourceRef = resources;
    backendRef = backend;
    textureSystemRef = system;

    return true;
}
