//
// Created by cmorg on 9/12/2026.
//

#pragma once
#include "src/modules/engine/Renderer/MaterialSystem.h"
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
    Shader* shader = nullptr;
    float nearClip = 0;
    float farClip = 0;
    Mat4 projectionMatrix{};
    Mat4 viewMatrix{};
    unsigned short diffuseMapLocation = INVALID_ID_U16;
    unsigned short diffuseColorLocation = INVALID_ID_U16;
    unsigned short modelLocation = INVALID_ID_U16;

public:
    bool initialize(unsigned long newSize) override;
    void shutdown() override;

    void resize(unsigned int newWidth, unsigned int newHeight) override;
    bool buildPacket(void *data, RenderViewPacket &outPacket, LinearAllocator& allocator) override;
    bool render(RenderViewPacket &outPacket, unsigned long frameNumber, unsigned long renderTargetIndex) override;
    void destroyPacket(RenderViewPacket &packet) override;

    bool regenerateAttachmentTarget(unsigned passIndex, RenderTargetAttachment &attachment) override {return true;}
};