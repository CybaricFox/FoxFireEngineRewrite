//
// Created by cmorg on 7/31/2026.
//

#include "ITextureSystem.h"

ITextureSystem::~ITextureSystem() {
    backendRef = nullptr;
    resourceRef = nullptr;
}

bool ITextureSystem::initialize(unsigned int initialCapacity, IRendererBackend *backend, ResourceSystem *resources) {
    backendRef = backend;
    resourceRef = resources;

    return true;
}
