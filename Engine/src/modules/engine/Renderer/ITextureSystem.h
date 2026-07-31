//
// Created by cmorg on 7/31/2026.
//

#pragma once

#include <foxfire_export.h>
#include "RendererBackend.h"

class FOXFIRE_API ITextureSystem {
public:
    virtual ~ITextureSystem() = default;

    virtual bool initialize(unsigned int initialCapacity, RendererBackend* backend) = 0;

    virtual Texture& getDefaultTexture() = 0;

    virtual Texture &acquireTexture(const String &fileName, bool autoRelease) = 0;
    virtual Texture &acquireTexture(const String &fileName, const String &subPath, bool autoRelease) = 0;
    virtual void releaseTexture(const String& name) = 0;
};