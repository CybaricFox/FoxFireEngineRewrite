//
// Created by cmorg on 9/6/2026.
//

#pragma once
#include "ITextureSystem.h"
#include "src/modules/engine/Resources/EngineResourceTypes.h"
#include "src/modules/engine/Renderer/IRendererBackend.h"

/**
 *  @file TextureUtils.h
 *  @layer Engine
 *  @module Renderer
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/6/2026
 *
 *  @copyright (c) 2026
 */

class TextureUtils {
private:
    static ITextureSystem* textureSystemRef;

public:
    static Texture* wrapTexture(const String &name, unsigned int width, unsigned int height, unsigned int channelCount, bool isTransparent, bool isWritable, bool registerTexture, void* data);
    static bool resizeTexture(Texture &texture, unsigned int width, unsigned int height, bool regenerateData, IRendererBackend* backendRef);

    static void setTextureSystemRef(ITextureSystem* system) {textureSystemRef = system;}
};