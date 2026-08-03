//
// Created by cmorg on 8/1/2026.
//

#pragma once

#include <foxfire_export.h>

#include "ITextureSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"

#define DEFAULT_MATERIAL_NAME "default"

class FOXFIRE_API IMaterialSystem {
protected:
    RendererBackend* backendRef = nullptr;
    ResourceSystem* resourceRef = nullptr;
    ITextureSystem* textureSystemRef = nullptr;
public:
    virtual ~IMaterialSystem();

    virtual bool initialize(unsigned int initialCapacity, ITextureSystem *system, RendererBackend *backend, ResourceSystem* resources);

    virtual Material& getDefaultMaterial() = 0;

    virtual Material& acquireMaterial(const String &name) = 0;
    virtual Material& acquireMaterial(const MaterialResourceData &config) = 0;
    virtual void releaseMaterial(const String &name) = 0;
};
