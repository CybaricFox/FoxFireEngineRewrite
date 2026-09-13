//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "src/modules/engine/Renderer/IMaterialSystem.h"
#include "src/modules/engine/Renderer/IRendererBackend.h"
#include "src/modules/engine/Renderer/IRenderView.h"
#include "src/modules/engine/Renderer/ShaderSystem.h"

/**
 *  @file UIRenderView.h
 *  @layer System
 *  @module FoxFire_Renders
 *  @author CybaricFox
 *  @brief
 *  @version 1.0
 *  @date 9/12/2026
 *
 *  @copyright (c) 2026
 */

class UIRenderView final : public IRenderView{
private:
    unsigned int shaderId = INVALID_ID_U32;
    float nearClip = 0;
    float farClip = 0;
    Mat4 projectionMatrix{};
    Mat4 viewMatrix{};

public:
    bool initialize(ShaderSystem *shaderSystem, unsigned long newSize) override;
    void shutdown() override;

    void resize(unsigned int newWidth, unsigned int newHeight) override;
    bool buildPacket(void *data, RenderViewPacket &outPacket) override;
    bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex, IRendererBackend *backendRef, IMaterialSystem *materialSystemRef) override;
};