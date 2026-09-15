//
// Created by cmorg on 9/15/2026.
//

#pragma once
#include "src/modules/engine/Renderer/IRenderView.h"

/**
 *  @file SkyboxRenderView.h
 *  @layer System
 *  @module FoxFire_Renders
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/15/2026
 *
 *  @copyright (c) 2026
 */

class SkyboxRenderView final : public IRenderView{
private:
    unsigned int shaderId = INVALID_ID_U32;
    float fov = 0;
    float nearClip = 0;
    float farClip = 0;
    Mat4 projectionMatrix{};
    unsigned int worldCamera = 0;
    unsigned short projectionLocation = 0;
    unsigned short viewLocation = 0;
    unsigned short cubeMapLocation = 0;

public:
    bool initialize(ShaderSystem *shaderRef, unsigned long newSize) override;
    void shutdown() override;

    void setCamera(const unsigned int camera) {worldCamera = camera;}

    void resize(unsigned newWidth, unsigned newHeight) override;
    bool buildPacket(void *data, RenderViewPacket &outPacket) override;
    bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend *backendRef, IMaterialSystem *materialSystemRef) override;
};