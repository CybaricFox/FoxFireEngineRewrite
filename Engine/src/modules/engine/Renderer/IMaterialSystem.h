//
// Created by cmorg on 8/1/2026.
//

#pragma once

#include <foxfire_export.h>

#include "ITextureSystem.h"
#include "src/defines.h"
#include "src/modules/engine/Resources/ResourceTypes.h"

struct MaterialConfig {
    String name{};
    bool bAutoRelease = false;
    Vector4f diffuseColor{};
    String mapName{};
};

class FOXFIRE_API IMaterialSystem {
public:
    virtual ~IMaterialSystem() = default;

    virtual bool initialize(unsigned int initialCapacity, ITextureSystem *system, RendererBackend *backend) = 0;

    virtual Material& acquireMaterial(const String &name, const String &subPath) = 0;
    virtual Material& acquireMaterial(const MaterialConfig &config) = 0;
};
