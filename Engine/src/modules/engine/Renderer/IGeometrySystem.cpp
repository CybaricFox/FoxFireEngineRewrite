//
// Created by cmorg on 8/2/2026.
//

#include "IGeometrySystem.h"

IGeometrySystem::~IGeometrySystem() {
    backendRef = nullptr;
    materialSystemRef = nullptr;
    resourceSystemRef = nullptr;
}

bool IGeometrySystem::initialize(unsigned int initialCapacity, RendererBackend *backend, IMaterialSystem *materialSystem, ResourceSystem *resources) {
    backendRef = backend;
    materialSystemRef = materialSystem;
    resourceSystemRef = resources;

    return true;
}