//
// Created by cmorg on 7/31/2026.
//

#pragma once

#include <foxfire_export.h>
#include "RendererBackend.h"

#define DEFAULT_TEXTURE_NAME "default"

class FOXFIRE_API ITextureSystem {
public:
    virtual ~ITextureSystem() = default;

    virtual bool initialize(unsigned int initialCapacity, RendererBackend *backend) = 0;

    virtual Texture& getDefaultTexture() = 0;

    virtual Texture &acquireTexture(bool autoRelease, const String &fileName, const String &subPath) = 0;
    virtual void releaseTexture(String name) = 0;
};