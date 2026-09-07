//
// Created by cmorg on 9/6/2026.
//

#include "TextureUtils.h"

#include "IRendererBackend.h"
#include "src/modules/system/FoxFire_Textures/FoxFire_TextureSystem.h"

ITextureSystem* TextureUtils::textureSystemRef = nullptr;

Texture * TextureUtils::wrapTexture(const String &name, const unsigned int width, const unsigned int height, const unsigned int channelCount, const bool isTransparent, const bool isWritable, const bool registerTexture, void *data) {
    Texture* texture = nullptr;

    if (registerTexture) {
        texture = &textureSystemRef->acquireTexture(false, true, name, TEXTURE_USE_MAP_DIFFUSE);
    } else {
        texture = static_cast<Texture *>(FF_Memory::ff_allocate(sizeof(Texture), TEXTURE));
        texture->id = INVALID_ID_U32;
        Logger::logDebug("Creating unregistered texture: " + name + ". This texture must be manually deallocated.");
    }

    texture->name = name;
    texture->width = width;
    texture->height = height;
    texture->channelCount = channelCount;
    texture->generation = INVALID_ID_U32;
    texture->flags |= isTransparent ? TEXTURE_BIT_TRANSPARENT : 0;
    texture->flags |= isWritable ? TEXTURE_BIT_WRITABLE : 0;
    texture->flags |= TEXTURE_BIT_WRAPPED;
    texture->data = data;

    return texture;
}

bool TextureUtils::resizeTexture(Texture &texture, const unsigned int width, const unsigned int height, const bool regenerateData, IRendererBackend* backendRef) {
    if (!(texture.flags & TEXTURE_BIT_WRITABLE)) {
        Logger::logWarn("Texture resize should only be called for writable textures.");
        return false;
    }

    texture.width = width;
    texture.height = height;

    if (!(texture.flags & TEXTURE_BIT_WRAPPED) && regenerateData) {
        backendRef->resizeTexture(texture, width, height);
        return false;
    }

    texture.generation++;
    return true;
}
