//
// Created by cmorg on 7/31/2026.
//

#pragma once

#include <foxfire_export.h>
#include "RendererBackend.h"
#include "src/modules/engine/Resources/ResourceSystem.h"

#define DEFAULT_TEXTURE_NAME "default"

class FOXFIRE_API ITextureSystem {
protected:
    RendererBackend* backendRef = nullptr;
    ResourceSystem* resourceRef = nullptr;

public:
    virtual ~ITextureSystem();

    virtual bool initialize(unsigned int initialCapacity, RendererBackend *backend, ResourceSystem* resources);

    virtual Texture& getDefaultTexture() = 0;

    virtual Texture &acquireTexture(bool autoRelease, const String &fileName) = 0;
    virtual void releaseTexture(String name) = 0;
};