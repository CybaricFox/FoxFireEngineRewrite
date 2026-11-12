//
// Created by cmorg on 9/6/2026.
//

#include "TextureUtils.h"

#include "IRendererBackend.h"

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
